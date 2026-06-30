#pragma once
#include "GameObject.h"

NS_BEGIN(Editor)
class CEdit_SlideZone final : public CGameObject
{
	union MyFloat4 {
		_vector Vec = XMVectorSet(1.f, 1.f, 1.f, 1.f);
		_float4 float_4;
		_float arr[4];
	};
public:
	typedef struct tagSlideDesc {
		_bool IsLoad = { false };
		_bool IsStart = { true };
		_uint iLevel;
		_float3 vExtends;
		_float4x4* WorldMatrix = { nullptr };
		_float4* pPath = { nullptr };
		_uint iPathSize = {};
	}SLIDE_DESC;

private:
	CEdit_SlideZone(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CEdit_SlideZone(const CEdit_SlideZone& Prototype);
	virtual ~CEdit_SlideZone() = default;

public:
	virtual		HRESULT		Initialize_Prototype();
	virtual		HRESULT		Initialize_Clone(void* pArg);
	virtual		void			Priority_Update(_float fTimeDelta);
	virtual		void			Update(_float fTimeDelta);
	virtual		void			Late_Update(_float fTimeDelta);
	virtual		void			Render();
	virtual		void			Render_Shadow();

	void Set_ImGuiOption();

	HRESULT Ready_Component(void* pArg = nullptr);

	void Bind_Resources();

	_uint ShaderPassWindow();
	void SaveData(ofstream& File);
	//INSTANCETYPE Get_Type() { return m_eInstanceType; }
	//void Set_Type(INSTANCETYPE eType) { m_eInstanceType = eType; }
private:
	void Ready_Events();

	_float3 m_vExtends = {};
	vector<_float4> m_vSlidePath;
	_bool m_IsStart = { true };
	_bool m_IsTestRender = { false };
	_uint m_iPickedIndex = {};
	MyFloat4 m_PickPos = {};
private:
	class CMap_Interface* m_pMapInterface = { nullptr };
	class CRigidbody* m_pRigidbodyCom = { nullptr };
public:
	static CEdit_SlideZone* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg)override;
	virtual void Free()override;

};

NS_END