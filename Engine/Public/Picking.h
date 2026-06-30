#pragma once
#include "Base.h"

NS_BEGIN(Engine)

class CPicking final : public CBase
{
private:
	explicit CPicking(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CPicking() = default;

public:
	POINT						Get_MousePoint() { return m_ptMouse; }

public:
	HRESULT			Initialize(HWND hWnd, _uint iWinSizeX, _uint iWinSizeY);
	void				Update();

	_bool				isPicked(_float3* pOut);
	_bool				GetCenterPos(_float3* pOut);
	_bool				Get_Points(_float fRange, vector<_float4>& pOut, _float4* pOutMousePos);
private:
	class CGameInstance*	m_pGameInstance = { nullptr };
	ID3D11Device*				m_pDevice = { nullptr };
	ID3D11DeviceContext*	m_pContext = { nullptr };
	ID3D11Texture2D*			m_pTexture2D = { nullptr };

	HWND						m_hWnd = { nullptr };
	POINT						m_ptMouse = {};
	_uint							m_iWinSizeX{}, m_iWinSizeY{};
	_float4* m_pPoints = { nullptr };
	vector<_float4> m_WorldPoints;
public:
	static CPicking*		Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, HWND hWnd, _uint iWinSizeX, _uint iWinSizeY);
	virtual void			Free() override;
};

NS_END