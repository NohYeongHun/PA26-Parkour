#pragma once
#include "Base.h"

NS_BEGIN(Engine)

class CAnimation final : public CBase
{
private:
	explicit CAnimation();
	explicit CAnimation(const CAnimation& Prototype);
	virtual ~CAnimation() = default;

public:
	const _char*		Get_Name() { return m_szName; }
	const vector<class CChannel*>& Get_Channels() const { return m_Channels; }
	//void				Set_CurrentTrackPosition(_float fTrackPos) { m_fCurrentTrackPosition = fTrackPos; m_iNotifyIndex = 0; }
	void				Set_CurrentTrackPosition(_float fTrackPos);
	_float				Get_Duration() { return m_fDuration; }
	_float				Get_AnimProgress() { return m_fCurrentTrackPosition / m_fDuration; }
	void				Release_Channels();
	
#ifdef _DEBUG
	_float*				Get_TrackPositionPtr() { return &m_fCurrentTrackPosition; }
	

	void				Print_MorphKeyIndices();

	void				Clear_AnimNotifies();
#endif // _DEBUG

public:
	void				Register_Notify(const NOTIFY& AnimNotify); 

	void				Load_Notify(const json& notifyJson, function<void(const _wstring&, _bool)> ColliderCallback, function<void(const _wstring&)> EffectCallback, function<void(const _wstring&)> ObjectCallback);
	

	void				Sort_Notify();
	void				Sort_AnimNotify();
	void				Reset_Status();

public:
	HRESULT			Initialize(ifstream& InputFile, const vector<class CBone*>& Bones, MODELTYPE eModelType = MODELTYPE::ANIM);
	_bool				Update_TransformationMatrices(_float fTimeDelta, const vector<class CBone*>& Bones, _float* pTrackPosition = nullptr);
	_bool				Update_RibTransformationMatrices(_float fTimeDelta, const vector<class CBone*>& Bones, _float* pTrackPosition = nullptr);

	_bool				Update_TransformationMatrices_All(_float fTimeDelta, const vector<class CBone*>& Bones, _float* pTrackPosition = nullptr);

	_bool				Blend_TransformationMatrices(_float fTimeDelta, const vector<class CBone*>& Bones, _float fTrackLength);

	_bool			Update_TrackPosition(_float fTimeDelta, _float* pTrackPosition);

	_bool	        Bind_MorphChannels(const vector<string>& modelShapeKeys);
	_bool			Update_MorphWeights(_float fTimeDelta, vector<float>& modelWeights);
private:
	_char									m_szName[MAX_PATH] = {};
	_float									m_fDuration = {};
	_float									m_fTickPerSecond = {};
	_float									m_fCurrentTrackPosition = {};

	_uint									m_iNumChannels = {};
	vector<class CChannel*>				m_Channels;
	vector<_uint>						m_CurrentFrameIndices;

	vector<NOTIFY>					m_Notifies; // 

	_uint							m_iNotifyIndex = {};
	
	// Notify 
	vector<class CAnimNotify*>		m_AnimNotifies;

#pragma region MORPH TARGET
	// Morph Target Data
	_uint							m_iNumMorphCurves = { }; // 기본 0
	vector<class CMorphChannel*>	m_MorphMeshChannels;

	vector<_uint>					m_CurrentMorphCurveIndicies; // 채널들에 대한 Index 관리.

	// 매핑 테이블 => 현재 MorphChannel[i]가 Model의 ShapeKeyWeights의 몇번째 인덱스인지 저장.
	vector<_int> 					m_MorphKeyIndicies; 


#pragma endregion

	



private:
	HRESULT Ready_NormalAnimations(ifstream& InputFile, const vector<class CBone*>& Bones, MODELTYPE eModelType = MODELTYPE::ANIM);
	HRESULT Ready_CharacterAnimations(ifstream& InputFile, const vector<class CBone*>& Bones, MODELTYPE eModelType = MODELTYPE::ANIM);


public:
	static CAnimation* Create(ifstream& InputFile, const vector<class CBone*>& Bones, MODELTYPE eModelType = MODELTYPE::ANIM);
	CAnimation* Clone();
	virtual void Free() override;
};

NS_END