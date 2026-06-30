#pragma once
#include "GameObject.h"

NS_BEGIN(Engine)

class ENGINE_DLL CStaticObject abstract : public CGameObject
{
protected:
	explicit CStaticObject(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CStaticObject(const CStaticObject& Prototype);
	virtual ~CStaticObject() = default;

public:
	void						Set_LOD(_uint iLOD = 0) { m_iLODIndex = iLOD; }
	_uint						Get_LOD() { return min(m_iNumLOD - 1, m_iLODIndex); }
	_float						Compute_Distance(const _fvector& vCamPos);

public:
	virtual		HRESULT			Initialize_Prototype() { return S_OK; };
	virtual		HRESULT			Initialize_Clone(void* pArg) { return __super::Initialize_Clone(pArg); };
	virtual		void			Priority_Update(_float fTimeDelta) {};
	virtual		void			Update(_float fTimeDelta) {};
	virtual		void			Late_Update(_float fTimeDelta) {};
	virtual		void			Render(ID3D11DeviceContext* pDeferredContext, _uint iIndex) {};
	virtual		void			Render_Shadow() {};
	virtual		void			Render_EnvMap(_float4 vCenter, _float4x4 ViewMatrix, _float4x4 ProjMatrix) {};

	virtual		BoundingBox*	Get_BoundingBox() { return m_pBoundingBox; }
	_uint						IsMaxLOD(_uint iLODIndex);
	virtual		void			Set_RenderTime(_uint iLODIndex, _float m_fTotalPlayTime) {};
protected:
	// LOD 개수
	_uint							m_iNumLOD = {};
	// LOD Index
	_uint							m_iLODIndex = {};
	// Render true/false
	BoundingBox*				m_pBoundingBox = { nullptr };
	vector<_uint>				m_Sectors;

protected:
	void							Sync_Sectors();

public:
	virtual CGameObject*		Clone(void* pArg) = 0;
	virtual void					Free() override;
};

NS_END