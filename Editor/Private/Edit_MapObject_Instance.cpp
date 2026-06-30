#include"Editorpch.h"
#include "Edit_MapObject_Instance.h"
#include"Model_Instance.h"
#include"Mesh_Instance.h"
#include"Event_Level.h"
#include"Map_Interface.h"

CEdit_MapObject_Instance::CEdit_MapObject_Instance(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    :CGameObject(pDevice,pContext)
{
}

CEdit_MapObject_Instance::CEdit_MapObject_Instance(const CEdit_MapObject_Instance& Prototype)
    :CGameObject(Prototype)
{
}


HRESULT CEdit_MapObject_Instance::Initialize_Prototype()
{
    if (FAILED(__super::Initialize_Prototype()))
        return E_FAIL;

    return S_OK;
}

HRESULT CEdit_MapObject_Instance::Initialize_Clone(void* pArg)
{
    if (FAILED(__super::Initialize_Clone(pArg)))
        return E_FAIL;

    if (FAILED(Ready_Component(pArg)))
        return E_FAIL;

    Ready_Events();
	INSTANCE_CREATE event(m_ModelName, m_iSaveIndex, this);
	m_pMapInterface = CMap_Interface::Create(m_pDevice, m_pContext);
    //MAP_CREATE event(m_ModelName, this);
    m_pGameInstance->Publish(ENUM_CLASS(LEVEL::STATIC), TEXT("Instance_Create"), event);

	m_pGameInstance->Subscribe<INSTANCE_SAVE>(ENUM_CLASS(LEVEL::STATIC), TEXT("Save_Instance") + to_wstring(m_iSaveIndex), [this](const INSTANCE_SAVE& event) {

		if (!m_isActivate)
			return;
		/*auto iter = event.ModelName.find(m_ModelName);
		if (iter == event.ModelName.end())
			event.ModelName.insert(m_ModelName);*/

		for (_uint i = 0; i < m_iNumInstance; ++i)
		{
			if (m_pInstanceMatrix[i].m[3][3] != 1.f)
				return;
		}
		*event.iNumTotalInstance += m_iNumInstance;

		for (_uint i = 0; i < m_iNumInstance; ++i)
		{
			if (m_pInstanceMatrix[i].m[3][3] == 1.f)
				event.Totalmatrix.push_back(m_pInstanceMatrix[i]);
			else
				int a = 0;
		}

		event.Objectmatrix.push_back(m_pTransformCom->Get_State(STATE::POSITION));
		});
	
    return S_OK;
}

void CEdit_MapObject_Instance::Priority_Update(_float fTimeDelta)
{
	if (!m_isActivate)
		return;
}

void CEdit_MapObject_Instance::Update(_float fTimeDelta)
{
	if (!m_isActivate)
		return;
	m_fTotalTime += fTimeDelta;
	if (m_fTotalTime >= 3.f)
		m_fTotalTime -= 3.f;
}

void CEdit_MapObject_Instance::Late_Update(_float fTimeDelta)
{
	if (m_isActivate)
    m_pGameInstance->Add_Render_Object(RENDERGROUP::NONLIGHT, this);
}

void CEdit_MapObject_Instance::Render()
{
	if (!m_isActivate)
		return;
    Bind_Resources();

	for (_uint i = 0; i < m_pModelComArray[0]->Get_NumMesh(); ++i)
	{
		m_pShaderCom->Bind_Texture("g_DiffuseTexture", nullptr);
		m_pShaderCom->Bind_Texture("g_NormalTexture", nullptr);
		m_pShaderCom->Bind_Texture("g_MaskTexture", nullptr);
		_bool HasNormal = { true };
		_bool HasMask = { true };
		{
			if (FAILED(m_pModelComArray[0]->Bind_Materials(m_pShaderCom, "g_MaskTexture", i, TEXTURETYPE::MASK)))
			{
				m_pShaderCom->Bind_Texture("g_MaskTexture", nullptr);
				HasMask = false;
			}


			if (HasMask)
			{
				m_pModelComArray[0]->Bind_Materials(m_pShaderCom, "g_DiffuseTexture", i, TEXTURETYPE::DIFFUSE);

				if (FAILED(m_pModelComArray[0]->Bind_Materials(m_pShaderCom, "g_NormalTexture", i, TEXTURETYPE::NORMAL)))
					HasNormal = false;
			}
			else
			{
				m_pModelComArray[0]->Bind_Materials(m_pShaderCom, "g_DiffuseTexture", i, TEXTURETYPE::DIFFUSE, 0);

				if (FAILED(m_pModelComArray[0]->Bind_Materials(m_pShaderCom, "g_NormalTexture", i, TEXTURETYPE::NORMAL, 0)))
					HasNormal = false;
			}

		}
		m_pShaderCom->Bind_Value("g_HasNormal", &HasNormal, sizeof(_bool));
		m_pShaderCom->Bind_Value("g_HasMask", &HasMask, sizeof(_bool));
		m_pShaderCom->Bind_Value("g_fRaidan", &m_fTotalTime, sizeof(_float));
		m_pShaderCom->Bind_Value("g_vDiffuseColor", &m_vDiffuseColor, sizeof(_float4));

		

		m_pShaderCom->Begin(m_iShaderPassIndex);

		m_pModelComArray[0]->Render(i);
	}
}

void CEdit_MapObject_Instance::Render_Shadow()
{
}

void CEdit_MapObject_Instance::Set_ImGuiOption()
{
    ImGuiID PickID = ImGui::GetID("MapPick");
    char Pick_buffer[30];
    sprintf_s(Pick_buffer, "%d", m_iPickedInstance);
    strcat_s(Pick_buffer, " : Picked");
    ImGui::Text(Pick_buffer);

    ImGui::BeginChildFrame(PickID, ImVec2(100, 200));

    for (_uint i = 0; i < m_iNumInstance; ++i)
    {
        char buffer[10];
        sprintf_s(buffer, "%d", i);
        if (ImGui::Button(buffer))
            m_iPickedInstance = i;
    }
    ImGui::EndChildFrame();
	
	_float vScale[3] = {};
	_float vRotation[3] = {};
	_float vTransfrom[3] = {};
	ImGuizmo::DecomposeMatrixToComponents(reinterpret_cast<_float*>(&m_pInstanceMatrix[m_iPickedInstance]), vTransfrom, vRotation, vScale);
	m_pGameInstance->Use_Gizmo_Offset(reinterpret_cast<_float3*>(&vScale), reinterpret_cast<_float3*>(&vRotation), reinterpret_cast<_float3*>(&vTransfrom));
}

HRESULT CEdit_MapObject_Instance::Ready_Component(void* pArg)
{
    MAP_LOAD* pDesc = static_cast<MAP_LOAD*>(pArg);
    CMesh_Instance::MESH_INST_DESC Desc{};
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
	m_vDiffuseColor = pDesc->vDiffuseColor;
	m_eInstanceType = pDesc->eInstanceType;

	_wstring ProtoName = TEXT("Prototype_Component_Model_Instance_");

	if (pDesc->IsLoaded)
	{


		_string NameTemp = pDesc->ModelName;
		_uint NumStartsPos = NameTemp.find_last_not_of("0123456789");
		NumStartsPos == string::npos ? NumStartsPos = 0 : NumStartsPos += 1;

		//이게 뒤에 다 뺀 LOD3까지 있는 이름.
		_string ModelName = NameTemp.substr(0, NumStartsPos+1);

		strcpy_s(m_ModelName, ModelName.c_str());

		m_pRotation = new _float4[m_iNumInstance];

		_uint V = ModelName[ModelName.length() - 1] - '0' + 1;
		m_pModelComArray.resize(V);
	

		ProtoName += StringToWString(ModelName);

		//얘는 뒷숫자 말고 앞 숫자를 바꿔야함.
		for (_uint i = 0; i < V; ++i)
		{
			_wstring ModelCom = ProtoName;
			ModelCom.pop_back();
			ModelCom += to_wstring(i);
			ModelCom += to_wstring(m_iSaveIndex);
			_char ModelName[MAX_PATH] = {};
			sprintf_s(ModelName, "Com_Model%d", i);

			if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::MAP), ModelCom,
				StringToWString(ModelName), reinterpret_cast<CComponent**>(&m_pModelComArray[i]), &Desc)))
				return E_FAIL;
		}
	}
	else
	{
		ProtoName += StringToWString(pDesc->ModelName);
		strcpy_s(m_ModelName, pDesc->ModelName);

		_uint V = m_ModelName[strlen(m_ModelName) - 1] - '0' + 1;
		m_pModelComArray.resize(V);

		//얘는 뒷숫자
		for (_uint i = 0; i < V; ++i)
		{
			_wstring ModelCom = ProtoName;
			ModelCom.pop_back();
			ModelCom += to_wstring(i);
			_char ModelName[MAX_PATH] = {};
			sprintf_s(ModelName, "Com_Model%d", i);

			if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::MAP), ModelCom,
				StringToWString(ModelName), reinterpret_cast<CComponent**>(&m_pModelComArray[i]), &Desc)))
				return E_FAIL;
		}
	}


    /*if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::MAP), ProtoName,
        TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom), &Desc)))
        return E_FAIL;*/

    if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::MAP), TEXT("Prototype_Component_Shader_NonAnimMesh_Instance"),
        TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom), nullptr)))
        return E_FAIL;

	m_pTransformCom->Set_WorldMatrix(XMLoadFloat4x4(&pDesc->WorldMatrix));
    return S_OK;
}

void CEdit_MapObject_Instance::Bind_Resources()
{
    m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::VIEW));
    m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::PROJ));
}

_uint CEdit_MapObject_Instance::ShaderPassWindow()
{
	m_pMapInterface->Set_ShaderPass(m_pShaderCom, &m_iShaderPassIndex);
	return m_iShaderPassIndex;
}

void CEdit_MapObject_Instance::Ready_Events()
{
    //m_pGameInstance->Subscribe()
}

CEdit_MapObject_Instance* CEdit_MapObject_Instance::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CEdit_MapObject_Instance* pInstance = new CEdit_MapObject_Instance(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Create : MapObject_Instance");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CEdit_MapObject_Instance::Clone(void* pArg)
{
    CEdit_MapObject_Instance* pInstance = new CEdit_MapObject_Instance(*this);

    if (FAILED(pInstance->Initialize_Clone(pArg)))
    {
        MSG_BOX("Failed to Create : MapObject_Instance (Clone)");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CEdit_MapObject_Instance::Free()
{
    __super::Free();
	for (auto& pModel : m_pModelComArray)
		Safe_Release(pModel);
    Safe_Release(m_pShaderCom);
    //마지막으로 깐 놈들 지우려면 이터레이터 이용해서 second 지우고 erase. 뒤에서부터 쭉~ 되게. 맵으로 추출할 때는 IsActive활성화 된 놈만.
	Safe_Delete_Array(m_pRotation);
	Safe_Delete_Array(m_pInstanceMatrix);

	Safe_Release(m_pMapInterface);
}