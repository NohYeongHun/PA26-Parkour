#pragma once
#include "Engine_Define.h"

NS_BEGIN(Engine)

class CContactListenerImpl : public ContactListener
{
public:
	explicit CContactListenerImpl();
	explicit CContactListenerImpl(BodyInterface* pInterface);
	virtual ~CContactListenerImpl();

public:
	void		IsChangeLevel(_bool isChangeLevel) { m_isChangeLevel = isChangeLevel; }

public:
	void		Remove_Update();
	void		Clear_Resource();

public:
	virtual		ValidateResult	OnContactValidate(const Body& inBody1, const Body& inBody2, RVec3Arg inBaseOffset, const CollideShapeResult& inCollisionResult) override;
	virtual		void				OnContactAdded(const Body& inBody1, const Body& inBody2, const ContactManifold& inManifold, ContactSettings& ioSettings) override;
	virtual		void				OnContactPersisted(const Body& inBody1, const Body& inBody2, const ContactManifold& inManifold, ContactSettings& ioSettings) override;
	virtual		void				OnContactRemoved(const SubShapeIDPair& inSubShapePair) override;

private:
	BodyInterface*				m_pBodyInterface = { nullptr };

	_bool										m_isChangeLevel = { false };
	vector<pair<BodyID, BodyID>>	m_RemoveIDs;
};

NS_END