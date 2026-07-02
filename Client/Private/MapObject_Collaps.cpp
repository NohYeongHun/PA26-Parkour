#include"ClientPch.h"
#include "MapObject_Collaps.h"
#include"GameSystem.h"


CMapObject_Collaps::CMapObject_Collaps(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CGameObject(pDevice,pContext)
{
}

CMapObject_Collaps::CMapObject_Collaps(const CMapObject_Collaps& Prototype)
	:CGameObject(Prototype),m_pGameSystem(CGameSystem::GetInstance())
{
	Safe_AddRef(m_pGameSystem);
}

HRESULT CMapObject_Collaps::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CMapObject_Collaps::Initialize_Clone(void* pArg)
{
	if (FAILED(__super::Initialize_Clone(pArg)))
		return E_FAIL;

	MAP_LOAD* pDesc = static_cast<MAP_LOAD*>(pArg);

	Ready_Components(pArg);

	// 1. 시작할때 Trigger를 GameSystem에 등록하고, 실행할때도 Index를 바탕으로 실행한다.

	return S_OK;
}

void CMapObject_Collaps::Priority_Update(_float fTimeDelta)
{
	m_pTransformCom->Save_PreviousPosition();
}

void CMapObject_Collaps::Update(_float fTimeDelta)
{
	if (m_IsTriggerd) // Trigger 실행 이후.
		LerpPos(fTimeDelta);

	if (m_pBoxRigidbodyCom)
		m_pBoxRigidbodyCom->Update_Rigidbody(m_pTransformCom->Get_WorldMatrix(), fTimeDelta);
	// 매프레임 Target Transform 비우기.
	m_pTargetTransform = nullptr;
}

void CMapObject_Collaps::Late_Update(_float fTimeDelta)
{
	m_pGameInstance->Add_Render_Object(RENDERGROUP::NONSTATIC, this);
}

void CMapObject_Collaps::Render()
{
	_uint m_iLODIndex = 0;
	if (m_iLODIndex > m_pModelCom->Get_LastLODIndex())
		return;

	_bool HasNormal = { true };
	_bool HasMask = { true };
	_uint iNumMesh = m_pModelCom->Get_NumMesh(m_iLODIndex);

	m_pTransformCom->Bind_Matrix(m_pShaderCom, "g_WorldMatrix");
	m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::VIEW));
	m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::PROJ));

	m_pGameInstance->Bind_SharedBuffer(0, m_pContext);

	for (_uint i = 0; i < iNumMesh; ++i)
	{
		if (m_pModelCom->Is_Overed(m_iLODIndex, i))
			return;
		if (FAILED(m_pModelCom->Bind_Materials(m_pShaderCom, "g_MaskTexture", m_iLODIndex, i, TEXTURETYPE::MASK)))
		{
			m_pShaderCom->Bind_Texture("g_MaskTexture", nullptr);
			HasMask = false;
		}

		if (HasMask)
		{
			m_pModelCom->Bind_Materials(m_pShaderCom, "g_DiffuseTexture", m_iLODIndex, i, TEXTURETYPE::DIFFUSE);

			if (FAILED(m_pModelCom->Bind_Materials(m_pShaderCom, "g_NormalTexture", m_iLODIndex, i, TEXTURETYPE::NORMAL)))
				HasNormal = false;
		}
		else
		{
			m_pModelCom->Bind_Materials(m_pShaderCom, "g_DiffuseTexture", m_iLODIndex, i, TEXTURETYPE::DIFFUSE, 0);

			if (FAILED(m_pModelCom->Bind_Materials(m_pShaderCom, "g_NormalTexture", m_iLODIndex, i, TEXTURETYPE::NORMAL, 0)))
				HasNormal = false;
		}
		m_pShaderCom->Bind_Value("g_HasNormal", &HasNormal, sizeof(_bool));
		m_pShaderCom->Bind_Value("g_HasMask", &HasMask, sizeof(_bool));

		m_pShaderCom->Begin(m_iShaderPassIndex);
		m_pModelCom->Render(m_iLODIndex, i);
	}

#ifdef _DEBUG
	if (m_pBoxRigidbodyCom)
		m_pBoxRigidbodyCom->Render();
#endif // _DEBUG

}

void CMapObject_Collaps::LerpPos(_float fTimeDelta)
{
	if (m_pPullUI)
	{
		//m_pGameSystem->Toggle_GrapplePoint(m_pPullUI, false);
		m_pPullUI = nullptr;
	}

	if (!m_IsSound)
	{
		_uint iSoundChannel = m_pGameInstance->Register_Channel();
		m_pGameInstance->Play_Sound_Dynamic(TEXT("Rock_Broken0"), iSoundChannel, 0.2f);
		m_pGameInstance->Return_Channel(iSoundChannel);
		m_IsSound = !m_IsSound;
	}
	m_fFall += fTimeDelta;
	_float Time = m_fFall / m_fDuration;

	_vector vNewTrans = XMVectorLerp(m_vSourTrans.Vec, m_vDestTrans.Vec, Time);
	_vector vNewRot = XMQuaternionSlerp(m_vSourRot.Vec, m_vDestRot.Vec, Time);

	m_pTransformCom->Set_WorldMatrix(XMMatrixAffineTransformation(m_vSourScale.Vec, XMVectorZero(), vNewRot, vNewTrans));

	if (m_fFall >= m_fDuration)
	{
#ifdef _DEBUG
		m_fFall = 0.f;
#endif
		m_IsTriggerd = false;
		m_pTransformCom->Set_WorldMatrix(XMLoadFloat4x4(&m_DestMat));
		m_pSourRigidbodyCom->Change_Layer(ENUM_CLASS(COLLISIONLAYER::NONE));
		m_pDestRigidbodyCom->Change_Layer(ENUM_CLASS(COLLISIONLAYER::MAP));

		_uint iSoundChannel = m_pGameInstance->Register_Channel();
		m_pGameInstance->Play_Sound_Dynamic(TEXT("Rock_Broken0"), iSoundChannel, 0.2f);
		m_pGameInstance->Return_Channel(iSoundChannel);


		CAMERA_SHAKE ShakeDesc{};
		ShakeDesc.fAmplitude = 1.f;
		ShakeDesc.fDuration = 0.8f;
		ShakeDesc.fFovKick = 0.f;
		ShakeDesc.fFrequency = 2.f;
		ShakeDesc.vRotation = _float3(0.005f, 0.075f, 0.f);
		ShakeDesc.vTranslation;
		m_pGameInstance->OnShake(ShakeDesc);

		PREFAB_INFO Info;
		if (m_iTriggerIndex == 33)
		{
			m_pGameInstance->Spawn_PoolingObject(TEXT("Smoke"), XMLoadFloat4x4(&m_DestMat), &Info);
		}
		else
		{
			m_pGameInstance->Spawn_PoolingObject(TEXT("Smoke"), XMLoadFloat4x4(&m_SmokePoint), &Info);
			m_pGameInstance->Spawn_PoolingObject(TEXT("Smoke"), XMLoadFloat4x4(&m_SmokePoint2), &Info);
		}
		return;
	}
}

void CMapObject_Collaps::OnCollider_During(_uint iLayer, void* pDesc, const ContactManifold& Manifold)
{
	// 1. Detect 감지되면?
	if (ENUM_CLASS(COLLISIONLAYER::PLAYER) != iLayer || m_IsTriggerd)
		return;

	// 2. CallBack 정보 가져오기
	CALLBACK_CLIENT* pcallDesc = static_cast<CALLBACK_CLIENT*>(pDesc);

	CTransform* pTargetTransform = static_cast<CTransform*>(pcallDesc->pTransform);
	if (nullptr == pTargetTransform)
		return;

	{
		lock_guard<mutex> lock(m_Mutex);
		m_pTargetTransform = pTargetTransform;
	}
}

void CMapObject_Collaps::Ready_Components(void* pArg)
{
	MAP_LOAD* pDesc = static_cast<MAP_LOAD*>(pArg);

	m_pTransformCom->Set_WorldMatrix(XMLoadFloat4x4(&pDesc->vSourWorldMatrix));

	m_fDuration = pDesc->fDuration;
	m_SourMat = pDesc->vSourWorldMatrix;
	m_DestMat = pDesc->vDestWorldMatrix;
	m_iTriggerIndex = pDesc->TriggerIndex;
	XMMatrixDecompose(&m_vSourScale.Vec, &m_vSourRot.Vec, &m_vSourTrans.Vec, XMLoadFloat4x4(&m_SourMat));
	XMMatrixDecompose(&m_vDestScale.Vec, &m_vDestRot.Vec, &m_vDestTrans.Vec, XMLoadFloat4x4(&m_DestMat));
	_tchar Model[MAX_PATH] = TEXT("Prototype_Component_Model_");
	lstrcat(Model, StringToWString(pDesc->ModelName).c_str());

	m_iShaderPassIndex = pDesc->iShaderPassIndex;

	_wstring WModelName = Model;
	WModelName.pop_back();
	WModelName.pop_back();
	WModelName.pop_back();
	WModelName.pop_back();
	WModelName.pop_back();
	if (FAILED(Add_Component(ENUM_CLASS(pDesc->iLevel), WModelName,
		TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom), nullptr)))
		CRASH("FAILED");

	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Shader_VtxMesh"),
		TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom), nullptr)))
		return;

	m_pTransformCom->Set_WorldMatrix(XMLoadFloat4x4(&pDesc->vSourWorldMatrix));

	CRigidbody::MESHBODY_DESC RigidbodyDesc = {};
	RigidbodyDesc.vScale = m_pTransformCom->Get_Scaled();
	XMStoreFloat4(&RigidbodyDesc.vQuat, m_pTransformCom->Get_Quaternion());
	RigidbodyDesc.eShape = SHAPE::MESH;
	XMStoreFloat3(&RigidbodyDesc.vPos, m_pTransformCom->Get_State(STATE::POSITION));
	RigidbodyDesc.eType = EMotionType::Static;
	RigidbodyDesc.eBodyType = CRigidbody::BODY; // 어떤 충돌 형태로 할ㅈ ㅣ정의
	RigidbodyDesc.iLayer = ENUM_CLASS(COLLISIONLAYER::GRAPPLE);
	RigidbodyDesc.pModel = m_pModelCom;

	Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Rigidbody"),
		TEXT("Com_SourRigidbody"), reinterpret_cast<CComponent**>(&m_pSourRigidbodyCom), &RigidbodyDesc);

	m_pTransformCom->Set_WorldMatrix(XMLoadFloat4x4(&pDesc->vDestWorldMatrix));

	RigidbodyDesc.vScale = m_pTransformCom->Get_Scaled();
	XMStoreFloat4(&RigidbodyDesc.vQuat, m_pTransformCom->Get_Quaternion());
	XMStoreFloat3(&RigidbodyDesc.vPos, m_pTransformCom->Get_State(STATE::POSITION));
	RigidbodyDesc.iLayer = ENUM_CLASS(COLLISIONLAYER::NONE);

	Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Rigidbody"),
		TEXT("Com_DestRigidbody"), reinterpret_cast<CComponent**>(&m_pDestRigidbodyCom), &RigidbodyDesc);

	m_pTransformCom->Set_WorldMatrix(XMLoadFloat4x4(&pDesc->vSourWorldMatrix));
}


CMapObject_Collaps* CMapObject_Collaps::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CMapObject_Collaps* pInstance = new CMapObject_Collaps(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Create : MapObject_Collaps");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CMapObject_Collaps::Clone(void* pArg)
{
	CMapObject_Collaps* pInstance = new CMapObject_Collaps(*this);

	if (FAILED(pInstance->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Create : MapObject_Collaps (Clone)");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CMapObject_Collaps::Free()
{
	__super::Free();
	m_pPullUI = nullptr;
	Safe_Release(m_pSourRigidbodyCom);
	Safe_Release(m_pDestRigidbodyCom);
	Safe_Release(m_pBoxRigidbodyCom);
	Safe_Release(m_pModelCom);
	Safe_Release(m_pShaderCom);
	Safe_Release(m_pGameSystem);

}
