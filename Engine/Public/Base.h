#pragma once

#include "EnginePch.h"

NS_BEGIN(Engine)

class ENGINE_DLL CBase
{
protected:
	explicit CBase();
	explicit CBase(const CBase& Copy) = default;
	virtual ~CBase() = default;

public:
	_uint		AddRef(); // 
	_uint		Release(); // 

private:
	_uint						m_iRefCnt = {}; 	// 
	mutex						m_BaseMutex;

public:
	virtual void Free();
};

NS_END