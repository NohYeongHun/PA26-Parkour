#pragma once

#include "VIBuffer_Instance.h"

NS_BEGIN(Engine)

class ENGINE_DLL CVIBuffer_Point_Instance final : public CVIBuffer_Instance
{
public:
	typedef struct tagPointInstanceDesc : public CVIBuffer_Instance::INSTANCE_DESC
	{
		_float3		vPivot;
		_float2		vSpeed;
		_float2		vLifeTime;
		_bool		IsLoop;

		//스폰 설정
		_bool IsSpawnBox = true;
		_bool IsSpawnRing = false;
		_bool IsRingAngle = false;
		_float fRmin = 0.f;
		_float fRmax = 0.f;
		_float2 fDegreeAngle = { 0.f, 360.f };

		//스트레치 빌보드 설정값 
		_bool		IsStretch = false;
		_float		fStretchWeight = 1.f;
		_float2		fStretchRange = { 1.f, 1.f };

		//스프라이트 이미지 설정값
		_bool		IsSprite = false;
		_float		fSpriteWeight = 1.f;
		_float		fDefualtSpeed = 2.5f;			//스프라이트 이미지가 바뀌는 속도 기본값.

		_bool		IsDelay = false;
		_float2		fDelay = { 0.f, 0.f };

		//가중치
		_float		fSpreadWeight = 0;
		_float		fDropWeight = 0;
		_float		fRotationWeight = 0;
		_float		fGravity = 9.8f;
	}POINT_INSTANCE_DESC;

private:
	CVIBuffer_Point_Instance(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CVIBuffer_Point_Instance(const CVIBuffer_Point_Instance& Prototype);
	virtual ~CVIBuffer_Point_Instance() = default;

public:
	virtual HRESULT Initialize_Prototype(const INSTANCE_DESC* pDesc);
	virtual HRESULT Initialize_Clone(void* pArg) override;
	virtual HRESULT Bind_Resources() override;
	virtual HRESULT Render() override;

	void Bind_CS_Pivot(_vector vRight, _vector vUp, _vector vLook); // 바꿀 일 있을 경우 여기에 값 추가해서 바꿔줘야함.
	void Bind_CS_Speed(_float fTimeDelta, PARTICLE_SPEEDCB* SpeedDesc = nullptr);
	void Bind_CSResources(class CComputeShader* pCShader);
	void Reset_UAV(class CComputeShader* pCShader);
	void Reset_CS_Option();


//public:
//	void Spread(_float fTimeDelta);
//	void Drop(_float fTimeDelta);
//	void Rotation(_float fTimeDelta);

private:
	_float3					m_vPivot = {};		
	_float*					m_pSpeeds = {};
	_bool					m_isLoop = {};

	ID3D11Buffer*		m_pOptionCBBuffer = {};
	ID3D11Buffer*		m_pSpeedCBBuffer = {};

	ID3D11Buffer*		m_pSRVBuffer = {};
	ID3D11Buffer*       m_pUABuffer = {};
	ID3D11Buffer*		m_pDefaultUAVBufer = {};

	ID3D11Buffer*		m_pDebugBuffer = {};

	ID3D11ShaderResourceView*	m_pSRV = {};
	ID3D11UnorderedAccessView*	m_pUAV = {};

public:
	static CVIBuffer_Point_Instance* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const INSTANCE_DESC* pDesc);
	virtual CComponent* Clone(void* pArg) override;
	virtual void Free() override;
};

NS_END