#pragma once
#include "Component.h"

NS_BEGIN(Engine)

class ENGINE_DLL CNavigation final : public CComponent
{
public:
	typedef struct tagNavigationDesc {
		_int		iCurrentCellIndex = { -1 };
	}NAVIGATION_DESC;
private:
	explicit CNavigation(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CNavigation(const CNavigation& Prototype);
	virtual ~CNavigation() = default;

public:
	_uint							Get_CurrentCellType();

	_int							Get_CurrentCellIndex() { return m_iCurrentCellIndex; }
	void							Set_CurrentCellIndex(_int iCellIndex) { m_iCurrentCellIndex = iCellIndex; }

	const _float3*				Get_Normal(_uint iLineIndex);

public:
	virtual		HRESULT			Initialize_Prototype(const _tchar* pFilePath);
	virtual		HRESULT			Initialize_Clone(void* pArg);

	void							Update(const _fmatrix& Matrix);

	_bool							IsMove(const _fvector& vWorldPos, const _fvector& vLookDir, _uint* iLineIndex);
	_bool							Compute_OnCell(const _fvector& vWorldPos, _float4* pOut);
	void							Compute_OnCell_Dir(const _fvector& vDir, _float3* pOut);

#ifdef _DEBUG
	HRESULT						Render();
#endif

private:
	_int							m_iCurrentCellIndex = { -1 };
	vector<class CCell*>			m_Cells;

	static _float4x4				m_WorldMatrix;

#ifdef _DEBUG
	class CShader*				m_pShader = { nullptr };
#endif

private:
	HRESULT						Load_File(const _tchar* pFilePath);
	HRESULT						SetUp_Neighbors();

public:
	static		CNavigation*	Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const _tchar* pFilePath);
	virtual		CComponent*	Clone(void* pArg);
	virtual		void				Free() override;
};

NS_END