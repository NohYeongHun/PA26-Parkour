#pragma once
#include "Base.h"

NS_BEGIN(Engine)

class ENGINE_DLL CCell final : public CBase
{
private:
	explicit CCell(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CCell() = default;

public:
	_fvector				Get_Point(POINTS ePoint) { return XMLoadFloat3(&m_Points[ENUM_CLASS(ePoint)]); }
	_uint					Get_Type() { return m_iType; }
	const _float3*		Get_Normal(_uint iLineIndex) { return &m_vNormals[iLineIndex]; }

public:
	HRESULT				Initialize(const CELL& pCell, _int iIndex);
	_bool					IsIn(const _fvector& vLocalPosition, const _fvector& vLookDir, _int* pNeighborIndex, _uint* pLineIndex);

	_bool					Compare_Points(const _fvector& vSrcPoint, const _fvector& vDstPoint);
	void					SetUp_Neighbor(LINE eLine, CCell* pCell);
	_float					Compute_Distance(const _fvector& vLocalPos, _float3* vOut);
	void					Compute_GoDir(const _fvector& vLocalDir, _float3* vOut);
	_float					Compute_Height(const _fvector& vLocalPos);

#ifdef _DEBUG
	const _float3*		Get_Points() { return m_Points; }
	void					Set_Type(_uint iType) { m_iType = iType; }
	HRESULT				Save_Cell(ofstream& OutPut);
	HRESULT				Render(class CShader* pShader, _bool isWire = true);
#endif

private:
	ID3D11Device*				m_pDevice = { nullptr };
	ID3D11DeviceContext*	m_pContext = { nullptr };

	_int							m_iIndex = {};
	_uint							m_iType = {};


	_float3						m_vPlaneNormal = {};
	_float							m_fMinY = {};				// Point??Y媛?以?理쒖냼 媛?
	_float							m_fMaxY = {};				// Point??Y媛?以?理쒖냼 媛?

	LINE							m_eHighLine = { LINE::END };	// ?쒖씪 ?믪? ?좊텇
	LINE							m_eLowLine = { LINE::END };		// ?쒖씪 ??? ?좊텇

	_float3						m_Points[ENUM_CLASS(POINTS::END)] = {};
	_float3						m_vNormals[ENUM_CLASS(LINE::END)] = {};
	_int							m_iNeighbors[ENUM_CLASS(LINE::END)] = { -1, -1, -1};

#ifdef _DEBUG
private:
	class CVIBuffer_Cell*		m_pVIBuffer = { nullptr };
#endif

private:
	void							Compute_Line();
	void							Compute_Slide(_uint iPointIndex, _float3* pSlide);

public:
	static		CCell*		Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const CELL& pCell, _int iIndex);
	virtual		void		Free() override;
};

NS_END