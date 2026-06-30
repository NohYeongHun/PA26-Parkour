#include "EnginePch.h"
#include "CharacterContactListenerImpl.h"

#include "CollideComponent.h"

CharacterContactListenerImpl::CharacterContactListenerImpl()
{
	
}

CharacterContactListenerImpl::CharacterContactListenerImpl(BodyInterface* pInterface)
	: m_pBodyInterface { pInterface }
{
}

void CharacterContactListenerImpl::OnContactAdded(const CharacterVirtual* inCharacter, const BodyID& inBodyID2, const SubShapeID& inSubShapeID2, RVec3Arg inContactPosition, Vec3Arg inContactNormal, CharacterContactSettings& ioSettings)
{
	if(EMotionType::Static ==  m_pBodyInterface->GetMotionType(inBodyID2))
		ioSettings.mCanPushCharacter = true;
	else
		ioSettings.mCanPushCharacter = false;
	//
	//COLLISION_DATA* pSrcData = reinterpret_cast<COLLISION_DATA*>(inCharacter->GetUserData());
	//COLLISION_DATA* pDstData = reinterpret_cast<COLLISION_DATA*>(m_pBodyInterface->GetUserData(inBodyID2));
	//
	//ContactManifold Manifold;
	//ZeroMemory(&Manifold, sizeof(ContactManifold));
	//Manifold.mBaseOffset = inContactPosition;
	//Manifold.mWorldSpaceNormal = inContactNormal;
	//
	//pSrcData->pComponent->OnCollide_Enter(m_pBodyInterface->GetObjectLayer(inBodyID2), pDstData->pDesc, Manifold);
	//
	//// Kinematic일때만 Body Callback
	//if (EMotionType::Kinematic == m_pBodyInterface->GetMotionType(inBodyID2))
	//{
	//	Manifold.mWorldSpaceNormal = inContactNormal * -1.f;
	//	pDstData->pComponent->OnCollide_Enter(m_pBodyInterface->GetObjectLayer(inCharacter->GetInnerBodyID()), pSrcData->pDesc, Manifold);
	//}
}

void CharacterContactListenerImpl::OnContactPersisted(const CharacterVirtual* inCharacter, const BodyID& inBodyID2, const SubShapeID& inSubShapeID2, RVec3Arg inContactPosition, Vec3Arg inContactNormal, CharacterContactSettings& ioSettings)
{
	if (EMotionType::Static == m_pBodyInterface->GetMotionType(inBodyID2))
		ioSettings.mCanPushCharacter = true;
	else
		ioSettings.mCanPushCharacter = false;
	//
	//COLLISION_DATA* pSrcData = reinterpret_cast<COLLISION_DATA*>(inCharacter->GetUserData());
	//COLLISION_DATA* pDstData = reinterpret_cast<COLLISION_DATA*>(m_pBodyInterface->GetUserData(inBodyID2));
	//
	//ContactManifold Manifold;
	//ZeroMemory(&Manifold, sizeof(ContactManifold));
	//Manifold.mBaseOffset = inContactPosition;
	//Manifold.mWorldSpaceNormal = inContactNormal;
	//
	//pSrcData->pComponent->OnCollide_Enter(m_pBodyInterface->GetObjectLayer(inBodyID2), pDstData->pDesc, Manifold);
	//
	//// Kinematic일때만 Body Callback
	//if (EMotionType::Kinematic == m_pBodyInterface->GetMotionType(inBodyID2) && 
	//	0 != m_pBodyInterface->GetObjectLayer(inBodyID2))
	//{
	//	Manifold.mWorldSpaceNormal = inContactNormal * -1.f;
	//	pDstData->pComponent->OnCollide_During(m_pBodyInterface->GetObjectLayer(inCharacter->GetInnerBodyID()), pSrcData->pDesc, Manifold);
	//}
}

void CharacterContactListenerImpl::OnContactRemoved(const CharacterVirtual* inCharacter, const BodyID& inBodyID2, const SubShapeID& inSubShapeID2)
{
	int a = 0;
}

void CharacterContactListenerImpl::OnCharacterContactAdded(const CharacterVirtual* inCharacter, const CharacterVirtual* inOtherCharacter, const SubShapeID& inSubShapeID2, RVec3Arg inContactPosition, Vec3Arg inContactNormal, CharacterContactSettings& ioSettings)
{
	ioSettings.mCanPushCharacter = false;
	//
	//COLLISION_DATA* pSrcData = reinterpret_cast<COLLISION_DATA*>(inCharacter->GetUserData());
	//COLLISION_DATA* pDstData = reinterpret_cast<COLLISION_DATA*>(inOtherCharacter->GetUserData());
	//
	//ContactManifold Manifold;
	//ZeroMemory(&Manifold, sizeof(ContactManifold));
	//Manifold.mBaseOffset = inContactPosition;
	//Manifold.mWorldSpaceNormal = inContactNormal;
	//
	//pSrcData->pComponent->OnCollide_Enter(m_pBodyInterface->GetObjectLayer(inOtherCharacter->GetInnerBodyID()), pDstData->pDesc, Manifold);
}

void CharacterContactListenerImpl::OnCharacterContactPersisted(const CharacterVirtual* inCharacter, const CharacterVirtual* inOtherCharacter, const SubShapeID& inSubShapeID2, RVec3Arg inContactPosition, Vec3Arg inContactNormal, CharacterContactSettings& ioSettings)
{
	ioSettings.mCanPushCharacter = false;
	//
	//COLLISION_DATA* pSrcData = reinterpret_cast<COLLISION_DATA*>(inCharacter->GetUserData());
	//COLLISION_DATA* pDstData = reinterpret_cast<COLLISION_DATA*>(inOtherCharacter->GetUserData());
	//
	//ContactManifold Manifold;
	//ZeroMemory(&Manifold, sizeof(ContactManifold));
	//Manifold.mBaseOffset = inContactPosition;
	//Manifold.mWorldSpaceNormal = inContactNormal;
	//
	//
	//
	//pSrcData->pComponent->OnCollide_During(m_pBodyInterface->GetObjectLayer(inOtherCharacter->GetInnerBodyID()), pDstData->pDesc, Manifold);
}

void CharacterContactListenerImpl::OnCharacterContactRemoved(const CharacterVirtual* inCharacter, const CharacterID& inOtherCharacterID, const SubShapeID& inSubShapeID2)
{
	int a = 0;
}
