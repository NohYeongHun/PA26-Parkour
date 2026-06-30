#pragma once
#include "Base.h"

NS_BEGIN(Engine)

class CGameInstance;

class CFrustrum final : public CBase
{
private:
	explicit CFrustrum();
	virtual ~CFrustrum() = default;

public:
	const _float4*		Get_Frustrum_WorldPoints() const { return m_vWorldPoints; }


public:
	HRESULT				Initialize();
	void				Update();

	_bool				IsIn_WorldSpace(_fvector vWorldPosition, _float fRange);						// Volume ?쇰줈 蹂寃?
	_bool				IsIn_LocalSpace(_fmatrix WorldMatrix, _fvector vLocalPosition, _float fRange);	// Volume ?쇰줈 蹂寃?
	_bool				IsIn_WorldSpace( const BoundingBox* pBoundingBox );

private:
	CGameInstance*		m_pGameInstance = { nullptr };
	_float4				m_vPoints[8] = {};

	_float4				m_vLocalPoints[8] = {};
	_float4				m_vLocalPlanes[6] = {};

	_float4				m_vWorldPoints[8] = {};
	_float4				m_vWorldPlanes[6] = {};
	
private:
	void		Make_Planes(const _float4* pPoints, _float4* pPlanes);
	void		Update_LocalPlanes(_fmatrix WorldMatrix);

public:
	static		CFrustrum*		Create();
	virtual		void			Free() override;
};

NS_END