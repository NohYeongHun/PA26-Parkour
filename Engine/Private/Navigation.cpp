#include "EnginePch.h"
#include "Navigation.h"

#include "GameInstance.h"

#include "Shader.h"

#include "Cell.h"

_float4x4 CNavigation::m_WorldMatrix = {};

CNavigation::CNavigation(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CComponent { pDevice, pContext }
{
}

CNavigation::CNavigation(const CNavigation& Prototype)
	: CComponent { Prototype },
     m_Cells { Prototype.m_Cells }
#ifdef _DEBUG
    , m_pShader { Prototype.m_pShader }
#endif 
{
    for (auto& pCell : m_Cells)
        Safe_AddRef(pCell);

#ifdef _DEBUG
    Safe_AddRef(m_pShader);
#endif 
}

_uint CNavigation::Get_CurrentCellType()
{
	if (-1 == m_iCurrentCellIndex)
		return 0;

	return m_Cells[m_iCurrentCellIndex]->Get_Type();
}

const _float3* CNavigation::Get_Normal(_uint iLineIndex)
{
	return m_Cells[m_iCurrentCellIndex]->Get_Normal(iLineIndex);
}

HRESULT CNavigation::Initialize_Prototype(const _tchar* pFilePath)
{
    XMStoreFloat4x4(&m_WorldMatrix, XMMatrixIdentity());

    if (FAILED(Load_File(pFilePath)))
        return E_FAIL;

    if (FAILED(SetUp_Neighbors()))
        return E_FAIL;

#ifdef _DEBUG
    m_pShader = CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Engine_Shader_Cell.hlsl"), VTXPOS::Elements, VTXPOS::iNumElements);
    if (nullptr == m_pShader)
        return E_FAIL;
#endif 

	return S_OK;
}

HRESULT CNavigation::Initialize_Clone(void* pArg)
{
    if (nullptr == pArg)
        return S_OK;

    NAVIGATION_DESC* pDesc = static_cast<NAVIGATION_DESC*>(pArg);
    m_iCurrentCellIndex = pDesc->iCurrentCellIndex;

	return S_OK;
}

void CNavigation::Update(const _fmatrix& Matrix)
{
    XMStoreFloat4x4(&m_WorldMatrix, Matrix);
}

_bool CNavigation::IsMove(const _fvector& vWorldPos, const _fvector& vLookDir, _uint* iLineIndex)
{
    _int iNeighborIndex = { -1 };

    _vector vLocalPos = XMVector3TransformCoord(vWorldPos, XMMatrixInverse(nullptr, XMLoadFloat4x4(&m_WorldMatrix)));

	if (true == m_Cells[m_iCurrentCellIndex]->IsIn(vLocalPos, vLookDir, &iNeighborIndex, iLineIndex))
	{
        return true;   
	}
    else
    {
        if (-1 != iNeighborIndex)
        {
            while (true)
            {
                if (-1 == iNeighborIndex)
                    return false;

                if (true == m_Cells[iNeighborIndex]->IsIn(vLocalPos, vLookDir, &iNeighborIndex, iLineIndex))
                    break;
            }
            m_iCurrentCellIndex = iNeighborIndex;
            return true;
        }
        else
            return false;
    }

}

_bool CNavigation::Compute_OnCell(const _fvector& vWorldPos, _float4* pOut)
{
    _vector vLocalPos = XMVector3TransformCoord(vWorldPos, XMMatrixInverse(nullptr, XMLoadFloat4x4(&m_WorldMatrix)));

	_uint iCellType = m_Cells[m_iCurrentCellIndex]->Get_Type();

	_float fHeight = m_Cells[m_iCurrentCellIndex]->Compute_Height(vLocalPos);

	if (fHeight > XMVectorGetY(vWorldPos))
	{
		XMStoreFloat4(pOut, XMVectorSetY(vWorldPos, fHeight));
		return true;
	}
	else
	{
		XMStoreFloat4(pOut, vWorldPos);
		return false;
	}
}

void CNavigation::Compute_OnCell_Dir(const _fvector& vDir, _float3* pOut)
{
	_vector vLocalDir = XMVector3TransformNormal(vDir, XMMatrixInverse(nullptr, XMLoadFloat4x4(&m_WorldMatrix)));

	m_Cells[m_iCurrentCellIndex]->Compute_GoDir(vLocalDir, pOut);
}

#ifdef _DEBUG
HRESULT CNavigation::Render()
{
    m_pShader->Bind_Matrix("g_WorldMatrix", &m_WorldMatrix);
    m_pShader->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::VIEW));
    m_pShader->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::PROJ));

    _float4 vColor = _float4(0.f, 1.f, 0.f, 1.f);
    m_pShader->Bind_Value("g_Color", &vColor, sizeof(_float4));

    m_pShader->Begin(1);

    for (auto& pCell : m_Cells)
        pCell->Render(m_pShader);

    return S_OK;
}
#endif

HRESULT CNavigation::Load_File(const _tchar* pFilePath)
{
    ifstream InputFile(pFilePath, ios::binary);

    if (false == InputFile.is_open())
        return E_FAIL;

    _uint iNumCells = {};
    InputFile.read(reinterpret_cast<_char*>(&iNumCells), sizeof(_uint));

    for (_uint i = 0; i < iNumCells; ++i)
    {
        CELL Cell = { };
        InputFile.read(reinterpret_cast<_char*>(&Cell), sizeof(CELL));
        CCell* pCell = CCell::Create(m_pDevice, m_pContext, Cell, m_Cells.size());
        m_Cells.push_back(pCell);
    }

    InputFile.close();

    return S_OK;
}

HRESULT CNavigation::SetUp_Neighbors()
{
    for (auto& pSrcCell : m_Cells)
    {
        for (auto& pDstCell : m_Cells)
        {
            if (pSrcCell == pDstCell)
                continue;

           if (true == pDstCell->Compare_Points(pSrcCell->Get_Point(POINTS::A), pSrcCell->Get_Point(POINTS::B)))
               pSrcCell->SetUp_Neighbor(LINE::AB, pDstCell);
           if (true == pDstCell->Compare_Points(pSrcCell->Get_Point(POINTS::B), pSrcCell->Get_Point(POINTS::C)))
               pSrcCell->SetUp_Neighbor(LINE::BC, pDstCell);
           if (true == pDstCell->Compare_Points(pSrcCell->Get_Point(POINTS::C), pSrcCell->Get_Point(POINTS::A)))
               pSrcCell->SetUp_Neighbor(LINE::CA, pDstCell);
        }
    }

    return S_OK;
}

CNavigation* CNavigation::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const _tchar* pFilePath)
{
    CNavigation* pInstance = new CNavigation(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype(pFilePath)))
    {
        MSG_BOX("Failed to Create : Navigation");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CComponent* CNavigation::Clone(void* pArg)
{
    CNavigation* pClone = new CNavigation(*this);

    if (FAILED(pClone->Initialize_Clone(pArg)))
    {
        MSG_BOX("Failed to Create : Navigation (Clone)");
        Safe_Release(pClone);
    }

    return pClone;
}

void CNavigation::Free()
{
    __super::Free();

    for (auto& pCell : m_Cells)
        Safe_Release(pCell);
    m_Cells.clear();

#ifdef _DEBUG
    Safe_Release(m_pShader);
#endif
}
