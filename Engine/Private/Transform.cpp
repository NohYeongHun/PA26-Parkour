#include "EnginePch.h"
#include "Transform.h"

#include "Shader.h"
#include "DeferredShader.h"
#include "Navigation.h"

CTransform::CTransform(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CComponent { pDevice, pContext } 
{
}

HRESULT CTransform::Initialize_Clone(void* pArg)
{
	TRANSFORM_DESC* pDesc = static_cast<TRANSFORM_DESC*>(pArg);
	if (nullptr != pDesc)
	{
		m_fSpeedPerSec = pDesc->fSpeedPerSec;
		m_fRotationPerSec = pDesc->fRotationPerSec;
	}

	XMStoreFloat4x4(&m_WorldMatrix, XMMatrixIdentity());

	return S_OK;
}

HRESULT CTransform::Bind_Matrix(class CShader* pShader, const _char* ConstantName)
{
	return pShader->Bind_Matrix(ConstantName, &m_WorldMatrix);
}

HRESULT CTransform::Bind_Matrix(CDeferredShader* pShader, const _char* ConstantName, ID3DX11Effect* pEffect)
{
	return pShader->Bind_Matrix(ConstantName, &m_WorldMatrix, pEffect);
}

void CTransform::Scale(_float3 vScale)
{
	Set_State(STATE::RIGHT, XMVector3Normalize(Get_State(STATE::RIGHT)) * vScale.x);
	Set_State(STATE::UP, XMVector3Normalize(Get_State(STATE::UP)) * vScale.y);
	Set_State(STATE::LOOK, XMVector3Normalize(Get_State(STATE::LOOK)) * vScale.z);
}

void CTransform::Scaling(_float3 vScale)
{
	Set_State(STATE::RIGHT, Get_State(STATE::RIGHT) * vScale.x);
	Set_State(STATE::UP, Get_State(STATE::UP) * vScale.y);
	Set_State(STATE::LOOK, Get_State(STATE::LOOK) * vScale.z);
}

void CTransform::Go_Straight(_float fTimeDelta)
{
	_vector vPosition = Get_State(STATE::POSITION);
	_vector vLook = Get_State(STATE::LOOK);

	vPosition += XMVector3Normalize(vLook) * m_fSpeedPerSec * fTimeDelta;

	Set_State(STATE::POSITION, vPosition);
}

void CTransform::Go_Backward(_float fTimeDelta)
{
	_vector vPosition = Get_State(STATE::POSITION);
	_vector vLook = Get_State(STATE::LOOK);

	vPosition -= XMVector3Normalize(vLook) * m_fSpeedPerSec * fTimeDelta;
	Set_State(STATE::POSITION, vPosition);
}

void CTransform::Go_Left(_float fTimeDelta)
{
	_vector vPosition = Get_State(STATE::POSITION);
	_vector vRight = Get_State(STATE::RIGHT);

	vPosition -= XMVector3Normalize(vRight) * m_fSpeedPerSec * fTimeDelta;
	Set_State(STATE::POSITION, vPosition);
}

void CTransform::Go_Right(_float fTimeDelta)
{
	_vector vPosition = Get_State(STATE::POSITION);
	_vector vRight = Get_State(STATE::RIGHT);

	vPosition += XMVector3Normalize(vRight) * m_fSpeedPerSec * fTimeDelta;
	Set_State(STATE::POSITION, vPosition);
}

void CTransform::Go_Dir(const _fvector& vMoveDir, _float fTimeDelta)
{
	_vector vPosition = Get_State(STATE::POSITION);

	vPosition += vMoveDir * m_fSpeedPerSec * fTimeDelta;
	Set_State(STATE::POSITION, vPosition);
}

void CTransform::Go_Dir(const _fvector& vMoveDir, CNavigation* pNavigationCom, _float fTimeDelta)
{
	_vector vPrePosition = Get_State(STATE::POSITION);

	_vector vPosition = vPrePosition + vMoveDir * m_fSpeedPerSec * fTimeDelta;
	
	_uint iLineIndex = {};
	if (true == pNavigationCom->IsMove(vPosition, Get_State(STATE::LOOK), &iLineIndex))
		Set_State(STATE::POSITION, vPosition);
	else
		Slide(vPosition - vPrePosition, -1.f * XMLoadFloat3(pNavigationCom->Get_Normal(iLineIndex)), pNavigationCom);
}

void CTransform::Go_Force(const _fvector& vForce, _float fTimeDelta)
{
	_vector vPosition = Get_State(STATE::POSITION);

	if (0.1f > XMVectorGetX(XMVector3Length(vForce)))
		return;

	vPosition += vForce * fTimeDelta;
	Set_State(STATE::POSITION, vPosition);
}

void CTransform::Go_Force(const _fvector& vForce, CNavigation* pNavigationCom, _float fTimeDelta)
{
	_vector vPrePosition = Get_State(STATE::POSITION);

	_vector vPosition = vPrePosition + vForce * fTimeDelta;

	_uint iLineIndex = {};
	if (true == pNavigationCom->IsMove(vPosition, Get_State(STATE::LOOK), &iLineIndex))
		Set_State(STATE::POSITION, vPosition);
	else
		Slide(vPosition - vPrePosition, -1.f * XMLoadFloat3(pNavigationCom->Get_Normal(iLineIndex)), pNavigationCom);
}

void CTransform::Slide(const _fvector& vMove, const _fvector& vNormal, CNavigation* pNavigationCom)
{
	_vector vPosition = Get_State(STATE::POSITION);

	_float fDot = XMVector3Dot(vMove, vNormal).m128_f32[0];

	_vector vSlide = vMove - fDot * vNormal;

	if (0.f < fDot)
		vPosition += vMove;
	else
		vPosition += vSlide;

	_uint iLineIndex = {};
	if (true == pNavigationCom->IsMove(vPosition, Get_State(STATE::LOOK), &iLineIndex))
		Set_State(STATE::POSITION, vPosition);
}

void CTransform::Rotation(const _fvector& vAxis, _float fRadian)
{
	_vector vRight = XMVectorSet(1.f, 0.f, 0.f, 0.f);
	_vector vUp = XMVectorSet(0.f, 1.f, 0.f, 0.f);
	_vector vLook = XMVectorSet(0.f, 0.f, 1.f, 0.f);

	_matrix RotationMatrix = XMMatrixRotationAxis(vAxis, fRadian);

	_float3 vScale = Get_Scaled();

	Set_State(STATE::RIGHT, XMVector3TransformNormal(vRight, RotationMatrix) * vScale.x);
	Set_State(STATE::UP, XMVector3TransformNormal(vUp, RotationMatrix) * vScale.y);
	Set_State(STATE::LOOK, XMVector3TransformNormal(vLook, RotationMatrix) * vScale.z);
}

void CTransform::Turn(const _fvector& vAxis, _float fTimeDelta)
{
	_vector vRight = XMVector3Normalize(Get_State(STATE::RIGHT));
	_vector vUp = XMVector3Normalize(Get_State(STATE::UP));
	_vector vLook = XMVector3Normalize(Get_State(STATE::LOOK));

	_matrix RotationMatrix = XMMatrixRotationAxis(vAxis, m_fRotationPerSec * fTimeDelta);

	_float3 vScale = Get_Scaled();

	Set_State(STATE::RIGHT, XMVector3TransformNormal(vRight, RotationMatrix) * vScale.x);
	Set_State(STATE::UP, XMVector3TransformNormal(vUp, RotationMatrix) * vScale.y);
	Set_State(STATE::LOOK, XMVector3TransformNormal(vLook, RotationMatrix) * vScale.z);
}

void CTransform::Turn_Dir(const _fvector& vDir, _float fTimeDelta)
{
	_vector vLook = Get_State(STATE::LOOK);
	vLook = XMVector4Normalize(vLook);

	_float fDot = XMVector3Dot(vLook, vDir).m128_f32[0];

	if (fDot > 0.99f)
		return;

	if (0 > XMVector3Cross(vLook, vDir).m128_f32[1]) // Left
		Turn(XMVectorSet(0.f, 1.f, 0.f, 0.f), -fTimeDelta);
	else
		Turn(XMVectorSet(0.f, 1.f, 0.f, 0.f), fTimeDelta);
}

void CTransform::Turn_Quaternion(const _fvector& vQuaternion)
{
	_vector vRight = XMVector3Normalize(Get_State(STATE::RIGHT));
	_vector vUp = XMVector3Normalize(Get_State(STATE::UP));
	_vector vLook = XMVector3Normalize(Get_State(STATE::LOOK));

	_matrix RotationMatrix = XMMatrixRotationQuaternion(vQuaternion);

	_float3 vScale = Get_Scaled();

	Set_State(STATE::RIGHT, XMVector3TransformNormal(vRight, RotationMatrix) * vScale.x);
	Set_State(STATE::UP, XMVector3TransformNormal(vUp, RotationMatrix) * vScale.y);
	Set_State(STATE::LOOK, XMVector3TransformNormal(vLook, RotationMatrix) * vScale.z);
}

void CTransform::Turn_Quaternion(const _float3& vRadian, _float fTimeDelta)
{
	_vector vRight = XMVector3Normalize(Get_State(STATE::RIGHT));
	_vector vUp = XMVector3Normalize(Get_State(STATE::UP));
	_vector vLook = XMVector3Normalize(Get_State(STATE::LOOK));

	_matrix RotationMatrix = XMMatrixRotationQuaternion(XMQuaternionRotationRollPitchYaw(vRadian.x * fTimeDelta, vRadian.y * fTimeDelta, vRadian.z * fTimeDelta));

	_float3 vScale = Get_Scaled();

	Set_State(STATE::RIGHT, XMVector3TransformNormal(vRight, RotationMatrix) * vScale.x);
	Set_State(STATE::UP, XMVector3TransformNormal(vUp, RotationMatrix) * vScale.y);
	Set_State(STATE::LOOK, XMVector3TransformNormal(vLook, RotationMatrix) * vScale.z);
}

void CTransform::Rotation_Quaternion(const _float3& vRadian)
{
	_vector vRight = XMVectorSet(1.f, 0.f, 0.f, 0.f);
	_vector vUp = XMVectorSet(0.f, 1.f, 0.f, 0.f);
	_vector vLook = XMVectorSet(0.f, 0.f, 1.f, 0.f);

	//_matrix RotationMatrix = XMMatrixRotationQuaternion(vQuaternion);
	_matrix RotationMatrix = XMMatrixRotationQuaternion(XMQuaternionRotationRollPitchYaw(vRadian.x, vRadian.y, vRadian.z));
	
	_float3 vScale = Get_Scaled();

	Set_State(STATE::RIGHT, XMVector3TransformNormal(vRight, RotationMatrix) * vScale.x);
	Set_State(STATE::UP, XMVector3TransformNormal(vUp, RotationMatrix) * vScale.y);
	Set_State(STATE::LOOK, XMVector3TransformNormal(vLook, RotationMatrix) * vScale.z);

}

void CTransform::Rotation_Quaternion(const _fvector& vQuaternion)
{
	_vector vRight = XMVectorSet(1.f, 0.f, 0.f, 0.f);
	_vector vUp = XMVectorSet(0.f, 1.f, 0.f, 0.f);
	_vector vLook = XMVectorSet(0.f, 0.f, 1.f, 0.f);

	_matrix RotationMatrix = XMMatrixRotationQuaternion(vQuaternion);

	_float3 vScale = Get_Scaled();

	Set_State(STATE::RIGHT, XMVector3TransformNormal(vRight, RotationMatrix) * vScale.x);
	Set_State(STATE::UP, XMVector3TransformNormal(vUp, RotationMatrix) * vScale.y);
	Set_State(STATE::LOOK, XMVector3TransformNormal(vLook, RotationMatrix) * vScale.z);
}

void CTransform::LookAt(const _fvector& vAt)
{
	_vector vLook = vAt - Get_State(STATE::POSITION);
	_vector vRight = XMVector3Cross(XMVectorSet(0.f, 1.f, 0.f, 0.f), vLook);
	_vector vUp = XMVector3Cross(vLook, vRight);

	_float3 vScale = Get_Scaled();

	Set_State(STATE::RIGHT, XMVector3Normalize(vRight) * vScale.x);
	Set_State(STATE::UP, XMVector3Normalize(vUp) * vScale.y);
	Set_State(STATE::LOOK, XMVector3Normalize(vLook) * vScale.z);
}

void CTransform::LookAt_KeepUp(const _fvector& vAt)
{
	_vector vLook = vAt - Get_State(STATE::POSITION);
	_vector vRight = XMVector3Cross(Get_State(STATE::UP), vLook);
	_vector vUp = XMVector3Cross(vLook, vRight);

	_float3 vScale = Get_Scaled();

	Set_State(STATE::RIGHT, XMVector3Normalize(vRight) * vScale.x);
	Set_State(STATE::UP, XMVector3Normalize(vUp) * vScale.y);
	Set_State(STATE::LOOK, XMVector3Normalize(vLook) * vScale.z);
}

void CTransform::LookDir(const _fvector& vDir)
{
	_vector vLook = vDir;
	_vector vRight = XMVector3Cross(XMVectorSet(0.f, 1.f, 0.f, 0.f), vLook);
	_vector vUp = XMVector3Cross(vLook, vRight);

	_float3 vScale = Get_Scaled();

	Set_State(STATE::RIGHT, XMVector3Normalize(vRight) * vScale.x);
	Set_State(STATE::UP, XMVector3Normalize(vUp) * vScale.y);
	Set_State(STATE::LOOK, XMVector3Normalize(vLook) * vScale.z);
}

void CTransform::LookLerp(const _fvector& vDir, _float fTimeDelta, _float fRate)
{
	if (XMVectorGetX(XMVector3LengthSq(vDir)) < 1e-8f)
		return;

	_vector vLook = XMVector3Normalize(Get_State(STATE::LOOK));
	_vector vTarget = XMVector3Normalize(vDir);

	_float fDot = XMVectorGetX(XMVector3Dot(vLook, vTarget));
	fDot = max(-1.f, min(1.f, fDot));

	_float fAngle = acosf(fDot);
	_float fMaxStep = m_fRotationPerSec * fTimeDelta * fRate;

	if (fAngle <= fMaxStep)
		vLook = vTarget;
	else
	{
		_vector vAxis = XMVector3Cross(vLook, vTarget);
		if (XMVectorGetX(XMVector3LengthSq(vAxis)) < 1e-6f)
			vAxis = XMVectorSet(0.f, 1.f, 0.f, 0.f);

		vLook = XMVector3Rotate(vLook, XMQuaternionRotationAxis(XMVector3Normalize(vAxis), fMaxStep));
	}

	_vector vRight = XMVector3Cross(XMVectorSet(0.f, 1.f, 0.f, 0.f), vLook);
	_vector vUp = XMVector3Cross(vLook, vRight);

	_float3 vScale = Get_Scaled();

	Set_State(STATE::RIGHT, XMVector3Normalize(vRight) * vScale.x);
	Set_State(STATE::UP, XMVector3Normalize(vUp) * vScale.y);
	Set_State(STATE::LOOK, XMVector3Normalize(vLook) * vScale.z);
}

void CTransform::Chase(const _fvector& vTargetPos, _float fTimeDelta, _float fLimit)
{
	_vector vPosition = Get_State(STATE::POSITION);
	_vector vMoveDir = vTargetPos - vPosition;
	_float fDistance = XMVectorGetX(XMVector3Length(vMoveDir));

	if (fDistance >= fLimit)
	{
		vPosition += XMVector3Normalize(vMoveDir) * m_fSpeedPerSec * fTimeDelta;
		Set_State(STATE::POSITION, vPosition);
	}
}

void CTransform::Lerp(const _fmatrix& StartMatrix, const _fmatrix& EndMatrix, _float fRatio)
{
	_vector vStartScale{}, vStartRotation{}, vStartTranslation{};
	_vector vEndScale{}, vEndRotation{}, vEndTranslation{};

	XMMatrixDecompose(&vStartScale, &vStartRotation, &vStartTranslation, StartMatrix);
	XMMatrixDecompose(&vEndScale, &vEndRotation, &vEndTranslation, EndMatrix);

	_vector vLerpScale{}, vLerpRotation{}, vLerpTranslation{};

	vLerpScale = XMVectorLerp(vStartScale, vEndScale, fRatio);
	vLerpRotation = XMQuaternionSlerp(vStartRotation, vEndRotation, fRatio);
	vLerpTranslation = XMVectorSetW(XMVectorLerp(vStartTranslation, vEndTranslation, fRatio), 1.f);

	XMStoreFloat4x4(&m_WorldMatrix, XMMatrixAffineTransformation(vLerpScale, XMVectorSet(0.f, 0.f, 0.f, 0.f), vLerpRotation, vLerpTranslation));
}

CTransform* CTransform::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	return new CTransform(pDevice, pContext);
}

void CTransform::Free()
{
	__super::Free();
}
