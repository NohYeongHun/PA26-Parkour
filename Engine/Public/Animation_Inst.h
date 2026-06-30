#pragma once
#include "Base.h"

NS_BEGIN(Engine)
// 애니메이션에 대한 정보로만 갖고있는 클래스, 각 인스턴스의 TrackPosition과 index는 개개의 객체에서 관리
class CAnimation_Inst final : public CBase
{
private:
	explicit CAnimation_Inst();
	explicit CAnimation_Inst(const CAnimation_Inst& Prototype);
	virtual ~CAnimation_Inst() = default;

public:
	const _char*		Get_Name() { return m_szName; }
	const vector<class CChannel*>& Get_Channels() const { return m_Channels; }
	//void				Set_CurrentTrackPosition(_float fTrackPos) { m_fCurrentTrackPosition = fTrackPos; m_iNotifyIndex = 0; }
	_float				Get_Duration() { return m_fDuration; }
	
#ifdef _DEBUG
	//_float*				Get_TrackPositionPtr() { return &m_fCurrentTrackPosition; }
	
#endif // _DEBUG

public:
	void				Register_Notify(const NOTIFY& AnimNotify); 

	void				Load_Notify(const json& notifyJson, function<void(const _wstring&, _bool)> ColliderCallback, function<void(const _wstring&)> EffectCallback, function<void(const _wstring&)> ObjectCallback);
	

	void				Sort_Notify();
	void				Sort_AnimNotify();

public:
	HRESULT				Initialize(ifstream& InputFile, const vector<class CBone*>& Bones);
	_bool				Update_TrackPosition(_float fTimeDelta, _float* pTrackPosition, _bool isRootMotion, _matrix* Out = nullptr);
private:
	_char									m_szName[MAX_PATH] = {};
	_float									m_fDuration = {};
	_float									m_fTickPerSecond = {};
	//_float									m_fCurrentTrackPosition = {};

	_uint									m_iNumChannels = {};
	vector<class CChannel*>				m_Channels;
	vector<_uint>						m_CurrentFrameIndices;

	vector<NOTIFY>					m_Notifies;

	_uint							m_iNotifyIndex = {};
	
	// Notify 
	vector<class CAnimNotify*> m_AnimNotifies;

private:

public:
	static CAnimation_Inst* Create(ifstream& InputFile, const vector<class CBone*>& Bones);
	CAnimation_Inst* Clone();
	virtual void Free() override;
};

NS_END