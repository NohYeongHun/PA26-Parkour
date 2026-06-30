#include "EnginePch.h"
#include "BlackBoard.h"

CBlackBoard::CBlackBoard()
{
}

HRESULT CBlackBoard::Add_Data(const _string& strDataTag, DATA_TYPE eType, void* pValue)
{
	if(!Find_Data(strDataTag))
	{
		if(eType >= ENUM_CLASS(DATA_TYPE::DATA_END))
			return E_FAIL;
		pair<DATA_TYPE, void*> pair = {eType, pValue};
		m_Datas.emplace(make_pair(strDataTag, pair));

		return S_OK;
	}
	else
	{
		// 이미 존재하는 데이터 키
		
		return E_FAIL;
	}
}

//void CBlackBoard::Add_Data(StringID uKey, ACCESSOR tAcc)
//{
//	m_Datas.emplace(uKey, tAcc);
//}
//template<class Owner, class T>
//CBlackBoard::ACCESSOR CBlackBoard::BindField(const Owner* pOwner, T Owner::* value)
//{
//	ASSERT_CRASH(!pOwner);
//
//	ACCESSOR tAcc;
//	tAcc.eType = DeduceType<T>();
//	tAcc.getter = [pOwner, value](_variant& out)->_bool{
//			out = _variant{pOwner.get()->*value};
//			return true;
//		};
//	tAcc.setter = [pOwner, value](const _variant& in)->_bool{
//		if(!TypeMatches(DeduceType<T>(), in))
//			return false;
//		pOwner->*value = std::get<T>(in);
//		return true;
//		};
//	return tAcc;
//}
//_bool CBlackBoard::Get_Data(StringID uDataKey, _variant& out)
//{
//	auto* pAccess = Find(uDataKey);
//	//pAccess가 nullptr이거나 getter가 없을 때
//	if(!pAccess || !pAccess->getter)
//		return false;
//	return pAccess->getter(out);
//}
//_bool CBlackBoard::Set_Data(StringID uDataKey, const _variant& in)
//{
//	auto* pAccess = Find(uDataKey);
//	if(!pAccess || !pAccess->setter) 
//		return false;
//	if(!TypeMatches(pAccess->eType, in)) 
//		return false;
//	if(!pAccess->setter(in)) 
//		return false;
//
//	return true;
//}

void* CBlackBoard::Get_Data(const _string& strDataTag)
{
	if(Find_Data(strDataTag))
		return m_Datas[strDataTag].second;
	else
		return nullptr;
}

HRESULT CBlackBoard::Add_Condition(const _string& stFuncTag, function<_bool()> Condition)
{
	if(Find_Condition(stFuncTag))
		return E_FAIL;

	m_Conditions.emplace(make_pair(stFuncTag, Condition)); 
	return S_OK;
}

_bool CBlackBoard::Get_Condition(const _string& strFuncTag)
{
	if(!Find_Condition(strFuncTag))
		CRASH(m_Conditions.find(strFuncTag))

	return m_Conditions[strFuncTag]();
}

_bool CBlackBoard::Necessary_Key_Check(vector<_string>& RequireKey)
{
	auto iter = RequireKey.begin();
	while(iter != RequireKey.end())
	{
		if(Find_Data(*iter))
		{
			iter = RequireKey.erase(iter);
		}
		else
			iter++;
	}

	iter = RequireKey.begin();
	while(iter != RequireKey.end())
	{
		if(Find_Condition(*iter))
		{
			iter = RequireKey.erase(iter);
		}
		else
			iter++;
	}
	return RequireKey.empty();
}

#ifdef _DEBUG
void CBlackBoard::Bind_Data_to_GUI()
{
	ImGui::Begin("BlackBoard");
	for(auto& Pair : m_Datas)
	{
		const char* strKey = Pair.first.c_str();
		DATA_TYPE eType = Pair.second.first;
		void* pValue = Pair.second.second;
		switch(eType)
		{
		case INT:
			ImGui::InputInt(strKey, static_cast<_int*>(pValue));
			break;
		case FLOAT:
			ImGui::InputFloat(strKey, static_cast<_float*>(pValue));
			break;
		case MASK:
		{
			ImGui::InputScalar(strKey, ImGuiDataType_U32,static_cast<_uint*>(pValue));
			ImGui::Text("%d", (1 << *static_cast<_uint*>(pValue)));
			break;
		}
		case BOOL:
			ImGui::Checkbox(strKey, static_cast<_bool*>(pValue));
			break;
		case VECTOR3:
			ImGui::InputFloat3(strKey, static_cast<_float*>(pValue));
			break;
		case VECTOR4:
			ImGui::InputFloat4(strKey, static_cast<_float*>(pValue));
			break;
		default:
			break;
		}
		/*_variant Var;
		Pair.second.getter(Var);
		switch(Pair.second.eType)
		{
		case INT:
			ImGui::Text("%d", Var);
			break;
		case FLOAT:
			ImGui::Text("%d", Var);
			break;
		case MASK:
		{
			ImGui::Text("%d", (1 << Var));
			break;
		}
		case BOOL:
			ImGui::Text("%b", Var);
			break;
		case VECTOR3:
			ImGui::Text("%.f %.f %.f", Var);
			break;
		case VECTOR4:
			ImGui::InputFloat4(strKey, static_cast<_float*>(pValue));
			break;*/
	}
	ImGui::End();
}

void CBlackBoard::Unbind_Data(const _string& strDataTag)
{
	if(m_Datas.find(strDataTag) == m_Datas.end())
		return;

	m_Datas.erase(strDataTag);
}

void CBlackBoard::Clear_Data()
{
	m_Datas.clear();
}
#endif // _DEBUG

//_bool CBlackBoard::TypeMatches(DATA_TYPE eType, const _variant& Var)
//{
//	switch(eType)
//	{
//	case Engine::CBlackBoard::INT:
//		return std::holds_alternative<_int>(Var);
//	case Engine::CBlackBoard::FLOAT:
//		return std::holds_alternative<_float>(Var);
//	case Engine::CBlackBoard::MASK:
//		return std::holds_alternative<_uint>(Var);
//	case Engine::CBlackBoard::BOOL:
//		return std::holds_alternative<_bool>(Var);
//	case Engine::CBlackBoard::VECTOR3:
//		return std::holds_alternative<_float3>(Var);
//	case Engine::CBlackBoard::VECTOR4:
//		return std::holds_alternative<_float4>(Var);	
//	}
//	return false;
//}

_bool CBlackBoard::Find_Data(const _string& strDataTag)
{
	return m_Datas.find(strDataTag) != m_Datas.end();
}

_bool CBlackBoard::Find_Condition(const _string& strDataTag)
{
	return  m_Conditions.find(strDataTag) != m_Conditions.end();
}

//CBlackBoard::ACCESSOR* CBlackBoard::Find(StringID uKey)
//{
//	auto iter = m_Datas.find(uKey);
//	return (iter == m_Datas.end()) ? nullptr : &iter->second;
//}
//
//const CBlackBoard::ACCESSOR* CBlackBoard::Find(StringID uKey) const
//{
//	auto iter = m_Datas.find(uKey);
//	return (iter == m_Datas.end()) ? nullptr : &iter->second;
//}

CBlackBoard* CBlackBoard::Create()
{
	CBlackBoard* pInstance = new CBlackBoard();
	return pInstance;
}

void CBlackBoard::Free()
{
	__super::Free();
#ifdef _DEBUG
	Clear_Data();
#endif // _DEBUG
}
