#pragma once
#include "Component.h"

NS_BEGIN(Client)
// IK 드라이버: 매 프레임 전달되는 IK_REQUEST 목록과 내부 활성 맵을 비교해서
// 새 목표는 블렌드 인, 사라진 목표는 블렌드 아웃한다.
class CIKDriver final : public CComponent
{
public:
	typedef struct tagIKDriverDesc : Engine::CComponent::COMPONENT_DESC
	{
	}IK_DRIVER_DESC;

protected:
	explicit CIKDriver(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CIKDriver(const CIKDriver& Prototype);
	virtual ~CIKDriver() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize_Clone(void* pArg) override;

public:
	void Execute(const vector<IK_REQUEST>& Requests, _float fTimeDelta);

private:
	typedef struct tagActiveIKEntry
	{
		IK_REQUEST	Req;
		_bool		isSeen = { false };		// 이번 프레임 요청 목록에 있었는지 확인
		_bool		isLatched = { false };	// ANCHOR + isFix 여부.
		_float3		vLatchedPos{};
		_float3		vLatchedNormal{};
	}ACTIVE_IK_ENTRY;

	void Apply_Requests(const vector<IK_REQUEST>& Requests);
	void Reap_Missing();
	void Resolve_Target(const _string& strGoal, ACTIVE_IK_ENTRY& Entry);

private:
	class CIKComponent* m_pIKCom = { nullptr };
	class CEnvironmentQueryComponent* m_pEnvQueryCom = { nullptr };
	class CTransform* m_pTransformCom = { nullptr };
	class CMeshAlignComponent* m_pMeshAlignCom = { nullptr };

private:
	unordered_map<_string, ACTIVE_IK_ENTRY> m_ActiveMap{};

public:
	static CIKDriver* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual Engine::CComponent* Clone(void* pArg) override;
	virtual void Free() override;
};
NS_END
