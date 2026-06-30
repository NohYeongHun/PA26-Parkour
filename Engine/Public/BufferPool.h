#pragma once
#include "Base.h"
#include"FreeList.h"

NS_BEGIN(Engine)
class CBufferPool final : public CBase
{
public:
	enum MODELLOAD { LOADING, LOADED, NONE, END };

	typedef struct ModelInfo {
		_uint NumIndices;
		_uint Offset;
		_uint Size;
	}MODEL_INFO;

private:
	CBufferPool(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CBufferPool() = default;

public:
	HRESULT						Initialize(_uint iVertexSize, _uint iIndexSize);
	void						Update(_float fTimeDelta);


	HRESULT Request_ModelLoad(const _char* pFilePath, _uint iLODIndex);

	_uint Allocate_Vertex(_uint iVertexSize);
	_uint Allocate_Index(_uint iIndexSize);
	void FreeMemory_Vertex(_uint iVertexOffset, _uint iVertexSize);
	void FreeMemory_Index(_uint iIndexOffSet, _uint iIndexSize);
	HRESULT Bind_BufferPool();
	HRESULT Bind_BufferPool(ID3D11DeviceContext* pDC);
	
	ID3D11Buffer* Get_VertexBuffer() { return m_pVertexBufferPool; }
	ID3D11Buffer* Get_IndexBuffer() { return m_pIndexBufferPool; }

	void		Clear_Resource();
private:
	ID3D11Device* m_pDevice = { nullptr };
	ID3D11DeviceContext* m_pContext = { nullptr };
	class CGameInstance* m_pGameInstance = { nullptr };

	CFreeList* m_pVertexFreeList = { nullptr };
	CFreeList* m_pIndexFreeList = { nullptr };

	ID3D11Buffer* m_pVertexBufferPool = { nullptr };
	ID3D11Buffer* m_pIndexBufferPool = { nullptr };
	
	_uint m_iVertexStride = sizeof(VTXMESH);
	DXGI_FORMAT m_eIndexFormat = DXGI_FORMAT_R32_UINT;
	D3D11_PRIMITIVE_TOPOLOGY m_ePrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

public:
	static CBufferPool* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext,_uint iVertexSize,_uint iIndexSize);
	virtual void				Free() override;
};

NS_END