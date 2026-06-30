#pragma once

#include "Engine_Define.h"

#include "Jolt/Renderer/DebugRendererSimple.h"

NS_BEGIN(Engine)

class CDebugRender final : public DebugRendererSimple
{
public:
	explicit CDebugRender(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CDebugRender();

public:
	void					Begin();
	void					End();
	virtual		void		DrawLine(RVec3Arg inFrom, RVec3Arg inTo, ColorArg inColor) override;
	virtual		void		DrawText3D(RVec3Arg inPosition, const string_view& inString, ColorArg inColor, float inHeight) override;

private:
	class CGameInstance*		m_pGameInstance = { nullptr };
	ID3D11Device*				m_pDevice = { nullptr };
	ID3D11DeviceContext*		m_pContext = { nullptr };

	PrimitiveBatch<VertexPositionColor>*		m_pBatch = { nullptr };
	BasicEffect*										m_pEffect = { nullptr };
	ID3D11InputLayout*							m_pInputLayout = { nullptr };
};

NS_END