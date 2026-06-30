#include "Editorpch.h"
#include "Effect_Prefab.h"
#include "Particle.h"
#include "Effect_Mesh.h"
#include "Trail_Mesh.h"
#include "Effect_Rect.h"
#include "Effect_Decal.h"
#include "Effect_Radial.h"
#include "TestVA.h"
#include "Effect_Light.h"

CEffect_Prefab::CEffect_Prefab(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CGameObject{ pDevice, pContext }
{
}

CEffect_Prefab::CEffect_Prefab(const CEffect_Prefab& Prototype)
    : CGameObject{ Prototype }
{
}

HRESULT CEffect_Prefab::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CEffect_Prefab::Initialize_Clone(void* pArg)
{
    PREFAB_DESC* pDesc = static_cast<PREFAB_DESC*>(pArg);

    if (FAILED(__super::Initialize_Clone(pArg)))
        return E_FAIL;

    m_strMyTag = pDesc->strPrefabTag;

    m_vLifeTime = pDesc->vLifeTime;
	m_IsLoop = pDesc->IsLoop;

    m_isActivate = false;

    XMStoreFloat4x4(&m_SpawnMatrix, XMMatrixIdentity());

    return S_OK;
}

void CEffect_Prefab::Priority_Update(_float fTimeDelta)
{
    if (!m_isActivate)
        return;

    m_fCurrentTime += fTimeDelta;

	for (auto& Frame : m_vFrames)
	{
		if (Frame.fActivateTime <= m_fCurrentTime && !Frame.bActivated)
		{
			EFFECT_INFO InfoDesc = {};

			InfoDesc.pBoneMatrixPtr = m_pBoneMatrixPtr;
			InfoDesc.pObjectMatrixPtr = m_pObjectMatrixPtr;
			InfoDesc.IsActive = true;

			_matrix OffsetMatrix = {};

			Children_Offset(Frame, OffsetMatrix, InfoDesc);
			Get_Children(Frame.strChildrenTag)->Reset(OffsetMatrix, &InfoDesc);

			Frame.bActivated = true;
		}
	}


    for (auto& Children : m_EffectChildren)
    {
        if (Children.second->IsActivate())
            Children.second->Priority_Update(fTimeDelta);
    }
}

void CEffect_Prefab::Update(_float fTimeDelta)
{
    if (!m_isActivate)
        return;

	if (!m_IsLoop)
	{
		if (m_vLifeTime.x >= m_vLifeTime.y)
		{
			m_isActivate = false;
			Reset_Prefab_Info();
		}
		else
			m_vLifeTime.x += fTimeDelta;
	}

    for (auto& Children : m_EffectChildren)
    {
        if (Children.second->IsActivate())
            Children.second->Update(fTimeDelta);
    }
}

void CEffect_Prefab::Late_Update(_float fTimeDelta)
{
    if (!m_isActivate)
        return;

    for (auto& Children : m_EffectChildren)
    {
        if (Children.second->IsActivate())
            Children.second->Late_Update(fTimeDelta);
    }
}

void CEffect_Prefab::Render()
{

}

void CEffect_Prefab::Reset(const _fmatrix& WorldMatrix, void* pArg)
{
	/*PREFAB_INFO* pDesc = static_cast<PREFAB_INFO*>(pArg);

	if (pDesc->pModelPtr != nullptr)
	{
		프리팹 안에 뼈에 붙어야 할 자식과 안붙어야 할 자식이 같이 있을 수 있음.
		그러니 프리팹에 기존 처리 + 만약 뼈에 붙어야할 얘가 있다면 추가적인 정보를 필요로 함 (BonePtr과 ObjectPtr필요)
		기존 처리 할 얘들을 위한 정보 + PTR이 필요하니 기존 처리도 해주고 뼈 정보도 저장해주는게 맞는거 같음. 일단은 그렇게 생각중.

		Reset_SpawnMatrix();
		Reset_Prefab_Info();

		_float4x4 PlayerMatrix = {};
		XMStoreFloat4x4(&PlayerMatrix, WorldMatrix);

		프리팹이 뼈에 붙을 이름을 알고 있게 해줘야함.
		_float4x4 BoneMatrix;

		if (m_strBoneTag == "")
			XMStoreFloat4x4(&BoneMatrix, XMMatrixIdentity());
		else
			BoneMatrix = *pDesc->pModelPtr->Get_BoneMatrixPtr(m_strBoneTag.c_str());

		위에서 꺼낸 본 매트릭스 그때 위치 갱신정보와 모델의 월드매트릭스 전달.
		Set_SpawnMatrix(PlayerMatrix, BoneMatrix);

		if (pDesc->pModelPtr != nullptr && pDesc->pMatrixPtr != nullptr)
		{
			null이 아니라는건 뼈에 완전히 붙여야한다는 것.
			m_pBoneMatrixPtr = pDesc->pModelPtr->Get_BoneMatrixPtr(m_strBoneTag.c_str());
			m_pObjectMatrixPtr = pDesc->pMatrixPtr;
		}

		m_isActivate = true;
	}
	else
	{
		툴용 초기화?

	}*/

	PREFAB_INFO* pDesc = static_cast<PREFAB_INFO*>(pArg);

	_float4x4 PlayerMatrix = {};
	_float4x4 BoneMatrix = {};

	if (pDesc->pModelPtr != nullptr)
	{
		//프리팹 안에 뼈에 붙어야 할 자식과 안붙어야 할 자식이 같이 있을 수 있음.
		//그러니 프리팹에 기존 처리 + 만약 뼈에 붙어야할 얘가 있다면 추가적인 정보를 필요로 함 (BonePtr과 ObjectPtr필요)
		//기존 처리 할 얘들을 위한 정보 + PTR이 필요하니 기존 처리도 해주고 뼈 정보도 저장해주는게 맞는거 같음. 일단은 그렇게 생각중.

		Reset_SpawnMatrix();
		Reset_Prefab_Info();

		XMStoreFloat4x4(&PlayerMatrix, WorldMatrix);

		if (m_strBoneTag == "")
			XMStoreFloat4x4(&BoneMatrix, XMMatrixIdentity());
		else
			BoneMatrix = *pDesc->pModelPtr->Get_BoneMatrixPtr(m_strBoneTag.c_str());

		//위에서 꺼낸 본 매트릭스 그때 위치 갱신정보와 모델의 월드매트릭스 전달.
		Set_SpawnMatrix(PlayerMatrix, BoneMatrix);

		m_pBoneMatrixPtr = pDesc->pModelPtr->Get_BoneMatrixPtr(m_strBoneTag.c_str());
		m_pObjectMatrixPtr = pDesc->pMatrixPtr;

		m_isActivate = true;
	}
	else if (pDesc->pModelPtr == nullptr)
	{
		//단순 오브젝트가 호출할경우 모델주소 비어있음.

		Reset_SpawnMatrix();
		Reset_Prefab_Info();

		_float4x4 PlayerMatrix = {};
		XMStoreFloat4x4(&PlayerMatrix, WorldMatrix);

		XMStoreFloat4x4(&BoneMatrix, XMMatrixIdentity());

		Set_SpawnMatrix(PlayerMatrix, BoneMatrix);

		m_isActivate = true;
	}
}

void CEffect_Prefab::Add_Children(void* pArg, EFFECT_TYPE eType)
{
    CGameObject* pChildren = {};
    _wstring strChildrenTag = {};
    
    //설정할 자식들 Desc 미리 선언
    CParticle::PARTICLE_DESC* pParticleDesc = {};
    CEffect_Mesh::EFFECTMESH_DESC* pMeshDesc = {};
    CTrail_Mesh::TRAILMESH_DESC* pTrailDesc = {};
	CEffect_Rect::FXRECT_DESC* pRectDesc = {};
	CEffect_Decal::DECAL_DESC* pDecalDesc = {};
	CEffect_Radial::RADIAL_DESC* pRadialDesc = {};
	CTestVA::VA_DESC* pVADesc = {};
	CEffect_Light::LIGHT_DESC* pLightDesc = {};


    //프리팹 프레임에 미리 추가.
    FRAME_DESC FrameDesc = {};

    switch (eType)
    {
    case EFFECT_TYPE::PARTICLE:
        pParticleDesc = static_cast<CParticle::PARTICLE_DESC*>(pArg);
        strChildrenTag = pParticleDesc->strMyTag;

        FrameDesc.strChildrenTag = pParticleDesc->strMyTag;
        FrameDesc.eChildrenType = eType;
        
        pChildren = static_cast<CGameObject*>(m_pGameInstance->Clone_Prototype(ENUM_CLASS(LEVEL::EFFECT), TEXT("Prototype_GameObject_Particle"), PROTOTYPE::GAMEOBJECT, pArg));
        break;
    case EFFECT_TYPE::MESH:
        pMeshDesc = static_cast<CEffect_Mesh::EFFECTMESH_DESC*>(pArg);
        strChildrenTag = pMeshDesc->strMyTag;

        FrameDesc.strChildrenTag = pMeshDesc->strMyTag;
        FrameDesc.eChildrenType = eType;

        pChildren = static_cast<CGameObject*>(m_pGameInstance->Clone_Prototype(ENUM_CLASS(LEVEL::EFFECT), TEXT("Prototype_GameObject_EffectMesh"), PROTOTYPE::GAMEOBJECT, pArg));
        break;
    case EFFECT_TYPE::TRAIL:
        pTrailDesc = static_cast<CTrail_Mesh::TRAILMESH_DESC*>(pArg);
        strChildrenTag = pTrailDesc->strMyTag;

        FrameDesc.strChildrenTag = pTrailDesc->strMyTag;
        FrameDesc.eChildrenType = eType;

        pChildren = static_cast<CGameObject*>(m_pGameInstance->Clone_Prototype(ENUM_CLASS(LEVEL::EFFECT), TEXT("Prototype_GameObject_TrailMesh"), PROTOTYPE::GAMEOBJECT, pArg));
        break;
	case EFFECT_TYPE::RECT:
		pRectDesc = static_cast<CEffect_Rect::FXRECT_DESC*>(pArg);
		strChildrenTag = pRectDesc->strMyTag;

		FrameDesc.strChildrenTag = pRectDesc->strMyTag;
		FrameDesc.eChildrenType = eType;

		pChildren = static_cast<CGameObject*>(m_pGameInstance->Clone_Prototype(ENUM_CLASS(LEVEL::EFFECT), TEXT("Prototype_GameObject_EffectRect"), PROTOTYPE::GAMEOBJECT, pArg));
		break;
	case EFFECT_TYPE::DECAL:
		pDecalDesc = static_cast<CEffect_Decal::DECAL_DESC*>(pArg);
		strChildrenTag = pDecalDesc->strMyTag;

		FrameDesc.strChildrenTag = pDecalDesc->strMyTag;
		FrameDesc.eChildrenType = eType;

		pChildren = static_cast<CGameObject*>(m_pGameInstance->Clone_Prototype(ENUM_CLASS(LEVEL::EFFECT), TEXT("Prototype_GameObject_EffectDecal"), PROTOTYPE::GAMEOBJECT, pArg));
		break;

	case EFFECT_TYPE::RADIAL:
		pRadialDesc = static_cast<CEffect_Radial::RADIAL_DESC*>(pArg);
		strChildrenTag = pRadialDesc->strMyTag;

		FrameDesc.strChildrenTag = pRadialDesc->strMyTag;
		FrameDesc.eChildrenType = eType;

		pChildren = static_cast<CGameObject*>(m_pGameInstance->Clone_Prototype(ENUM_CLASS(LEVEL::EFFECT), TEXT("Prototype_GameObject_EffectRadial"), PROTOTYPE::GAMEOBJECT, pArg));
		break;

	case EFFECT_TYPE::VA:
		pVADesc = static_cast<CTestVA::VA_DESC*>(pArg);
		strChildrenTag = pVADesc->strMyTag;

		FrameDesc.strChildrenTag = pVADesc->strMyTag;
		FrameDesc.eChildrenType = eType;

		pChildren = static_cast<CGameObject*>(m_pGameInstance->Clone_Prototype(ENUM_CLASS(LEVEL::EFFECT), TEXT("Prototype_GameObject_EffectVA"), PROTOTYPE::GAMEOBJECT, pArg));
		break;

	case EFFECT_TYPE::LIGHT:
		pLightDesc = static_cast<CEffect_Light::LIGHT_DESC*>(pArg);
		strChildrenTag = pLightDesc->strMyTag;

		FrameDesc.strChildrenTag = pLightDesc->strMyTag;
		FrameDesc.eChildrenType = eType;

		pChildren = static_cast<CGameObject*>(m_pGameInstance->Clone_Prototype(ENUM_CLASS(LEVEL::EFFECT), TEXT("Prototype_GameObject_EffectLight"), PROTOTYPE::GAMEOBJECT, pArg));
		break;

    case EFFECT_TYPE::END:
        CRASH("Failed Children Desc");
        break;
    }

    if (pChildren == nullptr)
        return;

    m_EffectChildren.emplace(strChildrenTag, pChildren);

    _bool bCheck = false;
    FrameDesc_Check(strChildrenTag, &bCheck);

    if (!bCheck)
        m_vFrames.push_back(FrameDesc);

}

void CEffect_Prefab::Remove_Children(_wstring& ChildrenTag)
{
    auto iter = m_EffectChildren.find(ChildrenTag);

    if (iter == m_EffectChildren.end())
        return;

    Safe_Release(iter->second);
    m_EffectChildren.erase(iter);
}

_int CEffect_Prefab::Get_Children_Count()
{
    if (m_EffectChildren.empty())
        return 0;

    return m_EffectChildren.size();
}

_wstring CEffect_Prefab::Get_Children_Tag(_int iIndex)
{
    _int iCheckIndex = 0;

    for (auto iter = m_EffectChildren.begin(); iter != m_EffectChildren.end();)
    {
        if (iCheckIndex == iIndex)
        {
            return iter->first;
        } 
        else
        {
            ++iter;
            ++iCheckIndex;
        }
    }
}

CGameObject* CEffect_Prefab::Get_Children(_wstring ChildrenTag)
{
    auto iter = m_EffectChildren.find(ChildrenTag);

    if (iter == m_EffectChildren.end())
        return nullptr;

    return iter->second;
}

void CEffect_Prefab::Bind_FrameDesc(PREFAB_DESC& PrefabDesc)
{
    for (size_t i = 0; i < PrefabDesc.ChildrenCount; i++)
    {
        FRAME_DESC FrameDesc = PrefabDesc.FrameDesc[i];

        m_vFrames.push_back(FrameDesc);
    }
}

void CEffect_Prefab::Set_FrameDesc(FRAME_DESC* pFrameDesc)
{
    for (auto& Frame : m_vFrames)
    {
        if (pFrameDesc->strChildrenTag == Frame.strChildrenTag)
        {
            Frame = *pFrameDesc;
			_matrix OffsetMatrix = {};
			EFFECT_INFO Info = {};

			Info.IsActive = true;
			Info.pBoneMatrixPtr = m_pBoneMatrixPtr;
			Info.pObjectMatrixPtr = m_pObjectMatrixPtr;

			Children_Offset(Frame, OffsetMatrix, Info);
			Get_Children(Frame.strChildrenTag)->Reset(OffsetMatrix, &Info);

			m_vLifeTime.x = 0.f;
            return;
        }
    }
}


void CEffect_Prefab::Set_SpawnMatrix(_float4x4 PlayerMatrix, _float4x4 BoneMatrix)
{
	XMStoreFloat4x4(&m_SpawnMatrix,
		XMLoadFloat4x4(&BoneMatrix) * XMLoadFloat4x4(&PlayerMatrix));
}

void CEffect_Prefab::Reset_SpawnMatrix()
{
    for (auto& Children : m_EffectChildren)
        Children.second->SetActivate(false);

    XMStoreFloat4x4(&m_SpawnMatrix, XMMatrixIdentity());
}

void CEffect_Prefab::Reset_Prefab_Info()
{
    for (auto& Frame : m_vFrames)
    {
        Frame.bActivated = false;
    }
    m_fCurrentTime = 0.f;

    m_vLifeTime.x = 0.f;


#ifdef _DEBUG
    OutPutDebugMatrix(TEXT("Spawn Matrix : "), m_SpawnMatrix);
#endif // DEBUG
}

void CEffect_Prefab::Children_Offset(const FRAME_DESC& Desc, _matrix& OutMatrix, EFFECT_INFO& Info)
{
    _matrix PositionMat = XMMatrixTranslationFromVector(XMVectorSet(Desc.vOffsetPos.x, Desc.vOffsetPos.y, Desc.vOffsetPos.z, 1.f));

    _matrix ScaleMat = XMMatrixScaling(Desc.vOffsetSize.x, Desc.vOffsetSize.y, Desc.vOffsetSize.z);

    _matrix RotMat = XMMatrixRotationRollPitchYaw(
        XMConvertToRadians(Desc.vOffsetRot.x),
        XMConvertToRadians(Desc.vOffsetRot.y),
        XMConvertToRadians(Desc.vOffsetRot.z));

    _matrix OffsetMatrix = ScaleMat * RotMat * PositionMat;

	Info.OffsetMatrix = OffsetMatrix;

	//m_SpawnMatrix 크기 영향 죽이기
	_vector vScale = {};
	_vector vPos = {};
	_vector vRot = {};
	XMMatrixDecompose(&vScale, &vRot, &vPos, XMLoadFloat4x4(&m_SpawnMatrix));

	_matrix SpawnMatrix = XMMatrixRotationQuaternion(vRot) * XMMatrixTranslationFromVector(vPos);

	OutMatrix = OffsetMatrix * SpawnMatrix;
}

void CEffect_Prefab::FrameDesc_Check(_wstring& ChildrenTag, _bool* bCheck)
{
    //자식 추가했을 때, 이전에 있던 자식 태그와 같은 얘라면 FrameDesc 삭제x 급하게 대충 만듦
    for (auto iterFrame = m_vFrames.begin(); iterFrame != m_vFrames.end();)
    {
        if (iterFrame->strChildrenTag == ChildrenTag)
        {
            *bCheck = true;
            break;
        }
        else
            ++iterFrame;
    }

    if (!bCheck)
    {
        for (auto iterFrameDesc = m_vFrames.begin(); iterFrameDesc != m_vFrames.end(); )
        {
            if (iterFrameDesc->strChildrenTag == ChildrenTag)
            {
                iterFrameDesc = m_vFrames.erase(iterFrameDesc);
                break;
            }
            else
                ++iterFrameDesc;
        }
    }

}

void CEffect_Prefab::Remove_FrameDesc(_wstring& ChildrenTag)
{
	for (auto iterFrameDesc = m_vFrames.begin(); iterFrameDesc != m_vFrames.end(); )
	{
		if (iterFrameDesc->strChildrenTag == ChildrenTag)
		{
			iterFrameDesc = m_vFrames.erase(iterFrameDesc);
			break;
		}
		else
			++iterFrameDesc;
	}
}

CEffect_Prefab* CEffect_Prefab::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CEffect_Prefab* pInstance = new CEffect_Prefab(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created : CEffect_Prefab");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CEffect_Prefab::Clone(void* pArg)
{
    CEffect_Prefab* pInstance = new CEffect_Prefab(*this);

    if (FAILED(pInstance->Initialize_Clone(pArg)))
    {
        MSG_BOX("Failed to Created : CParticle");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CEffect_Prefab::Free()
{
    __super::Free();

    for (auto& Children : m_EffectChildren)
        Safe_Release(Children.second);

    m_EffectChildren.clear();
}
