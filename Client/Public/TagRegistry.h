#pragma once
#include "Client_Define.h"

NS_BEGIN(Client)
class CStateBlackboard;

class CTagRegistry final
{
public:
	static HRESULT Load(const _string& strFilePath, CStateBlackboard* pBlackboard);
};
NS_END
