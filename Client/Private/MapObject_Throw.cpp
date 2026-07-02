#include"ClientPch.h"
#include "MapObject_Throw.h"
#include"GameSystem.h"
CMapObject_Throw::CMapObject_Throw(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CGameObject(pDevice,pContext)
{
}

CMapObject_Throw::CMapObject_Throw(const CMapObject_Throw& Prototype)
	:CGameObject(Prototype),m_pGameSystem(CGameSystem::GetInstance())
{
	Safe_AddRef(m_pGameSystem);
}

HRESULT CMapObject_Throw::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CMapObject_Throw::Initialize_Clone(void* pArg)
{
	if (FAILED(__super::Initialize_Clone(pArg)))
		return E_FAIL;

	Ready_Components(pArg);
	m_fFlyTime = 1.f;
    return S_OK;
}

void CMapObject_Throw::Priority_Update(_float fTimeDelta)
{
	// Interact 상황이 아닌 경우에는 기존의 위치에 고정(m_vOriginPos)
	if (!m_IsGrabbed && !m_IsThrow) 
	{
		m_pTransformCom->Set_State(STATE::POSITION, XMLoadFloat4(&m_vOriginPos));
		m_fThrowTime = 0.f;
		m_fattachTime = 0.f;
	}
}

void CMapObject_Throw::Update(_float fTimeDelta)
{
	Calc_CombinedMatrix();

	m_pDetectRigidbodyCom->Update_Rigidbody(m_pTransformCom->Get_WorldMatrix(), fTimeDelta);
	if (m_IsGrabbed && !m_IsThrow)
	{
		m_pCollideRigidbodyCom->IsActivate(false);
		if (m_fattachTime < 1.f)
		{
			m_fattachTime += fTimeDelta;
			Attach_Lerp(); // Character 로부터 전달받은 뼈행렬, World행렬을 이용해 Lerp.
		}
		else
			Attach_Pos();

		XMStoreFloat3(&m_vStartPos, m_pTransformCom->Get_State(STATE::POSITION));
		m_pGameInstance->GetCenterPos(&m_vTargetPos);

		_vector DisplaceMent = XMLoadFloat3(&m_vTargetPos) - XMLoadFloat3(&m_vStartPos);


		_vector vGravityAccel = XMVectorSet(0.f, -9.81f, 0.f, 0.f);
		_vector vGravityDrop = vGravityAccel * 0.5f * m_fFlyTime * m_fFlyTime;
		XMStoreFloat3(&m_vImpulse, (DisplaceMent - vGravityDrop) / m_fFlyTime);

		_float3 Grav(0.f, -9.8f, 0.f);
		m_pGameSystem->Req_Render_CurveTrace(m_vStartPos, m_vImpulse, Grav, &m_vTargetPos);
	}
	
	if (m_IsThrow)
	{
		m_fThrowTime += fTimeDelta;
		if (m_fThrowTime < m_fFlyTime)
		{
			_vector vt = XMLoadFloat3(&m_vImpulse) * m_fThrowTime;

			_vector gt2 = 0.5f * XMVectorSet(0.f, -9.81f, 0.f, 0.f) * m_fThrowTime * m_fThrowTime;
			_vector NewPos = XMVectorSetW(XMLoadFloat3(&m_vStartPos) + vt + gt2, 1.f);
			m_pTransformCom->Set_State(STATE::POSITION, NewPos);
		}
		else
		{
			m_fThrowTime = 0.f;
			m_IsThrow = false;
			//이펙트 호출.
			m_pCollideRigidbodyCom->IsActivate(true);
			m_pCollideRigidbodyCom->Update_Rigidbody(m_pTransformCom->Get_WorldMatrix(), fTimeDelta);
			_vector Pos = m_pTransformCom->Get_State(STATE::POSITION);
			PREFAB_INFO Info;
			_int vTemp = 1.f;
			for (_uint i = 0; i < 10; i++)
				m_pGameInstance->Spawn_PoolingObject(TEXT("Small_Smoke"), XMMatrixTranslationFromVector(Pos + XMVectorSet(m_pGameInstance->Rand(-vTemp, vTemp), m_pGameInstance->Rand(-vTemp, vTemp), m_pGameInstance->Rand(-vTemp, vTemp), 0.f)), &Info);

			_uint iSoundChannel = m_pGameInstance->Register_Channel();

			iSoundChannel = m_pGameInstance->Register_Channel();
			m_pGameInstance->Play_Sound_Dynamic(TEXT("StoneBroken"), iSoundChannel, 1.f);
			m_pGameInstance->Return_Channel(iSoundChannel);
		}
	}
}

void CMapObject_Throw::Late_Update(_float fTimeDelta)
{
	m_pGameInstance->Add_Render_Object(RENDERGROUP::NONSTATIC, this);
}

void CMapObject_Throw::Render()
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

	//m_pModelCom->Bind_Buffer(m_pContext, m_iLODIndex);
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
	m_pDetectRigidbodyCom->Render();
	m_pCollideRigidbodyCom->Render();
#endif
}

void CMapObject_Throw::Render_Shadow()
{
}

void CMapObject_Throw::OnCollider_During(_uint iLayer, void* pDesc, const ContactManifold& Manifold)
{
	if (ENUM_CLASS(COLLISIONLAYER::PLAYER) != iLayer)
		return;

	CALLBACK_CLIENT* pcallDesc = static_cast<CALLBACK_CLIENT*>(pDesc);

}

void CMapObject_Throw::Ready_Components(void* pArg)
{
	MAP_LOAD* pDesc = static_cast<MAP_LOAD*>(pArg);

	_tchar Model[MAX_PATH] = TEXT("Prototype_Component_Model_");
	lstrcat(Model, StringToWString(pDesc->ModelName).c_str());

	m_iShaderPassIndex = pDesc->iShaderPassIndex;

	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Shader_VtxMesh"),
		TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom), nullptr)))
		CRASH("FAILED");

	_wstring ModelName = Model;
	ModelName.pop_back();
	ModelName.pop_back();
	ModelName.pop_back();
	ModelName.pop_back();
	ModelName.pop_back();

	if (FAILED(Add_Component(ENUM_CLASS(pDesc->iLevel), ModelName,
		TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom), nullptr)))
		CRASH("FAILED");

	m_pTransformCom->Set_WorldMatrix(XMLoadFloat4x4(pDesc->WorldMatrix));
	XMStoreFloat4(&m_vOriginPos, m_pTransformCom->Get_State(STATE::POSITION));


	CRigidbody::BOXBODY_DESC RigidbodyDesc{};
	RigidbodyDesc.eShape = SHAPE::BOX;
	XMStoreFloat3(&RigidbodyDesc.vPos, m_pTransformCom->Get_State(STATE::POSITION));
	RigidbodyDesc.eType = EMotionType::Kinematic;
	RigidbodyDesc.iLayer = ENUM_CLASS(COLLISIONLAYER::DETECT);
	RigidbodyDesc.vExtent = _float3(3.f, 3.f, 3.f);
	//플레이어 감지용 1개
	Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Rigidbody"),
		TEXT("Com_DetectRigidbody"), reinterpret_cast<CComponent**>(&m_pDetectRigidbodyCom), &RigidbodyDesc);
		
	m_Desc.pTransform = m_pTransformCom;
	m_Desc.eObjectType = OBJECTTYPE::THROW;
	m_Desc.IsGrab  = &m_IsGrabbed;
	m_Desc.IsThrow = &m_IsThrow;
	m_pDetectRigidbodyCom->Set_Desc(&m_Desc);
		
	m_pDetectRigidbodyCom->SetUp_CallBack(COLLIDE_STATE::DURING, [this](_uint iLayer, void* pDesc, const ContactManifold& Manifold) {
		OnCollider_During(iLayer, pDesc, Manifold);
		});

	// DETECT에 감지 될 수 있게 등록.
	RigidbodyDesc = {};
	RigidbodyDesc.eBodyType = CRigidbody::BODY;
	RigidbodyDesc.eShape = SHAPE::BOX;
	RigidbodyDesc.eType = EMotionType::Kinematic;
	RigidbodyDesc.iLayer = ENUM_CLASS(COLLISIONLAYER::INTERACT_THROW);
	RigidbodyDesc.vExtent = _float3(10.f, 10.f, 10.f); 
	XMStoreFloat3(&RigidbodyDesc.vPos, m_pTransformCom->Get_State(STATE::POSITION));

	if (FAILED(Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Rigidbody"),
		TEXT("Com_ThrowRigidbody"), reinterpret_cast<CComponent**>(&m_pThrowRigidbodyCom), &RigidbodyDesc)))
		CRASH("Rigidbody");


	m_Desc.pTransform = m_pTransformCom;
	m_Desc.eObjectType = OBJECTTYPE::THROW;
	m_Desc.IsGrab = &m_IsGrabbed;
	m_Desc.IsThrow = &m_IsThrow;
	m_Desc.ppRefBoneMatrix = &m_pAttachBoneMatrix;
	m_Desc.ppRefWorldMatrix = &m_pAttachWorldMatrix;
	m_pThrowRigidbodyCom->Set_Desc(&m_Desc);

		
	CRigidbody::BOXBODY_DESC BurnRigidboydDesc{};
	XMStoreFloat4(&BurnRigidboydDesc.vQuat, m_pTransformCom->Get_Quaternion());
	BurnRigidboydDesc.eShape = SHAPE::BOX;
	XMStoreFloat3(&BurnRigidboydDesc.vPos, m_pTransformCom->Get_State(STATE::POSITION));
	BurnRigidboydDesc.eType = EMotionType::Kinematic;
	BurnRigidboydDesc.iLayer = ENUM_CLASS(COLLISIONLAYER::THROW);
	BurnRigidboydDesc.vExtent = _float3(2.f, 10.f, 2.f);

	//불타는 벽과 충돌 감지용
	Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Rigidbody"),
		TEXT("Com_CollideRigidbody"), reinterpret_cast<CComponent**>(&m_pCollideRigidbodyCom), &BurnRigidboydDesc);
	m_pCollideRigidbodyCom->IsActivate(false);
}

void CMapObject_Throw::Collide()
{
	m_pDetectRigidbodyCom->IsActivate(false);
	SetActivate(false);
}

void CMapObject_Throw::Graped()
{
	m_pCollideRigidbodyCom->Change_MotionType(EMotionType::Dynamic);
}

void CMapObject_Throw::Calc_CombinedMatrix()
{
	_matrix matResult = XMMatrixIdentity();
	XMStoreFloat4x4(&m_AttachMatrix, matResult);
	if (nullptr == m_pAttachBoneMatrix ||
		nullptr == m_pAttachWorldMatrix)
		return;

	_matrix matBone = XMLoadFloat4x4(m_pAttachBoneMatrix);
	_matrix matWorld = XMLoadFloat4x4(m_pAttachWorldMatrix);
	XMStoreFloat4x4(&m_AttachMatrix, matBone * matWorld);
}

void CMapObject_Throw::Attach_Lerp()
{
	// 1. Position 추출.
	_matrix matAttach = XMLoadFloat4x4(&m_AttachMatrix);

	// 2. vTrans가 TargetPos 대상.
	_vector vTrans = matAttach.r[3];
	if (m_fattachTime >= 1.f)
		m_fattachTime = 1.f;

	// 3. 현재 위치를 LerpPos로 업데이트.
	XMStoreFloat3(&m_vStartPos, m_pTransformCom->Get_State(STATE::POSITION));
	_vector LerpPos = XMVectorLerp(XMLoadFloat3(&m_vStartPos)
		, XMVectorSetY(vTrans, vTrans.m128_f32[1] += 1.f), m_fattachTime * m_fattachTime);
	m_pTransformCom->Set_State(STATE::POSITION, XMVectorSetW(LerpPos, 1.f));

}

void CMapObject_Throw::Attach_Pos()
{
	_matrix matAttach = XMLoadFloat4x4(&m_AttachMatrix);

	// 2. vTrans가 TargetPos 대상.
	//_vector vScale, vRot, vTrans;
	//XMMatrixDecompose(&vScale, &vRot, &vTrans, matAttach);

	_vector vTrans = matAttach.r[3];
	m_pTransformCom->Set_State(STATE::POSITION, 
		XMVectorSetY(vTrans, vTrans.m128_f32[1] += 1.f)
	);
	
}

CMapObject_Throw* CMapObject_Throw::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CMapObject_Throw* pInstance = new CMapObject_Throw(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Create : MapObject_Throw");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CMapObject_Throw::Clone(void* pArg)
{
	CMapObject_Throw* pInstance = new CMapObject_Throw(*this);

	if (FAILED(pInstance->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Create : MapObject_Throw (Clone)");
		Safe_Release(pInstance);
	}

	return pInstance;
}


void CMapObject_Throw::Free()
{
	__super::Free();
	
	m_pAttachBoneMatrix = nullptr;
	m_pAttachWorldMatrix = nullptr;

	Safe_Release(m_pModelCom);
	Safe_Release(m_pCollideRigidbodyCom);
	Safe_Release(m_pDetectRigidbodyCom);
	Safe_Release(m_pThrowRigidbodyCom);
	Safe_Release(m_pShaderCom);
	Safe_Release(m_pGameSystem);
}
