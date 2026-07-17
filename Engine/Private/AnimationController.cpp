#include "EnginePch.h"
#include "AnimationController.h"
#include "GameObject.h"
#include "Model.h"
#include "Transform.h"
#include <fstream>

NS_BEGIN(Engine)

CAnimationController::CAnimationController(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CComponent{ pDevice, pContext }
{
}

CAnimationController::CAnimationController(const CAnimationController& Prototype)
	: CComponent(Prototype)
{
}

HRESULT CAnimationController::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CAnimationController::Initialize_Clone(void* pArg)
{
	if (FAILED(__super::Initialize_Clone(pArg)))
		return E_FAIL;

	ASSERT_CRASH(m_pOwner);
	m_pModelCom = dynamic_cast<CModel*>(m_pOwner->Get_Component(TEXT("Com_Model")));
	if (nullptr == m_pModelCom) return E_FAIL;

	m_pTransformCom = dynamic_cast<CTransform*>(m_pOwner->Get_Component(TEXT("Com_Transform")));
	if (nullptr == m_pTransformCom) return E_FAIL;

	return S_OK;
}

void CAnimationController::Bind_Parameter(const _string& strName, const _float* pValue)
{
	m_Params1D[strName] = pValue;
}

void CAnimationController::Bind_Parameter2D(const _string& strName, const _float2* pValue)
{
	m_Params2D[strName] = pValue;
}

HRESULT CAnimationController::Load(const _string& strFilePath, const FnResolveKey& fnResolveKey)
{
	map<StateKey, map<_uint, CState::ANIM_DATA>> NewRegistry;
	_string strError;
	if (FAILED(Parse(strFilePath, fnResolveKey, NewRegistry, strError)))
	{
		MessageBoxA(nullptr, strError.c_str(), "AnimationController Load Failed", MB_OK);
		return E_FAIL;
	}

	m_strFilePath  = strFilePath;
	m_fnResolveKey = fnResolveKey;
	m_Registry     = move(NewRegistry);
	return S_OK;
}

void CAnimationController::Reload()
{
	map<StateKey, map<_uint, CState::ANIM_DATA>> NewRegistry;
	_string strError;
	if (FAILED(Parse(m_strFilePath, m_fnResolveKey, NewRegistry, strError)))
	{
		MessageBoxA(nullptr, strError.c_str(), "AnimationController Reload Failed (기존 유지)", MB_OK);
		return;
	}

	m_Registry = move(NewRegistry);
	++m_iVersion;
}

void CAnimationController::Request(const StateKey& Key, _uint iAnimId)
{
	m_CurrentKey      = Key;
	m_iCurrentAnimId  = iAnimId;
	m_fTrackPosition  = 0.f;
	m_isAnimEnd       = false;
}

void CAnimationController::Tick(_float fTimeDelta)
{
	auto itState = m_Registry.find(m_CurrentKey);
	if (itState == m_Registry.end()) return;
	auto itAnim = itState->second.find(m_iCurrentAnimId);
	if (itAnim == itState->second.end()) return;

	CState::ANIM_DATA& Data = itAnim->second;
	if (Data.eType == CState::EAnimSlotType::BLENDSPACE_1D)
	{
		m_pModelCom->Play_BlendSpace_CPU(Data.BlendSpaceDesc, Data.RootMotionDesc, fTimeDelta);
		m_isAnimEnd = false;
	}
	else if (Data.eType == CState::EAnimSlotType::BLENDSPACE_2D)
	{
		m_pModelCom->Play_BlendSpace2D_CPU(Data.BlendSpace2Desc, Data.RootMotionDesc, fTimeDelta);
		m_isAnimEnd = false;
	}
	else
		m_isAnimEnd = m_pModelCom->Play_Animation_CPU(Data.AnimPlayDesc, Data.RootMotionDesc, fTimeDelta);

	m_pModelCom->Sync_RootNode(m_pTransformCom, fTimeDelta);
}

const CState::ANIM_DATA* CAnimationController::Get_CurrentAnimData() const
{
	auto itState = m_Registry.find(m_CurrentKey);
	if (itState == m_Registry.end()) return nullptr;
	auto itAnim = itState->second.find(m_iCurrentAnimId);
	if (itAnim == itState->second.end()) return nullptr;
	return &itAnim->second;
}

HRESULT CAnimationController::Parse(const _string& strFilePath, const FnResolveKey& fnResolveKey,
	map<StateKey, map<_uint, CState::ANIM_DATA>>& OutRegistry, _string& strOutError)
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

		auto GetF = [](const json& j, const char* k, _float  fDef) { return j.contains(k) ? j[k].get<_float>() : fDef; };
		auto GetB = [](const json& j, const char* k, _bool   bDef) { return j.contains(k) ? j[k].get<_bool>()  : bDef; };

		for (const auto& [strStatePath, Entries] : Root.items())
		{
			StateKey Key{ 0, 0 };
			if (!fnResolveKey(strStatePath, Key))
			{
				strOutError = "알 수 없는 상태 이름: \"" + strStatePath + "\"";
				return E_FAIL;
			}
			if (!Entries.is_array())
			{
				strOutError = "\"" + strStatePath + "\"의 값이 배열이 아님";
				return E_FAIL;
			}

			map<_uint, CState::ANIM_DATA> AnimMap;
			for (const auto& Entry : Entries)
			{
				if (!Entry.contains("id") || !Entry.contains("type"))
				{
					strOutError = "\"" + strStatePath + "\": \"id\" 또는 \"type\" 없음";
					return E_FAIL;
				}
				const _uint   iId     = Entry["id"].get<_uint>();
				const _string strType = Entry["type"].get<_string>();
				const _string strWhere = "\"" + strStatePath + "\" id=" + to_string(iId);

				CState::ANIM_DATA Data{};

				if (Entry.contains("rootMotion"))
				{
					const auto& rm = Entry["rootMotion"];
					Data.RootMotionDesc.fRate       = GetF(rm, "rate",      0.1f);
					Data.RootMotionDesc.isEnable    = GetB(rm, "enable",    false);
					Data.RootMotionDesc.isRotate    = GetB(rm, "rotate",    false);
					Data.RootMotionDesc.isTranslate = GetB(rm, "translate", false);
				}

				if (strType == "CLIP")
				{
					if (!Entry.contains("clip"))
					{
						strOutError = strWhere + ": CLIP에 \"clip\" 없음";
						return E_FAIL;
					}
					Data.eType = CState::EAnimSlotType::CLIP;
					Data.AnimPlayDesc.pTrackPosition        = &m_fTrackPosition;
					Data.AnimPlayDesc.strAnimationName      = Entry["clip"].get<_string>();
					Data.AnimPlayDesc.fSpeed                = GetF(Entry, "speed",   1.f);
					Data.AnimPlayDesc.fBlendIn              = GetF(Entry, "blendIn", 0.2f);
					Data.AnimPlayDesc.fBlendOut             = GetF(Entry, "blendOut",0.2f);
					Data.AnimPlayDesc.fEscapeTrackPosition  = GetF(Entry, "escape",  0.f);
				}
				else if (strType == "BLENDSPACE_1D")
				{
					if (!Entry.contains("param") || !Entry.contains("samples"))
					{
						strOutError = strWhere + ": BLENDSPACE_1D에 \"param\" 또는 \"samples\" 없음";
						return E_FAIL;
					}
					Data.eType = CState::EAnimSlotType::BLENDSPACE_1D;
					const _string strParam = Entry["param"].get<_string>();
					auto itParam = m_Params1D.find(strParam);
					if (itParam == m_Params1D.end())
					{
						strOutError = strWhere + ": 미등록 1D 파라미터 \"" + strParam + "\"";
						return E_FAIL;
					}
					Data.BlendSpaceDesc.pParam   = itParam->second;
					Data.BlendSpaceDesc.fBlendIn  = GetF(Entry, "blendIn",  0.2f);
					Data.BlendSpaceDesc.fBlendOut = GetF(Entry, "blendOut", 0.2f);
					for (const auto& S : Entry["samples"])
					{
						BLENDSPACE_SAMPLE Sample{};
						Sample.strAnimationName = S["clip"].get<_string>();
						Sample.fXParamValue     = S["x"].get<_float>();
						Data.BlendSpaceDesc.Samples.push_back(move(Sample));
					}
				}
				else if (strType == "BLENDSPACE_2D")
				{
					if (!Entry.contains("param") || !Entry.contains("samples"))
					{
						strOutError = strWhere + ": BLENDSPACE_2D에 \"param\" 또는 \"samples\" 없음";
						return E_FAIL;
					}
					Data.eType = CState::EAnimSlotType::BLENDSPACE_2D;
					const _string strParam = Entry["param"].get<_string>();
					auto itParam = m_Params2D.find(strParam);
					if (itParam == m_Params2D.end())
					{
						strOutError = strWhere + ": 미등록 2D 파라미터 \"" + strParam + "\"";
						return E_FAIL;
					}
					Data.BlendSpace2Desc.pParam   = itParam->second;
					Data.BlendSpace2Desc.fBlendIn  = GetF(Entry, "blendIn",  0.2f);
					Data.BlendSpace2Desc.fBlendOut = GetF(Entry, "blendOut", 0.2f);
					for (const auto& S : Entry["samples"])
					{
						BLENDSPACE_SAMPLE Sample{};
						Sample.strAnimationName = S["clip"].get<_string>();
						Sample.fXParamValue     = S["x"].get<_float>();
						Sample.fYParamValue     = S.value("y", 0.f);
						Data.BlendSpace2Desc.Samples.push_back(move(Sample));
					}
				}
				else
				{
					strOutError = strWhere + ": 알 수 없는 type \"" + strType + "\"";
					return E_FAIL;
				}

				AnimMap.emplace(iId, move(Data));
			}

			OutRegistry.emplace(Key, move(AnimMap));
		}
	}
	catch (const json::exception& e)
	{
		strOutError = _string("JSON 처리 실패: ") + e.what();
		return E_FAIL;
	}

	return S_OK;
}

CAnimationController* CAnimationController::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CAnimationController* pInstance = new CAnimationController(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Create : CAnimationController");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CComponent* CAnimationController::Clone(void* pArg)
{
	CAnimationController* pInstance = new CAnimationController(*this);
	if (FAILED(pInstance->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Clone : CAnimationController");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CAnimationController::Free()
{
	__super::Free();
}

NS_END
