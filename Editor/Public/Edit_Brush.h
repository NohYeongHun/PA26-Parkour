#pragma once
#include "GameObject.h"

NS_BEGIN(Engine)
class CShader;
class CVIBuffer_Point;
NS_END

NS_BEGIN(Editor)
class CEdit_Brush final : public CGameObject
{
private:
	explicit CEdit_Brush(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CEdit_Brush(const CEdit_Brush& Prototype);
	virtual ~CEdit_Brush() = default;

public:
	virtual		HRESULT			Initialize_Prototype()override;
	virtual		HRESULT			Initialize_Clone(void* pArg)override;
	virtual		void			Priority_Update(_float fTimeDelta)override;
	virtual		void			Update(_float fTimeDelta)override;
	virtual		void			Late_Update(_float fTimeDelta)override;
	virtual		void			Render()override;

	void About_InstanceInfo();

	void Set_ModelName(const _wstring& pModelName);
private:
	void Bind_Resources();
	void Ready_Components();
	void Foliage();
	_bool IsSelected(const _tchar* szModelname) const
	{
		return szModelname != nullptr && wcslen(szModelname) != 0;
	}
	_bool Check_Duplicate();
	void Generate_WorldMatrices(vector<_float4x4>& TransformMatrices, const vector<_float4>& Points);
	_bool Generate_Instance(vector<_float4x4>& TransformMatrices,_float4 vMousePos);

	_uint GetVaildRandomIndex(const vector<_float4>& Points, _uint iMin, _uint iMax);
	_float4x4 Composite_WorldMatrix(const _float4& RandWorldPos, _float fRotation);

private:
	CShader* m_pShaderCom = { nullptr };
	CVIBuffer_Point* m_pVIBufferCom = { nullptr };
	map<_uint, vector<class CEdit_MapObject_Instance*>> m_SaveInstanceObjects;
	vector<_float4x4> m_TransformMatrices;
	vector<_float4> m_TempPoints;
	_float4 m_vMousePos = {};

	_float m_fRange = {};
	_uint m_iNumInstance = {};
	_float4* m_pPoints = { nullptr };
	_tchar m_szModelName[MAX_PATH] = {};

	_uint m_iMinNum = {};
	_uint m_iMaxNum = {};
	_float m_vMinRotation = {};
	_float m_vMaxRotation = {};

	_uint m_iCurSaveIndex = {};
	_uint m_iPickedSpecipic = {};
	_uint m_iShaderPassIndex = {};

	vector<_uint> m_ShaderPasses;

	union MyFloat4 {
		_vector Vec = XMVectorSet(1.f,1.f,1.f,1.f);
		_float4 float_4;
		_float arr[4];
	};

	MyFloat4 vDiffuseColor = {};
	INSTANCETYPE m_eInstanceType = { INSTANCETYPE::DEFAULT };
public:
	static CEdit_Brush* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) { return nullptr; }
	virtual void					Free() override;
};

NS_END
