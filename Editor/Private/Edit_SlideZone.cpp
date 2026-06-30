#include"EditorPch.h"
#include "Edit_SlideZone.h"
#include"Event_Level.h"
#include"Level_Map.h"
#include"Map_Interface.h"

CEdit_SlideZone::CEdit_SlideZone(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CGameObject(pDevice,pContext)
{
}

CEdit_SlideZone::CEdit_SlideZone(const CEdit_SlideZone& Prototype)
	:CGameObject(Prototype)
{
}

HRESULT CEdit_SlideZone::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CEdit_SlideZone::Initialize_Clone(void* pArg)
{
	if (FAILED(__super::Initialize_Clone(pArg)))
		return E_FAIL;

	Ready_Component(pArg);
	m_pMapInterface = CMap_Interface::Create(m_pDevice, m_pContext);

	_char Tag[MAX_PATH] = "Trigger";
	MAP_CREATE event(Tag, this);

	m_pGameInstance->Publish(ENUM_CLASS(LEVEL::STATIC), TEXT("Create_Object"), event);

	m_pGameInstance->Subscribe<MAP_SAVE>(ENUM_CLASS(LEVEL::STATIC), TEXT("Save_Map_Slide"), [this](const MAP_SAVE& event) {
		if (!m_isActivate)
			return;
		//true면 시작점. false면 도착점.
		event.File.write(reinterpret_cast<const _char*>(&m_IsStart), sizeof(_bool));
		event.File.write(reinterpret_cast<const char*>(&m_vExtends), sizeof(_float3));
		_float4x4 WorldMatrix;
		XMStoreFloat4x4(&WorldMatrix, m_pTransformCom->Get_WorldMatrix());
		event.File.write(reinterpret_cast<const _char*>(&WorldMatrix), sizeof(_float4x4));
		_uint iPathSize = m_vSlidePath.size();
		event.File.write(reinterpret_cast<const _char*>(&iPathSize), sizeof(_uint));
		
		for (auto Path : m_vSlidePath)
			event.File.write(reinterpret_cast<const _char*>(&Path), sizeof(_float4));

		});
    return S_OK;
}

void CEdit_SlideZone::Priority_Update(_float fTimeDelta)
{
}

void CEdit_SlideZone::Update(_float fTimeDelta)
{
	m_pRigidbodyCom->Update_Rigidbody(m_pTransformCom->Get_WorldMatrix(), fTimeDelta);
}

void CEdit_SlideZone::Late_Update(_float fTimeDelta)
{
}

void CEdit_SlideZone::Render()
{
}

void CEdit_SlideZone::Render_Shadow()
{
}

void CEdit_SlideZone::Set_ImGuiOption()
{
	_float3 vScale = _float3(1.f, 1.f, 1.f);
	_float3 vRot = _float3(0.f, 0.f, 0.f);

	m_pMapInterface->Set_Transform(m_pTransformCom);

	ImGui::Checkbox("IsStartBox", &m_IsStart);
	ImGui::SameLine();
	if (!m_IsStart)
		return;
	ImGui::Checkbox("Test Render", &m_IsTestRender);
	
	ImGuiID ShaderId = ImGui::GetID("SlidePath");
	ImGui::BeginChildFrame(ShaderId, ImVec2(100, 200));
	ImGui::Text("Current SlidePath");

	for (_uint i=0; i< m_vSlidePath.size();++i)
	{
		if (ImGui::Button(to_string(i).c_str())) {
			//픽해서 수정 가능하게.
			m_iPickedIndex = i;
		}

		if (ImGui::IsItemHovered())
		{
			if (i < m_vSlidePath.size())
				m_pGameInstance->Use_Gizmo_Offset(&vScale, &vRot, reinterpret_cast<_float3*>(&m_vSlidePath[i]));
		}
	}
	ImGui::EndChildFrame();

	if (m_pGameInstance->Get_DIKeyState(DIK_RSHIFT) == KEYSTATE::DOWN)
		m_PickPos.float_4 = CLevel_Map::m_vPickedPos;

		ImGui::InputFloat4("Slide Path : ", m_PickPos.arr);

		if (ImGui::Button("Add Path")|| m_pGameInstance->Get_DIKeyState(DIK_RCONTROL) == KEYSTATE::DOWN)
			m_vSlidePath.push_back(m_PickPos.float_4);

	ImGui::SameLine();

	if (ImGui::Button("Delete Path"))
		m_vSlidePath.erase(m_vSlidePath.begin() + m_iPickedIndex);

	if(m_IsTestRender)
		for (auto& Path : m_vSlidePath)
			m_pGameInstance->Use_Gizmo_Offset(&vScale, &vRot, reinterpret_cast<_float3*>(&Path));

	if (ImGui::Button("Destroy"))
	{
		m_isActivate = false;
		m_pRigidbodyCom->IsActivate(false);
	}
}

HRESULT CEdit_SlideZone::Ready_Component(void* pArg)
{
	SLIDE_DESC* pDesc = static_cast<SLIDE_DESC*>(pArg);

	// Components 생성.
	CRigidbody::BOXBODY_DESC RigidbodyDesc{};
	//RigidbodyDesc.vScale = m_pTransformCom->Get_Scaled();
	XMStoreFloat4(&RigidbodyDesc.vQuat, m_pTransformCom->Get_Quaternion());
	RigidbodyDesc.eShape = SHAPE::BOX;
	XMStoreFloat3(&RigidbodyDesc.vPos, m_pTransformCom->Get_State(STATE::POSITION));
	RigidbodyDesc.eType = EMotionType::Kinematic;
	RigidbodyDesc.iLayer = ENUM_CLASS(COLLISIONLAYER::DETECT);
	m_vExtends = RigidbodyDesc.vExtent = pDesc->vExtends;

	Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Rigidbody"),
		TEXT("Com_Rigidbody"), reinterpret_cast<CComponent**>(&m_pRigidbodyCom), &RigidbodyDesc);

	m_pTransformCom->Set_WorldMatrix(XMLoadFloat4x4(pDesc->WorldMatrix));

	if (pDesc->IsLoad)
	{
		for (_uint i = 0; i < pDesc->iPathSize; ++i)
			m_vSlidePath.push_back(pDesc->pPath[i]);
		m_IsStart = pDesc->IsStart;
		m_vExtends = pDesc->vExtends;
	}
    return S_OK;
}

void CEdit_SlideZone::Bind_Resources()
{
}

void CEdit_SlideZone::SaveData(ofstream& File)
{
}

void CEdit_SlideZone::Ready_Events()
{
}

CEdit_SlideZone* CEdit_SlideZone::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CEdit_SlideZone* pInstance = new CEdit_SlideZone(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Create : Edit_TriggerBox");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CEdit_SlideZone::Clone(void* pArg)
{
	CEdit_SlideZone* pInstance = new CEdit_SlideZone(*this);

	if (FAILED(pInstance->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Create : Edit_TriggerBox (Clone)");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CEdit_SlideZone::Free()
{
	__super::Free();

	Safe_Release(m_pRigidbodyCom);
	Safe_Release(m_pMapInterface);
}