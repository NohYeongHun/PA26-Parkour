#include "EnginePch.h"
#include "DebugRender.h"

#include "DebugDraw.h"
#include "GameInstance.h"

CDebugRender::CDebugRender(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: m_pDevice { pDevice }, m_pContext { pContext },
	m_pGameInstance { CGameInstance::GetInstance() }
{
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pContext);
	Safe_AddRef(m_pGameInstance);

	m_pBatch = new PrimitiveBatch<VertexPositionColor>(m_pContext);
	m_pEffect = new BasicEffect(m_pDevice);

	m_pEffect->SetVertexColorEnabled(true);

	const void* pShaderByteCode;
	size_t iShaderCodeLength;
	m_pEffect->GetVertexShaderBytecode(&pShaderByteCode, &iShaderCodeLength);

	if (FAILED(m_pDevice->CreateInputLayout(VertexPositionColor::InputElements, VertexPositionColor::InputElementCount,
		pShaderByteCode, iShaderCodeLength, &m_pInputLayout)))
		CRASH("InputLayout");
}

CDebugRender::~CDebugRender()
{
	Safe_Delete(m_pBatch);
	Safe_Delete(m_pEffect);
	Safe_Release(m_pInputLayout);

	Safe_Release(m_pDevice);
	Safe_Release(m_pContext);
	Safe_Release(m_pGameInstance);
}

void CDebugRender::Begin()
{
	m_pEffect->SetWorld(XMMatrixIdentity());
	m_pEffect->SetView(m_pGameInstance->Get_TransformState_Matrix(D3DTS::VIEW));
	m_pEffect->SetProjection(m_pGameInstance->Get_TransformState_Matrix(D3DTS::PROJ));

	m_pContext->IASetInputLayout(m_pInputLayout);
	m_pEffect->Apply(m_pContext);

	m_pBatch->Begin();
}

void CDebugRender::End()
{
	m_pBatch->End();
}

void CDebugRender::DrawLine(RVec3Arg inFrom, RVec3Arg inTo, ColorArg inColor)
{
	Float3 FromPos{}, ToPos{};
	inFrom.StoreFloat3(&FromPos);
	inTo.StoreFloat3(&ToPos);
	_float3 vFromPos{}, vToPos{};
	memcpy(&vFromPos, &FromPos, sizeof(_float3));
	memcpy(&vToPos, &ToPos, sizeof(_float3));

	//inColor = Color::sYellow;
	_float4 vColor = _float4(inColor.r, inColor.g, inColor.b, inColor.a);

	m_pBatch->DrawLine(
		VertexPositionColor(vFromPos, vColor),
		VertexPositionColor(vToPos, vColor)
	);
}

void CDebugRender::DrawText3D(RVec3Arg inPosition, const string_view& inString, ColorArg inColor, float inHeight)
{
}
