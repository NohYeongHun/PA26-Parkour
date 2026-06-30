#pragma once
#include "Base.h"
#include "Event.h"

NS_BEGIN(Engine)

class CEventBus final : public CBase
{
private:
	explicit CEventBus();
	virtual ~CEventBus() = default;

public:
	HRESULT			Initialize();

public:
	template<typename TEvent>
	void Subscribe(_uint iStatic, const _wstring& strEventTag, function<void(const TEvent&)> handler) {
		auto wrapper = [handler](const CEvent& event) {
				handler(static_cast<const TEvent&>(event));
			};
		{
			lock_guard<mutex>lock(m_Mutex);
			m_Listener[iStatic][strEventTag].push_back(wrapper);
		}
	}

	void Unscribe() {
		for (auto& Listener : m_Listener[ENUM_CLASS(STATIC::NONE)])
			Listener.second.clear();
	}

	void Publish(_uint iStatic, const _wstring& strEventTag, const CEvent& event) {
		auto iter = m_Listener[iStatic].find(strEventTag);
		if (iter == m_Listener[iStatic].end())
			return;

		for (auto& handler : iter->second)
			handler(event);
	}

private:
	unordered_map<_wstring, vector<function<void(const CEvent&)>>> m_Listener[2];
	typedef unordered_map<_wstring, vector<function<void(const CEvent&)>>> LISTENER;

	mutex m_Mutex;
public:
	static CEventBus* Create();
	virtual void Free() override;
};

NS_END