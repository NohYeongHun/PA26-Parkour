#pragma once
#include "Component.h"

NS_BEGIN(Engine)

class ENGINE_DLL CTransform final : public CComponent
{
public:
	typedef struct tagTransformDesc {
		_float		fSpeedPerSec = { 0.f };
		_float		fRotationPerSec = { 0.f };
	}TRANSFORM_DESC;

public:
	_vector		Get_State(STATE eState) const {
		return XMLoadFloat4(reinterpret_cast<const _float4*>(&m_WorldMatrix.m[ENUM_CLASS(eState)]));
	}

	_float3		Get_Scaled() const {
		return _float3(
			XMVectorGetX(XMVector3Length(Get_State(STATE::RIGHT))),
			XMVectorGetX(XMVector3Length(Get_State(STATE::UP))),
			XMVectorGetX(XMVector3Length(Get_State(STATE::LOOK)))
		);
	}

	void			Set_State(STATE eState, _fvector vState) {
		XMStoreFloat4(reinterpret_cast<_float4*>(&m_WorldMatrix.m[ENUM_CLASS(eState)]), vState);
	}

	_matrix	Get_WorldMatrix() { return XMLoadFloat4x4(&m_WorldMatrix); }
	_matrix	Get_WorldMatrix_Inv() { return XMMatrixInverse(nullptr, XMLoadFloat4x4(&m_WorldMatrix)); }
	const _float4x4* Get_WorldMatrixPtr() { return &m_WorldMatrix; }
	void		Set_WorldMatrix(const _fmatrix& Matrix) { XMStoreFloat4x4(&m_WorldMatrix, Matrix); }

	_vector	Get_Quaternion() const {
		_vector vScale{}, vQuat{}, vTrans{};
		XMMatrixDecompose(&vScale, &vQuat, &vTrans, XMLoadFloat4x4(&m_WorldMatrix));
		return vQuat;
	}

	void			Change_Speed(_float fSpeed) { m_fSpeedPerSec = fSpeed; }
#ifdef _DEBUG
#endif

private:
	explicit CTransform(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CTransform(const CTransform& Prototype) = default;
	virtual ~CTransform() = default;

public:
	HRESULT		Initialize_Clone(void* pArg);

	HRESULT		Bind_Matrix(class CShader* pShader, const _char* ConstantName);
	HRESULT		Bind_Matrix(class CDeferredShader* pShader, const _char* ConstantName, ID3DX11Effect* pEffect);

public:
	void			Scale(_float3 vScale);		// vScale 조정
	void			Scaling(_float3 vScale);		// vScale 배율 조정

	void			Go_Straight(_float fTimeDelta);
	void			Go_Backward(_float fTimeDelta);
	void			Go_Left(_float fTimeDelta);
	void			Go_Right(_float fTimeDelta);

	void			Go_Dir(const _fvector& vMoveDir, _float fTimeDelta);
	void			Go_Dir(const _fvector& vMoveDir, class CNavigation* pNavigationCom, _float fTimeDelta);
	void			Go_Force(const _fvector& vForce, _float fTimeDelta);
	void			Go_Force(const _fvector& vForce, class CNavigation* pNavigationCom, _float fTimeDelta);
	void			Slide(const _fvector& vMove, const _fvector& vNormal, class CNavigation* pNavigationCom);

	void			Rotation(const _fvector& vAxis, _float fRadian);
	void			Turn(const _fvector& vAxis, _float fTimeDelta);
	void			Turn_Dir(const _fvector& vDir, _float fTimeDelta);
	void			Turn_Quaternion(const _fvector& vQuaternion);
	void			Turn_Quaternion(const _float3& vRadian, _float fTimeDelta);
	void			Rotation_Quaternion(const _float3& vRadian);
	void			Rotation_Quaternion(const _fvector& vQuaternion);

	void			LookAt(const _fvector& vAt);
	void			LookAt_KeepUp(const _fvector& vAt);
	void			LookDir(const _fvector& vDir);
	// Dir 방향으로 보간하여 돈다.
	void			LookLerp(const _fvector& vDir, _float fTimeDelta, _float fRate = 1.f);
	void			Chase(const _fvector& vTargetPos, _float fTimeDelta, _float fLimit);

	// 행렬 사이 보간
	void			Lerp(const _fmatrix& StartMatrix, const _fmatrix& EndMatrix, _float fRatio);

	// 이전 위치 저장.
	void			Save_PreviousPosition() { XMStoreFloat4(&m_vPreviousPosition,Get_State(STATE::POSITION)); }
	// 이전 위치와 현재 위치 차이 반환
	_fvector		Get_Velocity() { return Get_State(STATE::POSITION) - XMLoadFloat4(&m_vPreviousPosition); }

private:
	_float4x4		m_WorldMatrix = {};

	_float			m_fSpeedPerSec = {};
	_float			m_fRotationPerSec = {};
	_float4		m_vPreviousPosition = {};	

public:
	static		CTransform*		Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual		CComponent*		Clone(void* pArg) { return nullptr; }
	virtual		void					Free() override;
};

NS_END