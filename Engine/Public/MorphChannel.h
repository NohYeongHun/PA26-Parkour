#pragma once
#include "Base.h"

NS_BEGIN(Engine)

// 역할 : Morphing Channel == Mesh가 가지고 있는 Shape Key와 매칭됩니다.
// Morphing Channel Name == Mesh Shape Key Name
// TrackPosition에 따라서 Weights를 변경합니다.
// TrackPosition으로 받은 
class CMorphChannel final : public CBase
{
private:
	explicit CMorphChannel();
	virtual ~CMorphChannel() = default;

public:
	const vector<KEYFRAME_CURVE>& Get_KeyframeCurves() const { return m_KeyFrames; }
	_uint Get_NumKeyframes() const { return m_iNumKeyFrame; }

	_float Get_CurrentWeight(_float fCurrentTrackPosition, _uint* pCurrentFrameIndex); // 현재 시간에 맞는 가중치(Weight) 반환
	const _char* Get_Name() const { return m_szName; }

#ifdef _DEBUG
	_float Get_Weight(_uint iFrameIndex);
#endif // _DEBUG



public:
	HRESULT					Initialize(ifstream& InputFile);
	void					Update_ShapeKeys(_float fCurrentTrackPosition, _uint* pCurrentFrameIndex, vector<_float>& InOutWeights, _int iTargetIndex);

private:
	_char						m_szName[MAX_PATH] = {};
	_uint						m_iNumKeyFrame = {};
	vector<KEYFRAME_CURVE>		m_KeyFrames;

	_uint						m_iCurrentKeyFrameIndex = {}; // 현재 키프레임 캐싱.
	vector<_uint>				m_CurrentFrameIndices;

public:
	static		CMorphChannel*	Create(ifstream& InputFile);
	virtual		void			Free() override;
};

NS_END