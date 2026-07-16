#pragma once
#include "Component.h"

NS_BEGIN(Client)
class CMeshAlignComponent final : public Engine::CComponent
{
protected:
	explicit CMeshAlignComponent(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CMeshAlignComponent(const CMeshAlignComponent& Prototype);
	virtual ~CMeshAlignComponent() = default;

public:
	virtual HRESULT		Initialize_Prototype() override;
	virtual HRESULT		Initialize_Clone(void* pArg) override;

public:
	void Request_Pose(_fvector qRot, const _float3& vLocalOffset, _float fBlendTime);
	void Clear_Pose(_float fBlendTime);
	void Update(_float fTimeDelta);

	_matrix Get_LocalMatrix() const;
	_bool   IsBlending() const { return m_fBlendElapsed < m_fBlendDuration; }

	void Set_SteerYaw(_float fTargetDeg) { m_fSteerYawTarget = fTargetDeg; }

private:
	_float4 m_qStartRot   = { 0.f, 0.f, 0.f, 1.f };
	_float4 m_qTargetRot  = { 0.f, 0.f, 0.f, 1.f };
	_float4 m_qCurrentRot = { 0.f, 0.f, 0.f, 1.f };

	_float3 m_vStartOffset   = {};
	_float3 m_vTargetOffset  = {};
	_float3 m_vCurrentOffset = {};

	_float m_fBlendDuration = 0.f;
	_float m_fBlendElapsed  = 0.f;

	static constexpr _float STEER_SMOOTH_TIME = 0.15f;

	_float m_fSteerYawTarget  = 0.f;
	_float m_fSteerYawCurrent = 0.f;

public:
	static	CMeshAlignComponent* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual Engine::CComponent* Clone(void* pArg) override;
	virtual void Free() override;
};
NS_END
