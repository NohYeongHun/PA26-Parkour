#include"EnginePch.h"
#include "FreeList.h"

#define MB (1024u * 1024u)
#define INVALID_OFFSET UINT_MAX

CFreeList::CFreeList()
{

}

HRESULT CFreeList::Initialize(_uint iMemorySize)
{
	m_iMemorySize = iMemorySize;
	Clear_Resource();
	
	return S_OK;
}

_uint CFreeList::Allocate(_uint iMemorySize)
{
#ifdef _DEBUG
	if (iMemorySize == 0)
		CRASH("Allocate Size 0");
#endif
	//Best-fit 알고리즘
	_uint iBestFitOffset = { INVALID_OFFSET };
	_uint iBestFitSize = { INVALID_OFFSET };
	auto BestFitIter = m_FreeBlocks.end();

	for (auto CurrentIter = m_FreeBlocks.begin(); CurrentIter != m_FreeBlocks.end(); ++CurrentIter)
	{
		_uint iCurrentBlockSize = CurrentIter->second;

		if (iCurrentBlockSize >= iMemorySize)
		{
			if (iCurrentBlockSize < iBestFitSize)
			{
				iBestFitOffset = CurrentIter->first;
				iBestFitSize = iCurrentBlockSize;
				BestFitIter = CurrentIter;
			}
		}
	}

	if (BestFitIter != m_FreeBlocks.end())
	{
		m_FreeBlocks.erase(BestFitIter);

		_uint iRemainderSize = iBestFitSize - iMemorySize;
		if (iRemainderSize > 0)
		{
			_uint iRemainOffset = iBestFitOffset + iMemorySize;
			m_FreeBlocks.emplace(iRemainOffset, iRemainderSize);
		}
	}

	//해당 함수를 호출한 클래스에서 반환값이 INVALID_OFFSET인 경우 실패 처리.
    return iBestFitOffset;
}

void CFreeList::Free(_uint iMemoryOffset, _uint iMemorySize)
{
#ifdef _DEBUG
	if (iMemorySize == 0)
	{
		CRASH("Free Size 0");
	}

	auto ExistIter = m_FreeBlocks.find(iMemoryOffset);

	if (ExistIter != m_FreeBlocks.end())
	{
		CRASH("Wrong Memory Block Returned");
	}
#endif

	//내 메모리 바로 뒤에 블럭이 있나 확인
	auto NextBlockIter = m_FreeBlocks.find(iMemoryOffset + iMemorySize);
	if (NextBlockIter != m_FreeBlocks.end())
	{
		//있음. 병합
		iMemorySize += NextBlockIter->second;
		m_FreeBlocks.erase(NextBlockIter);
	}

	//내 메모리 바로 앞에 블럭이 있나 확인. lower_bound는 해당 이터레이터와 가장 가까운 뒷 블록 반환.
	auto CurrentIt = m_FreeBlocks.lower_bound(iMemoryOffset);
	if (CurrentIt != m_FreeBlocks.begin())
	{
		auto PrevBlockIter = prev(CurrentIt);

		//사이즈 비교
		if (PrevBlockIter->first + PrevBlockIter->second == iMemoryOffset)
		{

			iMemorySize += PrevBlockIter->second;
			//오프셋 앞으로 갱신.
			iMemoryOffset = PrevBlockIter->first;
			m_FreeBlocks.erase(PrevBlockIter);
		}
	}

	m_FreeBlocks.emplace(iMemoryOffset, iMemorySize);
}

void CFreeList::Clear_Resource()
{
	m_FreeBlocks.clear();
	m_FreeBlocks.emplace(0, m_iMemorySize * MB);
}

CFreeList* CFreeList::Create(_uint iMemorySize)
{
	CFreeList* pInstance = new CFreeList;

	if (FAILED(pInstance->Initialize(iMemorySize)))
	{
		MSG_BOX("Failed to Create : FreeList");
		Safe_Release(pInstance);
	}
	
	return pInstance;
}

void CFreeList::Free()
{
	__super::Free();
	m_FreeBlocks.clear();
}
