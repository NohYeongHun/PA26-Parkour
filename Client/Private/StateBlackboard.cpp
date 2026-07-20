#include "ClientPch.h"
#include "StateBlackboard.h"

CStateBlackboard::CStateBlackboard(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CComponent{ pDevice, pContext }
{
}

CStateBlackboard::CStateBlackboard(const CStateBlackboard& Prototype)
	: CComponent(Prototype)
{
}

HRESULT CStateBlackboard::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CStateBlackboard::Initialize_Clone(void* pArg)
{
	return S_OK;
}

_uint CStateBlackboard::Register_Bool(const _string& strName)
{
	// 등록은 TagRegistry 한 곳에서만 일어난다 — 중복 등록은 이제 버그다.
	ASSERT_CRASH(m_BoolSlots.find(strName) == m_BoolSlots.end());

	const _uint iSlot = static_cast<_uint>(m_BoolValues.size());
	m_BoolSlots.emplace(strName, iSlot);
	m_BoolValues.push_back(false);
	return iSlot;
}

_uint CStateBlackboard::Find_Slot(const _string& strName) const
{
	const auto it = m_BoolSlots.find(strName);
	return (it != m_BoolSlots.end()) ? it->second : UINT_MAX;
}

vector<_uint> CStateBlackboard::Collect_Slots(const _string& strPrefix) const
{
	vector<_uint> Slots;
	const _string strDotted = strPrefix + ".";
	for (const auto& Pair : m_BoolSlots)
	{
		if (Pair.first == strPrefix ||
			0 == Pair.first.compare(0, strDotted.size(), strDotted))
			Slots.push_back(Pair.second);
	}
	return Slots;
}

void CStateBlackboard::Set(_uint iSlot, _bool isOn)
{
	ASSERT_CRASH(iSlot < m_BoolValues.size());
	m_BoolValues[iSlot] = isOn;
}

_bool CStateBlackboard::Get(_uint iSlot) const
{
	ASSERT_CRASH(iSlot < m_BoolValues.size());
	return m_BoolValues[iSlot];
}

void CStateBlackboard::Set(const _string& strName, _bool isOn)
{
	const auto it = m_BoolSlots.find(strName);
	ASSERT_CRASH(it != m_BoolSlots.end());
	m_BoolValues[it->second] = isOn;
}

_bool CStateBlackboard::Get(const _string& strName) const
{
	const auto it = m_BoolSlots.find(strName);
	ASSERT_CRASH(it != m_BoolSlots.end());
	return m_BoolValues[it->second];
}

void CStateBlackboard::Clear_Bools()
{
	fill(m_BoolValues.begin(), m_BoolValues.end(), false);

	for (const auto& Pair : m_NotifyLatch)
	{
		const auto it = m_BoolSlots.find(Pair.first);
		if (it != m_BoolSlots.end())
			m_BoolValues[it->second] = Pair.second;
	}
}

void CStateBlackboard::Set_Notify(const _string& strName, _bool isOn)
{
	const auto it = m_BoolSlots.find(strName);
	if (it == m_BoolSlots.end())
	{
#ifdef _DEBUG
		// 개명 후 낡은 노티파이 JSON(FlagName 미갱신)을 잡기 위한 경고
		cout << "[Blackboard] 미등록 노티파이 태그: " << strName << "\n";
#endif
		return;
	}

	m_NotifyLatch[strName] = isOn;
	m_BoolValues[it->second] = isOn;
}

void CStateBlackboard::Clear_Notifies()
{
	m_NotifyLatch.clear();
}

CStateBlackboard* CStateBlackboard::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CStateBlackboard* pInstance = new CStateBlackboard(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Create : CStateBlackboard");
		Safe_Release(pInstance);
	}
	return pInstance;
}

Engine::CComponent* CStateBlackboard::Clone(void* pArg)
{
	CStateBlackboard* pInstance = new CStateBlackboard(*this);
	if (FAILED(pInstance->Initialize_Clone(pArg)))
	{
		MSG_BOX("Clone Failed : CStateBlackboard");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CStateBlackboard::Free()
{
	__super::Free();
}
