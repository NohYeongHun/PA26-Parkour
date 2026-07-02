#pragma once
#include "Loader.h"

NS_BEGIN(Client)

class CLoader_Test final : public CLoader
{
private:
	explicit CLoader_Test(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CLoader_Test() = default;

public:
	virtual		HRESULT		Initialize() override;

private:
	HRESULT				Load_Texture();
	HRESULT				Load_Map();
	HRESULT				Load_Shader();
	HRESULT				Load_Object();
	HRESULT				Load_Player();

private:
	LEVEL m_eCurLevel = { LEVEL::TEST };

public:
	static		CLoader_Test*		Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual		void					Free() override;
};

NS_END