#pragma once
#include"Base.h"

NS_BEGIN(Editor)
class CEdit_MapEffectCollector final: public CBase
{
	union MyFloat3 {
		_float3 float_3;
		_float arr[3];
	};
	enum EFFECTTAG { HEARTH_FIRE, HEARTH_FIRE2, CAMPFIRE, STATUE,END };
public:
	typedef struct tagEternalEffect {
		EFFECTTAG EffectTag = {};
		_float4 vPos = {};
	}ETERNAL_EFFECT;
private:
	CEdit_MapEffectCollector();
	virtual ~CEdit_MapEffectCollector() = default;

	HRESULT Initialize();
public:
	void Set_ImGuiOption();
	void Map_Load(ETERNAL_EFFECT EffectDesc);

private:
	//필요한 거 거 : 좌표, 트리거 이벤트 번호?(안넣었을 때도 가정)
	//Collaps에 연기는 Collaps에 내가 직접 넣자. Destruction에도 그냥 내가 직접 뼈 위치에 호출하면 될듯?
	//얘는 영구 이펙트들 하면 될 거 같은데. 영구이펙트들 폴더 분리 요청?
	map<_uint, ETERNAL_EFFECT> m_EffectInfo;
	_uint m_iEffectIndex = {};
	class CGameInstance* m_pGameInstance = { nullptr };

	MyFloat3 m_vTempPos = {};
	EFFECTTAG m_eTempTag = { EFFECTTAG::HEARTH_FIRE };

	_bool m_IsRenderGizmo = { false };
public:
	static CEdit_MapEffectCollector* Create();
	virtual void Free()override;
};

NS_END