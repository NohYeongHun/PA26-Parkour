#pragma once
#include "Base.h"

NS_BEGIN(Engine)

class CPooling_Manager final : public CBase
{
private:
	explicit CPooling_Manager();
	virtual ~CPooling_Manager() = default;

public:
	_uint									Get_NumThread() { return m_iNumThread; }

public:
	HRESULT								Initialize();

	// Object Pooling
	HRESULT								Add_PoolingObject(_uint iPrototypeLevelID, const _wstring& strPrototypeTag, _uint iLayerLevelID, const _wstring& strLayerTag, const _wstring& strPoolingTag, _uint iNumObjects, void* pArg);
	HRESULT								Add_PoolingObject_ForStatic(_uint iPrototypeLevelID, const _wstring& strPrototypeTag, _uint iLayerLevelID, const _wstring& strLayerTag, const _wstring& strPoolingTag, _uint iNumObjects, void* pArg);

	HRESULT								Spawn_PoolingObject(const _wstring& strPoolingTag, const _fmatrix& WorldMatrix, void* pArg);
	HRESULT								Spawn_PoolingObject_ForStatic(const _wstring& strPoolingTag, const _fmatrix& WorldMatrix, void* pArg);
	HRESULT								Clear_Resource();
	void									Update_Pooling();

	// Thread Pooling
	void									Add_Work(function<void()> Work);
	void									Add_Render_Work(function<void()> Work);
	_bool									IsWorkFinish() { 
		this_thread::sleep_for(chrono::nanoseconds(1000));
		_int iLiveWork = m_iLiveWork.load(memory_order_acquire);
		_int iRemainWork = m_iRemainWork.load(memory_order_acquire);
		return 0 == iLiveWork && 0 == iRemainWork; }

	void									Wait_Thread_End();
	void	Drain_Works();

private:
	class CGameInstance*				m_pGameInstance = { nullptr };
	// Pooling Object
	map<const _wstring, queue<class CGameObject*>>			m_PoolingObjects;
	// Active Object
	map<const _wstring, list<class CGameObject*>>				m_ActiveObjects;
	// Pooling Object (Static)
	map<const _wstring, queue<class CGameObject*>>			m_PoolingStaticObjects;
	// Active Object (Static)
	map<const _wstring, list<class CGameObject*>>				m_ActiveStaticObjects;

	// Thread
	vector<thread>					m_Threads;
	// Render Thread
	vector<thread>					m_RenderThreads;

	// Hardware Supported CPU Core
	_uint								m_iNumThread = {};
	// CPU Works 
	queue<function<void()>>	m_Works;
	// Render Works
	queue<function<void()>>	m_RenderWorks;
	// Mutex (Data)
	mutex								m_Mutex;
	// Mutex (Render)
	mutex								m_RenderMutex;

	// Thread Wait (CPU)
	condition_variable				m_CV;
	// Thread Wait (GPU)
	condition_variable				m_RenderCV;

	// Thread All Stop
	_bool								m_isAllStop = { false };
	// 진행중인 Work Count
	atomic<_int>					m_iLiveWork = {};
	// 남아있는 Work Count
	atomic<_int>					m_iRemainWork = {};

private:
	void									Work_Thread();
	void									Render_Thread();

public:
	static		CPooling_Manager*	Create();
	virtual		void						Free();
};

NS_END