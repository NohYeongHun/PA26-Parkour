
#include"EditorPch.h"
#include "Edit_PreViewModel.h"

CEdit_PreViewModel::CEdit_PreViewModel(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    :CGameObject(pDevice,pContext)
{
}

CEdit_PreViewModel::CEdit_PreViewModel(const CEdit_PreViewModel& Prototype)
    :CGameObject(Prototype)
{
}

HRESULT CEdit_PreViewModel::Initialize_Prototype(_uint iLevel)
{
    if (FAILED(__super::Initialize_Clone(nullptr)))
        int a = 0;
    m_iLevel = iLevel;
    if (FAILED(__super::Add_Component(iLevel, TEXT("Prototype_Component_Shader_NonAnimMesh"),
        TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom), nullptr)))
        return E_FAIL;

    return S_OK;
}

void CEdit_PreViewModel::Priority_Update(_float fTimeDelta)
{
}

void CEdit_PreViewModel::Update(_float fTimeDelta)
{
}

void CEdit_PreViewModel::Late_Update(_float fTimeDelta, _wstring ModelName)
{
#ifdef _DEBUG
    m_pGameInstance->Add_Render_Object(RENDERGROUP::RD_DEBUG, this);
    _wstring Name = ModelName;
    Name.pop_back();
    m_szModelName = Name + to_wstring(0);
#endif 


    m_fViewTime += fTimeDelta;
    if (0.f <= m_fViewTime && m_fViewTime < 2.f)
        m_eViewType = X;
    else if (2.f <= m_fViewTime && m_fViewTime < 4.f)
        m_eViewType = Y;
    else if (4.f <= m_fViewTime && m_fViewTime < 6.f)
        m_eViewType = Z;
    else
        m_fViewTime = 0.f;
}

void CEdit_PreViewModel::Render()
{
    /*auto Pair = m_Models.find(ProtoModelName + m_szModelName);
    if (Pair == m_Models.end())
        return;

    CModel* pModel = Pair->second;

    Sync_BoundingBox(pModel->Get_BoundingBox(), m_pTransformCom->Get_WorldMatrix());

    _float3 vMinExt = _float3(FLT_MAX, FLT_MAX, FLT_MAX);
    _float3 vMaxExt = _float3(FLT_MIN, FLT_MIN, FLT_MIN);

    for (_uint i = 0; i < pModel->Get_NumMesh(); ++i)
    {
        BoundingBox* Box = pModel->Get_BoundingBox();
        vMinExt.x = min(vMinExt.x, Box->Center.x - Box->Extents.x);
        vMinExt.y = min(vMinExt.y, Box->Center.y - Box->Extents.y);
        vMinExt.z = min(vMinExt.z, Box->Center.z - Box->Extents.z);

        vMaxExt.x = max(vMaxExt.x, Box->Center.x + Box->Extents.x);
        vMaxExt.y = max(vMaxExt.y, Box->Center.y + Box->Extents.y);
        vMaxExt.z = max(vMaxExt.z, Box->Center.z + Box->Extents.z);
        Box = nullptr;
    }
    _float4x4 ViewMatrix{};
    _vector eyepos{};
    switch (m_eViewType)
    {
    case X:
        eyepos = XMVectorSet(-vMaxExt.x * 2.f, vMaxExt.y * 0.5f, 0.f, 1.f);
        break;
    case Y:
        eyepos = XMVectorSet(vMaxExt.x * 0.5f, -vMaxExt.y * 2.f, vMaxExt.z * 0.5f, 1.f);
        break;
    case Z:
        eyepos = XMVectorSet(0.f, vMaxExt.y * 0.5f, -vMaxExt.z * 2.f, 1.f);
        break;
    }

    XMStoreFloat4x4(&ViewMatrix, XMMatrixLookAtLH(eyepos, m_pTransformCom->Get_State(STATE::POSITION), XMVectorSet(0.f, 1.f, 0.f, 0.f)));
    m_pTransformCom->Bind_Matrix(m_pShaderCom, "g_WorldMatrix");
    m_pShaderCom->Bind_Matrix("g_ViewMatrix", &ViewMatrix);
    m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::PROJ));

    for (_uint i = 0; i < pModel->Get_NumMesh(); ++i)
    {
        pModel->Bind_Materials(m_pShaderCom, "g_DiffuseTexture", i, TEXTURETYPE::DIFFUSE);
        pModel->Bind_Materials(m_pShaderCom, "g_NormalTexture", i, TEXTURETYPE::NORMAL);

        if (FAILED(pModel->Bind_Materials(m_pShaderCom, "g_MaskTexture", i, TEXTURETYPE::MASK)))
            m_pShaderCom->Bind_Texture("g_MaskTexture", nullptr);
        m_pShaderCom->Begin(4);
        pModel->Render(i);
    }*/
}

void CEdit_PreViewModel::Add_Model(_wstring ModelName)
{
    //m_pGameInstance->Add_Work([=, ModelCom = L"Com_"s + ModelName, Name = ModelName]() {

    //    //_wstring ModelCom = L"Com_"s + ModelName;

    //    if (FAILED(Add_Component(m_iLevel, Name,
    //        ModelCom, reinterpret_cast<CComponent**>(&m_Models[Name]), nullptr)))
    //        CRASH("Clone Failed");
    //    });
    _wstring ModelCom = L"Com_"s + ModelName;

    if (FAILED(Add_Component(m_iLevel, ModelName,
        ModelCom, reinterpret_cast<CComponent**>(&m_Models[ModelName]), nullptr)))
        CRASH("Clone Failed");
}

CEdit_PreViewModel* CEdit_PreViewModel::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, _uint iLevel)
{
    CEdit_PreViewModel* pInstance = new CEdit_PreViewModel(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype(iLevel)))
    {
        MSG_BOX("Failed to Create : CEdit_PreViewModel");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CEdit_PreViewModel::Free()
{
    __super::Free();
    for (auto& Pair : m_Models)
        Safe_Release(Pair.second);

    Safe_Release(m_pShaderCom);

    m_Models.clear();
}
