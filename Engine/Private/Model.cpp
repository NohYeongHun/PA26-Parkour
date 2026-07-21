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

namespace {
	constexpr _float MW_EPS            = 1e-4f;
	constexpr _float MW_MAX_WARP_RATIO = 3.0f;
	constexpr _float MW_MAX_ABS_STEP   = 0.3f;
}

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
	_vector vPosA = XMLoadFloat4(&vPosition);
	_vector vPosB = XMLoadFloat4(&Other.vPosition);
	_vector vQuatA = XMLoadFloat4(&vQuat);
	_vector vQuatB = XMLoadFloat4(&Other.vQuat);

	XMStoreFloat4(&Result.vPosition, XMVectorLerp(vPosA, vPosB, fAlpha));
	XMStoreFloat4(&Result.vQuat, XMQuaternionSlerp(vQuatA, vQuatB, fAlpha));
	Result.fTimeInSeconds = fTimeInSeconds + (Other.fTimeInSeconds - fTimeInSeconds) * fAlpha;
	return Result;
}

_float CModel::Get_Duration(const _string& strAnimName)
{
	return m_Animations.at(strAnimName)->Get_Duration();
}

#ifdef _DEBUG
_float* CModel::Get_TrackPositionPtr(const _string& strAnimName)
{
	return m_Animations.at(strAnimName)->Get_TrackPositionPtr();
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

void CModel::Debug_RootMotionDraw(const _string& strAnimName, _fmatrix StartWorldMatrix, _float fTimeStepSec, const ROOTMOTION_DESC& rootDesc)
{
	vector<TrajectorySample> Samples = Get_RootMotionTrajectory(strAnimName, fTimeStepSec);
	if (Samples.size() < 2)
		return;

	CGameInstance* pGI = CGameInstance::GetInstance();

	const JPH::Color LineColor(0.f, 255.f, 0.f, 1.f);   
	const JPH::Color PointColor(0.f, 255.f, 255.f, 1.f);

	_vector vPrevWorld{};
	_bool   bHasPrev = false;

	for (const TrajectorySample& sample : Samples)
	{
		_vector vRelPos = XMLoadFloat4(&sample.vPosition); // w=1
		_vector qRelRot = XMLoadFloat4(&sample.vQuat);

		if (rootDesc.isEnable)
		{
			if (!rootDesc.isTranslate)
				vRelPos = XMVectorSet(0.f, 0.f, 0.f, 1.f);
			else
				vRelPos = XMVectorSetW(XMVectorScale(vRelPos, rootDesc.fRate), 1.f);

			if (!rootDesc.isRotate)
				qRelRot = XMQuaternionIdentity();
		}

		_matrix matLocal = XMMatrixAffineTransformation(
			XMVectorSet(1.f, 1.f, 1.f, 1.f),
			XMVectorSet(0.f, 0.f, 0.f, 1.f),
			qRelRot,
			vRelPos);

		_matrix matWorld = matLocal * StartWorldMatrix;
		_vector vWorldPos = matWorld.r[3];

		pGI->Add_DebugSphere(vWorldPos, 0.03f, PointColor);
		if (bHasPrev)
			pGI->Add_DebugLine(vPrevWorld, vWorldPos, LineColor);

		vPrevWorld = vWorldPos;
		bHasPrev = true;
	}
}

void CModel::Dump_RootMotionCurve(const _string& strAnimName)
{
	CAnimation* pAnim = Get_AnimationOrNull(strAnimName);
	if (nullptr == pAnim)
		return;

	const _float fDur = pAnim->Get_Duration();
	cout << "=== RootMotion: " << strAnimName << " (dur " << fDur << ") ===" << endl;
	cout << "trk\tX\tY\tZ" << endl;
	for (_float t = 1.f; t <= fDur; t += 1.f)
	{
		ROOT_MOTION_DELTA d = Extract_RootMotion(strAnimName, 0.f, t);  // 0→t 누적
		cout << t << "\t" << d.vTranslate.x << "\t" << d.vTranslate.y << "\t" << d.vTranslate.z << endl;
	}
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

void CModel::Register_AllNotifies(const _string& strNotifyFolderPath
	, function<void(const _wstring&, _bool)> ColliderCallback
	, function<void(const _wstring&)> EffectCallback
	, function<void(const _wstring&)> ObjectCallback
	, function<void(const _string&, _bool)> StateFlagCallback
	, function<void(const _string&, _bool, _float, _bool, _bool)> WarpCallback
	, function<void(const vector<IK_BINDING>& Bindings, _float fBlendSec, _bool isBegin)> IKCallback)
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
				pAnimation->Load_Notify(notifyData["Notifies"], ColliderCallback, EffectCallback, ObjectCallback, StateFlagCallback, WarpCallback, IKCallback);
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



void CModel::Clear_Animation(const _string& strAnimationName, _float fTrackPosition)
{
	// CAnimator::Clear_Animation, hich delegates here.
	if (strAnimationName == "")
		return;

	auto iter = m_Animations.find(strAnimationName);
	if (iter == m_Animations.end())
		return;

	m_Animations.at(strAnimationName)->Reset_Status();
	m_Animations.at(strAnimationName)->Set_CurrentTrackPosition(fTrackPosition);
	fill(m_ShapeKeyWeights.begin(), m_ShapeKeyWeights.end(), 0.0f);

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

void CModel::Update_BoneMatrix_Map(_uint iStartIndex)
{
	for (_uint i = iStartIndex; i < static_cast<_uint>(m_Bones.size()); ++i)
		m_Bones[i]->Update_CombinedTransformationMatrix(XMLoadFloat4x4(&m_PreTransformMatrix), m_Bones);
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

vector<TrajectorySample> CModel::Get_RootMotionTrajectory(const _string& strAnimName, _float fTimeStepSec)
{
	vector<TrajectorySample> Samples;

	if (fTimeStepSec <= 0.f)
		return Samples;

	CAnimation* pAnim = Get_AnimationOrNull(strAnimName);
	if (nullptr == pAnim)
		return Samples;

	const _float fTPS = pAnim->Get_TickPerSecond();
	if (fTPS <= 0.f)
		return Samples;

	const _float fDurSec = pAnim->Get_Duration() / fTPS;
	if (fDurSec <= 0.f)
		return Samples;

	// 루트 본 채널 탐색
	CChannel* pRootChannel = nullptr;
	for (CChannel* pChannel : pAnim->Get_Channels())
	{
		if (pChannel->Get_BoneIndex() == m_iRootBoneIndex)
		{
			pRootChannel = pChannel;
			break;
		}
	}
	if (nullptr == pRootChannel)
		return Samples;

	_matrix matConversion = XMLoadFloat4x4(&m_ConversionMatrix);
	_vector qConversion = XMQuaternionRotationMatrix(matConversion);

	// t=0 기준값
	_vector vScale0{}, vRot0{}, vTrans0{};
	pRootChannel->Sample_SRT(0.f, vScale0, vRot0, vTrans0);
	_vector vConvT0 = XMVector3Transform(vTrans0, matConversion);
	_vector qConvR0 = XMQuaternionMultiply(qConversion, vRot0);
	_vector qConvR0Inv = XMQuaternionInverse(qConvR0);

	auto AddSampleAt = [&](_float t)
		{
			_vector vScale{}, vRot{}, vTrans{};
			pRootChannel->Sample_SRT(t * fTPS, vScale, vRot, vTrans);

			_vector vConvT = XMVector3Transform(vTrans, matConversion);
			_vector qConvR = XMQuaternionMultiply(qConversion, vRot);

			// 애니 시작 기준 상대화 (Get_RootMotionTotalDisplacement와 동일한 변환·스케일)
			_vector vRelPos = XMVectorScale(vConvT - vConvT0, m_fPreScale);
			_vector qRelRot = XMQuaternionMultiply(qConvR, qConvR0Inv);

			TrajectorySample sample{};
			XMStoreFloat4(&sample.vPosition, XMVectorSetW(vRelPos, 1.f));
			XMStoreFloat4(&sample.vQuat, qRelRot);
			sample.fTimeInSeconds = t;
			Samples.push_back(sample);
		};

	// 초 단위 샘플 + 끝점 항상 포함
	for (_float t = 0.f; t < fDurSec - 1e-4f; t += fTimeStepSec)
		AddSampleAt(t);
	AddSampleAt(fDurSec);

	return Samples;
}

ROOT_MOTION_DELTA CModel::Extract_RootMotion(const _string& strAnimName, _float fStartTrackPos, _float fEndTrackPos)
{
	const ROOT_MOTION_DELTA ZeroDelta{ _float3{ 0.f, 0.f, 0.f }, _float4{ 0.f, 0.f, 0.f, 1.f } };

	CAnimation* pAnim = Get_AnimationOrNull(strAnimName);
	if (nullptr == pAnim)
		return ZeroDelta;

	const _float fDuration = pAnim->Get_Duration(); // 틱 단위
	if (fDuration <= 0.f)
		return ZeroDelta;

	// 루트 본 채널 탐색
	CChannel* pRootChannel = nullptr;
	for (CChannel* pChannel : pAnim->Get_Channels())
	{
		if (pChannel->Get_BoneIndex() == m_iRootBoneIndex)
		{
			pRootChannel = pChannel;
			break;
		}
	}
	if (nullptr == pRootChannel)
		return ZeroDelta;

	// 순방향 구간만 지원
	const _float fStart = max(0.f, min(fStartTrackPos, fDuration));
	const _float fEnd   = max(0.f, min(fEndTrackPos, fDuration));
	if (fEnd <= fStart)
		return ZeroDelta;

	_matrix matConversion = XMLoadFloat4x4(&m_ConversionMatrix);
	_vector qConversion   = XMQuaternionRotationMatrix(matConversion);

	_vector vScaleS{}, vRotS{}, vTransS{};
	pRootChannel->Sample_SRT(fStart, vScaleS, vRotS, vTransS);
	_vector vScaleE{}, vRotE{}, vTransE{};
	pRootChannel->Sample_SRT(fEnd, vScaleE, vRotE, vTransE);

	_vector vConvTS = XMVector3Transform(vTransS, matConversion);
	_vector vConvTE = XMVector3Transform(vTransE, matConversion);
	_vector vRelPos = XMVectorScale(vConvTE - vConvTS, m_fPreScale);

	_vector qConvRS = XMQuaternionMultiply(qConversion, vRotS);
	_vector qConvRE = XMQuaternionMultiply(qConversion, vRotE);
	_vector qRelRot = XMQuaternionMultiply(qConvRE, XMQuaternionInverse(qConvRS));

	ROOT_MOTION_DELTA Delta{};
	XMStoreFloat3(&Delta.vTranslate, vRelPos);
	XMStoreFloat4(&Delta.qRotation, qRelRot);
	return Delta;
}

_float CModel::Get_AnimProgress(const _string& strAnimName)
{
	return m_Animations[strAnimName]->Get_AnimProgress();
}


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



