#include "EditorPch.h"
#include "SpringCamera_Edit.h"
#include "Event_Scene_Edit.h"

CSpringCamera_Edit::CSpringCamera_Edit(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CCamera { pDevice, pContext }
{
}

CSpringCamera_Edit::CSpringCamera_Edit(const CSpringCamera_Edit& Prototype)
	: CCamera { Prototype }
{
}

HRESULT CSpringCamera_Edit::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CSpringCamera_Edit::Initialize_Clone(void* pArg)
{
	if (FAILED(__super::Initialize_Clone(pArg)))
		CRASH("Camera");

	Ready_Component();
	Ready_Event();

	m_fDistance = 5.f;
	m_fFixedDistance = 5.f;
	m_fLerpSpeed = 1.5f;
	m_fMinDistance = 1.f;

	m_fStiffness = 3.f;

	m_fLockOnOffsetY = 35.f;

    return S_OK;
}

void CSpringCamera_Edit::Priority_Update(_float fTimeDelta)
{
}

void CSpringCamera_Edit::Update(_float fTimeDelta)
{
	// GUI
	ImGui::Begin("Camera Tool");
	if(CAMERA_STATE::LOCKON == m_eCameraState)
		ImGui::Text("Lock On");
	_char szPos[MAX_PATH] = {};
	_vector vCamPos = m_pTransformCom->Get_State(STATE::POSITION);
	sprintf_s(szPos, MAX_PATH, "%.2f, %.2f, %.2f", vCamPos.m128_f32[0], vCamPos.m128_f32[1], vCamPos.m128_f32[2]);
	ImGui::Text(szPos);
	_char szQuat[MAX_PATH] = {};
	_float4 vQuat = {};
	XMStoreFloat4(&vQuat, m_pTransformCom->Get_Quaternion());
	sprintf_s(szQuat, MAX_PATH, "%.4f, %.4f, %.4f, %.4f", vQuat.x, vQuat.y, vQuat.z, vQuat.w);
	ImGui::Text(szQuat);
	_char szDistance[MAX_PATH] = {};
	sprintf_s(szDistance, MAX_PATH, "Distance : %.2f / Fixed Distance : %.2f", m_fDistance, m_fFixedDistance);
	ImGui::Text(szDistance);
	_char szFOV[MAX_PATH] = {};
	sprintf_s(szFOV, MAX_PATH, "FOV : %.2f", m_fFovy);
	ImGui::Text(szFOV);
	ImGui::End();

	// Look Position Init
	m_vLookPosition = m_vTargetPosition;
	m_vLookPosition.y += m_fOffsetY;

	// Action
	if (CAMERA_STATE::ACTION == m_eCameraState)
	{
		if (true == m_isRecovery)
			Recovery(fTimeDelta);
		else
			Action(fTimeDelta);
	}
	else
	{
		if(m_pGameInstance->Get_DIKeyState(DIK_APOSTROPHE) == KEYSTATE::PRESS)
			Mouse_Scroll(fTimeDelta);
		// 0. Cam Rotate
		if (CAMERA_STATE::TARGET == m_eCameraState && m_pGameInstance->Get_DIKeyState(DIK_LCONTROL) == KEYSTATE::PRESS)
			__super::Mouse_Move_Up();
	}

	// Spring
	if (CAMERA_STATE::SPRING == m_eCameraState)
		Spring(fTimeDelta);
	else if(CAMERA_STATE::ACTION != m_eCameraState)
		Lerp_Distance(fTimeDelta);

	// Lock-On
	//m_fFovy = XMConvertToRadians(60.f);
	if (CAMERA_STATE::LOCKON == m_eCameraState)
	{
		//m_fFovy = XMConvertToRadians(90.f);
		Sorting_Target();
		Dual_Targeting(fTimeDelta);
	}

	// Target Transform Rest
	m_TargetTransforms.clear();
	m_pTargetTransform = nullptr;
	
	// 1. �Ÿ� �������� ���� ���� ����
	Compute_CamPos();
	// 2. Ray Cast �̿��Ͽ� ����, ������Ʈ�� �浹
	//Check_Ray();

	//m_pRigidbodyCom->Update_Rigidbody(m_pTransformCom->Get_WorldMatrix(), fTimeDelta);

	// Test
	if (m_pGameInstance->Get_DIMouseState(MOUSEKEYSTATE::WB) == KEYSTATE::DOWN)
		Lock_On();
}

void CSpringCamera_Edit::Late_Update(_float fTimeDelta)
{
	if (CAMERA_STATE::LOCKON == m_eCameraState && 0 == m_TargetTransforms.size())
		m_eCameraState = CAMERA_STATE::TARGET;
}

void CSpringCamera_Edit::Render()
{
}

void CSpringCamera_Edit::OnCollide_During(_uint iLayer, void* pDesc, const ContactManifold& Manifold)
{
	CTransform* pTargetTransform = static_cast<CTransform*>(pDesc);
	if (nullptr == pTargetTransform)
		return;
	m_TargetTransforms.push_back(pTargetTransform);
}

void CSpringCamera_Edit::Lerp_Distance(_float fTimeDelta)
{
	if (0.1f < fabsf(m_fFixedDistance - m_fDistance))
		m_fDistance += (m_fFixedDistance - m_fDistance) * fTimeDelta * m_fLerpSpeed;
}

void CSpringCamera_Edit::Mouse_Scroll(_float fTimeDelta)
{
	m_fFixedDistance -= m_pGameInstance->Get_DIMouseMove(MOUSEMOVESTATE::WHEEL) * fTimeDelta * 0.2f;
}

void CSpringCamera_Edit::Spring(_float fTimeDelta)
{
	_float fDistance = XMVectorGetX(XMVector3Length(XMLoadFloat4(&m_vLookPosition) - m_pTransformCom->Get_State(STATE::POSITION)));

	if (fDistance < m_fDestination)
	{
		m_eCameraState = CAMERA_STATE::TARGET;
		m_fDistance = m_fDestination;
		return;
	}
	m_fDistance += (m_fDestination - m_fFixedDistance) * m_fStiffness / m_fSpringDuration * fTimeDelta;
}

void CSpringCamera_Edit::Compute_CamPos()
{
	_vector vLook = XMVector3Normalize(m_pTransformCom->Get_State(STATE::LOOK));

	// Offset Y Adjust
	_vector vTargetPos;
	vTargetPos = XMLoadFloat4(&m_vLookPosition);

	_vector vCamPos = XMVectorSetW(vTargetPos - vLook * m_fDistance, 1.f);

	m_pTransformCom->Set_State(STATE::POSITION, vCamPos);
}

void CSpringCamera_Edit::Check_Ray()
{
	_vector vCamPos = m_pTransformCom->Get_State(STATE::POSITION);

	_vector vStartPos = XMLoadFloat4(&m_vLookPosition);
	_float4 vOut;
	if (true == m_pGameInstance->Ray_Cast(vStartPos, vCamPos, &vOut))
		m_pTransformCom->Set_State(STATE::POSITION, XMLoadFloat4(&vOut));
}

void CSpringCamera_Edit::Lerp_Move(_float fTimeDelta)
{
	// ���� ȸ����
	_vector vPreQuat = m_pTransformCom->Get_Quaternion();

	// ���� Dir
	_vector vDestinationDir = XMVector3Normalize(XMLoadFloat4(&m_vLookPosition) - XMLoadFloat4(&m_vTargetPosition));

	_vector vCamPos = XMLoadFloat4(&m_vLookPosition) - vDestinationDir * m_fDistance;
	vCamPos.m128_f32[1] += m_fLockOnOffsetY;
	_vector vLookDir = XMLoadFloat4(&m_vLookPosition) - vCamPos;
	// ��ǥ Dir
	m_pTransformCom->LookDir(vLookDir);
	_vector vCurrentQuat = m_pTransformCom->Get_Quaternion();

	_float fDot = XMVectorGetX(XMQuaternionDot(vPreQuat, vCurrentQuat));
	// ȸ�� ����
	_float fLerp = {};
	if (fDot < cos(XMConvertToRadians(25.f)))
		fLerp = 1.f - exp(-1.f * fTimeDelta * 10.f);
	else
		fLerp = 1.f - exp(-1.f * fTimeDelta * 10.f * (cos(XMConvertToRadians(25.f) - fDot)));
	m_pTransformCom->Rotation_Quaternion(XMQuaternionSlerp(vPreQuat, vCurrentQuat, fLerp));
}

void CSpringCamera_Edit::Sorting_Target()
{
	sort(m_TargetTransforms.begin(), m_TargetTransforms.end(), [this](CTransform* pSrcTransform, CTransform* pDstTransform)->_bool {
		_float fSrcDistance = XMVectorGetX(XMVector3Length(XMLoadFloat4(m_pGameInstance->Get_CamPos()) - pSrcTransform->Get_State(STATE::POSITION)));
		_float fDstDistance = XMVectorGetX(XMVector3Length(XMLoadFloat4(m_pGameInstance->Get_CamPos()) - pDstTransform->Get_State(STATE::POSITION)));
		return fSrcDistance < fDstDistance;
		});

	if (0 < m_TargetTransforms.size())
		m_pTargetTransform = m_TargetTransforms[0];
}

void CSpringCamera_Edit::Dual_Targeting(_float fTimeDelta)
{
	if (nullptr == m_pTargetTransform)
		return;

	_float fRatio = 0.1f;
	XMStoreFloat4(&m_vLookPosition, XMLoadFloat4(&m_vTargetPosition) * (1.f - fRatio) + m_pTargetTransform->Get_State(STATE::POSITION) * fRatio);
	// Dynamic Distance
	Dynamic_Distance();
	// Moving Lerp
	Lerp_Move(fTimeDelta);
}

void CSpringCamera_Edit::Dynamic_Distance()
{
	if (nullptr == m_pTargetTransform)
		return;
	//_float fDistance = XMVectorGetX(XMVector3Length(XMLoadFloat4(&m_vLookPosition) - XMLoadFloat4(&m_vTargetPosition)));
	_float fDistance = XMVectorGetX(XMVector3Length(m_pTargetTransform->Get_State(STATE::POSITION) - XMLoadFloat4(&m_vTargetPosition)));

	m_fFixedDistance = max(m_fMinDistance, sqrt(fDistance * fDistance + m_fLockOnOffsetY * m_fLockOnOffsetY));
}

void CSpringCamera_Edit::Action(_float fTimeDelta)
{
	// Action End
	if (m_iFrameIndex >= static_cast<_int>(m_Frames.size() - 1))
	{
		if (false == m_isMaintain && false == m_isEscape)
			SetUp_Recovery();
		if (true == m_isEscape)
			m_eCameraState = CAMERA_STATE::TARGET;
		return;
	}

	_float fStartFrame{}, fEndFrame{};
	fStartFrame = -1 == m_iFrameIndex ? m_fFirstFrame : m_Frames[m_iFrameIndex].fStartFrame;
	fEndFrame = m_Frames[m_iFrameIndex + 1].fStartFrame;

	_float fRatio = (m_fTrackPosition - fStartFrame) / (fEndFrame - fStartFrame);
	m_fTrackPosition += fTimeDelta * m_fTrackPerSec;

	// Rotation
	_vector vPreQuaternion = {};
	_vector vPreTranslation = {};
	_float fPreDistance = {};
	if (-1 == m_iFrameIndex)
	{
		vPreQuaternion = XMLoadFloat4(&m_vPreQuaternion);
		vPreTranslation = XMLoadFloat3(&m_vPreTranslation);
		fPreDistance = m_fDistance;
		m_fPreFovy = m_fFovy;
	}
	else
	{
		vPreQuaternion = XMLoadFloat4(&m_Frames[m_iFrameIndex].vRotation);
		vPreTranslation = XMLoadFloat3(&m_Frames[m_iFrameIndex].vTranslation);
		vPreTranslation = XMVector3TransformNormal(vPreTranslation, XMMatrixRotationQuaternion(vPreQuaternion));
		fPreDistance = m_Frames[m_iFrameIndex].fDistance;
		// Fov
		m_fPreFovy = XMConvertToRadians(m_Frames[m_iFrameIndex].fFovy);
	}

	m_isLerp = m_Frames[m_iFrameIndex + 1].isLerp;

	if (true == m_isLerp)
	{
		// Lerp
		_vector vDestQuat = XMLoadFloat4(&m_Frames[m_iFrameIndex + 1].vRotation);
		_vector vLerpQuat = XMQuaternionSlerp(vPreQuaternion, vDestQuat, fRatio);
		m_pTransformCom->Rotation_Quaternion(vLerpQuat);

		// Translation Offset
		_vector vDestTranslation = XMLoadFloat3(&m_Frames[m_iFrameIndex + 1].vTranslation);
		vDestTranslation = XMVector3TransformNormal(vDestTranslation, XMMatrixRotationQuaternion(vDestQuat));
		_vector vLerpTranslation = XMVectorLerp(vPreTranslation, vDestTranslation, fRatio);
		XMStoreFloat4(&m_vLookPosition, XMVectorSetW(XMLoadFloat4(&m_vLookPosition) + vLerpTranslation, 1.f));

		// Distance
		//m_fFixedDistance = m_Frames[m_iFrameIndex + 1].fDistance;
		_float fDestDistance = m_Frames[m_iFrameIndex + 1].fDistance;
		m_fDistance = lerp(fPreDistance, fDestDistance, fRatio);

		// Fov Lerp
		m_fFovy = m_fPreFovy + (XMConvertToRadians(m_Frames[m_iFrameIndex + 1].fFovy) - m_fPreFovy) * fRatio;
	}
	// None Lerp (Teleport)
	else
	{
		_vector vDestQuat = XMLoadFloat4(&m_Frames[m_iFrameIndex + 1].vRotation);
		m_pTransformCom->Rotation_Quaternion(vDestQuat);

		_vector vDestTranslation = XMLoadFloat3(&m_Frames[m_iFrameIndex + 1].vTranslation);
		vDestTranslation = XMVector3TransformNormal(vDestTranslation, XMMatrixRotationQuaternion(vDestQuat));
		XMStoreFloat4(&m_vLookPosition, XMVectorSetW(XMLoadFloat4(&m_vLookPosition) + vDestTranslation, 1.f));

		//m_fFixedDistance = m_fFixedDistance = m_Frames[m_iFrameIndex + 1].fDistance;
		//m_fDistance = m_fFixedDistance;
		m_fDistance = m_Frames[m_iFrameIndex + 1].fDistance;

		m_fFovy = XMConvertToRadians(m_Frames[m_iFrameIndex + 1].fFovy);
	}

	if (m_Frames[m_iFrameIndex + 1].fStartFrame < m_fTrackPosition)
		++m_iFrameIndex;
}

void CSpringCamera_Edit::Recovery(_float fTimeDelta)
{
	m_fTrackPosition += fTimeDelta;

	if (m_fTrackPosition >= 1.5f)
	{
		m_isRecovery = false;
		m_eCameraState = CAMERA_STATE::TARGET;
		return;
	}

	_vector vLerpQuat = XMQuaternionSlerp(XMLoadFloat4(&m_vEndQuaternion), XMLoadFloat4(&m_vPreQuaternion), m_fTrackPosition / 1.5f);
	m_pTransformCom->Rotation_Quaternion(vLerpQuat);
	_vector vLerpTranslation = XMVectorLerp(XMLoadFloat3(&m_vEndTranslation), XMLoadFloat3(&m_vPreTranslation), m_fTrackPosition / 1.5f);
	XMStoreFloat4(&m_vLookPosition, XMVectorSetW(XMLoadFloat4(&m_vLookPosition) + vLerpTranslation, 1.f));
}

void CSpringCamera_Edit::SetUp_Recovery()
{
	m_fTrackPosition = 0.f;
	if (false == m_isRecovery)
		m_isRecovery = true;
	m_fFixedDistance = m_fPreFixedDistance;
	XMStoreFloat4(&m_vEndQuaternion, m_pTransformCom->Get_Quaternion());
	m_vEndTranslation = _float3(0.f, 0.f, 0.f);
}

void CSpringCamera_Edit::Ready_Component()
{
	// Com_Rigidbody
	CRigidbody::BOXBODY_DESC RigidbodyDesc = {};
	RigidbodyDesc.eBodyType = CRigidbody::BODY;
	RigidbodyDesc.eShape = SHAPE::BOX;
	RigidbodyDesc.eType = EMotionType::Kinematic;
	RigidbodyDesc.iLayer = ENUM_CLASS(COLLISIONLAYER::NONE);
	RigidbodyDesc.vExtent = _float3(1000.f, 400.f, 1000.f);
	XMStoreFloat3(&RigidbodyDesc.vPos, m_pTransformCom->Get_State(STATE::POSITION));

	if(FAILED(Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Rigidbody"), 
		TEXT("Com_Rigidbody"), reinterpret_cast<CComponent**>(&m_pRigidbodyCom), &RigidbodyDesc)))
		CRASH("Rigidbody");

	m_pRigidbodyCom->SetUp_CallBack(COLLIDE_STATE::DURING, [this](_uint iLayer, void* pDesc, const ContactManifold& Manifold) {
			OnCollide_During(iLayer, pDesc, Manifold);
		});
}

void CSpringCamera_Edit::Ready_Event()
{
	m_pGameInstance->Subscribe<CAMERA_ACTION_EVENT>(ENUM_CLASS(STATIC::NONE), TEXT("Event_Camera_Action"), [this](const CAMERA_ACTION_EVENT& event) {
			if (CAMERA_STATE::ACTION != m_eCameraState && true == event.isAction)
			{
				m_iFrameIndex = -1;
				m_fTrackPosition = static_cast<_float>(event.iStart);
				m_fFirstFrame = m_fTrackPosition;
				m_eCameraState = CAMERA_STATE::ACTION;
				m_Frames = event.pFrame;
				XMStoreFloat4(&m_vPreQuaternion, m_pTransformCom->Get_Quaternion());
				m_vPreTranslation = _float3(0.f, 0.f, 0.f);
				m_fPreFixedDistance = m_fFixedDistance;
				m_fDuration = static_cast<_float>(event.iEnd - event.iStart);
				m_isEscape = event.isEscape;
			}
			else
			{
				SetUp_Recovery();
			}
		});
}

CSpringCamera_Edit* CSpringCamera_Edit::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CSpringCamera_Edit* pInstance = new CSpringCamera_Edit(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Create : SpringCamera_Edit");
		Safe_Release(pInstance);
	}

    return pInstance;
}

CGameObject* CSpringCamera_Edit::Clone(void* pArg)
{
	CSpringCamera_Edit* pClone = new CSpringCamera_Edit(*this);

	if (FAILED(pClone->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Create : SpringCamera_Edit (Clone)");
		Safe_Release(pClone);
	}

	return pClone;
}

void CSpringCamera_Edit::Free()
{
	__super::Free();

	Safe_Release(m_pRigidbodyCom);
	m_TargetTransforms.clear();
	m_pTargetTransform = nullptr;
}
