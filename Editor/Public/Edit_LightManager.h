#pragma once
#include "Base.h"
NS_BEGIN(Editor)
class CEdit_LightManager final : public CBase
{
	union MyFloat4 {
		_vector Vec = XMVectorSet(1.f, 1.f, 1.f, 1.f);
		_float4 float_4;
		_float arr[4];
	};
private:
	explicit CEdit_LightManager();
	virtual ~CEdit_LightManager() = default;

public:
	HRESULT Initialize();
	void Set_ImGuiOption();
	void Create_Light();
	void Map_Load(LIGHT_DESC& Desc);
private:
	map<_uint, class CEdit_LightObject*> m_Lights;
	class CEdit_LightObject* m_pPickedLight = { nullptr };

	LIGHT_DESC m_LightDesc = {};
	MyFloat4 m_vLightDiffuse = {};
	MyFloat4 m_vLightAmbient = {};
	MyFloat4 m_vLightSpec = {};

	class CGameInstance* m_pGameInstance = { nullptr };

public:
	static CEdit_LightManager* Create();
	virtual void Free()override;
};

NS_END