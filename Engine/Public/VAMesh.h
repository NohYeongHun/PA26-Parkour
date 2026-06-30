#pragma once
#include "VIBuffer.h"

NS_BEGIN(Engine)

class ENGINE_DLL CVAMesh final : public CVIBuffer
{
private:
	explicit CVAMesh(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CVAMesh(const CVAMesh& Prototype);
	virtual ~CVAMesh() = default;

public:
	_float							Get_MaxFrame(_uint iTextureIndex);

public:
	virtual		HRESULT				Initialize_Prototype(const _tchar* pFilePath, _fmatrix PreTransformMatrix, _uint iNumAnimation);
	virtual		HRESULT				Initialize_Clone(void* pArg) override;

public:
	HRESULT							Bind_VAT(class CShader* pShader, const _char* pConstantName, _uint iTextureIndex);

private:
	class		CHdrTexture*		m_pHdrTexture = { nullptr };	

public:
	static		CVAMesh*			Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const _tchar* pFilePath, _fmatrix PreTransformMatrix, _uint iNumAnimation);
	virtual		CComponent*		Clone(void* pArg) override;
	virtual		void					Free() override;
};

NS_END