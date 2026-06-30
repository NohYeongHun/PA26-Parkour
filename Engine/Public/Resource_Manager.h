#pragma once
#include "Base.h"

NS_BEGIN(Engine)

class CResource_Manager final : public CBase
{
private:
	explicit CResource_Manager(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CResource_Manager() = default;

public:
	void										Load_Resource(const _char* pFolderPath);
	ID3D11ShaderResourceView*		Get_Resource(const _string& strResourceTag);
	void										Clear_Resource();

private:
	ID3D11Device*							m_pDevice = { nullptr };
	ID3D11DeviceContext*				m_pContext = { nullptr };

	map<const _string, ID3D11ShaderResourceView*> m_Resources;

private:
	ID3D11ShaderResourceView*		Find_Resource(const _string& strResourceTag);

public:
	static		CResource_Manager*		Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual		void							Free() override;
};

NS_END