#pragma once
#include "Component.h"

NS_BEGIN(Client)
// 이름↔월드좌표 매핑 + 엔진 워프 위임. EnvQuery 목표점을 상태가 이름표 붙여 등록한다.
class CMotionWarpingComponent final : public Engine::CComponent
{
public:
	typedef struct tagMotionWarpDesc : Engine::CComponent::COMPONENT_DESC
	{
	}MOTION_WARP_DESC;

protected:
	explicit CMotionWarpingComponent(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CMotionWarpingComponent(const CMotionWarpingComponent& Prototype);
	virtual ~CMotionWarpingComponent() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize_Clone(void* pArg) override;

public:
	void Set_WarpTarget(const _string& strName, const _float3& vPos);
	void Set_WarpTarget(const _string& strName, const _float3& vPos, const _float4& qRot);
	void Clear_WarpTargets();
	void Abort_Warp();
	void On_WarpNotify(const _string& strName, _bool isStart,
	                   _float fWindowEndTrackPos, _bool bTrans, _bool bRot);

#ifdef _DEBUG
	void Update_DebugTrail();
	void Reset_DebugTrail();
#endif

private:
	map<_string, WARP_TARGET> m_WarpTargets;
	class CModel* m_pOwnerModelCom = { nullptr };

#ifdef _DEBUG
	void Draw_DebugTrail();                                  
	class CTransform* m_pOwnerTransformCom = { nullptr };   
	vector<_float3>   m_DebugTrail;                        
#endif

public:
	static CMotionWarpingComponent* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual Engine::CComponent* Clone(void* pArg) override;
	virtual void Free() override;
};
NS_END
