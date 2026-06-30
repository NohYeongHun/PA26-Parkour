#pragma once
#include "Base.h"
#include"Edit_FireFly.h"
NS_BEGIN(Editor)
class CEdit_FireFly_Manager : public CBase
{
	union MyFloat4 {
		_vector Vec = XMVectorSet(1.f, 1.f, 1.f, 1.f);
		_float4 float_4;
		_float arr[4];
	};
	union MyFloat2 {
		_float2 float_2;
		_float arr[2];
	};
private:
	explicit CEdit_FireFly_Manager(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CEdit_FireFly_Manager() = default;

public:
	HRESULT Initialize();
	void Set_ImGuiOption();
	void Create_Fly();
	void Create_Fly(_uint InstanceNum, _float2 Range, _uint ShaderPassIndex, _float2 Sin, _float2 Cos, _float2 Sin2);
	void Map_Load(CEdit_FireFly::MAP_LOAD& Desc);
private:
	ID3D11Device* m_pDevice = { nullptr };
	ID3D11DeviceContext* m_pContext = { nullptr };
	class CGameInstance* m_pGameInstance = { nullptr };
	map<_uint, class CEdit_FireFly*> m_Fly;
	_uint m_iPickedIndex = {};
	MyFloat4 m_vPickedPos = {};

	_uint m_iTempInstanceNum = {};
	MyFloat2 m_iTempRangePerInstance = {};
	_uint m_iTempShaderPassIndex = {};
	MyFloat2 m_vSinPerInstance = {};
	MyFloat2 m_vCosPerInstance = {};
	MyFloat2 m_vSin2PerInstance = {};


	_bool m_IsRenderGizmo = { false };
	_uint m_iLevel = {};
	class CEdit_FireFly* m_pPickedFly = { nullptr };
public:
	static _uint g_iFlyIndex;
public:
	static CEdit_FireFly_Manager* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual void Free()override;
};

NS_END