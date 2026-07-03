#pragma once
#include "Base.h"

NS_BEGIN(Engine)

class CChannel final : public CBase
{
private:
	explicit CChannel();
	virtual ~CChannel() = default;

public:
	const vector<KEYFRAME>& Get_Keyframes() const { return m_KeyFrames; }
	_uint Get_NumKeyframes() const { return m_iNumKeyFrame; }
	_uint Get_BoneIndex() const { return m_iBoneIndex; }

#ifdef _DEBUG
public:
	const _char* Get_Name() const { return m_szName; }
#endif // _DEBUG



public:
	HRESULT					Initialize(ifstream& InputFile, const vector<class CBone*>& Bones);
	void					Update_TransformationMatrix(_float fCurrentTrackPosition, const vector<class CBone*>& Bones, _uint* pCurrentFrameIndex);
	void					Update_RibTransformationMatrix(_float fCurrentTrackPosition, const vector<class CBone*>& Bones, _uint* pCurrentFrameIndex);
	void					Update_TransformationMatrix_All(_float fCurrentTrackPosition, const vector<class CBone*>& Bones, _uint* pCurrentFrameIndex);
	//void					Blend_TransformationMatrix(_float fCurrentTrackPosition, const vector<class CBone*>& Bones, _float fTrackLength);
	void					Blend_TransformationMatrix(_float fCurrentTrackPosition, const vector<class CBone*>& Bones, _float fWeight);
private:
	_char						m_szName[MAX_PATH] = {};

	_uint						m_iBoneIndex = {};

	_uint						m_iNumKeyFrame = {};
	vector<KEYFRAME>			m_KeyFrames;

	_float3					m_BlendScale = {};
	_float4					m_BlendRotation = {};
	_float3					m_BlendPosition = {};

public:
	static		CChannel*	Create(ifstream& InputFile, const vector<class CBone*>& Bones);
	virtual		void			Free() override;
};

NS_END