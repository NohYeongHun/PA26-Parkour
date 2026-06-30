#include "EnginePch.h"
#include "Camera.h"

#include "GameInstance.h"

CCamera::CCamera(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject { pDevice, pContext }
{
}

CCamera::CCamera(const CCamera& Prototype)
	: CGameObject { Prototype }
{
}

void CCamera::Set_Far(_float fFar)
{
	m_fFar = fFar;
	m_pGameInstance->SetUp_CameraNF();
}

void CCamera::OnShake(const CAMERA_SHAKE& tData)
{
	if (0 == m_tShakeDatas.size())
	{
		// Origin Store
		XMStoreFloat4(&m_vOriginQuaternion, m_pTransformCom->Get_Quaternion());
		m_fOriginFov = m_fFovy;
	}

	m_tShakeDatas.push(tData);
}

HRESULT CCamera::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CCamera::Initialize_Clone(void* pArg)
{
	if (FAILED(__super::Initialize_Clone(pArg)))
		return E_FAIL;

	_uint iNumViewport = { 1 };
	D3D11_VIEWPORT ViewPort = {};
	m_pContext->RSGetViewports(&iNumViewport, &ViewPort);

	CAMERA_DESC* pDesc = static_cast<CAMERA_DESC*>(pArg);
	m_fFovy = pDesc->fFovy;
	m_fAspect = ViewPort.Width / ViewPort.Height;
	m_fNear = pDesc->fNear;
	m_fFar = pDesc->fFar;
	m_fMouseSensor = pDesc->fMouseSensor;

	m_pTransformCom->Set_State(STATE::POSITION, XMLoadFloat4(&(pDesc->vEye)));
	m_pTransformCom->LookAt(XMLoadFloat4(&(pDesc->vAt)));

	return S_OK;
}

void CCamera::Priority_Update(_float fTimeDelta)
{
}

void CCamera::Update(_float fTimeDelta)
{


}

void CCamera::Late_Update(_float fTimeDelta)
{

	
}

void CCamera::Render()
{
}

void CCamera::Update_Matrix()
{
	m_pGameInstance->Set_PrevTransformState(D3DTS::VIEW, m_PrevTransformMatrixes[ENUM_CLASS(D3DTS::VIEW)]);
	m_pGameInstance->Set_PrevTransformState(D3DTS::PROJ, m_PrevTransformMatrixes[ENUM_CLASS(D3DTS::PROJ)]);

	_matrix CurViewMatrix = m_pTransformCom->Get_WorldMatrix_Inv();
	_matrix CurProjMatrix = XMMatrixPerspectiveFovLH(m_fFovy, m_fAspect, m_fNear, m_fFar);

	m_pGameInstance->Set_TransformState(D3DTS::VIEW, CurViewMatrix);
	m_pGameInstance->Set_TransformState(D3DTS::PROJ, CurProjMatrix);

	XMStoreFloat4x4(&m_PrevTransformMatrixes[ENUM_CLASS(D3DTS::VIEW)], CurViewMatrix);
	XMStoreFloat4x4(&m_PrevTransformMatrixes[ENUM_CLASS(D3DTS::PROJ)], CurProjMatrix);
}

void CCamera::Key_Move(_float fTimeDelta)
{
	if (m_pGameInstance->Get_DIKeyState(DIK_UP) == KEYSTATE::PRESS)
		m_pTransformCom->Go_Straight(fTimeDelta);
	if (m_pGameInstance->Get_DIKeyState(DIK_DOWN) == KEYSTATE::PRESS)
		m_pTransformCom->Go_Backward(fTimeDelta);
	if (m_pGameInstance->Get_DIKeyState(DIK_LEFT) == KEYSTATE::PRESS)
		m_pTransformCom->Go_Left(fTimeDelta);
	if (m_pGameInstance->Get_DIKeyState(DIK_RIGHT) == KEYSTATE::PRESS)
		m_pTransformCom->Go_Right(fTimeDelta);
}

void CCamera::Mouse_Move()
{
	m_pTransformCom->Turn(m_pTransformCom->Get_State(STATE::UP), m_pGameInstance->Get_DIMouseMove(MOUSEMOVESTATE::X) * m_fMouseSensor);
	m_pTransformCom->Turn(m_pTransformCom->Get_State(STATE::RIGHT), m_pGameInstance->Get_DIMouseMove(MOUSEMOVESTATE::Y) * m_fMouseSensor);
}

void CCamera::Mouse_Move_Up()
{
	_float fTurnValueX = m_pGameInstance->Get_DIMouseMove(MOUSEMOVESTATE::X) * m_fMouseSensor;
	_float fTurnValueY = m_pGameInstance->Get_DIMouseMove(MOUSEMOVESTATE::Y) * m_fMouseSensor;
	
	m_pTransformCom->Turn_Quaternion(XMQuaternionRotationRollPitchYaw(0.f, fTurnValueX, 0.f));

	// Camera 위 아래 방향 전환 시, Camera 시야가 위를 바라볼 때 뒤로 넘어가지 않도록 조정
	_vector vPreLook = m_pTransformCom->Get_State(STATE::LOOK);
	m_pTransformCom->Turn_Quaternion(XMQuaternionRotationAxis(m_pTransformCom->Get_State(STATE::RIGHT), fTurnValueY));
	_vector vLook = m_pTransformCom->Get_State(STATE::LOOK);
	vLook = XMVector4Normalize(vLook);
	_vector vUp = XMVectorSet(0.f, 1.f, 0.f, 0.f);
	_float fDot = XMVector3Dot(vLook, vUp).m128_f32[0];
	if (cosf(XMConvertToRadians(10.f)) <= fabsf(fDot))
		m_pTransformCom->LookDir(XMVector4Normalize(vPreLook));
}

void CCamera::SetUp_Shake()
{
	m_isShake = true;
	m_fShakeTimeAcc = 0.f;
	m_fRandTimeAcc = 0.f;

	m_tShakeData = m_tShakeDatas.front();

	m_fADSRStart = min(0.04f, m_tShakeData.fDuration * 0.2f);
	m_fADSREnd = max(0.12f, m_tShakeData.fDuration * 0.6f);
}

void CCamera::Shaking(_float fTimeDelta)
{
	if (0 == m_tShakeDatas.size())
		return;
	// Shake Data 들어왔을 때, Shake 아닌 경우
	else if (false == m_isShake)
		SetUp_Shake();
	

	m_fShakeTimeAcc += fTimeDelta;
	m_fRandTimeAcc += fTimeDelta;

	if (m_fShakeTimeAcc > m_tShakeData.fDuration)
	{
		m_isShake = false;
		m_tShakeDatas.pop();
		m_fFovy = m_fOriginFov;
		return;
	}

	if (m_fRandTimeAcc >= 1.f / m_tShakeData.fFrequency)
	{
		m_vRand = _float3(m_pGameInstance->Rand(-1.f, 1.f), m_pGameInstance->Rand(-1.f, 1.f), m_pGameInstance->Rand(-1.f, 1.f));
		m_fRandTimeAcc = 0.f;
	}

	_float fWeight = ADSR();

	// Rotation
	_float fPitch = m_tShakeData.vRotation.x * m_vRand.x * fWeight * m_fDecay * m_tShakeData.fAmplitude;
	_float fYaw = m_tShakeData.vRotation.y * m_vRand.y * fWeight * m_fDecay * m_tShakeData.fAmplitude;
	_float fRoll = m_tShakeData.vRotation.z * m_vRand.z * fWeight * m_fDecay * m_tShakeData.fAmplitude;
	_vector vQuat = XMQuaternionRotationRollPitchYaw(fPitch, fYaw, fRoll);
	m_pTransformCom->Rotation_Quaternion(XMQuaternionMultiply(XMLoadFloat4(&m_vOriginQuaternion), vQuat));

	m_tShakeData.fAmplitude *= 0.5f;
	//m_tShakeData.fAmplitude *= 0.98f;

	// Fov
	m_fFovy = m_fOriginFov + m_tShakeData.fFovKick * fWeight * m_fDecay;

	// 감쇠
	m_fDecay = 1.f - (m_fShakeTimeAcc / m_tShakeData.fDuration);
	m_fDecay *= m_fDecay;
}

_float CCamera::ADSR()
{
	if (m_fShakeTimeAcc <= m_fADSRStart) return m_fShakeTimeAcc / m_fADSRStart;
	_float fTail = m_tShakeData.fDuration - m_fShakeTimeAcc;
	if (fTail <= m_fADSREnd) return max(0.f, fTail / m_fADSREnd);
	return 1.0f;
}

void CCamera::Free()
{
	__super::Free();
}
