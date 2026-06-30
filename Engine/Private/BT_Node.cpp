#include "EnginePch.h"
#include "BT_Node.h"

CBT_Node::CBT_Node()
{
}

CBT_Node::CBT_Node(const CBT_Node& Prototype)
{
}

CBT_Node* CBT_Node::Clone(void* pArg)
{
	return nullptr;
}

void CBT_Node::Free()
{
	__super::Free();
}
