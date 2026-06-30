#pragma once
#include "Base.h"

NS_BEGIN(Engine)
class CFreeList final : public CBase
{
private:
	CFreeList();
	virtual ~CFreeList() = default;

public:

	HRESULT Initialize(_uint iMemorySize);
	_uint Allocate(_uint iMemorySize);
	void Free(_uint iMemoryOffset, _uint iMemorySize);
	void Clear_Resource();

private:
	map<_uint, _uint> m_FreeBlocks;
	_uint m_iMemorySize = {};

public:

	static CFreeList* Create(_uint iMemorySize);
	virtual void Free()override;
};

NS_END