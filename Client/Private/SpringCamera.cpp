#include "ClientPch.h"
#include "SpringCamera.h"

#include "GameSystem.h"

CSpringCamera::CSpringCamera(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CCamera { pDevice, pContext },
	m_pGameSystem { CGameSystem::GetInstance() }
{
	Safe_AddRef(m_pGameSystem);
}

CSpringCamera::CSpringCamera(const CSpringCamera& Prototype)
	: CCamera { Prototype },
	m_pGameSystem{ CGameSystem::GetInstance() }
{
	Safe_AddRef(m_pGameSystem);
}

void CSpringCamera::Update_Target(const _fvector & TargetPos, _float fOffsetY)
{
	m_fOffsetY = fOffsetY;
	XMStoreFloat4(&m_vTargetPosition, TargetPos);
}

void CSpringCamera::Lock_On(CTransform* pTargetTransform, const _float4x4* pBoneMatrix, _bool IsLockOn)
{
	m_pTargetTransform = pTargetTransform;

	if (nullptr == pTargetTransform || false == IsLockOn)
	{
		if (CAMERA_STATE::LOCKON == m_eCameraState)
			m_eCameraState = CAMERA_STATE::TARGET;
	}
	else if(CAMERA_STATE::ACTION != m_eCameraState)
	{
		// LockOn Position 계산
		_matrix LockOnMatrix = XMLoadFloat4x4(pBoneMatrix) * pTargetTransform->Get_WorldMatrix();
		_vector vScale{}, vQuat{}, vPos{};
		XMMatrixDecompose(&vScale, &vQuat, &vPos, LockOnMatrix);
		XMStoreFloat4(&m_LockOnPosition, vPos);

		// LockOn 처음에 초기 셋팅
		if (CAMERA_STATE::LOCKON != m_eCameraState)
		{
			//XMStoreFloat4(&m_vLookPosition, XMLoadFloat4(&m_vTargetPosition) * (1.f - m_fRatio) + m_pTargetTransform->Get_State(STATE::POSITION) * m_fRatio);
			XMStoreFloat4(&m_vLookPosition, XMLoadFloat4(&m_vTargetPosition) * (1.f - m_fRatio) + XMLoadFloat4(&m_LockOnPosition) * m_fRatio);
			_vector vLook = XMLoadFloat4(&m_vLookPosition) - m_pTransformCom->Get_State(STATE::POSITION);
			m_pTransformCom->LookDir(XMVector3Normalize(vLook));
		}
		m_eCameraState = CAMERA_STATE::LOCKON;
	}
}

_vector CSpringCamera::Get_LookVector()
{
	_vector vLook = XMVector3Normalize(m_pTransformCom->Get_State(STATE::LOOK));
	return vLook;
}

_vector CSpringCamera::Get_LookVector_NoPitch()
{
	_vector vLook = XMVector3Normalize(m_pTransformCom->Get_State(STATE::LOOK));
	vLook = XMVectorSetY(vLook, 0.f);
	return XMVector3Normalize(vLook);
}

_vector CSpringCamera::Get_RightVector_NoPitch()
{
	_vector vRight = XMVector3Normalize(m_pTransformCom->Get_State(STATE::RIGHT));
	vRight = XMVectorSetY(vRight, 0.f);  // Pitch ����
	return XMVector3Normalize(vRight);
}


HRESULT CSpringCamera::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CSpringCamera::Initialize_Clone(void* pArg)
{
	if (FAILED(__super::Initialize_Clone(pArg)))
		CRASH("Camera");

	m_fDistance = 3.f;
	m_fFixedDistance = 3.f;
	m_fLerpSpeed = 1.5f;
	m_fMinDistance = 1.f;
	m_fMaxDistance = 4.f;

	m_fLockOnMinDistance = 4.f;
	m_fRatio = 0.4f;

	m_fStiffness = 0.3f;

	//m_fLockOnOffsetY = 3.5f;
	m_fLockOnOffsetY = 0.f;
    return S_OK;
}

void CSpringCamera::Priority_Update(_float fTimeDelta)
{
}

void CSpringCamera::Update(_float fTimeDelta)
{
	m_pTransformCom->Save_PreviousPosition();

	// Look Position Init
	m_vLookPosition = m_vTargetPosition;
	m_vLookPosition.y += m_fOffsetY;

	// Spring
	if (CAMERA_STATE::SPRING == m_eCameraState)
		Spring(fTimeDelta);
	else if(CAMERA_STATE::ACTION != m_eCameraState)
		Lerp_Distance(fTimeDelta);

	// Lock-On
	if (CAMERA_STATE::LOCKON == m_eCameraState)
	{
		Dual_Targeting(fTimeDelta);
		Dynamic_Fov(fTimeDelta);
	}
	else
		m_fFovy = XMConvertToRadians(60.f);

	// Action
	if (CAMERA_STATE::ACTION == m_eCameraState)
	{
		if(false == m_isRecovery)
			Action(fTimeDelta);
		if (true == m_isRecovery)
			Recovery(fTimeDelta);
	}
	else if(true == m_pGameSystem->IsFix())
	{
		Mouse_Scroll(fTimeDelta);
		// 0. Cam Rotate
		if (CAMERA_STATE::TARGET == m_eCameraState)
			__super::Mouse_Move_Up();
	}

	// 1. Camera Position 계산
	Compute_CamPos();
	// 2. Ray Cast => 벽 충돌
	if(CAMERA_STATE::TARGET == m_eCameraState)
		Check_Ray();

	m_pGameInstance->Update_Listener(m_pTransformCom, fTimeDelta);

	Shaking(fTimeDelta);
}

void CSpringCamera::Late_Update(_float fTimeDelta)
{
}

void CSpringCamera::Render()
{
}

void CSpringCamera::Lerp_Distance(_float fTimeDelta)
{
	if (0.1f < fabsf(m_fFixedDistance - m_fDistance))
		m_fDistance += (m_fFixedDistance - m_fDistance) * fTimeDelta * m_fLerpSpeed;
}

void CSpringCamera::Mouse_Scroll(_float fTimeDelta)
{
	m_fFixedDistance -= m_pGameInstance->Get_DIMouseMove(MOUSEMOVESTATE::WHEEL) * fTimeDelta * 0.05f;

	m_fFixedDistance = max(m_fMinDistance, min(m_fMaxDistance, m_fFixedDistance));
}

void CSpringCamera::Spring(_float fTimeDelta)
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

void CSpringCamera::Compute_CamPos()
{
	_vector vLook = XMVector3Normalize(m_pTransformCom->Get_State(STATE::LOOK));

	// Offset Y Adjust
	_vector vTargetPos;
	vTargetPos = XMLoadFloat4(&m_vLookPosition);

	m_fLockOnDistanceOffset = 0.f;
	if(CAMERA_STATE::LOCKON ==  m_eCameraState)
		Adjust_LockOn_Distance();
	//cout << "LDO : " << m_fLockOnDistanceOffset << endl;
	_vector vCamPos = XMVectorSetW(vTargetPos - vLook * (m_fDistance + m_fLockOnDistanceOffset), 1.f);

	m_pTransformCom->Set_State(STATE::POSITION, vCamPos);
}

void CSpringCamera::Check_Ray()
{
	_vector vCamPos = m_pTransformCom->Get_State(STATE::POSITION);

	_vector vStartPos = XMLoadFloat4(&m_vLookPosition);
	//vStartPos.m128_f32[1] += m_fOffsetY;
	_float4 vOut;
	if (true == m_pGameInstance->Ray_Cast(vStartPos, vCamPos, &vOut))
	{
		_float fLength = XMVectorGetX(XMVector3Length(XMLoadFloat4(&m_vTargetPosition) - XMLoadFloat4(&vOut)));
		if (fLength < 0.4f)
			return;
		m_pTransformCom->Set_State(STATE::POSITION, XMLoadFloat4(&vOut));
	}
}


void CSpringCamera::Lerp_Move(_float fTimeDelta)
{
	// ���� ȸ����
	_vector vPreQuat = m_pTransformCom->Get_Quaternion();

	// ���� Dir
	_vector vDestinationDir = XMVector3Normalize(XMVectorSetY(XMLoadFloat4(&m_vLookPosition), 0.f) - XMVectorSetY(XMLoadFloat4(&m_vTargetPosition), 0.f));

	_vector vCamPos = XMLoadFloat4(&m_vLookPosition) - vDestinationDir * m_fDistance;

	// LockOnOffsetY 조정
	m_fLockOnOffsetY += fTimeDelta * 1.f;
	m_fLockOnOffsetY = max(m_fOffsetY, min(m_fLockOnOffsetY, 0.5f));

	vCamPos.m128_f32[1] += m_fLockOnOffsetY;
	_vector vLookDir = XMLoadFloat4(&m_vLookPosition) - vCamPos;
	_float fLength = XMVectorGetX(XMVector3Length(vLookDir));

	// Position 겹칠 때 예외 처리
	if(0.f < fLength)
		m_pTransformCom->LookDir(vLookDir);
	_vector vCurrentQuat = m_pTransformCom->Get_Quaternion();

	_float fDot = XMVectorGetX(XMQuaternionDot(vPreQuat, vCurrentQuat));
	// ȸ�� ����
	_float fLerp = {};
	if (fDot < cos(XMConvertToRadians(25.f)))
		fLerp = 1.f - exp(-1.f * fTimeDelta * 2.5f);
	else
		fLerp = 1.f - exp(-1.f * fTimeDelta * 1.25f * min(1.f, (cos(XMConvertToRadians(25.f) - fDot))));

	fLerp = max(0.f, min(1.f, fLerp));
	m_pTransformCom->Rotation_Quaternion(XMQuaternionSlerp(vPreQuat, vCurrentQuat, fLerp));
}

void CSpringCamera::Dual_Targeting(_float fTimeDelta)
{
	if (nullptr == m_pTargetTransform)
		return;

	XMStoreFloat4(&m_vLookPosition, XMLoadFloat4(&m_vTargetPosition) * (1.f - m_fRatio) + XMLoadFloat4(&m_LockOnPosition) * m_fRatio);
	// Dynamic Distance
	Dynamic_Distance();
	// Moving Lerp
	Lerp_Move(fTimeDelta);
}

void CSpringCamera::Dynamic_Distance()
{
	if (nullptr == m_pTargetTransform)
		return;
	
	//_vector vLockOnPos = m_pTargetTransform->Get_State(STATE::POSITION);
	_vector vLockOnPos = XMLoadFloat4(&m_LockOnPosition);
	_vector vTargetPos = XMLoadFloat4(&m_vTargetPosition);
	vLockOnPos.m128_f32[1] = 0.f;
	vTargetPos.m128_f32[1] = 0.f;

	_float fDistance = XMVectorGetX(XMVector3Length(vLockOnPos - vTargetPos));

	m_fFixedDistance = max(m_fLockOnMinDistance, sqrt(fDistance * fDistance + m_fLockOnOffsetY * m_fLockOnOffsetY));
}

void CSpringCamera::Adjust_LockOn_Distance()
{
	_vector vLook = m_pTransformCom->Get_State(STATE::LOOK);
	_vector vLookRemoveY = vLook;
	vLookRemoveY.m128_f32[1] = 0.f;

	_float fRadian = XMVectorGetX(XMVector3Dot(XMVector3Normalize(vLook), XMVector3Normalize(vLookRemoveY)));

	_float fLength = XMVectorGetX(XMVector3Length(XMLoadFloat4(&m_vTargetPosition) - XMLoadFloat4(&m_vLookPosition)));
	
	if (0.f == fRadian)
		fRadian = 1.f;
	m_fLockOnDistanceOffset = fLength / fRadian;
}

void CSpringCamera::Dynamic_Fov(_float fTimeDelta)
{
	if (nullptr == m_pTargetTransform)
		return;

	_vector vTargetPos = XMLoadFloat4(&m_vTargetPosition);
	//_vector vLockOnPos = m_pTargetTransform->Get_State(STATE::POSITION);
	_vector vLockOnPos = XMLoadFloat4(&m_LockOnPosition);

	_float fGapY = fabsf(vLockOnPos.m128_f32[1] - vTargetPos.m128_f32[1]);

	m_fFovy = XMConvertToRadians(min(60.f + fGapY, 85.f));
}

void CSpringCamera::Action(_float fTimeDelta)
{
	// Action End
	if (m_iFrameIndex >= static_cast<_int>(m_Frames.size() - 1))
	{
		if (false == m_isMaintain && false == m_isEscape)
		{
			//m_pGameSystem->HUD_FadeIn();
			SetUp_Recovery();
		}
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
		vPreQuaternion = XMQuaternionMultiply(vPreQuaternion, XMQuaternionRotationMatrix(XMLoadFloat4x4(&m_OwnerMatrix)));
		vPreTranslation = XMLoadFloat3(&m_Frames[m_iFrameIndex].vTranslation);
		vPreTranslation = XMVector3TransformNormal(vPreTranslation, XMMatrixRotationQuaternion(vPreQuaternion));
		fPreDistance = m_Frames[m_iFrameIndex].fDistance;
		// Fov
		m_fPreFovy = XMConvertToRadians(m_Frames[m_iFrameIndex].fFovy);
	}

	m_isLerp = m_Frames[m_iFrameIndex + 1].isLerp;

	if (true == m_isLerp)
	{
		_vector vDestQuat = XMLoadFloat4(&m_Frames[m_iFrameIndex + 1].vRotation);
		vDestQuat = XMQuaternionMultiply(vDestQuat, XMQuaternionRotationMatrix(XMLoadFloat4x4(&m_OwnerMatrix)));
		_vector vLerpQuat = XMQuaternionSlerp(vPreQuaternion, vDestQuat, fRatio);
		m_pTransformCom->Rotation_Quaternion(vLerpQuat);

		// Translation Offset
		_vector vDestTranslation = XMLoadFloat3(&m_Frames[m_iFrameIndex + 1].vTranslation);
		vDestTranslation = XMVector3TransformNormal(vDestTranslation, XMMatrixRotationQuaternion(vDestQuat));
		_vector vLerpTranslation = XMVectorLerp(vPreTranslation, vDestTranslation, fRatio);
		XMStoreFloat4(&m_vLookPosition, XMVectorSetW(XMLoadFloat4(&m_vLookPosition) + vLerpTranslation, 1.f));
		XMStoreFloat3(&m_vEndTranslation, vDestTranslation);

		// Distance
		_float fDestDistance = m_Frames[m_iFrameIndex + 1].fDistance;
		m_fDistance = lerp(fPreDistance, fDestDistance, fRatio);

		// Fov Lerp
		m_fFovy = m_fPreFovy + (XMConvertToRadians(m_Frames[m_iFrameIndex + 1].fFovy) - m_fPreFovy) * fRatio;
	}
	// None Lerp (TELEPORT)
	else
	{
		_vector vDestQuat = XMLoadFloat4(&m_Frames[m_iFrameIndex + 1].vRotation);
		vDestQuat = XMQuaternionMultiply(vDestQuat, XMQuaternionRotationMatrix(XMLoadFloat4x4(&m_OwnerMatrix)));
		m_pTransformCom->Rotation_Quaternion(vDestQuat);

		_vector vDestTranslation = XMLoadFloat3(&m_Frames[m_iFrameIndex + 1].vTranslation);
		vDestTranslation = XMVector3TransformNormal(vDestTranslation, XMMatrixRotationQuaternion(vDestQuat));
		XMStoreFloat4(&m_vLookPosition, XMVectorSetW(XMLoadFloat4(&m_vLookPosition) + vDestTranslation, 1.f));
		XMStoreFloat3(&m_vEndTranslation, vDestTranslation);

		m_fDistance = m_Frames[m_iFrameIndex + 1].fDistance;

		m_fFovy = XMConvertToRadians(m_Frames[m_iFrameIndex + 1].fFovy);
	}

	if (m_Frames[m_iFrameIndex + 1].fStartFrame < m_fTrackPosition)
		++m_iFrameIndex;
}

void CSpringCamera::Recovery(_float fTimeDelta)
{
	m_fTrackPosition += fTimeDelta;

	if (m_fTrackPosition > 1.5f)
	{
		m_isRecovery = false;
		m_eCameraState = CAMERA_STATE::TARGET;
		m_pTransformCom->Rotation_Quaternion(XMLoadFloat4(&m_vPreQuaternion));
		return;
	}

	_vector vLerpQuat = XMQuaternionSlerp(XMLoadFloat4(&m_vEndQuaternion), XMLoadFloat4(&m_vPreQuaternion), m_fTrackPosition / 1.5f);
	m_pTransformCom->Rotation_Quaternion(vLerpQuat);
	_vector vLerpTranslation = XMVectorLerp(XMLoadFloat3(&m_vEndTranslation), XMLoadFloat3(&m_vPreTranslation), m_fTrackPosition / 1.5f);
	XMStoreFloat4(&m_vLookPosition, XMVectorSetW(XMLoadFloat4(&m_vLookPosition) + vLerpTranslation, 1.f));
}

void CSpringCamera::SetUp_Recovery()
{
	m_fTrackPosition = 0.f;
	if (false == m_isRecovery)
		m_isRecovery = true;
	m_fFixedDistance = m_fPreFixedDistance;
	XMStoreFloat4(&m_vEndQuaternion, m_pTransformCom->Get_Quaternion());
	m_isMaintain = false;
	//m_vEndTranslation = _float3(0.f, 0.f, 0.f);
}


CSpringCamera* CSpringCamera::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CSpringCamera* pInstance = new CSpringCamera(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Create : SpringCamera");
		Safe_Release(pInstance);
	}

    return pInstance;
}

CGameObject* CSpringCamera::Clone(void* pArg)
{
	CSpringCamera* pClone = new CSpringCamera(*this);

	if (FAILED(pClone->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Create : SpringCamera (Clone)");
		Safe_Release(pClone);
	}

	return pClone;
}

void CSpringCamera::Free()
{
	__super::Free();

	//m_TargetTransforms.clear();
	m_pTargetTransform = nullptr;

	Safe_Release(m_pGameSystem);
}
