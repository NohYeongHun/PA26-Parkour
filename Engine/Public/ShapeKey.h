#pragma once
#include "Base.h"

NS_BEGIN(Engine)
// Mesh에 등록될 Shape Key (Morph Animation을 위한)
class CShapeKey final : public CBase
{
private:
	explicit CShapeKey(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CShapeKey() = default;

public:
	HRESULT Initialize(const _char* pShapeKeyName, _uint iNumVertices, const vector<_float3>& vDeltaPos, const vector<_float3>& vDeltaNormal, const _fmatrix& PreTransformationMatrix);

public:
	const _char* Get_Name() { return m_szName; }
	_uint Get_NumVertices() const { return m_iNumVertices; }
	const vector<_float3>& Get_DeltaPositions() const { return m_DeltaPositions; }
	const vector<_float3>& Get_DeltaNormals() const { return m_DeltaNormals; }

	void Set_GlobalWeightIndex(_uint iIndex);
	_uint Get_GlobalWeightIndex() { return m_iGlobalWeightIndex; }

	// GPU 버퍼 (나중에 Morph GPU 스키닝 할 때 필요)
	ID3D11Buffer* Get_DeltaPositionBuffer() { return m_pDeltaPosBuffer; }
	ID3D11Buffer* Get_DeltaNormalBuffer() { return m_pDeltaNormBuffer; }



private:
	ID3D11Device* m_pDevice = { nullptr };
	ID3D11DeviceContext* m_pContext = { nullptr };

	_char m_szName[MAX_PATH] = {};
	_uint m_iNumVertices = 0;

	vector<_float3> m_DeltaPositions;
	vector<_float3> m_DeltaNormals;  // nullptr 가능 → vector로 관리하면 안전!

	_uint m_iGlobalWeightIndex = {};

	// GPU 버퍼 (옵션: 나중에 필요하면 생성)
	ID3D11Buffer* m_pDeltaPosBuffer = nullptr;
	ID3D11Buffer* m_pDeltaNormBuffer = nullptr;

public:
	static CShapeKey* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const _char* pShapeKeyName
		, _uint iNumVertices, const vector<_float3>& vDeltaPos, const vector<_float3>& vDeltaNormal, const _fmatrix& PreTransformationMatrix);
	virtual void Free() override;
};
NS_END

