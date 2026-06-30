#include"EditorPch.h"
#include "Edit_FireFly.h"
#include"Model_Instance_FireFly.h"
#include"Mesh_Instance_FireFly.h"
#include"Event_Level.h"
#include"Map_Interface.h"

CEdit_FireFly::CEdit_FireFly(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CGameObject(pDevice,pContext)
{
}

CEdit_FireFly::CEdit_FireFly(const CEdit_FireFly& Prototype)
	:CGameObject(Prototype)
{
}

HRESULT CEdit_FireFly::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CEdit_FireFly::Initialize_Clone(void* pArg)
{
	if (FAILED(__super::Initialize_Clone(pArg)))
		return E_FAIL;

	if (FAILED(Ready_Component(pArg)))
		return E_FAIL;
	FLY event(this);
	m_pGameInstance->Publish(ENUM_CLASS(LEVEL::STATIC), TEXT("FLY"), event);
	m_pMapInterface = CMap_Interface::Create(m_pDevice, m_pContext);
    return S_OK;
}

void CEdit_FireFly::Priority_Update(_float fTimeDelta)
{
}

void CEdit_FireFly::Update(_float fTimeDelta)
{
	if (!m_isActivate)
		return;
	//남은거-
	
	//셰이더 만들기
	//렌더 함수 만들기?
	//확인하기
	//에디터에서 매니저 클래스 만들어서 위치 찍기.
	m_fTotalTime += fTimeDelta;// *5.f;

	if (m_fTotalTime >= 3600.f)
		m_fTotalTime -= 3600.f;

}

void CEdit_FireFly::Late_Update(_float fTimeDelta)
{
	if (!m_isActivate)
		return;

	m_pGameInstance->Add_Render_Object(RENDERGROUP::NONBLEND, this);
}

void CEdit_FireFly::Render()
{
	if (!m_isActivate)
		return;

	Bind_Resources();

	m_pShaderCom->Bind_Texture("g_DiffuseTexture", nullptr);
	m_pShaderCom->Bind_Texture("g_NormalTexture", nullptr);
	m_pShaderCom->Bind_Texture("g_MaskTexture", nullptr);
	_bool HasNormal = { true };
	_bool HasMask = { true };
	{
		if (FAILED(m_pModelCom->Bind_Materials(m_pShaderCom, "g_MaskTexture", 0, TEXTURETYPE::MASK)))
		{
			m_pShaderCom->Bind_Texture("g_MaskTexture", nullptr);
			HasMask = false;
		}


		if (HasMask)
		{
			m_pModelCom->Bind_Materials(m_pShaderCom, "g_DiffuseTexture", 0, TEXTURETYPE::DIFFUSE);

			if (FAILED(m_pModelCom->Bind_Materials(m_pShaderCom, "g_NormalTexture", 0, TEXTURETYPE::NORMAL)))
				HasNormal = false;
		}
		else
		{
			m_pModelCom->Bind_Materials(m_pShaderCom, "g_DiffuseTexture", 0, TEXTURETYPE::DIFFUSE, 0);

			if (FAILED(m_pModelCom->Bind_Materials(m_pShaderCom, "g_NormalTexture", 0, TEXTURETYPE::NORMAL, 0)))
				HasNormal = false;
		}

	}
	m_pShaderCom->Bind_Value("g_HasNormal", &HasNormal, sizeof(_bool));
	m_pShaderCom->Bind_Value("g_HasMask", &HasMask, sizeof(_bool));
	m_pShaderCom->Bind_Value("g_fRaidan", &m_fTotalTime, sizeof(_float));

	m_pShaderCom->Begin(m_iShaderPassIndex);

	m_pModelCom->Render(0);

}

void CEdit_FireFly::Render_Shadow()
{
}

void CEdit_FireFly::Set_ImGuiOption()
{
	ImGui::Begin("FireFly Pos");
	m_pMapInterface->Set_Transform(m_pTransformCom);
#ifdef _DEBUG
	if (ImGui::Button("Change Pos") || m_pGameInstance->Get_DIKeyState(DIK_RETURN) == KEYSTATE::PRESS)
		m_pModelCom->Change_Pos(m_pTransformCom->Get_State(STATE::POSITION));
#endif
	ImGui::End();
}

HRESULT CEdit_FireFly::Ready_Component(void* pArg)
{
	MAP_LOAD* pDesc = static_cast<MAP_LOAD*>(pArg);
	m_Desc = *pDesc;

	CMesh_Instance_FireFly::MESH_INST_DESC Desc{};
	m_iSaveIndex = pDesc->iSaveIndex;

	//있던 거 로드할 때 안 터지게 처리할것. 저장하는 Instance 숫자 돌려놓는 거랑 깊은복사. 풀 색 셰이더에서 곱할 수도 있게.ㅇㅇ
	m_iNumInstance = pDesc->iNumInstance;
	if (pDesc->IsLoaded)
		Desc.iNumInstance = 1;
	else
		Desc.iNumInstance = m_iNumInstance;
	Desc.pTransformMatrix = pDesc->InstanceWorldMatrix;
	m_pInstanceMatrix = new _float4x4[pDesc->iNumInstance];
	memcpy(m_pInstanceMatrix, pDesc->InstanceWorldMatrix, sizeof(_float4x4) * pDesc->iNumInstance);
	m_iShaderPassIndex = pDesc->iShaderPassIndex;
	Desc.vPerMoveCos = pDesc->vPerCos;
	Desc.vPerMoveSin = pDesc->vPerSin;
	Desc.vPerMoveSin2 = pDesc->vPerSin2;
	Desc.vRange = pDesc->vRange;
	_wstring ProtoName = TEXT("Prototype_Component_Model_Instance_");


	{
		ProtoName += StringToWString(pDesc->ModelName);
		strcpy_s(m_ModelName, pDesc->ModelName);

		_uint V = m_ModelName[strlen(m_ModelName) - 1] - '0' + 1;
		m_pModelComArray.resize(V);
		if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::MAP), StringToWString(pDesc->ModelName),
			TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom), &Desc)))
			return E_FAIL;
		//얘는 뒷숫자
		//for (_uint i = 0; i < V; ++i)
		//{
		//	_wstring ModelCom = ProtoName;
		//	ModelCom.pop_back();
		//	ModelCom += to_wstring(i);
		//	_char ModelName[MAX_PATH] = {};
		//	sprintf_s(ModelName, "Com_Model%d", i);

		//	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::MAP), ModelCom,
		//		StringToWString(ModelName), reinterpret_cast<CComponent**>(&m_pModelComArray[i]), &Desc)))
		//		return E_FAIL;
		//}
	}


	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::MAP), TEXT("Prototype_Component_Shader_FireFly"),
		TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom), nullptr)))
		return E_FAIL;
	XMStoreFloat4x4(&pDesc->WorldMatrix, XMMatrixIdentity());
	m_pTransformCom->Set_WorldMatrix(XMLoadFloat4x4(pDesc->InstanceWorldMatrix));
	return S_OK;
}
void CEdit_FireFly::Move(_fvector vPos)
{
	m_pTransformCom->Set_State(STATE::POSITION, vPos);
#ifdef _DEBUG
	m_pModelCom->Change_Pos(vPos);
#endif
}
void CEdit_FireFly::Bind_Resources()
{
	m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::VIEW));
	m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::PROJ));
}

_uint CEdit_FireFly::ShaderPassWindow()
{
    return 0;
}

void CEdit_FireFly::SaveData(ofstream& File)
{
	File.write(reinterpret_cast<const char*>(&m_Desc.iNumInstance), sizeof(_uint));
	File.write(reinterpret_cast<const char*>(&m_Desc.iShaderPassIndex), sizeof(_uint));
	_float4x4 TransformMat;
	XMStoreFloat4x4(&TransformMat, m_pTransformCom->Get_WorldMatrix());
	File.write(reinterpret_cast<const char*>(&TransformMat), sizeof(_float4x4));

	File.write(reinterpret_cast<const char*>(&m_Desc.vRange), sizeof(_float2));
	File.write(reinterpret_cast<const char*>(&m_Desc.vPerSin), sizeof(_float2));
	File.write(reinterpret_cast<const char*>(&m_Desc.vPerCos), sizeof(_float2));
	File.write(reinterpret_cast<const char*>(&m_Desc.vPerSin2), sizeof(_float2));
}

void CEdit_FireFly::Ready_Events()
{
}

CEdit_FireFly* CEdit_FireFly::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CEdit_FireFly* pInstance = new CEdit_FireFly(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype())) {
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CEdit_FireFly::Clone(void* pArg)
{
	CEdit_FireFly* pInstance = new CEdit_FireFly(*this);

	if (FAILED(pInstance->Initialize_Clone(pArg))) {
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CEdit_FireFly::Free()
{
	__super::Free();
	Safe_Release(m_pShaderCom);
	Safe_Release(m_pModelCom);
	Safe_Delete_Array(m_pInstanceMatrix);
	Safe_Release(m_pMapInterface);
}
