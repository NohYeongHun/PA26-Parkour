#include "EnginePch.h"
#include "Channel.h"

#include "Bone.h"

CChannel::CChannel()
{
}

HRESULT CChannel::Initialize(ifstream& InputFile, const vector<class CBone*>& Bones)
{
	_uint iLength = {};
	InputFile.read(reinterpret_cast<_char*>(&iLength), sizeof(_uint));
	InputFile.read(m_szName, iLength);

	auto iter = find_if(Bones.begin(), Bones.end(), [&](CBone* pBone)->_bool {
			return !strcmp(m_szName, pBone->Get_Name());
		});
	if (iter == Bones.end())
		return E_FAIL;

	m_iBoneIndex = static_cast<_uint>(iter - Bones.begin());

	InputFile.read(reinterpret_cast<_char*>(&m_iNumKeyFrame), sizeof(_uint));

	for (size_t i = 0; i < m_iNumKeyFrame; ++i)
	{
		KEYFRAME KeyFrame = {};
		InputFile.read(reinterpret_cast<_char*>(&KeyFrame), sizeof(KEYFRAME));
		m_KeyFrames.push_back(KeyFrame);
	}

	return S_OK;
}

void CChannel::Update_TransformationMatrix(_float fCurrentTrackPosition, const vector<class CBone*>& Bones, _uint* pCurrentFrameIndex)
{
	if (m_iNumKeyFrame == 2)
		return;

	if (0.f == fCurrentTrackPosition)
		*pCurrentFrameIndex = 0;
	
	if (m_KeyFrames.back().fTrackPosition <= fCurrentTrackPosition)
	{
		*pCurrentFrameIndex = m_iNumKeyFrame - 1;
	}
	else
	{
#ifdef _DEBUG
		while (m_KeyFrames[*pCurrentFrameIndex].fTrackPosition > fCurrentTrackPosition)
			--*pCurrentFrameIndex;
#endif // _DEBUG

		while (m_KeyFrames[*pCurrentFrameIndex + 1].fTrackPosition <= fCurrentTrackPosition)
			++*pCurrentFrameIndex;

		_float3 vLeftScale = m_KeyFrames[*pCurrentFrameIndex].vScale;
		_float4 vLeftRotation = m_KeyFrames[*pCurrentFrameIndex].vRotation;
		_float3 vLeftPosition = m_KeyFrames[*pCurrentFrameIndex].vTranslation;

		_float3 vRightScale = m_KeyFrames[*pCurrentFrameIndex + 1].vScale;
		_float4 vRightRotation = m_KeyFrames[*pCurrentFrameIndex + 1].vRotation;
		_float3 vRightPosition = m_KeyFrames[*pCurrentFrameIndex + 1].vTranslation;

		_float fRatio = (fCurrentTrackPosition - m_KeyFrames[*pCurrentFrameIndex].fTrackPosition) / (m_KeyFrames[*pCurrentFrameIndex + 1].fTrackPosition - m_KeyFrames[*pCurrentFrameIndex].fTrackPosition);

		_vector vLerpScale = XMVectorLerp(XMLoadFloat3(&vLeftScale), XMLoadFloat3(&vRightScale), fRatio);
		_vector vLerpRotation = XMQuaternionSlerp(XMLoadFloat4(&vLeftRotation), XMLoadFloat4(&vRightRotation), fRatio);
		_vector vLerpPosition = XMVectorSetW(XMVectorLerp(XMLoadFloat3(&vLeftPosition), XMLoadFloat3(&vRightPosition), fRatio), 1.f);

		_matrix LerpMatrix = {};
		LerpMatrix = XMMatrixAffineTransformation(vLerpScale, XMVectorSet(0.f, 0.f, 0.f, 1.f), vLerpRotation, vLerpPosition);

		Bones[m_iBoneIndex]->Set_TransformationMatrix(LerpMatrix);
	}
}

void CChannel::Update_RibTransformationMatrix(_float fCurrentTrackPosition, const vector<class CBone*>& Bones, _uint* pCurrentFrameIndex)
{
	if (m_iNumKeyFrame == 2)
	{
		return;
	}		

	if (0.f == fCurrentTrackPosition)
		*pCurrentFrameIndex = 0;
	
	if (m_KeyFrames.back().fTrackPosition <= fCurrentTrackPosition)
	{
		*pCurrentFrameIndex = m_iNumKeyFrame - 1;
	}
	else
	{
#ifdef _DEBUG
		while (m_KeyFrames[*pCurrentFrameIndex].fTrackPosition > fCurrentTrackPosition)
			--*pCurrentFrameIndex;
#endif // _DEBUG

		while (m_KeyFrames[*pCurrentFrameIndex + 1].fTrackPosition <= fCurrentTrackPosition)
			++*pCurrentFrameIndex;

		_float3 vLeftScale = m_KeyFrames[*pCurrentFrameIndex].vScale;
		_float4 vLeftRotation = m_KeyFrames[*pCurrentFrameIndex].vRotation;
		_float3 vLeftPosition = m_KeyFrames[*pCurrentFrameIndex].vTranslation;

		_float3 vRightScale = m_KeyFrames[*pCurrentFrameIndex + 1].vScale;
		_float4 vRightRotation = m_KeyFrames[*pCurrentFrameIndex + 1].vRotation;
		_float3 vRightPosition = m_KeyFrames[*pCurrentFrameIndex + 1].vTranslation;

		_float fRatio = (fCurrentTrackPosition - m_KeyFrames[*pCurrentFrameIndex].fTrackPosition) / (m_KeyFrames[*pCurrentFrameIndex + 1].fTrackPosition - m_KeyFrames[*pCurrentFrameIndex].fTrackPosition);

		_vector vLerpScale = XMVectorLerp(XMLoadFloat3(&vLeftScale), XMLoadFloat3(&vRightScale), fRatio);
		_vector vLerpRotation = XMQuaternionSlerp(XMLoadFloat4(&vLeftRotation), XMLoadFloat4(&vRightRotation), fRatio);
		_vector vLerpPosition = XMVectorSetW(XMVectorLerp(XMLoadFloat3(&vLeftPosition), XMLoadFloat3(&vRightPosition), fRatio), 1.f);

		_matrix LerpMatrix = {};
		LerpMatrix = XMMatrixAffineTransformation(vLerpScale, XMVectorSet(0.f, 0.f, 0.f, 1.f), vLerpRotation, vLerpPosition);

		_matrix PrevMatrix = XMLoadFloat4x4(Bones[m_iBoneIndex]->Get_TransformationMatrix());
		_matrix FinalMatrix = PrevMatrix * LerpMatrix;
		Bones[m_iBoneIndex]->Set_TransformationMatrix(FinalMatrix);
	}
	
}

void CChannel::Update_TransformationMatrix_All(_float fCurrentTrackPosition, const vector<class CBone*>& Bones, _uint* pCurrentFrameIndex)
{
	if (0.f == fCurrentTrackPosition)
		*pCurrentFrameIndex = 0;

	if (m_KeyFrames.back().fTrackPosition <= fCurrentTrackPosition)
	{
		*pCurrentFrameIndex = m_iNumKeyFrame - 1;
	}
	else
	{
#ifdef _DEBUG 
		while (*pCurrentFrameIndex > 0 && m_KeyFrames[*pCurrentFrameIndex].fTrackPosition > fCurrentTrackPosition)
			--*pCurrentFrameIndex;
#endif // _DEBUG

		while (m_KeyFrames[*pCurrentFrameIndex + 1].fTrackPosition <= fCurrentTrackPosition)
			++*pCurrentFrameIndex;

		_float3 vLeftScale = m_KeyFrames[*pCurrentFrameIndex].vScale;
		_float4 vLeftRotation = m_KeyFrames[*pCurrentFrameIndex].vRotation;
		_float3 vLeftPosition = m_KeyFrames[*pCurrentFrameIndex].vTranslation;

		_float3 vRightScale = m_KeyFrames[*pCurrentFrameIndex + 1].vScale;
		_float4 vRightRotation = m_KeyFrames[*pCurrentFrameIndex + 1].vRotation;
		_float3 vRightPosition = m_KeyFrames[*pCurrentFrameIndex + 1].vTranslation;

		_float fRatio = (fCurrentTrackPosition - m_KeyFrames[*pCurrentFrameIndex].fTrackPosition) / (m_KeyFrames[*pCurrentFrameIndex + 1].fTrackPosition - m_KeyFrames[*pCurrentFrameIndex].fTrackPosition);

		_vector vLerpScale = XMVectorLerp(XMLoadFloat3(&vLeftScale), XMLoadFloat3(&vRightScale), fRatio);


		_vector qLeft = XMQuaternionNormalize(XMLoadFloat4(&vLeftRotation));
		_vector qRight = XMQuaternionNormalize(XMLoadFloat4(&vRightRotation));

		// 추가: Dot 계산 후 체크
		_vector dot = XMVector4Dot(qLeft, qRight);
		dot = XMVectorClamp(dot, XMVectorSplatOne() * -1.0f, XMVectorSplatOne());  // -1 ~ 1 강제

		float cosOmega = XMVectorGetX(dot);
		float omega = acosf(cosOmega);
		float sinOmega = sinf(omega);

		_vector vLerpRotation;
		if (fabs(sinOmega) < 1e-6f) {  // SinOmega ≈ 0이면 fallback to Lerp or left
			vLerpRotation = (fRatio < 0.5f) ? qLeft : qRight;  // 또는 XMVectorLerp(qLeft, qRight, fRatio);
		}
		else {
			vLerpRotation = XMQuaternionSlerp(qLeft, qRight, fRatio);
		}


		_vector vLerpPosition = XMVectorSetW(XMVectorLerp(XMLoadFloat3(&vLeftPosition), XMLoadFloat3(&vRightPosition), fRatio), 1.f);

		_matrix LerpMatrix = {};
		LerpMatrix = XMMatrixAffineTransformation(vLerpScale, XMVectorSet(0.f, 0.f, 0.f, 1.f), vLerpRotation, vLerpPosition);

		Bones[m_iBoneIndex]->Set_TransformationMatrix(LerpMatrix);
	}
}

//void CChannel::Blend_TransformationMatrix(_float fCurrentTrackPosition, const vector<class CBone*>& Bones, _float fTrackLength)
//{
//	if (0.f == fCurrentTrackPosition)
//	{
//		_matrix PreBoneMatrix = XMLoadFloat4x4(Bones[m_iBoneIndex]->Get_TransformationMatrix());
//		_vector vLeftScale{}, vLeftRotation{}, vLeftPosition{};
//		XMMatrixDecompose(&vLeftScale, &vLeftRotation, &vLeftPosition, PreBoneMatrix);
//		XMStoreFloat3(&m_BlendScale, vLeftScale);
//		XMStoreFloat4(&m_BlendRotation, vLeftRotation);
//		XMStoreFloat3(&m_BlendPosition, vLeftPosition);
//	}
//
//	_float3 vRightScale = m_KeyFrames[0].vScale;
//	_float4 vRightRotation = m_KeyFrames[0].vRotation;
//	_float3 vRightPosition = m_KeyFrames[0].vTranslation;
//
//	_float fRatio = fCurrentTrackPosition / fTrackLength;
//
//	_vector vLerpScale = XMVectorLerp(XMLoadFloat3(&m_BlendScale), XMLoadFloat3(&vRightScale), fRatio);
//	_vector vLerpRotation = XMQuaternionSlerp(XMLoadFloat4(&m_BlendRotation), XMLoadFloat4(&vRightRotation), fRatio);
//	_vector vLerpPosition = XMVectorSetW(XMVectorLerp(XMLoadFloat3(&m_BlendPosition), XMLoadFloat3(&vRightPosition), fRatio), 1.f);
//
//	_matrix LerpMatrix = {};
//	LerpMatrix = XMMatrixAffineTransformation(vLerpScale, XMVectorSet(0.f, 0.f, 0.f, 1.f), vLerpRotation, vLerpPosition);
//
//	Bones[m_iBoneIndex]->Set_TransformationMatrix(LerpMatrix);
//}

void CChannel::Blend_TransformationMatrix(_float fCurrentTrackPosition, const vector<class CBone*>& Bones, _float fWeight)
{
	// 현재 Channel에 해당하는 Bone의 행렬 정보를 분해해서 스냅샷.
	if (0.f == fCurrentTrackPosition)
	{
		_matrix PreBoneMatrix = XMLoadFloat4x4(Bones[m_iBoneIndex]->Get_TransformationMatrix());
		_vector vLeftScale{}, vLeftRotation{}, vLeftPosition{};
		XMMatrixDecompose(&vLeftScale, &vLeftRotation, &vLeftPosition, PreBoneMatrix);
		XMStoreFloat3(&m_BlendScale, vLeftScale);
		XMStoreFloat4(&m_BlendRotation, vLeftRotation);
		XMStoreFloat3(&m_BlendPosition, vLeftPosition);
	}

	_float3 vRightScale = m_KeyFrames[0].vScale;
	_float4 vRightRotation = m_KeyFrames[0].vRotation;
	_float3 vRightPosition = m_KeyFrames[0].vTranslation;

	_float fRatio = fWeight;

	_vector vLerpScale = XMVectorLerp(XMLoadFloat3(&m_BlendScale), XMLoadFloat3(&vRightScale), fRatio);
	_vector vLerpRotation = XMQuaternionSlerp(XMLoadFloat4(&m_BlendRotation), XMLoadFloat4(&vRightRotation), fRatio);
	_vector vLerpPosition = XMVectorSetW(XMVectorLerp(XMLoadFloat3(&m_BlendPosition), XMLoadFloat3(&vRightPosition), fRatio), 1.f);

	_matrix LerpMatrix = {};
	LerpMatrix = XMMatrixAffineTransformation(vLerpScale, XMVectorSet(0.f, 0.f, 0.f, 1.f), vLerpRotation, vLerpPosition);

	Bones[m_iBoneIndex]->Set_TransformationMatrix(LerpMatrix);
}

void CChannel::Compute_SRT(_float fTrackPosition, _vector& vScale, _vector& vRotation, _vector& vTranslation) const
{
	if (m_KeyFrames.back().fTrackPosition <= fTrackPosition)
	{
		const KEYFRAME& K = m_KeyFrames.back();
		vScale = XMLoadFloat3(&K.vScale);
		vRotation = XMQuaternionNormalize(XMLoadFloat4(&K.vRotation));
		vTranslation = XMVectorSetW(XMLoadFloat3(&K.vTranslation), 1.f);
		return;
	}

	if (fTrackPosition <= m_KeyFrames.front().fTrackPosition)
	{
		const KEYFRAME& K = m_KeyFrames.front();
		vScale = XMLoadFloat3(&K.vScale);
		vRotation = XMQuaternionNormalize(XMLoadFloat4(&K.vRotation));
		vTranslation = XMVectorSetW(XMLoadFloat3(&K.vTranslation), 1.f);
		return;
	}

	// 이분 탐색: KeyFrames[iLow].fTrackPosition <= fTrackPosition < KeyFrames[iLow+1].fTrackPosition
	size_t iLow = 0;
	size_t iHigh = m_KeyFrames.size() - 1;
	while (iLow + 1 < iHigh)
	{
		size_t iMid = (iLow + iHigh) / 2;
		if (m_KeyFrames[iMid].fTrackPosition <= fTrackPosition)
			iLow = iMid;
		else
			iHigh = iMid;
	}

	const KEYFRAME& Left = m_KeyFrames[iLow];
	const KEYFRAME& Right = m_KeyFrames[iLow + 1];

	_float fRatio = (fTrackPosition - Left.fTrackPosition) / (Right.fTrackPosition - Left.fTrackPosition);

	vScale = XMVectorLerp(XMLoadFloat3(&Left.vScale), XMLoadFloat3(&Right.vScale), fRatio);
	vRotation = XMQuaternionSlerp(XMQuaternionNormalize(XMLoadFloat4(&Left.vRotation)),
								XMQuaternionNormalize(XMLoadFloat4(&Right.vRotation)), fRatio);
	vTranslation = XMVectorSetW(XMVectorLerp(XMLoadFloat3(&Left.vTranslation), XMLoadFloat3(&Right.vTranslation), fRatio), 1.f);
}

void CChannel::Sample_TransformationMatrix(_float fTrackPosition, const vector<class CBone*>& Bones)
{
	_vector vScale{}, vRotation{}, vTranslation{};
	Compute_SRT(fTrackPosition, vScale, vRotation, vTranslation);

	Bones[m_iBoneIndex]->Set_TransformationMatrix(
		XMMatrixAffineTransformation(vScale, XMVectorSet(0.f, 0.f, 0.f, 1.f), vRotation, vTranslation));
}

void CChannel::Blend_TransformationMatrix_At(_float fTrackPosition, const vector<class CBone*>& Bones, _float fWeight)
{
	_vector vClipScale{}, vClipRotation{}, vClipTranslation{};
	Compute_SRT(fTrackPosition, vClipScale, vClipRotation, vClipTranslation);

	_matrix CurMatrix = XMLoadFloat4x4(Bones[m_iBoneIndex]->Get_TransformationMatrix());
	_vector vCurScale{}, vCurRotation{}, vCurTranslation{};
	if (false == XMMatrixDecompose(&vCurScale, &vCurRotation, &vCurTranslation, CurMatrix))
	{
		// 분해 불가(비정상 행렬)면 클립 포즈로 덮어쓴다
		Bones[m_iBoneIndex]->Set_TransformationMatrix(
			XMMatrixAffineTransformation(vClipScale, XMVectorSet(0.f, 0.f, 0.f, 1.f), vClipRotation, vClipTranslation));
		return;
	}

	_vector vScale = XMVectorLerp(vCurScale, vClipScale, fWeight);
	_vector vRotation = XMQuaternionSlerp(vCurRotation, vClipRotation, fWeight);
	_vector vTranslation = XMVectorSetW(XMVectorLerp(vCurTranslation, vClipTranslation, fWeight), 1.f);

	Bones[m_iBoneIndex]->Set_TransformationMatrix(
		XMMatrixAffineTransformation(vScale, XMVectorSet(0.f, 0.f, 0.f, 1.f), vRotation, vTranslation));
}

CChannel* CChannel::Create(ifstream& InputFile, const vector<class CBone*>& Bones)
{
	CChannel* pInstance = new CChannel();

	if (FAILED(pInstance->Initialize(InputFile, Bones)))
	{
		MSG_BOX("Failed to Create : Channel");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CChannel::Free()
{
	__super::Free();
}
