#include "EnginePch.h"
#include "Model.h"
#include "GameInstance.h"

#include "Mesh.h"
#include "ShapeKey.h"
#include "MeshMaterial.h"
#include "Bone.h"
#include "Animation.h"
#include "Channel.h"
#include "ComputeShader.h"
#include "Model_Streaming.h"
#include "BoneReadbackProfiler.h"

const string CModel::kRibPrefix = "Rib_";

CModel::CModel(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CComponent{ pDevice, pContext }
{
}

CModel::CModel(const CModel& Prototype)
	: CComponent{ Prototype },
	m_eType{ Prototype.m_eType },
	m_iNumMeshes{ Prototype.m_iNumMeshes },
	m_Meshes{ Prototype.m_Meshes },
	m_iNumMaterials{ Prototype.m_iNumMaterials },
	m_Materials{ Prototype.m_Materials },
	m_PreTransformMatrix{ Prototype.m_PreTransformMatrix },
	m_vPreRootPosition{ Prototype.m_vPreRootPosition },
	m_RootMatrix{ Prototype.m_RootMatrix },
	m_iRootBoneIndex{ Prototype.m_iRootBoneIndex },
	m_iNumAnimations{ Prototype.m_iNumAnimations },
	m_AnimationNameToIndex{ Prototype.m_AnimationNameToIndex },
	//m_Buffers{ Prototype.m_Buffers },
	//m_SRVs{ Prototype.m_SRVs },
	m_isRibAnimation{ Prototype.m_isRibAnimation },
	m_ShapeKeyNames{ Prototype.m_ShapeKeyNames },
	m_ShapeKeyIndices{ Prototype.m_ShapeKeyIndices },
	m_fPreScale{ Prototype.m_fPreScale },
	m_ConversionMatrix{ Prototype.m_ConversionMatrix }
	//m_pBoundingBox{ Prototype.m_pBoundingBox }
{
	for (auto& pMesh : m_Meshes)
		Safe_AddRef(pMesh);

	for (auto& pMaterial : m_Materials)
		Safe_AddRef(pMaterial);

	for (auto& pBone : Prototype.m_Bones)
		m_Bones.push_back(pBone->Clone());

	for (auto& Pair : Prototype.m_Animations)
		m_Animations.emplace(Pair.first, Pair.second->Clone());

	//  Prototype 생성 Buffer와 Instance 생성 Buffer가 다르기 때문에 nullptr 체크를 해줍니다.
	//for (auto& pBuffer : m_Buffers)
	//{
	//	if (nullptr != pBuffer)
	//		Safe_AddRef(pBuffer);
	//}

	////  SRV는 Prototype, Instance 모두 동일하게 사용.
	//for (auto& pSRV : m_SRVs)
	//{
	//	if (nullptr != pSRV)
	//		Safe_AddRef(pSRV);
	//}

	//// 크기만 지정.
	//m_UAVs.resize(Prototype.m_UAVs.size());

	//// 가중치 배열은 Instance 각각이 소유 => 복제본마다 다를 수 있음.
	//m_ShapeKeyWeights.resize(Prototype.m_ShapeKeyWeights.size(), 0.f);



#ifdef _DEBUG
	m_AnimationNames = Prototype.m_AnimationNames;
#endif


}

const ROOT_MOTION_DELTA& CModel::Get_RootMotionDelta()
{
	return m_RootMotionDelta;
}

_uint CModel::Get_NumBones(_uint iMeshIndex)
{
	if (iMeshIndex >= m_iNumMeshes)
		return 0;

	return m_Meshes[iMeshIndex]->Get_NumBones();;
}

void CModel::Copy_BoneMatrices(_float4x4* pOutMatrices, _uint iMeshIndex)
{

	if (nullptr == pOutMatrices ||
		m_iNumMeshes < iMeshIndex)
		return;

	_uint iNumBones = m_Meshes[iMeshIndex]->Get_NumBones();
	m_Meshes[iMeshIndex]->Copy_BoneMatrices(pOutMatrices, iNumBones);
}


void CModel::Sync_RootNode(CTransform* pOwnerTransform, _float fTimeDelta)
{
	_matrix matWorld = pOwnerTransform->Get_WorldMatrix();
	_matrix ResultMatrix = m_RootMatrix * pOwnerTransform->Get_WorldMatrix();

	pOwnerTransform->Set_WorldMatrix(ResultMatrix);
}

const _float4x4* CModel::Get_BoneMatrixPtr(const _char* pBoneName)
{
	auto iter = find_if(m_Bones.begin(), m_Bones.end(), [&](CBone* pBone)->_bool {
		if (0 == strcmp(pBoneName, pBone->Get_Name()))
			return true;
		return false;
		});

	if (iter == m_Bones.end())
		return nullptr;

	return (*iter)->Get_CombinedTransformationMatrix();
}


const vector<_uint>& CModel::Get_Indices(_uint iIndex)
{
	if (iIndex >= m_iNumMeshes)
		CRASH("Mesh Index Error");
	return m_Meshes[iIndex]->Get_Indices();
}

const vector<_float3>& CModel::Get_VerticesPos(_uint iIndex)
{
	if (iIndex >= m_iNumMeshes)
		CRASH("Mesh Index Error");
	return m_Meshes[iIndex]->Get_VerticesPos();
}

void CModel::Set_ShapeKeyWeight(const _string& strKeyName, _float fWeight)
{
	// 1. 이름으로 인덱스 찾기.
	auto iter = m_ShapeKeyIndices.find(strKeyName);

	if (iter == m_ShapeKeyIndices.end())
		return;

	// 2. 인덱스 가져오기
	_uint iIndex = iter->second;

	// 3. 가중치 배열 갱신.
	if (iIndex < m_ShapeKeyWeights.size())
		m_ShapeKeyWeights[iIndex] = fWeight;
}

void CModel::Set_TrackPosition(const _string& strAnimName, const _float fTrackPosition)
{
	m_Animations[strAnimName]->Set_CurrentTrackPosition(fTrackPosition);
}


#ifdef _DEBUG
void TrajectorySample::SetTransform(const _float4x4& TransformMatrix)
{
	_vector vScale{}, vRotation{}, vTranslation{};
	XMMatrixDecompose(&vScale, &vRotation, &vTranslation, XMLoadFloat4x4(&TransformMatrix));
	XMStoreFloat4(&vQuat, vRotation);
	XMStoreFloat4(&vPosition, vTranslation);
}

TrajectorySample TrajectorySample::TrajectoryLerp(const TrajectorySample& Other, _float fAlpha) const
{

	TrajectorySample Result{};
	return Result;
}

_float* CModel::Get_TrackPositionPtr(const _string& strAnimName)
{
	return m_Animations.at(strAnimName)->Get_TrackPositionPtr();
}

_float CModel::Get_Duration(const _string& strAnimName)
{
	return m_Animations.at(strAnimName)->Get_Duration();
}

HRESULT CModel::Bind_Bone_to_GUI(_int& iBoneIndex, _fmatrix TransformMatrix)
{
	_int iNextBoneIndex = iBoneIndex + 1;
	ImGuiTreeNodeFlags iFlag = 0;
	if ((iNextBoneIndex >= m_Bones.size()) || (m_Bones[iNextBoneIndex]->Get_ParentIndex() != iBoneIndex))
		iFlag |= ImGuiTreeNodeFlags_Leaf;
	else
		iFlag |= (ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_DefaultOpen);

	if (ImGui::TreeNodeEx(m_Bones[iBoneIndex]->Get_Name(), iFlag))
	{
		//Selecting Interaction 
		if (ImGui::IsItemClicked())
		{
			std::cout << "selected : " << m_Bones[iBoneIndex]->Get_Name() << std::endl;
			m_iSelectIndex = iBoneIndex;
		}

		while (iNextBoneIndex < m_Bones.size() && m_Bones[iNextBoneIndex]->Get_ParentIndex() == iBoneIndex)
		{
			Bind_Bone_to_GUI(iNextBoneIndex, TransformMatrix);
		}

		ImGui::TreePop();
	}
	iBoneIndex = iNextBoneIndex;
	return S_OK;
}
void CModel::Render_Gizmo(_fmatrix TransformMatrix)
{
	_fmatrix BoneLocalMatrix = XMLoadFloat4x4(m_Bones[m_iSelectIndex]->Get_CombinedTransformationMatrix());
	m_pGameInstance->Render_Gizmo(BoneLocalMatrix * TransformMatrix);
}

_bool CModel::Find_Animation(const _string& strAnimName)
{
	for (_uint i = 0; i < m_AnimationNames.size(); i++)
	{
		if (m_AnimationNames[i] == strAnimName)
			return true;
	}
	return false;
}

void CModel::Print_ShapeKeyWeights()
{
	_wstring outString = {};

	for (size_t i = 0; i < m_ShapeKeyWeights.size(); i++)
	{
		outString += to_wstring(m_ShapeKeyWeights[i]);
		outString += _wstring(L"\n");
	}

	OutputDebugString(outString.c_str());
}

void CModel::Debug_RootMotionDraw(_fmatrix WorldMatrix)
{
}

#endif // _DEBUG

void CModel::Register_Notify(const _string& strFilePath, const vector<function<void()>>& Functions)
{
	ifstream InputFile(strFilePath);

	json InputData;
	InputFile >> InputData;

	for (auto& Data : InputData)
	{
		_string strAnimName = Data["AnimName"];
		_float fTrackPosition = Data["TrackPosition"];
		_uint iEventID = Data["EventID"];

		auto iter = m_Animations.find(strAnimName);
		if (iter == m_Animations.end())
			return;
		m_Animations.at(strAnimName)->Register_Notify({ fTrackPosition, Functions[iEventID] });
	}

	for (auto& Pair : m_Animations)
		Pair.second->Sort_Notify();
}

void CModel::Register_AllNotifies(const _string& strNotifyFolderPath, function<void(const _wstring&, _bool)> ColliderCallback, function<void(const _wstring&)> EffectCallback, function<void(const _wstring&)> ObjectCallback, function<void(const _string&, _bool)> StateFlagCallback)
{

	for (auto& pair : m_Animations)
	{
		const _string& animName = pair.first;
		CAnimation* pAnimation = pair.second;

		_string filePath = strNotifyFolderPath + "/" + animName + ".json";


		ifstream inputFile(filePath);
		// 1. 열리면?
		if (inputFile.is_open())
		{
			json notifyData;
			inputFile >> notifyData;
			inputFile.close();

			if (notifyData.contains("Notifies") && notifyData["Notifies"].is_array())
			{
				pAnimation->Load_Notify(notifyData["Notifies"], ColliderCallback, EffectCallback, ObjectCallback, StateFlagCallback);
			}
		}

	}

	for (auto& pair : m_Animations)
		pair.second->Sort_AnimNotify();

}

#ifdef _DEBUG
void CModel::Clear_AllNotifies()
{
	for (auto& pair : m_Animations)
	{
		pair.second->Clear_AnimNotifies();
	}
}
#endif // _DEBUG



HRESULT CModel::Initialize_Prototype(MODELTYPE eType, _fmatrix PreTransformMatrix, const _char* pFilePath)
{
	m_eType = eType;
	XMStoreFloat4x4(&m_PreTransformMatrix, PreTransformMatrix);

	//m_fPreScale = 0.01f; // 기본
	m_fPreScale = 1.f; // 기본
	_matrix matConversion = XMMatrixIdentity();

	ifstream InputFile(pFilePath, ios::binary);
	if (false == InputFile.is_open())
	{
		MSG_BOX("Failed Open : Model");
		return E_FAIL;
	}

	if (MODELTYPE::CHARACTER == m_eType)
	{
		matConversion = XMMatrixRotationY(XM_PI) * XMMatrixScaling(1.f, 1.f, 1.f);
		XMStoreFloat4x4(&m_ConversionMatrix, matConversion);
		m_fPreScale = 0.01f; // Character의 경우 Blender에서 Animation 이동량이 0.01배 되서 들어올 것이므로 1.f처리. 
		// Animation의 경우는 FilePath를 이용해서 새로운 FileStream을 생성해서 읽어들임.
		if (FAILED(Ready_CharacterModel(PreTransformMatrix, pFilePath, InputFile)))
			return E_FAIL;
	}
	else if (MODELTYPE::ANIM == m_eType)
	{
		matConversion = XMMatrixRotationX(XM_PIDIV2) * XMMatrixRotationY(XM_PI) * XMMatrixScaling(1.f, 1.f, 1.f);
		XMStoreFloat4x4(&m_ConversionMatrix, matConversion);
		if (FAILED(Ready_AnimModel(PreTransformMatrix, pFilePath, InputFile)))
			return E_FAIL;
	}
	else if (MODELTYPE::ECO == m_eType)
	{

		if (FAILED(Ready_EchoModel(PreTransformMatrix, pFilePath, InputFile)))
			return E_FAIL;
	}
	else
	{
		// Map NonAnim
		if (FAILED(Ready_NonAnimModel(PreTransformMatrix, pFilePath, InputFile)))
			return E_FAIL;
	}



	InputFile.close();

	m_vPreRootRotation = _float4(0.f, 0.f, 0.f, 1.f);
	m_vPreRootPosition = _float4(0.f, 0.f, 0.f, 1.f);
	m_RootMatrix = XMMatrixIdentity();

	
	return S_OK;
}

HRESULT CModel::Initialize_Clone(void* pArg)
{
	//if ((MODELTYPE::ANIM == m_eType) || (MODELTYPE::CHARACTER == m_eType))
	//{
	//	// 0. Copy Resource 용도 멤버 변수 벡터 생성
	//	m_vLocalMatrices.resize(m_Bones.size());

	//	// 1. Rib Prefix 추가 
	//	

	//	// 2. Instance 전용 버퍼 생성.
	//	if (FAILED(Ready_Instance_Buffers()))
	//		return E_FAIL;
	//}

	return S_OK;
}

HRESULT CModel::Bind_Materials(CShader* pShader, const _char* pConstantName, _uint iMeshIndex, TEXTURETYPE eTextureType, _uint iTextureIndex)
{
	if (iMeshIndex >= m_Meshes.size())
		return E_FAIL;

	return m_Materials[m_Meshes[iMeshIndex]->Get_MaterialIndex()]->Bind_Resource(pShader, pConstantName, eTextureType, iTextureIndex);
}

HRESULT CModel::Bind_Materials(CShader* pShader, const _char* pConstantName, _uint iMeshIndex, TEXTURETYPE eTextureType)
{
	if (iMeshIndex >= m_Meshes.size())
		return S_OK;

	return m_Materials[m_Meshes[iMeshIndex]->Get_MaterialIndex()]->Bind_Resource(pShader, pConstantName, eTextureType);

}

HRESULT CModel::Bind_Materials(CDeferredShader* pShader, const _char* pConstantName, _uint iMeshIndex, TEXTURETYPE eTextureType, _uint iTextureIndex, ID3DX11Effect* pEffect)
{
	if (iMeshIndex >= m_Meshes.size())
		return E_FAIL;

	return m_Materials[m_Meshes[iMeshIndex]->Get_MaterialIndex()]->Bind_Resource(pShader, pConstantName, eTextureType, iTextureIndex, pEffect);
}

HRESULT CModel::Bind_Materials(CDeferredShader* pShader, const _char* pConstantName, _uint iMeshIndex, TEXTURETYPE eTextureType, ID3DX11Effect* pEffect)
{
	if (iMeshIndex >= m_Meshes.size())
		return S_OK;

	return m_Materials[m_Meshes[iMeshIndex]->Get_MaterialIndex()]->Bind_Resource(pShader, pConstantName, eTextureType, pEffect);
}

HRESULT CModel::Bind_BoneMatrices(CShader* pShader, const _char* pConstantName, _uint iMeshIndex)
{
	return m_Meshes[iMeshIndex]->Bind_BoneMatrices(pShader, pConstantName, m_Bones);
}


HRESULT CModel::Bind_MorphedResult(CShader* pShader, _uint iMeshIndex, const _char* pConstantName)
{
	if (iMeshIndex >= m_Meshes.size())
		return E_FAIL;

	return m_Meshes[iMeshIndex]->Bind_MorphedResult(pShader, pConstantName);
}

HRESULT CModel::Clear_Materials(CDeferredShader* pShader, const _char* pConstanceName, _uint iMeshIndex, TEXTURETYPE eTextureType, ID3DX11Effect* pEffect)
{
	if (iMeshIndex >= m_Meshes.size())
		return S_OK;

	return m_Materials[m_Meshes[iMeshIndex]->Get_MaterialIndex()]->Clear_Resource(pShader, pConstanceName, eTextureType, pEffect);
}


_bool CModel::Play_Animation_CPU(const _string& strAnimationName, _float fTimeDelta, _float* pTrackPosition, _bool isBlend, _bool isRootMotion, _bool IsRootMotionRotate, _bool IsRootMotionTranslate, _float fRootMotionRate)
{
	// Animation 종료 시, 다음 Animation 처음 KeyFrame과 Blend => 사실상 안쓰고 있음.
	auto iter = m_Animations.find(strAnimationName);
	if (iter == m_Animations.end())
		return false;

	_float fTrackPosition = {};

	if (m_strPreAnimation != strAnimationName)
	{
		m_isChangeAnimation = true;
		m_strPreAnimation = strAnimationName;
		Clear_Animation(strAnimationName);
	}

	// 1. Bone Local Matrix 계산 
	_bool IsAnimationEnd = iter->second->Update_TransformationMatrices_All(fTimeDelta, m_Bones, &fTrackPosition);
	if (nullptr != pTrackPosition)
		*pTrackPosition = fTrackPosition;


	// Root Node Translation 조정
	if (true == isRootMotion)
		Compute_RootAnimation(fRootMotionRate, IsRootMotionRotate, IsRootMotionTranslate);


	// 4. 애니메이션이 끝났다면? Clear 작업을 진행하고 Animation을 클리어해줍니다.
	if (IsAnimationEnd)
	{
		Clear_Animation(strAnimationName);
		return true; // 애니메이션 종료
	}

	for (auto& pBone : m_Bones)
		pBone->Update_CombinedTransformationMatrix(XMLoadFloat4x4(&m_PreTransformMatrix), m_Bones);


	return false;
}

_bool CModel::Play_Animation_CPU(const ANIMATION_PLAY_DESC& playDesc, const ROOTMOTION_DESC& rootMotionDesc, _float fTimeDelta)
{
	auto iter = m_Animations.find(playDesc.strAnimationName);
	if (iter == m_Animations.end())
		return false;

	Handle_PlaybackChange(ETransitionSourceType::CLIP, playDesc.strAnimationName, playDesc.fBlendIn);
	m_fCurBlendOut = playDesc.fBlendOut;
	m_fCurPlaySpeed = playDesc.fSpeed;
	m_pCurBS1 = nullptr;
	m_pCurBS2 = nullptr;

	_float fTrackPosition = {};
	_bool IsAnimationEnd = false;

	_float fWeight = Update_TransitionSource(fTimeDelta);
	if (fWeight < 1.f)
	{
		// 크로스페이드 중
		IsAnimationEnd = iter->second->Update_TrackPosition(playDesc.fSpeed * fTimeDelta, &fTrackPosition);
		iter->second->Blend_AtTrackPosition(fTrackPosition, m_Bones, fWeight);

		// 루트본은 블렌딩에서 제외하고 들어오는 클립의 원본 값으로 덮어쓴다.
		iter->second->Sample_BoneAtTrackPosition(fTrackPosition, m_Bones, m_iRootBoneIndex);
	}
	else
	{
		IsAnimationEnd = iter->second->Update_TransformationMatrices_All(fTimeDelta * playDesc.fSpeed, m_Bones, &fTrackPosition);
	}

	if (nullptr != playDesc.pTrackPosition)
		*playDesc.pTrackPosition = fTrackPosition;
	m_fCurPlayTrackPos = fTrackPosition;

	Compute_RootAnimation(rootMotionDesc.fRate, rootMotionDesc.isRotate, rootMotionDesc.isTranslate, rootMotionDesc.isEnable);

	if (IsAnimationEnd)
	{
		Clear_Animation(playDesc.strAnimationName);
		m_TransitionSource.eType = ETransitionSourceType::NONE; // 종료 시 소스도 정리
		return true;
	}

	for (auto& pBone : m_Bones)
		pBone->Update_CombinedTransformationMatrix(XMLoadFloat4x4(&m_PreTransformMatrix), m_Bones);

	return false;
}

_bool CModel::Play_BlendSpace_CPU(const BLENDSPACE_1D_DESC& desc, const ROOTMOTION_DESC& rootMotionDesc, _float fTimeDelta)
{
	if (nullptr == desc.pParam || desc.Samples.size() < 2)
		return false;

	Handle_PlaybackChange(ETransitionSourceType::BLENDSPACE_1D, desc.Samples[0].strAnimationName, desc.fBlendIn);
	m_fCurBlendOut = desc.fBlendOut;
	m_fCurPlaySpeed = desc.fPlayRate;
	m_pCurBS1 = &desc;
	m_pCurBS2 = nullptr;

	_float fWeight = Update_TransitionSource(fTimeDelta);
	Layer_BlendSpace1D(desc, *desc.pParam, m_fBlendSpacePhase, fWeight, fTimeDelta);

	Compute_RootAnimation(rootMotionDesc.fRate,
		rootMotionDesc.isEnable && rootMotionDesc.isRotate,
		rootMotionDesc.isEnable && rootMotionDesc.isTranslate);

	for (auto& pBone : m_Bones)
		pBone->Update_CombinedTransformationMatrix(XMLoadFloat4x4(&m_PreTransformMatrix), m_Bones);

	return false;
}

_bool CModel::Play_BlendSpace2D_CPU(const BLENDSPACE_2D_DESC& desc, const ROOTMOTION_DESC& rootMotionDesc, _float fTimeDelta)
{
	if (nullptr == desc.pParam || desc.Samples.size() < 3)
		return false;

	Handle_PlaybackChange(ETransitionSourceType::BLENDSPACE_2D, desc.Samples[0].strAnimationName, desc.fBlendIn);
	m_fCurBlendOut = desc.fBlendOut;
	m_fCurPlaySpeed = desc.fPlayRate;
	m_pCurBS1 = nullptr;
	m_pCurBS2 = &desc;

	_float fWeight = Update_TransitionSource(fTimeDelta);
	Layer_BlendSpace2D(desc, desc.pParam->x, desc.pParam->y, m_fBlendSpacePhase, fWeight, fTimeDelta);

	Compute_RootAnimation(rootMotionDesc.fRate,
		rootMotionDesc.isEnable && rootMotionDesc.isRotate,
		rootMotionDesc.isEnable && rootMotionDesc.isTranslate);

	for (auto& pBone : m_Bones)
		pBone->Update_CombinedTransformationMatrix(XMLoadFloat4x4(&m_PreTransformMatrix), m_Bones);

	return false;
}


void CModel::Freeze_TransitionSource()
{
	m_TransitionSource.FrozenPose.resize(m_Bones.size());
	for (size_t i = 0; i < m_Bones.size(); ++i)
		m_TransitionSource.FrozenPose[i] = *m_Bones[i]->Get_TransformationMatrix();
	m_TransitionSource.eType = ETransitionSourceType::FROZEN;
}

void CModel::Handle_PlaybackChange(ETransitionSourceType eNewType, const _string& strNewKey, _float fNewBlendIn)
{
	if (m_eCurPlayType == eNewType && m_strCurPlayKey == strNewKey)
	{
		m_fBlendOverride = -1.f; // 전환 아님 — 대기 중이던 오버라이드는 다음 Play 1회에만 유효하므로 폐기
		return;
	}

	// 크로스페이드 시간 결정: 오버라이드 > min(직전 BlendOut, 새 BlendIn)
	_float fDuration;
	if (m_fBlendOverride >= 0.f)
	{
		fDuration = m_fBlendOverride;
		m_fBlendOverride = -1.f; // 1회 소비
	}
	else
		fDuration = min(m_fCurBlendOut, fNewBlendIn);

	if (ETransitionSourceType::NONE == m_eCurPlayType || fDuration <= 0.f)
	{
		// 최초 재생 또는 스냅 지정 — 크로스페이드 없음
		m_TransitionSource.eType = ETransitionSourceType::NONE;
	}
	else if (ETransitionSourceType::NONE != m_TransitionSource.eType)
	{
		// 인터럽트: 진행 중이던 합성 포즈를 동결 소스로 강등
		Freeze_TransitionSource();
		m_TransitionSource.fElapsed = 0.f;
		m_TransitionSource.fDuration = fDuration;
	}
	else
	{
		// 직전 재생물을 전환 소스로 승격 — 블렌드 동안 계속 재생된다
		m_TransitionSource.eType = m_eCurPlayType;
		m_TransitionSource.fElapsed = 0.f;
		m_TransitionSource.fDuration = fDuration;
		// m_pCurBS1/2는 직전 프레임의 나가는 쪽 Play 호출이 그 프레임에 직접 세팅한 포인터 — 승격 시점에 항상 신선함
		switch (m_eCurPlayType)
		{
		case ETransitionSourceType::CLIP:
			m_TransitionSource.strClip = m_strCurPlayKey;
			m_TransitionSource.fTrackPos = m_fCurPlayTrackPos;
			m_TransitionSource.fSpeed = m_fCurPlaySpeed;
			break;
		case ETransitionSourceType::BLENDSPACE_1D:
			m_TransitionSource.BS1 = *m_pCurBS1; // 샘플 목록 복사 (pParam은 이후 사용 안 함)
			m_TransitionSource.fFrozenParamX = *m_pCurBS1->pParam; // 전환 순간 값으로 고정
			m_TransitionSource.fPhase = m_fBlendSpacePhase;
			break;
		case ETransitionSourceType::BLENDSPACE_2D:
			m_TransitionSource.BS2 = *m_pCurBS2;
			m_TransitionSource.fFrozenParamX = m_pCurBS2->pParam->x;
			m_TransitionSource.fFrozenParamY = m_pCurBS2->pParam->y;
			m_TransitionSource.fPhase = m_fBlendSpacePhase;
			break;
		}
	}

	// 새 재생물 준비
	if (ETransitionSourceType::CLIP == eNewType)
		Clear_Animation(strNewKey); // 트랙 초기화 + 루트모션 기준 리셋 (순간이동 방지 포함)
	else
	{
		// 블렌드스페이스 진입: 위상 초기화 + 루트모션 기준 리셋 (Clear_Animation의 루트 처리와 동일)
		m_fBlendSpacePhase = 0.f;
		m_isChangeAnimation = true;
		m_vPreRootRotation = _float4(0.f, 0.f, 0.f, 1.f);
		m_vPreRootPosition = _float4(0.f, 0.f, 0.f, 1.f);
		m_RootMatrix = XMMatrixIdentity();
	}

	m_eCurPlayType = eNewType;
	m_strCurPlayKey = strNewKey;
}

_float CModel::Update_TransitionSource(_float fTimeDelta)
{
	if (ETransitionSourceType::NONE == m_TransitionSource.eType)
		return 1.f;

	m_TransitionSource.fElapsed += fTimeDelta;
	if (m_TransitionSource.fElapsed >= m_TransitionSource.fDuration)
	{
		m_TransitionSource.eType = ETransitionSourceType::NONE; // 크로스페이드 완료
		return 1.f;
	}

	_float t = Saturate(m_TransitionSource.fElapsed / m_TransitionSource.fDuration);
	_float fWeight = t * t * (3.f - 2.f * t); // 스무스스텝 (기존 곡선 유지)

	// 소스 포즈를 본에 기록(가중치 1 = 덮어쓰기)하고 진행
	switch (m_TransitionSource.eType)
	{
	case ETransitionSourceType::CLIP:
	{
		auto iter = m_Animations.find(m_TransitionSource.strClip);
		if (iter == m_Animations.end())
		{
			// 이론상 불가: 클립 소실 시 현재 포즈를 동결하고 계속 진행 (크래시 방지)
			Freeze_TransitionSource();
			break;
		}
		iter->second->Sample_AtTrackPosition(m_TransitionSource.fTrackPos, m_Bones);
		m_TransitionSource.fTrackPos += iter->second->Get_TickPerSecond() * m_TransitionSource.fSpeed * fTimeDelta;
		break;
	}
	case ETransitionSourceType::BLENDSPACE_1D:
		Layer_BlendSpace1D(m_TransitionSource.BS1, m_TransitionSource.fFrozenParamX,
			m_TransitionSource.fPhase, 1.f, fTimeDelta);
		break;
	case ETransitionSourceType::BLENDSPACE_2D:
		Layer_BlendSpace2D(m_TransitionSource.BS2, m_TransitionSource.fFrozenParamX,
			m_TransitionSource.fFrozenParamY, m_TransitionSource.fPhase, 1.f, fTimeDelta);
		break;
	case ETransitionSourceType::FROZEN:
		for (size_t i = 0; i < m_Bones.size(); ++i)
			m_Bones[i]->Set_TransformationMatrix(XMLoadFloat4x4(&m_TransitionSource.FrozenPose[i]));
		break;
	}

	return fWeight;
}

void CModel::Layer_BlendSpace1D(const BLENDSPACE_1D_DESC& desc, _float fParam, _float& fPhase, _float fLayerWeight, _float fTimeDelta)
{
	if (desc.Samples.size() < 2 || fLayerWeight <= 0.f)
		return;

	// 파라미터를 감싸는 이웃 샘플 2개 탐색 (기존 Play_BlendSpace_CPU와 동일)
	_uint iA = 0;
	_uint iB = 1;
	const _uint iCount = static_cast<_uint>(desc.Samples.size());
	for (_uint i = 0; i + 1 < iCount; ++i)
	{
		if (fParam <= desc.Samples[i + 1].fXParamValue)
		{
			iA = i;
			iB = i + 1;
			break;
		}
		iA = i;
		iB = i + 1;
	}

	auto iterA = m_Animations.find(desc.Samples[iA].strAnimationName);
	auto iterB = m_Animations.find(desc.Samples[iB].strAnimationName);
	if (iterA == m_Animations.end() || iterB == m_Animations.end())
		return;

	_float fRange = desc.Samples[iB].fXParamValue - desc.Samples[iA].fXParamValue;
	_float fU = (fRange > 0.f) ? ((fParam - desc.Samples[iA].fXParamValue) / fRange) : 0.f;
	fU = max(0.f, min(1.f, fU));

	// 위상 동기화: 두 클립의 실제 길이를 가중 보간한 속도로 phase 진행
	_float fDurA = iterA->second->Get_Duration();
	_float fDurB = iterB->second->Get_Duration();
	_float fTickA = iterA->second->Get_TickPerSecond();
	_float fTickB = iterB->second->Get_TickPerSecond();
	_float fRealDurA = (fTickA > 0.f) ? (fDurA / fTickA) : fDurA;
	_float fRealDurB = (fTickB > 0.f) ? (fDurB / fTickB) : fDurB;
	_float fBlendedDur = fRealDurA + fU * (fRealDurB - fRealDurA);
	if (fBlendedDur > 0.f)
		fPhase += (fTimeDelta * desc.fPlayRate) / fBlendedDur;
	if (fPhase >= 1.f)
		fPhase -= 1.f;

	_float fTrackA = fPhase * fDurA;
	_float fTrackB = fPhase * fDurB;

	// 증분 정규화 합성: 기존 본 포즈의 누적 가중치 = 1 - fLayerWeight
	_float wA = fLayerWeight * (1.f - fU);
	_float wB = fLayerWeight * fU;
	_float S = 1.f - fLayerWeight;
	if (wA > 0.f) { iterA->second->Blend_AtTrackPosition(fTrackA, m_Bones, wA / (S + wA)); S += wA; }
	if (wB > 0.f) { iterB->second->Blend_AtTrackPosition(fTrackB, m_Bones, wB / (S + wB)); S += wB; }
}

void CModel::Layer_BlendSpace2D(const BLENDSPACE_2D_DESC& desc, _float fParamX, _float fParamY, _float& fPhase, _float fLayerWeight, _float fTimeDelta)
{
	if (desc.Samples.size() < 3 || fLayerWeight <= 0.f)
		return;

	_float fX = max(-1.f, min(1.f, fParamX));
	_float fY = max(-1.f, min(1.f, fParamY));

	_float sx = (fX >= 0.f) ? 1.f : -1.f;
	_float sy = (fY >= 0.f) ? 1.f : -1.f;
	_float tx = fabsf(fX);
	_float ty = fabsf(fY);

	auto Find = [&](_float x, _float y) -> const BLENDSPACE_SAMPLE* {
		for (auto& s : desc.Samples)
			if (fabsf(s.fXParamValue - x) < 1e-4f && fabsf(s.fYParamValue - y) < 1e-4f)
				return &s;
		return nullptr;
	};
	const BLENDSPACE_SAMPLE* pIdle = Find(0.f, 0.f);
	const BLENDSPACE_SAMPLE* pX = Find(sx, 0.f);
	const BLENDSPACE_SAMPLE* pY = Find(0.f, sy);
	if (!pIdle || !pX || !pY) return;

	auto iterI = m_Animations.find(pIdle->strAnimationName);
	auto iterX = m_Animations.find(pX->strAnimationName);
	auto iterY = m_Animations.find(pY->strAnimationName);
	if (iterI == m_Animations.end() || iterX == m_Animations.end() || iterY == m_Animations.end())
		return;

	_float fSum = tx + ty;
	_float wX, wY, wIdle;
	if (fSum < 1e-5f) { wIdle = 1.f; wX = 0.f; wY = 0.f; }
	else { wX = (tx * tx) / fSum; wY = (ty * ty) / fSum; wIdle = 1.f - wX - wY; }

	auto RealDur = [](CAnimation* pAnim) {
		_float dur = pAnim->Get_Duration(), tick = pAnim->Get_TickPerSecond();
		return (tick > 0.f) ? (dur / tick) : dur;
	};
	_float fBlendedDur = wIdle * RealDur(iterI->second) + wX * RealDur(iterX->second) + wY * RealDur(iterY->second);
	if (fBlendedDur > 0.f)
		fPhase += (fTimeDelta * desc.fPlayRate) / fBlendedDur;
	if (fPhase >= 1.f)
		fPhase -= 1.f;

	_float trackI = fPhase * iterI->second->Get_Duration();
	_float trackX = fPhase * iterX->second->Get_Duration();
	_float trackY = fPhase * iterY->second->Get_Duration();

	// 증분 정규화 합성 (기존 본 포즈 누적 가중치 = 1 - fLayerWeight)
	wIdle *= fLayerWeight;
	wX *= fLayerWeight;
	wY *= fLayerWeight;
	_float S = 1.f - fLayerWeight;
	if (wIdle > 0.f) { iterI->second->Blend_AtTrackPosition(trackI, m_Bones, wIdle / (S + wIdle)); S += wIdle; }
	if (wX > 0.f)    { iterX->second->Blend_AtTrackPosition(trackX, m_Bones, wX / (S + wX));       S += wX; }
	if (wY > 0.f)    { iterY->second->Blend_AtTrackPosition(trackY, m_Bones, wY / (S + wY));       S += wY; }
}

//_bool CModel::Play_Animation_CPU(const _string& strAnimationName, const ANIMATION_PLAY_DESC& playDesc, const ROOTMOTION_DESC& rootMotionDesc)
//{
//	// Animation 종료 시, 다음 Animation 처음 KeyFrame과 Blend => 사실상 안쓰고 있음.
//	auto iter = m_Animations.find(strAnimationName);
//	if (iter == m_Animations.end())
//		return false;
//
//	_float fTrackPosition = {};
//
//	if (m_strPreAnimation != strAnimationName)
//	{
//		m_isChangeAnimation = true;
//		m_strPreAnimation = strAnimationName;
//		Clear_Animation(strAnimationName);
//	}
//
//	// Bone Local Matrix 계산 
//	_bool IsAnimationEnd = iter->second->Update_TransformationMatrices_All(playDesc.fTimeDelta, m_Bones, &fTrackPosition);
//	if (nullptr != playDesc.pTrackPosition)
//		*playDesc.pTrackPosition = fTrackPosition;
//
//
//	// Root Node Translation 조정
//	if (true == rootMotionDesc.isEnable)
//		Compute_RootAnimation(rootMotionDesc.fRate, rootMotionDesc.isRotate, rootMotionDesc.isTranslate);
//
//
//	// 애니메이션이 끝났다면? Clear 작업을 진행하고 Animation을 클리어해줍니다.
//	if (IsAnimationEnd)
//	{
//		Clear_Animation(strAnimationName);
//		return true; // 애니메이션 종료
//	}
//
//	for (auto& pBone : m_Bones)
//		pBone->Update_CombinedTransformationMatrix(XMLoadFloat4x4(&m_PreTransformMatrix), m_Bones);
//
//
//	return false;
//}

_bool CModel::Play_Animation_GPU(CComputeShader* pComputeShaderCom, const ANIMATION_PLAY_DESC& playDesc, const ROOTMOTION_DESC& rootMotionDesc, _float fTimeDelta)
{
	CAnimation* pAnimation = Get_AnimationOrNull(playDesc.strAnimationName);
	if (nullptr == pComputeShaderCom || nullptr == playDesc.pTrackPosition || nullptr == pAnimation)
		return false;

	HandleAnimationChange(playDesc.strAnimationName);

	// 현재 트랙 포지션을 가져옵니다. (트랙 포지션은 애니메이션 클래스에서 갱신을 받습니다.)
	_bool isAnimationEnd = Update_TrackPosition(pAnimation, playDesc.pTrackPosition, playDesc.fSpeed * fTimeDelta);
	// 뼈_행렬 계산 부분을 Compute Shader에 전달 및 갱신.
	FetchLocalMatrices_FromCompute(pComputeShaderCom, *playDesc.pTrackPosition, playDesc.strAnimationName);
	// Root Motion 조정.
	if (true == rootMotionDesc.isEnable)
		Compute_RootAnimation(rootMotionDesc.fRate, rootMotionDesc.isRotate, rootMotionDesc.isTranslate);
	else
		m_RootMatrix = XMMatrixIdentity();
	
	for (_uint i = 0; i < m_Bones.size(); i++)
		m_Bones[i]->Update_CombinedTransformationMatrix(XMLoadFloat4x4(&m_PreTransformMatrix), m_Bones);

	//  애니메이션이 끝났다면? Clear 작업을 진행하고 Animation을 클리어해줍니다.
	if (isAnimationEnd)
	{
		Clear_Animation(playDesc.strAnimationName);
		return true; // 애니메이션 종료
	}

	return false;
}

_bool CModel::Play_Animation_GPU(CComputeShader* pComputeShaderCom, CComputeShader* pMorphComputeShaderCom, const ANIMATION_PLAY_DESC& playDesc, const ROOTMOTION_DESC& rootMotionDesc, _float fTimeDelta)
{
	CAnimation* pAnimation = Get_AnimationOrNull(playDesc.strAnimationName);
	if (nullptr == pComputeShaderCom || nullptr == pMorphComputeShaderCom ||
		nullptr == playDesc.pTrackPosition || nullptr == pAnimation)
		return false;

	HandleAnimationChange(playDesc.strAnimationName);
	_bool isAnimationEnd = Update_TrackPosition(pAnimation, playDesc.pTrackPosition, playDesc.fSpeed * fTimeDelta);

	// 뼈_행렬 계산 부분을 Compute Shader에 전달 및 갱신.
	FetchLocalMatrices_FromCompute(pComputeShaderCom, *playDesc.pTrackPosition, playDesc.strAnimationName);

	// Morph 애니메이션 갱신.
	Update_MorphAnimation(pAnimation, pMorphComputeShaderCom, playDesc.fSpeed * fTimeDelta, playDesc.isFacial);

	// Root Motion 조정.
	if (true == rootMotionDesc.isEnable)
		Compute_RootAnimation(rootMotionDesc.fRate, rootMotionDesc.isRotate, rootMotionDesc.isTranslate);
	else
		m_RootMatrix = XMMatrixIdentity();


	for (_uint i = 0; i < m_Bones.size(); i++)
		m_Bones[i]->Update_CombinedTransformationMatrix(XMLoadFloat4x4(&m_PreTransformMatrix), m_Bones);

	// 애니메이션이 끝났다면? Clear 작업을 진행하고 Animation을 클리어해줍니다.
	if (isAnimationEnd)
	{
		Clear_Animation(playDesc.strAnimationName);
		return true; // 애니메이션 종료
	}

	return false;
}

_bool CModel::Play_NonRibAnimation_GPU(CComputeShader* pComputeShaderCom, const ANIMATION_PLAY_DESC& playDesc, const ROOTMOTION_DESC& rootMotionDesc, _float fTimeDelta)
{
	CAnimation* pAnimation = Get_AnimationOrNull(playDesc.strAnimationName);
	if (nullptr == pComputeShaderCom || nullptr == playDesc.pTrackPosition || nullptr == pAnimation)
		return false;

	HandleAnimationChange(playDesc.strAnimationName);
	_bool isAnimationEnd = Update_TrackPosition(pAnimation, playDesc.pTrackPosition, playDesc.fSpeed * fTimeDelta);

	// 뼈_행렬 계산 부분을 Compute Shader에 전달 및 갱신.
	FetchLocalMatrices_FromComputeNonRib(pComputeShaderCom, *playDesc.pTrackPosition, playDesc.strAnimationName);

	// Root Motion 설정.
	if (true == rootMotionDesc.isEnable)
		Compute_RootAnimation(rootMotionDesc.fRate, rootMotionDesc.isRotate, rootMotionDesc.isTranslate);
	else
		m_RootMatrix = XMMatrixIdentity();


	//  Combined는 한번만.
	for (_uint i = 0; i < m_Bones.size(); i++)
		m_Bones[i]->Update_CombinedTransformationMatrix(XMLoadFloat4x4(&m_PreTransformMatrix), m_Bones);

	// 애니메이션이 끝났다면? Clear 작업을 진행하고 Animation을 클리어해줍니다.
	if (isAnimationEnd)
	{
		Clear_Animation(playDesc.strAnimationName);
		return true;
	}

	return false;
}

_bool CModel::Play_FlyAnimation_GPU(class CComputeShader* pComputeShaderCom, const ANIMATION_PLAY_DESC& playDesc, const ROOTMOTION_DESC& rootMotionDesc, const GPU_BLEND_INFO& gpuBlendInfo, _float fTimeDelta)
{

	CAnimation* pAnimation = Get_AnimationOrNull(playDesc.strAnimationName);
	if (nullptr == pComputeShaderCom || nullptr == playDesc.pTrackPosition || nullptr == pAnimation)
		return false;

	HandleAnimationChange(playDesc.strAnimationName);
	_bool isAnimationEnd = Update_TrackPosition(pAnimation, playDesc.pTrackPosition, playDesc.fSpeed * fTimeDelta);

	// 뼈_행렬 계산 부분을 Compute Shader에 전달 및 갱신.
	FetchLocalMatrices_FromComputeFly(pComputeShaderCom, *playDesc.pTrackPosition, playDesc.strAnimationName, gpuBlendInfo);

	// Root Motion 설정.
	if (true == rootMotionDesc.isEnable)
		Compute_RootAnimation(rootMotionDesc.fRate, rootMotionDesc.isRotate, rootMotionDesc.isTranslate);
	else
		m_RootMatrix = XMMatrixIdentity();

	//Combined는 한번만.
	for (_uint i = 0; i < m_Bones.size(); i++)
		m_Bones[i]->Update_CombinedTransformationMatrix(XMLoadFloat4x4(&m_PreTransformMatrix), m_Bones);

	if (isAnimationEnd)
	{
		Clear_Animation(playDesc.strAnimationName);
		return true;
	}


	return false;
}

_bool CModel::Play_FlyAnimation_GPU(CComputeShader* pComputeShaderCom, CComputeShader* pMorphComputeShaderCom, const ANIMATION_PLAY_DESC& playDesc, const ROOTMOTION_DESC& rootMotionDesc, const GPU_BLEND_INFO& gpuBlendInfo, _float fTimeDelta)
{
	CAnimation* pAnimation = Get_AnimationOrNull(playDesc.strAnimationName);
	if (nullptr == pComputeShaderCom || nullptr == pMorphComputeShaderCom ||
		nullptr == playDesc.pTrackPosition || nullptr == pAnimation)
		return false;

	HandleAnimationChange(playDesc.strAnimationName);
	_bool isAnimationEnd = Update_TrackPosition(pAnimation, playDesc.pTrackPosition, playDesc.fSpeed * fTimeDelta);

	// 3. 뼈_행렬 계산 부분을 Compute Shader에 전달 및 갱신.
	FetchLocalMatrices_FromComputeFly(pComputeShaderCom, *playDesc.pTrackPosition, playDesc.strAnimationName, gpuBlendInfo);

	// Morph 애니메이션 갱신.
	Update_MorphAnimation(pAnimation, pMorphComputeShaderCom, playDesc.fSpeed * fTimeDelta, playDesc.isFacial);

	// Root Node Translation 조정
	if (true == rootMotionDesc.isEnable)
		Compute_RootAnimation(rootMotionDesc.fRate, rootMotionDesc.isRotate, rootMotionDesc.isTranslate);
	else
		m_RootMatrix = XMMatrixIdentity();


	// Combined는 한번만.
	for (_uint i = 0; i < m_Bones.size(); i++)
		m_Bones[i]->Update_CombinedTransformationMatrix(XMLoadFloat4x4(&m_PreTransformMatrix), m_Bones);

	// 애니메이션이 끝났다면? Clear 작업을 진행하고 Animation을 클리어해줍니다.
	if (isAnimationEnd)
	{
		Clear_Animation(playDesc.strAnimationName);
		return true; // 애니메이션 종료
	}


	return false;
}

void CModel::Clear_Animation(const _string& strAnimationName, _float fTrackPosition)
{
	if (strAnimationName == "")
		return;

	m_isChangeAnimation = true;
	m_vPreRootRotation = _float4(0.f, 0.f, 0.f, 1.f);
	m_vPreRootPosition = _float4(0.f, 0.f, 0.f, 1.f);
	m_RootMatrix = XMMatrixIdentity();

	auto iter = m_Animations.find(strAnimationName);
	if (iter == m_Animations.end())
		return;

	m_Animations.at(strAnimationName)->Reset_Status();
	m_Animations.at(strAnimationName)->Set_CurrentTrackPosition(fTrackPosition);
	fill(m_ShapeKeyWeights.begin(), m_ShapeKeyWeights.end(), 0.0f);

	// 스테이징 버퍼 상태 리셋
	m_iCurStagingFlip = 0;
	m_bIsStagingFilled = false;

}

const _float4x4* CModel::Get_BoneMatrixPtr(_uint iBoneIndex)
{
	return m_Bones[iBoneIndex]->Get_CombinedTransformationMatrix();
}

void CModel::Update_BoneMatrix_Map()
{
	for (auto& pBone : m_Bones)
		pBone->Update_CombinedTransformationMatrix(XMLoadFloat4x4(&m_PreTransformMatrix), m_Bones);
}

// Render 단위가 Mesh 단위.
HRESULT CModel::Render(_uint iMeshIndex)
{
	if (FAILED(m_Meshes[iMeshIndex]->Bind_Resources()))
		return E_FAIL;
	m_Meshes[iMeshIndex]->Render();

	return S_OK;
}



HRESULT CModel::Render(_uint iMeshIndex, ID3D11DeviceContext* pDC)
{
	if (FAILED(m_Meshes[iMeshIndex]->Bind_Resources(pDC)))
		return E_FAIL;
	m_Meshes[iMeshIndex]->Render(pDC);

	return S_OK;
}

#ifdef _DEBUG
_bool CModel::Is_Picked(const _fvector& vRayPos, const _fvector& vRayDir, _float* pDistance)
{
	_float fMin = FLT_MAX;
	for (_uint i = 0; i < m_iNumMeshes; ++i)
	{
		_float fDistance = {};
		if (true == m_Meshes[i]->Is_Picked(vRayPos, vRayDir, &fDistance) && fMin > fDistance)
			fMin = fDistance;
	}

	if (fMin < FLT_MAX)
	{
		*pDistance = fMin;
		return true;
	}

	return false;
}
#endif

void CModel::FetchLocalMatrices_FromCompute(CComputeShader* pComputeShaderCom, _float fTrackPosition, const _string& strAnimationName)
{
	if (nullptr == pComputeShaderCom)
		return;

	// 상수 버퍼 업데이트
	Update_AnimConstantBuffer(strAnimationName, fTrackPosition);
	
	// 리소스 바인딩
	Bind_AnimationResource(pComputeShaderCom);

	// Compute Shader 실행 (Dispatch)
	_uint iNumBones = static_cast<_uint>(m_Bones.size());
	_uint iGroupCount = (iNumBones + (pComputeShaderCom->Get_ThreadInfo().iThreadGroupX - 1)) /
		pComputeShaderCom->Get_ThreadInfo().iThreadGroupX;
	pComputeShaderCom->Dispatch(iGroupCount, 1, 1);
	
	// Dobule Buffering을 활용하여 GPU가 계산한 최신 로컬 행렬을 CPU로 가져옵니다.
	Readback_BoneMatrices();
}

void CModel::FetchLocalMatrices_FromComputeFly(CComputeShader* pComputeShaderCom, _float fTrackPosition, const _string& strAnimationName, const GPU_BLEND_INFO& gpuBlendInfo)
{
	ASSERT_CRASH(pComputeShaderCom);
	Update_FlyAnimConstantBuffer(gpuBlendInfo, strAnimationName, fTrackPosition);
	Bind_FlyAnimationResource(pComputeShaderCom);
	_uint iNumBones = static_cast<_uint>(m_Bones.size());
	_uint iGroupCount = (iNumBones + (pComputeShaderCom->Get_ThreadInfo().iThreadGroupX - 1)) / pComputeShaderCom->Get_ThreadInfo().iThreadGroupX;
	pComputeShaderCom->Dispatch(iGroupCount, 1, 1);
	Readback_BoneMatrices();
}

void CModel::FetchLocalMatrices_FromComputeNonRib(CComputeShader* pComputeShaderCom, _float fTrackPosition, const _string& strAnimationName)
{
	ASSERT_CRASH(pComputeShaderCom);

	// 상수 버퍼 업데이트
	Update_NonRibAnimConstantBuffer(strAnimationName, fTrackPosition);
	// 리소스 바인딩
	Bind_AnimationResource(pComputeShaderCom);

	// Compute Shader 실행 (Dispatch)
	_uint iNumBones = static_cast<_uint>(m_Bones.size());
	_uint iGroupCount = (iNumBones + (pComputeShaderCom->Get_ThreadInfo().iThreadGroupX - 1)) / pComputeShaderCom->Get_ThreadInfo().iThreadGroupX;
	pComputeShaderCom->Dispatch(iGroupCount, 1, 1);

	Readback_BoneMatrices();
}


CAnimation* CModel::Get_AnimationOrNull(const string& strAnimationName)
{
	auto it = m_Animations.find(strAnimationName);

	return (it == m_Animations.end()) ? nullptr : it->second;
}

_float3 CModel::Get_RootMotionTotalDisplacement(const _string& strAnimationName)
{
	CAnimation* pAnim = Get_AnimationOrNull(strAnimationName);
	if (!pAnim) return _float3{};

	for (CChannel* pChannel : pAnim->Get_Channels())
	{
		if (pChannel->Get_BoneIndex() != m_iRootBoneIndex)
			continue;

		const auto& kf = pChannel->Get_Keyframes();
		if (kf.size() < 2)
			return _float3{};

		_vector vFirst     = XMLoadFloat3(&kf.front().vTranslation);
		_vector vLast      = XMLoadFloat3(&kf.back().vTranslation);
		_vector vConverted = XMVector3Transform(vLast - vFirst, XMLoadFloat4x4(&m_ConversionMatrix));
		vConverted         = XMVectorScale(vConverted, m_fPreScale);

		_float3 vOut{};
		XMStoreFloat3(&vOut, vConverted);
		return vOut;
	}
	return _float3{};
}

_float CModel::Get_AnimProgress(const _string& strAnimName)
{
	return m_Animations[strAnimName]->Get_AnimProgress();
}

void CModel::Compute_RootAnimation(_float fRootMotionRate, _bool isRootMotionRotation, _bool isRootMotionTranslate, _bool IsRootMotionEnable)
{
	// PreTransform의 스케일 추출
	// 1. GPU 계산 로컬본 전체 가져오기.
	_vector vScale{}, vRotation{}, vTranslation{};
	XMMatrixDecompose(&vScale, &vRotation, &vTranslation, XMLoadFloat4x4(m_Bones[m_iRootBoneIndex]->Get_TransformationMatrix()));

	// 2. 스켈레톤의 루트 본은 '제자리'에서 (Scale, Rotation)만 하도록
	_matrix RootBoneLocalMatrix = XMMatrixAffineTransformation(vScale, XMVectorSet(0.f, 0.f, 0.f, 1.f), vRotation, XMVectorSet(0.f, 0.f, 0.f, 1.f));
	m_Bones[m_iRootBoneIndex]->Set_TransformationMatrix(RootBoneLocalMatrix);


	_matrix matConversion = XMLoadFloat4x4(&m_ConversionMatrix);
	_vector qConversion = XMQuaternionRotationMatrix(matConversion);

	// 현재 프레임의 T, R을 '엔진 좌표계'로 변환
	_vector vConvertedTranslation = XMVector3Transform(vTranslation, matConversion);
	_vector vConvertedRotation = XMQuaternionMultiply(qConversion, vRotation);

	// 이동 변화량 계산
	_vector vLocalTranslate = vConvertedTranslation - XMLoadFloat4(&m_vPreRootPosition);

	// 회전 변화량 계산
	_vector vRotationDelta = XMQuaternionMultiply(vConvertedRotation, XMQuaternionInverse(XMLoadFloat4(&m_vPreRootRotation)));

	// RootMotion 회전 껐으면 0으로.
	if (!isRootMotionRotation)
		vRotationDelta = XMQuaternionIdentity();

	if (!isRootMotionTranslate)
		vLocalTranslate = XMVectorSet(0.f, 0.f, 0.f, 1.f);


	// 애니메이션 변경 시 순간이동 방지
	if (true == m_isChangeAnimation)
	{  
		m_isChangeAnimation = false;
		vLocalTranslate = XMVectorSet(0.f, 0.f, 0.f, 1.f);
		vRotationDelta = XMQuaternionIdentity();
	}

	/* 애니메이션 구동 이후에 보정할 Delta값을 가지고 옵니다. */
	if (!IsRootMotionEnable)
	{
		XMStoreFloat3(&m_RootMotionDelta.vTranslate, vLocalTranslate * m_fPreScale);
		XMStoreFloat4(&m_RootMotionDelta.qRotation, vRotationDelta);
	}
		
	

	if (IsRootMotionEnable)
	{
		m_RootMatrix = XMMatrixAffineTransformation(
			XMVectorSet(1.f, 1.f, 1.f, 1.f), // 스케일 델타 (없음)
			XMVectorSet(0.f, 0.f, 0.f, 1.f), // 원점
			vRotationDelta,                  // 회전 델타
			vLocalTranslate * m_fPreScale * fRootMotionRate // 이동 델타
		);
	}
	else
	{
		m_RootMatrix = XMMatrixIdentity();
	}

	// 다음 프레임을 위해 '변환된' T, R 값을 저장합니다.
	XMStoreFloat4(&m_vPreRootPosition, vConvertedTranslation);
	XMStoreFloat4(&m_vPreRootRotation, vConvertedRotation);
}

void CModel::HandleAnimationChange(const _string& strAnimationName)
{
	if (m_strPreAnimation == strAnimationName)
		return;

	m_isChangeAnimation = true;
	m_strPreAnimation = strAnimationName;
	Clear_Animation(strAnimationName);
}

void CModel::Update_MorphAnimation(CAnimation* pAnimation, CComputeShader* pMorphComputeShaderCom, _float fTimeDelta, _bool isFacial)
{
	if (m_eType != MODELTYPE::CHARACTER || !isFacial) 
		return;
	if (nullptr == m_Buffers[BUFFER_MORPH_WEIGHT] || nullptr == m_SRVs[SRV_MORPH_WEIGHT])
		return;
	
	pAnimation->Update_MorphWeights(fTimeDelta, m_ShapeKeyWeights);

	D3D11_MAPPED_SUBRESOURCE MappedSubResource;
	if (SUCCEEDED(m_pContext->Map(m_Buffers[BUFFER_MORPH_WEIGHT], 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedSubResource)))
	{
		memcpy(MappedSubResource.pData, m_ShapeKeyWeights.data(), sizeof(_float) * m_ShapeKeyWeights.size());
		m_pContext->Unmap(m_Buffers[BUFFER_MORPH_WEIGHT], 0);
	}

	for (auto& pMesh : m_Meshes)
	{
		if (false == pMesh->HasMorphTargets())
			continue;

		pMesh->Compute_Morph(pMorphComputeShaderCom, m_SRVs[SRV_MORPH_WEIGHT]);
	}
}

_bool CModel::Update_TrackPosition(CAnimation* pAnimation, _float* pTrackPosition, _float fTimeDelta)
{
	_float fTrackPosition = 0.f;

	_bool isAnimationEnd = pAnimation->Update_TrackPosition(fTimeDelta, &fTrackPosition);
	*pTrackPosition = fTrackPosition;

	return isAnimationEnd;
}

void CModel::Update_NonRibAnimConstantBuffer(const _string& strAnimationName, _float fTrackPosition)
{
	// 1. 상수 버퍼(CB) 업데이트
	D3D11_MAPPED_SUBRESOURCE MappedSubResource;
	m_pContext->Map(m_Buffers[BUFFER_ANIM_INFOCB], 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedSubResource);

	// 2. 애니메이션 정보 CB 구조체에 정보 바인딩.
	ANIMATION_CBINFO* pAnimCBInfo = static_cast<ANIMATION_CBINFO*>(MappedSubResource.pData);
	pAnimCBInfo->fTrackPosition = fTrackPosition;
	pAnimCBInfo->iAnimindex = m_AnimationNameToIndex[strAnimationName]; /* 애니메이션 이름(strAnimationName)에 해당하는 인덱스 */;
	pAnimCBInfo->iRibAnimUsed = 0;
	pAnimCBInfo->iRibbonAnimIndex = 0;

	m_pContext->Unmap(m_Buffers[BUFFER_ANIM_INFOCB], 0);
}

void CModel::Update_AnimConstantBuffer(const _string& strAnimationName, _float fTrackPosition)
{
	//  상수 버퍼(CB) 업데이트
	D3D11_MAPPED_SUBRESOURCE MappedSubResource;
	m_pContext->Map(m_Buffers[BUFFER_ANIM_INFOCB], 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedSubResource);

	_bool IsRibAnimUsed = false;
	// 애니메이션 정보 CB 구조체 => 현재 AnimIndex와 TrackPosition을 소유.
	ANIMATION_CBINFO* pAnimCBInfo = static_cast<ANIMATION_CBINFO*>(MappedSubResource.pData);
	pAnimCBInfo->fTrackPosition = fTrackPosition;
	pAnimCBInfo->iAnimindex = m_AnimationNameToIndex[strAnimationName]; /* 애니메이션 이름(strAnimationName)에 해당하는 인덱스 */;
	pAnimCBInfo->iRibAnimUsed = 0;
	pAnimCBInfo->iRibbonAnimIndex = 0;

	// Ribbon 애니메이션이 존재한다면 정보 바인딩
	_string strRibAnimationName = kRibPrefix + strAnimationName;
	auto iter = m_Animations.find(strRibAnimationName);
	if (iter == m_Animations.end())
	{
		pAnimCBInfo->iRibAnimUsed = 0;
	}
	else
	{
		pAnimCBInfo->iRibAnimUsed = 1;
		pAnimCBInfo->iRibbonAnimIndex = m_AnimationNameToIndex[strRibAnimationName];
	}

	m_pContext->Unmap(m_Buffers[BUFFER_ANIM_INFOCB], 0);
}

void CModel::Update_FlyAnimConstantBuffer(const GPU_BLEND_INFO& gpuBlendInfo, const _string& strAnimationName, _float fTrackPosition)
{
	// 1. 상수 버퍼(CB) 업데이트
	D3D11_MAPPED_SUBRESOURCE MappedSubResource;
	m_pContext->Map(m_Buffers[BUFFER_ANIM_INFOFLYCB], 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedSubResource);

	_bool IsRibAnimUsed = false;
	// 애니메이션 정보 CB 구조체 => 현재 AnimIndex와 TrackPosition을 소유.
	ANIMATIONFLY_CBINFO* pAnimCBInfo = static_cast<ANIMATIONFLY_CBINFO*>(MappedSubResource.pData);
	pAnimCBInfo->fTrackPosition = fTrackPosition;
	pAnimCBInfo->iAnimindex = m_AnimationNameToIndex[strAnimationName]; /* 애니메이션 이름(strAnimationName)에 해당하는 인덱스 */;
	pAnimCBInfo->iRibAnimUsed = 0;
	pAnimCBInfo->iRibbonAnimIndex = 0;

	// 2. Ribbon 애니메이션이 존재한다면 정보 바인딩
	_string strRibAnimationName = kRibPrefix + strAnimationName;
	auto iter = m_Animations.find(strRibAnimationName);
	if (iter == m_Animations.end())
	{
		pAnimCBInfo->iRibAnimUsed = 0;
	}
	else
	{
		pAnimCBInfo->iRibAnimUsed = 1;
		pAnimCBInfo->iRibbonAnimIndex = m_AnimationNameToIndex[strRibAnimationName];
	}

	// Blend Enabled가 True 라면? 정보 바인딩.
	if (gpuBlendInfo.IsBlendEnabled)
	{
		pAnimCBInfo->IsBlendEnabled = gpuBlendInfo.IsBlendEnabled;
		pAnimCBInfo->fBlendParamLR = gpuBlendInfo.fBlendParamLR;
		pAnimCBInfo->fBlendParamDU = gpuBlendInfo.fBlendParamDU;
		pAnimCBInfo->fPadding = 0.f;

		// RL
		pAnimCBInfo->iClipIndexL = GetSafeIndex(gpuBlendInfo.strClipxL);
		pAnimCBInfo->iClipIndexMidLR = GetSafeIndex(gpuBlendInfo.strClipMidLR);
		pAnimCBInfo->iClipIndexR = GetSafeIndex(gpuBlendInfo.strClipxR);
		pAnimCBInfo->iWeightClipLR = GetSafeIndex(gpuBlendInfo.strWeightClipLR);

		// UD
		pAnimCBInfo->iClipIndexD = GetSafeIndex(gpuBlendInfo.strClipxD);
		pAnimCBInfo->iClipIndexMidDU = GetSafeIndex(gpuBlendInfo.strClipMidDU);
		pAnimCBInfo->iClipIndexU = GetSafeIndex(gpuBlendInfo.strClipxU);
		pAnimCBInfo->iWeightClipDU = GetSafeIndex(gpuBlendInfo.strWeightClipDU);
	}
	else //
	{
		// HLSL에서 쓰레기 값을 읽는 것을 방지하기 위해
		// 나머지 블렌드 관련 필드들을 0으로 초기화합니다.
		pAnimCBInfo->fBlendParamLR = 0.f;
		pAnimCBInfo->fBlendParamDU = 0.f;

		pAnimCBInfo->iClipIndexL = 0;
		pAnimCBInfo->iClipIndexMidLR = 0;
		pAnimCBInfo->iClipIndexR = 0;
		pAnimCBInfo->iWeightClipLR = 0;

		pAnimCBInfo->iClipIndexD = 0;
		pAnimCBInfo->iClipIndexMidDU = 0;
		pAnimCBInfo->iClipIndexU = 0;
		pAnimCBInfo->iWeightClipDU = 0;
	}

	m_pContext->Unmap(m_Buffers[BUFFER_ANIM_INFOFLYCB], 0);
}

void CModel::Bind_AnimationResource(CComputeShader* pComputeShaderCom)
{
	//  Compute Shader에 리소스 바인딩
	pComputeShaderCom->Set_SRV("g_AllKeyframes", m_SRVs[SRV_KEY_FRAME]);
	pComputeShaderCom->Set_SRV("g_AllAnimInfos", m_SRVs[SRV_ANIM_INFO]);
	pComputeShaderCom->Set_SRV("g_ChannelInfos", m_SRVs[SRV_BONE_CHANNEL]);
	pComputeShaderCom->Set_UAV("g_OutLocalMatrices", m_UAVs[UAV_FINAL_BONEMATRIX]);
	pComputeShaderCom->Set_ConstantBuffer("AnimationInfoCB", m_Buffers[BUFFER_ANIM_INFOCB]);

	
}

void CModel::Bind_FlyAnimationResource(CComputeShader* pComputeShaderCom)
{
	// 3. Compute Shader에 리소스 바인딩
	pComputeShaderCom->Set_SRV("g_AllKeyframes", m_SRVs[SRV_KEY_FRAME]);
	pComputeShaderCom->Set_SRV("g_AllAnimInfos", m_SRVs[SRV_ANIM_INFO]);
	pComputeShaderCom->Set_SRV("g_ChannelInfos", m_SRVs[SRV_BONE_CHANNEL]);
	pComputeShaderCom->Set_UAV("g_OutLocalMatrices", m_UAVs[UAV_FINAL_BONEMATRIX]);
	pComputeShaderCom->Set_ConstantBuffer("AnimationInfoCB", m_Buffers[BUFFER_ANIM_INFOFLYCB]);
}

//#define USE_DOUBLE_BUFFERING

//void CModel::Readback_BoneMatrices()
//{
//	_uint iNumBones = static_cast<_uint>(m_Bones.size());
//
//	// 쓰기 인덱스와 읽기 인덱스 계산 0번 Write, 1번 Read
//	_uint iWriteIdx = BUFFER_STAGING_0 + m_iCurStagingFlip;
//	const _bool bUseDoubleBuffering = BoneReadbackProfiler::GetUseDoubleBuffering();
//
//	/*_uint iReadIdx = bUseDoubleBuffering
//		? BUFFER_STAGING_0 + ((m_iCurStagingFlip + 1) % 2)
//		: iWriteIdx;*/
//	//_uint iReadIdx = iWriteIdx;
//	_uint iReadIdx = BUFFER_STAGING_0;
//	
//
//	// GPU의 출력 버퍼(m_pFinalBoneMatrix_Buffer) 내용을 Staging 버퍼로 복사합니다.
//	//m_pContext->CopyResource(m_Buffers[iWriteIdx], m_Buffers[BUFFER_FINAL_BONEMATRIX]);
//	m_pContext->CopyResource(m_Buffers[iReadIdx], m_Buffers[BUFFER_FINAL_BONEMATRIX]);
//
//	// 이전 프레임에 복사 명령을 내려두었던 Staging[iReadIdx]를 Map하여 CPU로 가져옵니다.
//	if (m_bIsStagingFilled)
//	{
//
//		D3D11_MAPPED_SUBRESOURCE Mapped;
//		// Map 직전 — 순수 GPU 대기 시간만 측정
//		LARGE_INTEGER t0{}, t1{};
//		//QueryPerformanceCounter(&t0);
//		const HRESULT hr = m_pContext->Map(m_Buffers[iReadIdx], 0, D3D11_MAP_READ, 0, &Mapped);
//		//const HRESULT hr = m_pContext->Map(m_Buffers[iWriteIdx], 0, D3D11_MAP_READ, 0, &Mapped);
//		//QueryPerformanceCounter(&t1);
//
//		
//
//		// Map 대기 시간 누적
//		/*LARGE_INTEGER freq{};
//		QueryPerformanceFrequency(&freq);
//		if (freq.QuadPart != 0)
//		{
//			const double ms = static_cast<double>(t1.QuadPart - t0.QuadPart) * 1000.0
//				/ static_cast<double>(freq.QuadPart);
//			BoneReadbackProfiler::AddReadbackMs(ms);
//		}*/
//
//		if (SUCCEEDED(hr))
//		{
//			memcpy(m_vLocalMatrices.data(), Mapped.pData, sizeof(_float4x4) * iNumBones);
//			m_pContext->Unmap(m_Buffers[iReadIdx], 0);
//
//			// m_Bones 배열에 로컬 행렬 적용
//			for (size_t i = 0; i < iNumBones; ++i)
//				m_Bones[i]->Set_TransformationMatrix(XMLoadFloat4x4(&m_vLocalMatrices[i]));
//		}
//	}
//	else
//		m_bIsStagingFilled = true;
//
//	// 다음 프레임을 위한 인덱스 교체
//	m_iCurStagingFlip = (m_iCurStagingFlip + 1) % 2;
//}

void CModel::Readback_BoneMatrices()
{
	_uint iNumBones = static_cast<_uint>(m_Bones.size());
	_uint iReadIdx = BUFFER_STAGING_0;


	// GPU의 출력 버퍼(m_pFinalBoneMatrix_Buffer) 내용을 Staging 버퍼로 복사합니다.
	m_pContext->CopyResource(m_Buffers[iReadIdx], m_Buffers[BUFFER_FINAL_BONEMATRIX]);

	// 이전 프레임에 복사 명령을 내려두었던 Staging[iReadIdx]를 Map하여 CPU로 가져옵니다.
	D3D11_MAPPED_SUBRESOURCE Mapped;
	LARGE_INTEGER t0{}, t1{};
	const HRESULT hr = m_pContext->Map(m_Buffers[iReadIdx], 0, D3D11_MAP_READ, 0, &Mapped);


	if (SUCCEEDED(hr))
	{
		memcpy(m_vLocalMatrices.data(), Mapped.pData, sizeof(_float4x4) * iNumBones);
		m_pContext->Unmap(m_Buffers[iReadIdx], 0);

		// m_Bones 배열에 로컬 행렬 적용
		for (size_t i = 0; i < iNumBones; ++i)
			m_Bones[i]->Set_TransformationMatrix(XMLoadFloat4x4(&m_vLocalMatrices[i]));
	}

	// 다음 프레임을 위한 인덱스 교체
	m_iCurStagingFlip = (m_iCurStagingFlip + 1) % 2;
}

//void CModel::Readback_BoneMatrices()
//{
//	LARGE_INTEGER t0{}, t1{};
//	QueryPerformanceCounter(&t0);
//
//	_uint iNumBones = static_cast<_uint>(m_Bones.size());
//	// 쓰기 인덱스와 읽기 인덱스 계산 0번 Write, 1번 Read
//	_uint iWriteIdx = BUFFER_STAGING_0 + m_iCurStagingFlip;
//	const _bool bUseDoubleBuffering = BoneReadbackProfiler::GetUseDoubleBuffering();
//	_uint iReadIdx = bUseDoubleBuffering
//		? BUFFER_STAGING_0 + ((m_iCurStagingFlip + 1) % 2)
//		: iWriteIdx;
//
//	// GPU의 출력 버퍼(m_pFinalBoneMatrix_Buffer) 내용을 Staging 버퍼로 복사합니다.
//	m_pContext->CopyResource(m_Buffers[iWriteIdx], m_Buffers[BUFFER_FINAL_BONEMATRIX]);
//
//	
//	{
//		// 이전 프레임에 복사 명령을 내려두었던 Staging[iReadIdx]를 Map하여 CPU로 가져옵니다.
//		if (m_bIsStagingFilled)
//		{
//			ID3DUserDefinedAnnotation* pAnnotation;
//			m_pContext->QueryInterface(__uuidof(pAnnotation), (void**)&pAnnotation);
//
//			D3D11_MAPPED_SUBRESOURCE Mapped;
//			// 1프레임 전의 버퍼이므로 이미 복사가 끝나있어 CPU 대기 시간이 거의 없습니다.
//
//			pAnnotation->BeginEvent(L"AnimReadback_Map");
//			if (SUCCEEDED(m_pContext->Map(m_Buffers[iReadIdx], 0, D3D11_MAP_READ, 0, &Mapped)))
//			{
//				memcpy(m_vLocalMatrices.data(), Mapped.pData, sizeof(_float4x4) * iNumBones);
//				m_pContext->Unmap(m_Buffers[iReadIdx], 0);
//
//				// 8. m_Bones 배열에 로컬 행렬 적용
//				for (size_t i = 0; i < iNumBones; ++i)
//					m_Bones[i]->Set_TransformationMatrix(XMLoadFloat4x4(&m_vLocalMatrices[i]));
//			}
//
//			pAnnotation->EndEvent();
//
//		}
//		else
//			m_bIsStagingFilled = true;
//	}
//	
//
//	// 다음 프레임을 위한 인덱스 교체
//	m_iCurStagingFlip = (m_iCurStagingFlip + 1) % 2;
//
//	QueryPerformanceCounter(&t1);
//
//	LARGE_INTEGER freq{};
//	QueryPerformanceFrequency(&freq);
//	if (freq.QuadPart != 0)
//	{
//		const double ms = static_cast<double>(t1.QuadPart - t0.QuadPart) * 1000.0
//			/ static_cast<double>(freq.QuadPart);
//		BoneReadbackProfiler::AddReadbackMs(ms);
//	}
//}

_uint CModel::GetSafeIndex(const _string& strAnimName)
{
	auto iter = m_AnimationNameToIndex.find(strAnimName);

	if (iter != m_AnimationNameToIndex.end())
		return m_AnimationNameToIndex.at(strAnimName);

	return 0;
}





HRESULT CModel::Ready_NonAnimModel(_fmatrix PreTransformMatrix, const _char* pFilePath, ifstream& InputFile)
{
	if (FAILED(Ready_Mesh(InputFile)))
		return E_FAIL;

	if (FAILED(Ready_Material(pFilePath)))
		return E_FAIL;

	return S_OK;
}

HRESULT CModel::Ready_AnimModel(_fmatrix PreTransformMatrix, const _char* pFilePath, ifstream& InputFile)
{
	if (FAILED(Ready_Bone(InputFile, -1)))
		return E_FAIL;

	//if (FAILED(Ready_Animation(pFilePath)))
	//	return E_FAIL;

	if (FAILED(Ready_Mesh(InputFile)))
		return E_FAIL;

	if (FAILED(Ready_Material(pFilePath)))
		return E_FAIL;

	// 순서 테스트. => 마지막으로 돌려도 무방함.
	if (FAILED(Ready_Animation(pFilePath)))
		return E_FAIL;

	return S_OK;
}

HRESULT CModel::Ready_CharacterModel(_fmatrix PreTransformMatrix, const _char* pFilePath, ifstream& InputFile)
{

#pragma region 1. 파일 스트림에서 데이터 로드
	// 1. Ready Bone 동일.
	if (FAILED(Ready_Bone(InputFile, -1)))
		return E_FAIL;

	// 2. ShapeKeyMesh는 다름.
	if (FAILED(Ready_ShapeKeyMesh(InputFile)))
		return E_FAIL;

	// 3. Matreial은 동일.
	if (FAILED(Ready_Material(pFilePath)))
		return E_FAIL;

	// 4. ShapeKey 인덱스 정리 =>
	if (FAILED(Organize_ShapeKeyIndices()))
		return E_FAIL;

	// 4. Animation Import가 다름. => 별개의 파일.
	if (FAILED(Ready_MorphAnimation(pFilePath)))
		return E_FAIL;

	InputFile.close();
#pragma endregion


#pragma region 2. Mesh들에게 MorphBuffer 생성 명령 => 한번만 호출되어야함.
	if (FAILED(Ready_Mesh_MorphBuffers()))
		return E_FAIL;
#pragma endregion


	return S_OK;
}

HRESULT CModel::Ready_EchoModel(_fmatrix PreTransformMatrix, const _char* pFilePath, ifstream& InputFile)
{
	if (FAILED(Ready_Bone(InputFile, -1)))
		return E_FAIL;

	if (FAILED(Ready_Mesh(InputFile)))
		return E_FAIL;

	if (FAILED(Ready_Material(pFilePath)))
		return E_FAIL;

	return S_OK;
}


HRESULT CModel::Ready_Bone(ifstream& InputFile, _int iParentIndex)
{
	_uint iNumChild = {};
	InputFile.read(reinterpret_cast<_char*>(&iNumChild), sizeof(_uint));
	_uint iLength = {};
	InputFile.read(reinterpret_cast<_char*>(&iLength), sizeof(_uint));
	_char szName[MAX_PATH] = {};
	InputFile.read(szName, iLength);
	_float4x4 TransformationMatrix = {};
	InputFile.read(reinterpret_cast<_char*>(&TransformationMatrix), sizeof(_float4x4));
	XMStoreFloat4x4(&TransformationMatrix, XMMatrixTranspose(XMLoadFloat4x4(&TransformationMatrix)));

	CBone* pBone = CBone::Create(szName, XMLoadFloat4x4(&TransformationMatrix), iParentIndex);
	if (nullptr == pBone)
		return E_FAIL;

	m_Bones.push_back(pBone);

	_int iIndex = m_Bones.size() - 1;
	// Root Bone Index 저장
	//if (0 == strcmp(szName, "Root") || 0 == strcmp(szName,"mixamorig:Hips"))
	if (0 == strcmp(szName, "Root"))
		m_iRootBoneIndex = iIndex;

	for (size_t i = 0; i < iNumChild; ++i)
	{
		if (FAILED(Ready_Bone(InputFile, iIndex)))
			return E_FAIL;
	}

	return S_OK;
}

HRESULT CModel::Ready_Mesh(ifstream& InputFile)
{
	InputFile.read(reinterpret_cast<_char*>(&m_iNumMeshes), sizeof(_uint));

	for (size_t i = 0; i < m_iNumMeshes; ++i)
	{
		CMesh* pMesh = CMesh::Create(m_pDevice, m_pContext, m_eType, m_Bones, XMLoadFloat4x4(&m_PreTransformMatrix), InputFile);
		ASSERT_CRASH(pMesh);
		m_Meshes.push_back(pMesh);
	}
	return S_OK;
}

HRESULT CModel::Ready_ShapeKeyMesh(ifstream& InputFile)
{
	// 1. Mesh 개수 저장.
	InputFile.read(reinterpret_cast<_char*>(&m_iNumMeshes), sizeof(_uint));

	// 2. Mesh 개수 만큼 순회돌면서 Mesh 생성.
	for (size_t i = 0; i < m_iNumMeshes; ++i)
	{
		CMesh* pMesh = CMesh::Create(m_pDevice, m_pContext, m_eType, m_Bones, XMLoadFloat4x4(&m_PreTransformMatrix), InputFile);
		ASSERT_CRASH(pMesh);
		m_Meshes.push_back(pMesh);
	}

	return S_OK;
}

// ShapeKeyIndex를 활용해서 어떤 정보들을 정리할건지?
HRESULT CModel::Organize_ShapeKeyIndices()
{
	m_ShapeKeyIndices.clear();
	m_ShapeKeyWeights.clear();
	m_ShapeKeyNames.clear();

	_uint iCurrentGlobalIndex = 0;

	// 모든 메쉬 순회 => 명조의 경우에는 Mesh가 하나 뿐이므로 하나만 봐도됨 모든 메쉬가 공통적인 ShapeKey 정보를 소유한다?
	// 그럼에도 모든 메쉬를 순회해야하는 이유 => 모든 Mesh의 Key에 동일한 전역 인덱스를 심어주기 위해서?

	if (m_Meshes.size() < 1)
		return E_FAIL;

	for (auto& pMesh : m_Meshes)
	{
		for (auto& pKey : pMesh->Get_ShapeKeys())
		{
			string strName = pKey->Get_Name();

			// 1. 인덱스 부여 로직 (Map 이용)
			auto iter = m_ShapeKeyIndices.find(strName);
			_uint iAssignedIndex = 0;

			// 2. 전역 인덱스 부여.
			if (iter == m_ShapeKeyIndices.end())
			{
				// 처음 발견된 이름 -> 전역 인덱스 발급.
				iAssignedIndex = iCurrentGlobalIndex++;
				m_ShapeKeyIndices.emplace(strName, iAssignedIndex); // 메쉬의 ShapeKey Index => 0, 1, 2, 3, 4, 5
				m_ShapeKeyWeights.emplace_back(0.f); // 가중치 배열도 늘려줌
				m_ShapeKeyNames.emplace_back(strName); // 애니메이션에서 해당 목록을 보고 인덱스를 탐색.
			}
			else
			{
				// 이미 등록된 이름 -> 기존 번호표 사용
				iAssignedIndex = iter->second;
			}

			// 3. 쉐이프 키에  => 전역 인덱스 심어주기.
			pKey->Set_GlobalWeightIndex(iAssignedIndex);
		}

	}

	return S_OK;
}

// 반드시 Mesh가 생성된 이후에 생성해야합니다.
HRESULT CModel::Ready_Mesh_MorphBuffers()
{
	if (m_eType == MODELTYPE::CHARACTER)
	{
		for (auto& pMesh : m_Meshes)
		{
			if (FAILED(pMesh->Ready_SharedBuffers_ForMorph()))
				return E_FAIL;

		}
	}

	return S_OK;
}

HRESULT CModel::Ready_Material(const _char* pFilePath)
{
	_char szMaterialFilePath[MAX_PATH] = {};
	_char szMaterialDrivePath[MAX_PATH] = {};
	_char szMaterialDirPath[MAX_PATH] = {};
	_char szMaterialFileName[MAX_PATH] = {};
	_splitpath_s(pFilePath, szMaterialDrivePath, MAX_PATH, szMaterialDirPath, MAX_PATH, szMaterialFileName, MAX_PATH, nullptr, 0);

	strcpy_s(szMaterialFilePath, szMaterialDrivePath);
	strcat_s(szMaterialFilePath, szMaterialDirPath);
	strcat_s(szMaterialFilePath, "Mat/");
	strcat_s(szMaterialFilePath, szMaterialFileName);
	strcat_s(szMaterialFilePath, ".json");

	ifstream MaterialFile(szMaterialFilePath);
	if (false == MaterialFile.is_open())
	{
		MSG_BOX("Failed Open : Material");
		return E_FAIL;
	}

	json MaterialsData;
	MaterialFile >> MaterialsData;

	m_iNumMaterials = MaterialsData["NumMaterial"];

	for (auto& MaterialData : MaterialsData["Materials"])
	{
		CMeshMaterial* pMeshMaterial = CMeshMaterial::Create(m_pDevice, m_pContext, szMaterialFilePath, MaterialData);
		if (nullptr == pMeshMaterial)
			return E_FAIL;
		m_Materials.push_back(pMeshMaterial);
	}

	MaterialFile.close();

	return S_OK;
}

HRESULT CModel::Ready_Animation(const _char* pFilePath)
{
	_char szDrivePath[MAX_PATH] = {};
	_char szDirPath[MAX_PATH] = {};
	_char szFileName[MAX_PATH] = {};

	_splitpath_s(pFilePath, szDrivePath, MAX_PATH, szDirPath, MAX_PATH, szFileName, MAX_PATH, nullptr, 0);

	_char szFilePath[MAX_PATH] = {};
	strcpy_s(szFilePath, szDrivePath);
	strcat_s(szFilePath, szDirPath);
	strcat_s(szFilePath, "Animation/");
	strcat_s(szFilePath, szFileName);
	strcat_s(szFilePath, "_Anim.dat");

	ifstream AnimationFile(szFilePath, ios::binary);

	AnimationFile.read(reinterpret_cast<_char*>(&m_iNumAnimations), sizeof(_uint));

	for (size_t i = 0; i < m_iNumAnimations; ++i)
	{
		CAnimation* pAnimation = CAnimation::Create(AnimationFile, m_Bones);
		if (nullptr == pAnimation)
			return E_FAIL;
		m_Animations.emplace(pAnimation->Get_Name(), pAnimation);
#ifdef _DEBUG
		m_AnimationNames.push_back(pAnimation->Get_Name());
#endif
	}

	AnimationFile.close();


	// Compute Shader 계산을 위한 Animation Index 저장.

	_uint iAnimIdx = 0;
	m_AnimationNameToIndex.clear();
	for (auto& pair : m_Animations)
		m_AnimationNameToIndex.emplace(pair.first, iAnimIdx++);


	return S_OK;
}

HRESULT CModel::Ready_MorphAnimation(const _char* pFilePath)
{
	_uint iNumAnimations = {};
	_uint iNumMorphAnimations = {};
#pragma region 두번 불러오는 방법. MORPH ANIMATION은?

	if (MODELTYPE::CHARACTER == m_eType)
	{
		_char szMorphDrivePath[MAX_PATH] = {};
		_char szMorphDirPath[MAX_PATH] = {};
		_char szMorphFileName[MAX_PATH] = {};

		_splitpath_s(pFilePath, szMorphDrivePath, MAX_PATH, szMorphDirPath, MAX_PATH, szMorphFileName, MAX_PATH, nullptr, 0);

		_char szMorphFilePath[MAX_PATH] = {};
		strcpy_s(szMorphFilePath, szMorphDrivePath);
		strcat_s(szMorphFilePath, szMorphDirPath);
		strcat_s(szMorphFilePath, "Animation/");
		strcat_s(szMorphFilePath, szMorphFileName);
		strcat_s(szMorphFilePath, "_MorphAnim.dat");


		// 새로운 파일 스트림 열기.
		ifstream MorphAnimationFile(szMorphFilePath, ios::binary);


		MorphAnimationFile.read(reinterpret_cast<_char*>(&iNumMorphAnimations), sizeof(_uint));

		for (size_t i = 0; i < iNumMorphAnimations; ++i)
		{
			CAnimation* pAnimation = CAnimation::Create(MorphAnimationFile, m_Bones, MODELTYPE::CHARACTER);
			if (nullptr == pAnimation)
				return E_FAIL;
			m_Animations.emplace(pAnimation->Get_Name(), pAnimation);
			pAnimation->Bind_MorphChannels(m_ShapeKeyNames);
#ifdef _DEBUG
			m_AnimationNames.push_back(pAnimation->Get_Name());
#endif
		}

		MorphAnimationFile.close();
	}


#pragma endregion

#pragma region 기존 애니메이션 영역.
	_char szDrivePath[MAX_PATH] = {};
	_char szDirPath[MAX_PATH] = {};
	_char szFileName[MAX_PATH] = {};

	_splitpath_s(pFilePath, szDrivePath, MAX_PATH, szDirPath, MAX_PATH, szFileName, MAX_PATH, nullptr, 0);

	_char szFilePath[MAX_PATH] = {};
	strcpy_s(szFilePath, szDrivePath);
	strcat_s(szFilePath, szDirPath);
	strcat_s(szFilePath, "Animation/");
	strcat_s(szFilePath, szFileName);
	strcat_s(szFilePath, "_Anim.dat");


	// 새로운 파일 스트림 열기.
	ifstream AnimationFile(szFilePath, ios::binary);

	AnimationFile.read(reinterpret_cast<_char*>(&iNumAnimations), sizeof(_uint));

	for (size_t i = 0; i < iNumAnimations; ++i)
	{
		CAnimation* pAnimation = CAnimation::Create(AnimationFile, m_Bones, MODELTYPE::ANIM);
		if (nullptr == pAnimation)
			return E_FAIL;
		m_Animations.emplace(pAnimation->Get_Name(), pAnimation);
		/*pAnimation->Bind_MorphChannels(m_ShapeKeyNames);*/
#ifdef _DEBUG
		m_AnimationNames.push_back(pAnimation->Get_Name());
#endif
	}

	AnimationFile.close();
#pragma endregion



	m_iNumAnimations = iNumAnimations + iNumMorphAnimations;


	// Compute Shader 계산을 위한 Animation Index 저장.

	_uint iAnimIdx = 0;
	m_AnimationNameToIndex.clear();
	for (auto& pair : m_Animations)
		m_AnimationNameToIndex.emplace(pair.first, iAnimIdx++);

	return S_OK;
}

// Prototype에서 Shared Buffers 생성.
HRESULT CModel::Ready_Shared_Buffers()
{
	ASSERT_CRASH(m_pDevice);

	// 애니메이션, 캐릭터 모델이 아니면 생성하지 않음.

	HRESULT hr = S_OK;

	m_Buffers.resize(BUFFER_END);
	m_SRVs.resize(SRV_END);
	m_UAVs.resize(UAV_END);

	//  --- 1. 데이터 수집을 위한 vector 준비 ---
	vector<ANIMINFO>        vAllAnimInfos;		  // Depth1
	vector<GPU_CHANNELINFO> vAllChannelBoneInfos; // Depth2
	vector<GPU_KEYFRAME>    vAllKeyframes;        // Depth3

	// Depth1에 대한 설정.
	for (const auto& Pair : m_Animations)
	{
		CAnimation* pAnimation = Pair.second;
		ANIMINFO animInfo = {};

		// 시작 인덱스, 개수, 지속시간.
		animInfo.iStartChannelIndexOffset = static_cast<_uint>(vAllChannelBoneInfos.size()); // 순차 탐색 AnimInfo에서 0부터 재생.
		animInfo.iNumChannels = static_cast<_uint>(pAnimation->Get_Channels().size());  // 모든 채널의 개수
		animInfo.fDuration = pAnimation->Get_Duration();

		// Depth2에 대한 설정.
		for (const auto& pChannel : pAnimation->Get_Channels())
		{
			// 시작 키프레임(누적 인덱스), 키프레임 개수, 채널이 관리하는 뼈 인덱스
			GPU_CHANNELINFO channelInfo = {};
			channelInfo.iStartKeyframeOffset = static_cast<_uint>(vAllKeyframes.size());
			channelInfo.iNumKeyframes = pChannel->Get_NumKeyframes();
			channelInfo.iBoneIndex = pChannel->Get_BoneIndex();

			// Depth3에 대한 설정.
			for (const auto& keyframe : pChannel->Get_Keyframes())
			{
				// 키프레임에 대한 정보 복사. Scale, Rotation, Translation, 트랙 위치.
				GPU_KEYFRAME gpuKeyframe = {};
				gpuKeyframe.vScale = _float4(keyframe.vScale.x, keyframe.vScale.y, keyframe.vScale.z, 1.f);
				gpuKeyframe.vRotation = keyframe.vRotation;
				gpuKeyframe.vTranslation = _float4(keyframe.vTranslation.x, keyframe.vTranslation.y, keyframe.vTranslation.z, 1.f);
				gpuKeyframe.fTrackPosition = keyframe.fTrackPosition;
				vAllKeyframes.push_back(gpuKeyframe);
			}
			vAllChannelBoneInfos.push_back(channelInfo);
		}
		vAllAnimInfos.push_back(animInfo);
	}


	for (const auto& Pair : m_Animations)
	{
		CAnimation* pAnimation = Pair.second;

		// GPU Buffer를 만든 객체들은 모든 키프레임 채널들을 제거한다?
		// pAnimation->Release_Channels();
	}



	// --- 2. 수집된 데이터로 실제 GPU 버퍼 생성 ---

	D3D11_BUFFER_DESC bufferDesc = {};
	// 2-1. 애니메이션 키프레임 정보 버퍼.
	bufferDesc.ByteWidth = sizeof(GPU_KEYFRAME) * static_cast<_uint>(vAllKeyframes.size());
	bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	bufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	bufferDesc.StructureByteStride = sizeof(GPU_KEYFRAME);
	D3D11_SUBRESOURCE_DATA subresourceData = { vAllKeyframes.data() };
	hr = m_pDevice->CreateBuffer(&bufferDesc, &subresourceData, &m_Buffers[BUFFER_KEY_FRAME]);
	if (FAILED(hr)) return E_FAIL;
	hr = m_pDevice->CreateShaderResourceView(m_Buffers[BUFFER_KEY_FRAME], nullptr, &m_SRVs[SRV_KEY_FRAME]);
	if (FAILED(hr)) return E_FAIL;


#ifdef _DEBUG
	//if (m_eType == MODELTYPE::CHARACTER)
	//	cout << "CHARACTER GPU_KEYFRAME BYTE : " << bufferDesc.ByteWidth << endl;
#endif // _DEBUG


	// 2-2. 애니메이션 정보 버퍼 (g_AllAnimInfos)
	bufferDesc.ByteWidth = sizeof(ANIMINFO) * static_cast<_uint>(vAllAnimInfos.size());
	bufferDesc.StructureByteStride = sizeof(ANIMINFO);
	subresourceData.pSysMem = vAllAnimInfos.data();
	hr = m_pDevice->CreateBuffer(&bufferDesc, &subresourceData, &m_Buffers[BUFFER_ANIM_INFO]);
	if (FAILED(hr)) return E_FAIL;
	hr = m_pDevice->CreateShaderResourceView(m_Buffers[BUFFER_ANIM_INFO], nullptr, &m_SRVs[SRV_ANIM_INFO]);
	if (FAILED(hr)) return E_FAIL;

#ifdef _DEBUG
	//if (m_eType == MODELTYPE::CHARACTER)
	//	cout << "Character ANIMINFO BYTE : " << bufferDesc.ByteWidth << endl;
#endif // _DEBUG

	// 2-3. 뼈(채널)별 정보 버퍼 (g_ChannelInfos)
	bufferDesc.ByteWidth = sizeof(GPU_CHANNELINFO) * static_cast<_uint>(vAllChannelBoneInfos.size());
	bufferDesc.StructureByteStride = sizeof(GPU_CHANNELINFO);
	subresourceData.pSysMem = vAllChannelBoneInfos.data();
	hr = m_pDevice->CreateBuffer(&bufferDesc, &subresourceData, &m_Buffers[BUFFER_BONE_CHANNEL]);
	if (FAILED(hr)) return E_FAIL;
	hr = m_pDevice->CreateShaderResourceView(m_Buffers[BUFFER_BONE_CHANNEL], nullptr, &m_SRVs[SRV_BONE_CHANNEL]);
	if (FAILED(hr)) return E_FAIL;

#ifdef _DEBUG
	//if (m_eType == MODELTYPE::CHARACTER)
	//	cout << "Character GPU_CHANNELINFO BYTE : " << bufferDesc.ByteWidth << endl;
#endif // _DEBUG

	return S_OK;
}

HRESULT CModel::Ready_Instance_Buffers()
{
	HRESULT hr = S_OK;
	D3D11_BUFFER_DESC bufferDesc = {};
	// 2-4. 최종 로컬 행렬 출력(Output) 버퍼 (g_OutLocalMatrices)
	bufferDesc = {};
	bufferDesc.ByteWidth = sizeof(_float4x4) * static_cast<_uint>(m_Bones.size());
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
	bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	bufferDesc.StructureByteStride = sizeof(_float4x4);
	hr = m_pDevice->CreateBuffer(&bufferDesc, nullptr, &m_Buffers[BUFFER_FINAL_BONEMATRIX]);
	if (FAILED(hr)) return E_FAIL;
	hr = m_pDevice->CreateUnorderedAccessView(m_Buffers[BUFFER_FINAL_BONEMATRIX], nullptr, &m_UAVs[UAV_FINAL_BONEMATRIX]);
	if (FAILED(hr)) return E_FAIL;
	hr = m_pDevice->CreateShaderResourceView(m_Buffers[BUFFER_FINAL_BONEMATRIX], nullptr, &m_SRVs[SRV_FINAL_BONEMATRIX]);
	if (FAILED(hr)) return E_FAIL;

	// 2-5. 매 프레임 업데이트할 상수 버퍼 (AnimationInfo)
	ZeroMemory(&bufferDesc, sizeof(D3D11_BUFFER_DESC));
	bufferDesc.ByteWidth = sizeof(ANIMATION_CBINFO);
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	hr = m_pDevice->CreateBuffer(&bufferDesc, nullptr, &m_Buffers[BUFFER_ANIM_INFOCB]);
	if (FAILED(hr)) return E_FAIL;



	// 2-5. 매 프레임 업데이트할 상수 버퍼 (ANIMATIONFLY_CBINFO)
	ZeroMemory(&bufferDesc, sizeof(D3D11_BUFFER_DESC));
	bufferDesc.ByteWidth = sizeof(ANIMATIONFLY_CBINFO);
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	hr = m_pDevice->CreateBuffer(&bufferDesc, nullptr, &m_Buffers[BUFFER_ANIM_INFOFLYCB]);
	if (FAILED(hr)) return E_FAIL;

	// 2-6. GPU -> CPU 복사를 위한 Staging 버퍼 0, 1
	ZeroMemory(&bufferDesc, sizeof(D3D11_BUFFER_DESC));
	bufferDesc.ByteWidth = sizeof(_float4x4) * static_cast<_uint>(m_Bones.size());
	bufferDesc.Usage = D3D11_USAGE_STAGING;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	hr = m_pDevice->CreateBuffer(&bufferDesc, nullptr, &m_Buffers[BUFFER_STAGING_0]);
	if (FAILED(hr)) return E_FAIL;

	hr = m_pDevice->CreateBuffer(&bufferDesc, nullptr, &m_Buffers[BUFFER_STAGING_1]);
	if (FAILED(hr)) return E_FAIL;

	if (MODELTYPE::CHARACTER == m_eType)
	{
		if (FAILED(Ready_MorphInstance_Buffers()))
			return E_FAIL;
	}

	return S_OK;
}

// MorphInstance Buffer 호출.
HRESULT CModel::Ready_MorphInstance_Buffers()
{
	if (m_ShapeKeyWeights.empty())
		return E_FAIL;

	D3D11_BUFFER_DESC BufferDesc;
	ZeroMemory(&BufferDesc, sizeof(D3D11_BUFFER_DESC));
	BufferDesc.ByteWidth = sizeof(_float) * static_cast<_uint>(m_ShapeKeyWeights.size());
	BufferDesc.Usage = D3D11_USAGE_DYNAMIC; // 매 프레임 CPU -> GPU 복사
	BufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	BufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	BufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	BufferDesc.StructureByteStride = sizeof(_float);

	// m_Buffers에 공간이 없다면 enum 추가 필요
	if (FAILED(m_pDevice->CreateBuffer(&BufferDesc, nullptr, &m_Buffers[BUFFER_MORPH_WEIGHT])))
		return E_FAIL;

	D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc;
	ZeroMemory(&SRVDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
	SRVDesc.Format = DXGI_FORMAT_UNKNOWN;
	SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	SRVDesc.Buffer.NumElements = static_cast<_uint>(m_ShapeKeyWeights.size());

	if (FAILED(m_pDevice->CreateShaderResourceView(m_Buffers[BUFFER_MORPH_WEIGHT], &SRVDesc, &m_SRVs[SRV_MORPH_WEIGHT])))
		return E_FAIL;

	// 각 Mesh들도 Instance Buffer 생성하게 호출
	for (auto& pMesh : m_Meshes)
		pMesh->Ready_InstanceBuffers_ForMorph();

	return S_OK;
}

CModel* CModel::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, MODELTYPE eType, _fmatrix PreTransformMatrix, const _char* pFilePath)
{
	CModel* pInstance = new CModel(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype(eType, PreTransformMatrix, pFilePath)))
	{
		MSG_BOX("Failed to Create : Model");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CComponent* CModel::Clone(void* pArg)
{
	CModel* pClone = new CModel(*this);

	if (FAILED(pClone->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Create : Model (Clone)");
		Safe_Release(pClone);
	}

	return pClone;
}

void CModel::Free()
{
	__super::Free();

	for (auto& pBone : m_Bones)
		Safe_Release(pBone);
	m_Bones.clear();

	for (auto& pPair : m_Animations)
		Safe_Release(pPair.second);
	m_Animations.clear();

	for (auto& pMesh : m_Meshes)
		Safe_Release(pMesh);
	m_Meshes.clear();

	for (auto& pMaterial : m_Materials)
		Safe_Release(pMaterial);
	m_Materials.clear();


	/* GPU Buffer 내용 제거 */
	for (auto& pBuffer : m_Buffers)
		Safe_Release(pBuffer);
	m_Buffers.clear();

	for (auto& pSRV : m_SRVs)
		Safe_Release(pSRV);
	m_SRVs.clear();

	for (auto& pUAV : m_UAVs)
		Safe_Release(pUAV);
	m_UAVs.clear();

}



