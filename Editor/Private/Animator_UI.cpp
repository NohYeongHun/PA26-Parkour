#include "EditorPch.h"
#include "Animator_UI.h"




CAnimator_UI::CAnimator_UI(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CComponent(pDevice, pContext)
{
}

CAnimator_UI::CAnimator_UI(const CAnimator_UI& Prototype)
    : CComponent(Prototype)
{
}

HRESULT CAnimator_UI::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CAnimator_UI::Initialize_Clone(void* pArg)
{
    ASSERT_CRASH(pArg);

    ANIMATOR_UI_DESC* pDesc = static_cast<ANIMATOR_UI_DESC*>(pArg);
    m_pOwner = pDesc->pOwner;



	return S_OK;
}

void CAnimator_UI::Priority_Update(_float fTimeDelta)
{

}

void CAnimator_UI::Update(_float fTimeDelta)
{
    m_fElapsedTime += fTimeDelta;
}

void CAnimator_UI::Late_Update(_float fTimeDelta)
{

}

HRESULT CAnimator_UI::Render()
{
    Update_Animation();
	return S_OK;
}

HRESULT CAnimator_UI::Insert_Animation(CLevel_UI::UI_ANIM_DESC& Desc)
{
    if (Find_Animation(Desc.strAnimName))
        return E_FAIL;

    m_vecAnimationDescs.push_back(Desc);
    return S_OK;
}

HRESULT CAnimator_UI::Remove_Animation(_wstring strAnimName)
{
    CLevel_UI::UI_ANIM_DESC* pDesc = nullptr;

    _uint iIndex = 0;
    for (auto& animDesc : m_vecAnimationDescs)
    {
        if (animDesc.strAnimName == strAnimName)
        {
            pDesc = &animDesc;
            break;
        }
        iIndex++;
    }

    if (m_pCurAnimDesc == pDesc)
        m_pCurAnimDesc = nullptr;

    if (!pDesc)
        return E_FAIL;

    m_vecAnimationDescs.erase(m_vecAnimationDescs.begin() + iIndex);
    return S_OK;
}

HRESULT CAnimator_UI::Clear_Animation()
{
    m_pCurAnimDesc = nullptr;
    m_vecAnimationDescs.clear();

    return S_OK;
}

HRESULT CAnimator_UI::Change_Animation(_wstring strAnimName)
{
    CLevel_UI::UI_ANIM_DESC* pDesc = Find_Animation(strAnimName);

    if (!pDesc)
        return E_FAIL;
    
    m_pCurAnimDesc = pDesc;
    m_fElapsedTime = 0;

    return S_OK;
}

HRESULT CAnimator_UI::Change_Animation(_uint iAnimIndex)
{
    CLevel_UI::UI_ANIM_DESC* pDesc = Find_Animation(iAnimIndex);

    if (!pDesc)
        return E_FAIL;

    m_pCurAnimDesc = pDesc;
    m_fElapsedTime = 0;

    return S_OK;
}

HRESULT CAnimator_UI::Deselect_Animation()
{
    m_pCurAnimDesc = nullptr;

    return S_OK;
}

CLevel_UI::UI_ANIM_DESC* CAnimator_UI::Find_Animation(_wstring strAnimName)
{
    CLevel_UI::UI_ANIM_DESC* pDesc = nullptr;

    for (auto& animDesc : m_vecAnimationDescs)
        if (animDesc.strAnimName == strAnimName)
        {
            pDesc = &animDesc;
            break;
        }
    
    return (pDesc) ? pDesc : nullptr;
}

CLevel_UI::UI_ANIM_DESC* CAnimator_UI::Find_Animation(_uint iAnimIndex)
{
    if (iAnimIndex >= m_vecAnimationDescs.size())
        return nullptr;

    return &m_vecAnimationDescs[iAnimIndex];
}

_float CAnimator_UI::Fix_LerpRatio(_float fIn, _uint iLerpType)
{
    switch (static_cast<UI_LERPTYPE>(iLerpType))
    {
    default:        
    case UI_LERPTYPE::END:
    case UI_LERPTYPE::LINEAR:               return fIn;
    case UI_LERPTYPE::LT:                   return sqrt(1.0f - pow(fIn - 1.0f, 2.0f));
    case UI_LERPTYPE::RB:                   return 1.0f - sqrt(1.0f - pow(fIn, 2.0f));
    case UI_LERPTYPE::CUBIC:                return fIn * fIn * (3.0f - 2.0f * fIn);
    }
}

_float CAnimator_UI::Calc_LerpRatio(_float fStart, _float fEnd, _float fRatio)
{
    if (fRatio > 1.f || fRatio < 0.f)
        CRASH("Unclamped Data. fRatio must between [0, 1]");

    return (fStart * (1 - fRatio)) + (fEnd * fRatio);
}

_float3 CAnimator_UI::Calc_Lerp_Position_CMR(_uint iKeyframe)
{
    if (!m_pCurAnimDesc)
        return _float3();

    _vector     vPoses[4]           = {};

    _uint       iKeyframeTimeStart  = UINT_MAX;
    _uint       iKeyframeTimeEnd    = UINT_MAX;
    const _bool isLoop              = m_pCurAnimDesc->isLoop;
    const _uint iLastKeyframeIndex  = (_uint)(m_pCurAnimDesc->vecKeyFrames.size() - 1);
    _uint       iKeyframeIndex      = 0;

    // 현재 키프레임의 vector 내 인덱스를 검색
    for (_uint i = 0; i < m_pCurAnimDesc->vecKeyFrames.size(); i++)
    {
        if (m_pCurAnimDesc->vecKeyFrames[i].iKeyframeIndex > iKeyframe)
            break;
        iKeyframeIndex = i;
    }
    
    // lerp에 사용할 값들 할당
    for (_uint i = 0; i < 4; i++)
    {
        // 1, 2 사잇값을 사용할 것.
        // 다만 인덱스를 벗어나는 경우에 대해 정의. 이는 loop 여부에 따라 다름.
        _uint iIndex = iKeyframeIndex + i - 1;
        if (iIndex < 0)
            iIndex = (isLoop) ? iLastKeyframeIndex : 0;
        else if (iIndex > iLastKeyframeIndex)
            iIndex = (isLoop) ? 0 : iLastKeyframeIndex;

        vPoses[i] = XMLoadFloat3(&m_pCurAnimDesc->vecKeyFrames[iIndex].vPos);
        
        if (i == 1) iKeyframeTimeStart  = m_pCurAnimDesc->vecKeyFrames[iIndex].iKeyframeIndex;
        if (i == 2) iKeyframeTimeEnd    = m_pCurAnimDesc->vecKeyFrames[iIndex].iKeyframeIndex;
        if (iKeyframeTimeStart == iKeyframeTimeEnd) iKeyframeTimeEnd = m_pCurAnimDesc->vecKeyFrames[(iIndex + 1) % m_pCurAnimDesc->vecKeyFrames.size()].iKeyframeIndex;
    }

    _float fKeyframeRatio = {};

    if ((iKeyframeTimeEnd - iKeyframeTimeStart) == 0)       fKeyframeRatio = 0;
    else        fKeyframeRatio = (_float)(iKeyframe - iKeyframeTimeStart) / (iKeyframeTimeEnd - iKeyframeTimeStart);

    _vector vResultPos = XMVectorCatmullRom(vPoses[0], vPoses[1], vPoses[2], vPoses[3], fKeyframeRatio);
    _float3 vResult = {};
    XMStoreFloat3(&vResult, vResultPos);

    return vResult;
}

void CAnimator_UI::Update_Animation()
{
    if (!(m_pOwner && m_pOwner->Get_Component(L"Com_Shader")))
        return;

	CLevel_UI::UI_ANIM_KEYFRAME_DESC tDesc = {};

	CAnimator_UI* pParentAnimator = nullptr;
	CLevel_UI::UI_ANIM_KEYFRAME_DESC* pParentCombinedDesc = nullptr;
	CCustom_UI* pParentUI = dynamic_cast<CCustom_UI*>(m_pOwner->Get_UIDesc().pParentObject);

	if (pParentUI)
		pParentAnimator = dynamic_cast<CAnimator_UI*>(pParentUI->Get_Component(L"Com_Animator_UI"));
	if (pParentAnimator && pParentAnimator->Get_CurAnimation() != nullptr)
		pParentCombinedDesc = pParentAnimator->Get_CurCombinedAnimKeyframeDesc();


	CShader* pTargetShader = dynamic_cast<CShader*>(m_pOwner->Get_Component(L"Com_Shader"));

	if (m_pOwner->Get_UIDesc().strUIName == L"Inst_EnergyItems")
		int i = 10;
	if (m_pOwner->Get_UIDesc().strUIName == L"EnergyBar")
		int i = 10;


    if (m_pCurAnimDesc == nullptr) // 자식들도 부모의 움직임이랑 알파값 그런건 따라와야하거늘
    {
		_float fCombinedAlpha = (pParentCombinedDesc) ? (1.f - (1.f - pParentCombinedDesc->fAlpha) * (1.f - tDesc.fAlpha)) : tDesc.fAlpha;
        pTargetShader->Bind_Value("g_AlphaStrength", &fCombinedAlpha, sizeof(fCombinedAlpha));
  //      pTargetShader->Bind_Value("g_ScreenLT", &tDesc.vScreenLT, sizeof(tDesc.vScreenLT));
  //      pTargetShader->Bind_Value("g_ScreenRB", &tDesc.vScreenRB, sizeof(tDesc.vScreenRB));
  //      pTargetShader->Bind_Value("g_BlendToOuterWidth", &tDesc.vBlendToOuterWidth, sizeof(tDesc.vBlendToOuterWidth));

		m_tCombinedKeyFrameDesc.fAlpha = fCombinedAlpha;
        return;
    }

    // Calculate Frame..
    const _uint     iKeyFrameRate       = 60; // 기준 초당 프레임
    const _float    fSingleFrameTime    = 1.f / iKeyFrameRate;

    _float fCurFrame = m_fElapsedTime / fSingleFrameTime;                   // 현재 키프레임

    if (fCurFrame >= m_pCurAnimDesc->vecKeyFrames.back().iKeyframeIndex)
    {
        if (m_pCurAnimDesc->isLoop)
            m_fElapsedTime = 0.f;                                           // 루프 시, 범위 넘어가면 0으로
    }

    fCurFrame = m_fElapsedTime / fSingleFrameTime;                          // 최종 현재 키프레임
    _uint iCurFrame = static_cast<_uint>(fCurFrame);                        // 최종 현재 키프레임 (int로 내림)


    // ==============================
    // * Calculate Ratio..
    // ==============================
    _uint iFrame_LerpStart = {};        // 프레임 값
    _uint iFrame_LerpEnd = {};          // 프레임 값
    _uint iFrame_StartIndex = {};       // 순수 인덱스
    _uint iFrame_EndIndex = {};         // 순수 인덱스

    for (_uint i = 0; i < m_pCurAnimDesc->vecKeyFrames.size(); i++)
    {
        if (m_pCurAnimDesc->vecKeyFrames[i].iKeyframeIndex < iCurFrame) // ksta
        {
            iFrame_LerpStart = m_pCurAnimDesc->vecKeyFrames[i].iKeyframeIndex;
            iFrame_StartIndex = i;

            if (m_pCurAnimDesc->vecKeyFrames.size() > (i + 1))
            {
                iFrame_LerpEnd = m_pCurAnimDesc->vecKeyFrames[i + 1].iKeyframeIndex;
                iFrame_EndIndex = i + 1;
            }
            else
            {
                iFrame_LerpEnd = m_pCurAnimDesc->vecKeyFrames[0].iKeyframeIndex;
                iFrame_EndIndex = 0;
            }
        }
        else
            break;
    }

    _float fRawLerpRatio = static_cast<_float>((fCurFrame - iFrame_LerpStart) / (iFrame_LerpEnd - iFrame_LerpStart));
	if ((iFrame_LerpEnd - iFrame_LerpStart) == 0) fRawLerpRatio = 0.f;
    _float fFixedLerpRatio = Fix_LerpRatio(fRawLerpRatio, m_pCurAnimDesc->vecKeyFrames[iFrame_StartIndex].iLerpType); // 이전 키프레임와 현재 키프레임 간의 최종 보간 비율
	if (fFixedLerpRatio < 0) fFixedLerpRatio = 0;
	if (fFixedLerpRatio > 1) fFixedLerpRatio = 1;

    // ==============================
    // * Calculate Results..
    // ==============================
    _uint iResultTexIndex = m_pCurAnimDesc->vecKeyFrames[iFrame_StartIndex].iTexIndex;
    _float fResultAlpha = Calc_LerpRatio(
        m_pCurAnimDesc->vecKeyFrames[iFrame_StartIndex].fAlpha, 
        m_pCurAnimDesc->vecKeyFrames[iFrame_EndIndex].fAlpha, 
        fFixedLerpRatio
    );

    _float3 vResultPos = Calc_Lerp_Position_CMR(iCurFrame);

    _float3 vResultRot = {};
    XMStoreFloat3(&vResultRot, XMVectorLerp(
        XMLoadFloat3(&m_pCurAnimDesc->vecKeyFrames[iFrame_StartIndex].vRot),
        XMLoadFloat3(&m_pCurAnimDesc->vecKeyFrames[iFrame_EndIndex].vRot),
        fFixedLerpRatio)
    );
    _float3 vResultRotRad = { DegreesToRadians(vResultRot.x), DegreesToRadians(vResultRot.y), DegreesToRadians(vResultRot.z) };
    _float3 vResultSca = {};
    XMStoreFloat3(&vResultSca, XMVectorLerp(
        XMLoadFloat3(&m_pCurAnimDesc->vecKeyFrames[iFrame_StartIndex].vSca),
        XMLoadFloat3(&m_pCurAnimDesc->vecKeyFrames[iFrame_EndIndex].vSca),
        fFixedLerpRatio
    ));


    _float2 vResultScreenLT = {};
    XMStoreFloat2(&vResultScreenLT, XMVectorLerp(
        XMLoadFloat2(&m_pCurAnimDesc->vecKeyFrames[iFrame_StartIndex].vScreenLT),
        XMLoadFloat2(&m_pCurAnimDesc->vecKeyFrames[iFrame_EndIndex].vScreenLT),
        fFixedLerpRatio
    ));

    _float2 vResultScreenRB = {};
    XMStoreFloat2(&vResultScreenRB, XMVectorLerp(
        XMLoadFloat2(&m_pCurAnimDesc->vecKeyFrames[iFrame_StartIndex].vScreenRB),
        XMLoadFloat2(&m_pCurAnimDesc->vecKeyFrames[iFrame_EndIndex].vScreenRB),
        fFixedLerpRatio
    ));

    _float4 vResultOuterWidth = {};
    XMStoreFloat4(&vResultOuterWidth, XMVectorLerp(
        XMLoadFloat4(&m_pCurAnimDesc->vecKeyFrames[iFrame_StartIndex].vBlendToOuterWidth),
        XMLoadFloat4(&m_pCurAnimDesc->vecKeyFrames[iFrame_EndIndex].vBlendToOuterWidth),
        fFixedLerpRatio
    ));

    // ==============================
    // * Apply Results..
    // ==============================
		// 셰이더에 바인드
	pTargetShader->Bind_Value("g_AlphaStrength", &fResultAlpha, sizeof(fResultAlpha));
	pTargetShader->Bind_Value("g_ScreenLT", &vResultScreenLT, sizeof(vResultScreenLT));
	pTargetShader->Bind_Value("g_ScreenRB", &vResultScreenRB, sizeof(vResultScreenRB));
	pTargetShader->Bind_Value("g_BlendToOuterWidth", &vResultOuterWidth, sizeof(vResultOuterWidth));

	// 위치 적용
	CTransform* pOwnerTransformCom = dynamic_cast<CTransform*>(m_pOwner->Get_Component(L"Com_Transform"));

	_matrix matPos = XMMatrixTranslationFromVector(XMLoadFloat3(&vResultPos));
	_matrix matRot = XMMatrixRotationRollPitchYawFromVector(XMLoadFloat3(&vResultRotRad));
	_matrix matSca = XMMatrixScalingFromVector(XMLoadFloat3(&vResultSca));

	_matrix matTransform = matSca * matRot * matPos;

    m_pOwner->Set_CurTexIndex(iResultTexIndex);


	// 복사
	m_tCombinedKeyFrameDesc = m_pCurAnimDesc->vecKeyFrames[iFrame_StartIndex];
	m_tCombinedKeyFrameDesc.vPos = vResultPos;
	m_tCombinedKeyFrameDesc.vRot = vResultRotRad;
	m_tCombinedKeyFrameDesc.vSca = vResultSca;
	m_tCombinedKeyFrameDesc.vScreenLT = vResultScreenLT;
	m_tCombinedKeyFrameDesc.vScreenRB = vResultScreenRB;
	m_tCombinedKeyFrameDesc.vBlendToOuterWidth = vResultOuterWidth;
	m_tCombinedKeyFrameDesc.fAlpha = fResultAlpha;

	if (pParentCombinedDesc)
		m_tCombinedKeyFrameDesc.fAlpha = fResultAlpha * pParentCombinedDesc->fAlpha;		// 알파값만 부모 키프레임값을 가져와 계산
	

    pOwnerTransformCom->Set_WorldMatrix(matTransform);
}

CAnimator_UI* CAnimator_UI::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CAnimator_UI* pInstance = new CAnimator_UI(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created : CAnimator_UI");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CComponent* CAnimator_UI::Clone(void* pArg)
{
    CAnimator_UI* pInstance = new CAnimator_UI(*this);

    if (FAILED(pInstance->Initialize_Clone(pArg)))
    {
        MSG_BOX("Failed to Created : CAnimator_UI");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CAnimator_UI::Free()
{
    __super::Free();
}
