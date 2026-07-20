#include "ClientPch.h"
#include "TransitionTable.h"
#include "TraceurStateNames.h"
#include <fstream>

using json = nlohmann::json;
using Engine::StateKey;

namespace
{
	HRESULT Parse_Rule(const StateKey& SourceKey, _bool isCategoryKey, _uint iForcedAnimGuard,
		const nlohmann::json& Rule, const _string& strWhere,
		TRANSITION_RULE_DATA& OutData, _string& strOutError)
	{
		if (!Rule.contains("goto") || !Rule["goto"].is_string())
		{
			strOutError = strWhere + ": \"goto\"가 없거나 문자열이 아님";
			return E_FAIL;
		}
		const _string strGoto = Rule["goto"].get<_string>();
		if (!CTraceurStateNames::Resolve_StateKey(strGoto, OutData.Next))
		{
			strOutError = strWhere + ": 알 수 없는 goto 상태 \"" + strGoto + "\"";
			return E_FAIL;
		}

		if (Rule.contains("when"))
		{
			if (!Rule["when"].is_array())
			{
				strOutError = strWhere + ": \"when\"이 배열이 아님";
				return E_FAIL;
			}
			for (const auto& Flag : Rule["when"])
			{
				if (!Flag.is_string())
				{
					strOutError = strWhere + ": \"when\" 원소가 문자열이 아님";
					return E_FAIL;
				}
				OutData.WhenFlags.push_back(Flag.get<_string>());
			}
		}

		if (Rule.contains("whenNot"))
		{
			if (!Rule["whenNot"].is_array())
			{
				strOutError = strWhere + ": \"whenNot\"이 배열이 아님";
				return E_FAIL;
			}
			for (const auto& Flag : Rule["whenNot"])
			{
				if (!Flag.is_string())
				{
					strOutError = strWhere + ": \"whenNot\" 원소가 문자열이 아님";
					return E_FAIL;
				}
				OutData.WhenNotFlags.push_back(Flag.get<_string>());
			}
		}

		if (Rule.contains("whenAnim"))
		{
			if (iForcedAnimGuard != UINT_MAX)
			{
				strOutError = strWhere + ": byAnim 그룹 안에서는 \"whenAnim\"을 쓸 수 없음";
				return E_FAIL;
			}
			if (isCategoryKey)
			{
				strOutError = strWhere + ": 카테고리 공통 규칙에서는 \"whenAnim\"을 쓸 수 없음 (상태 규칙 또는 byAnim 사용)";
				return E_FAIL;
			}
			if (!Rule["whenAnim"].is_string())
			{
				strOutError = strWhere + ": \"whenAnim\"이 문자열이 아님";
				return E_FAIL;
			}
			const _string strAnim = Rule["whenAnim"].get<_string>();
			if (!CTraceurStateNames::Resolve_AnimIndex(SourceKey, strAnim, OutData.iAnimGuard))
			{
				strOutError = strWhere + ": 알 수 없는 whenAnim \"" + strAnim + "\"";
				return E_FAIL;
			}
		}

		if (Rule.contains("enterAnim"))
		{
			if (!Rule["enterAnim"].is_string())
			{
				strOutError = strWhere + ": \"enterAnim\"이 문자열이 아님";
				return E_FAIL;
			}
			const _string strAnim = Rule["enterAnim"].get<_string>();
			if (!CTraceurStateNames::Resolve_AnimIndex(OutData.Next, strAnim, OutData.iNextAnim))
			{
				strOutError = strWhere + ": 알 수 없는 enterAnim \"" + strAnim + "\" (대상 " + strGoto + ")";
				return E_FAIL;
			}
		}

		if (Rule.contains("blend"))
		{
			if (!Rule["blend"].is_number() || Rule["blend"].get<_float>() < 0.f)
			{
				strOutError = strWhere + ": \"blend\"는 0 이상의 숫자여야 함";
				return E_FAIL;
			}
			OutData.fBlendOverride = Rule["blend"].get<_float>();
		}

		if (iForcedAnimGuard != UINT_MAX)
			OutData.iAnimGuard = iForcedAnimGuard;

		return S_OK;
	}
}

HRESULT CTransitionTable::Load(const _string& strFilePath)
{
	map<StateKey, vector<TRANSITION_RULE_DATA>> NewTable;
	map<_uint, vector<TRANSITION_RULE_DATA>> NewCategoryTable;
	_string strError;
	if (FAILED(Parse(strFilePath, NewTable, NewCategoryTable, strError)))
	{
		MessageBoxA(nullptr, strError.c_str(), "TransitionTable Load Failed", MB_OK);
		return E_FAIL;
	}

	m_strFilePath = strFilePath;
	m_Table = move(NewTable);
	m_CategoryTable = move(NewCategoryTable);
	return S_OK;
}

void CTransitionTable::Reload()
{
	map<StateKey, vector<TRANSITION_RULE_DATA>> NewTable;
	map<_uint, vector<TRANSITION_RULE_DATA>> NewCategoryTable;
	_string strError;
	if (FAILED(Parse(m_strFilePath, NewTable, NewCategoryTable, strError)))
	{
		MessageBoxA(nullptr, strError.c_str(), "TransitionTable Reload Failed (기존 테이블 유지)", MB_OK);
		return;
	}

	m_Table = move(NewTable);
	m_CategoryTable = move(NewCategoryTable);
	++m_iVersion;
}

const vector<TRANSITION_RULE_DATA>* CTransitionTable::Get_Rules(const StateKey& Key) const
{
	const auto it = m_Table.find(Key);
	return it == m_Table.end() ? nullptr : &it->second;
}

HRESULT CTransitionTable::Parse(const _string& strFilePath,
	map<StateKey, vector<TRANSITION_RULE_DATA>>& OutTable,
	map<_uint, vector<TRANSITION_RULE_DATA>>& OutCategoryTable, _string& strOutError)
{
	std::ifstream File(strFilePath);
	if (!File.is_open())
	{
		strOutError = "파일을 열 수 없음: " + strFilePath;
		return E_FAIL;
	}

	try
	{
		json Root;
		File >> Root;

		if (!Root.is_object())
		{
			strOutError = "최상위가 오브젝트가 아님";
			return E_FAIL;
		}

		for (const auto& [strStatePath, Rules] : Root.items())
		{
			const _bool isCategoryKey = (strStatePath.find('/') == _string::npos);

			StateKey SourceKey{ 0, 0 };
			_uint iCategory = 0;
			if (isCategoryKey)
			{
				if (!CTraceurStateNames::Resolve_Category(strStatePath, iCategory))
				{
					strOutError = "알 수 없는 카테고리 이름: \"" + strStatePath + "\"";
					return E_FAIL;
				}
			}
			else if (!CTraceurStateNames::Resolve_StateKey(strStatePath, SourceKey))
			{
				strOutError = "알 수 없는 상태 이름: \"" + strStatePath + "\"";
				return E_FAIL;
			}
			vector<TRANSITION_RULE_DATA> RuleList;

			if (Rules.is_array())
			{
				// 기존 배열 포맷
				_uint iRuleIndex = 0;
				for (const auto& Rule : Rules)
				{
					const _string strWhere = "\"" + strStatePath + "\" 규칙 #" + to_string(iRuleIndex);
					TRANSITION_RULE_DATA Data{};
					if (FAILED(Parse_Rule(SourceKey, isCategoryKey, UINT_MAX, Rule, strWhere, Data, strOutError)))
						return E_FAIL;
					RuleList.push_back(move(Data));
					++iRuleIndex;
				}
			}
			else if (Rules.is_object() && !isCategoryKey)
			{
				if (Rules.contains("byAnim"))
				{
					if (!Rules["byAnim"].is_object())
					{
						strOutError = "\"" + strStatePath + "\": \"byAnim\"이 오브젝트가 아님";
						return E_FAIL;
					}
					for (const auto& [strAnim, AnimRules] : Rules["byAnim"].items())
					{
						_uint iAnimGuard = UINT_MAX;
						if (!CTraceurStateNames::Resolve_AnimIndex(SourceKey, strAnim, iAnimGuard))
						{
							strOutError = "\"" + strStatePath + "\": 알 수 없는 byAnim 애니 \"" + strAnim + "\"";
							return E_FAIL;
						}
						if (!AnimRules.is_array())
						{
							strOutError = "\"" + strStatePath + "\" byAnim \"" + strAnim + "\"의 값이 배열이 아님";
							return E_FAIL;
						}

						_uint iRuleIndex = 0;
						for (const auto& Rule : AnimRules)
						{
							const _string strWhere = "\"" + strStatePath + "\" byAnim \"" + strAnim + "\" 규칙 #" + to_string(iRuleIndex);
							TRANSITION_RULE_DATA Data{};
							if (FAILED(Parse_Rule(SourceKey, false, iAnimGuard, Rule, strWhere, Data, strOutError)))
								return E_FAIL;
							RuleList.push_back(move(Data));
							++iRuleIndex;
						}
					}
				}
				if (Rules.contains("rules"))
				{
					if (!Rules["rules"].is_array())
					{
						strOutError = "\"" + strStatePath + "\": \"rules\"가 배열이 아님";
						return E_FAIL;
					}
					_uint iRuleIndex = 0;
					for (const auto& Rule : Rules["rules"])
					{
						const _string strWhere = "\"" + strStatePath + "\" rules 규칙 #" + to_string(iRuleIndex);
						TRANSITION_RULE_DATA Data{};
						if (FAILED(Parse_Rule(SourceKey, false, UINT_MAX, Rule, strWhere, Data, strOutError)))
							return E_FAIL;
						RuleList.push_back(move(Data));
						++iRuleIndex;
					}
				}
			}
			else
			{
				strOutError = "\"" + strStatePath + "\"의 값이 배열/오브젝트가 아님 (카테고리 키는 배열만 허용)";
				return E_FAIL;
			}

			if (isCategoryKey)
			{
				const auto Result = OutCategoryTable.emplace(iCategory, move(RuleList));
				if (!Result.second)
				{
					strOutError = "\"" + strStatePath + "\" 카테고리 키가 중복됨";
					return E_FAIL;
				}
			}
			else
			{
				const auto Result = OutTable.emplace(SourceKey, move(RuleList));
				if (!Result.second)
				{
					strOutError = "\"" + strStatePath + "\" 키가 중복됨";
					return E_FAIL;
				}
			}
		}
	}
	catch (const json::exception& e)
	{
		strOutError = _string("JSON 처리 실패: ") + e.what();
		return E_FAIL;
	}

	return S_OK;
}

CTransitionTable* CTransitionTable::Create(const _string& strFilePath)
{
	CTransitionTable* pInstance = new CTransitionTable();
	if (FAILED(pInstance->Load(strFilePath)))
	{
		Safe_Release(pInstance);
		MSG_BOX("Failed to Create : CTransitionTable");
		return nullptr;
	}
	return pInstance;
}

void CTransitionTable::Free()
{
	__super::Free();
}
