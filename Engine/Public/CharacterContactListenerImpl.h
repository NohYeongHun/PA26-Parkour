#pragma once
#include "Engine_Define.h"

class CharacterContactListenerImpl : public CharacterContactListener
{
public:
	explicit CharacterContactListenerImpl();
	explicit CharacterContactListenerImpl(BodyInterface* pInterface);
	virtual ~CharacterContactListenerImpl() = default;
	// 罹먮┃?곌? ?몄떇?섎뒗 臾쇱껜???띾룄瑜?議곗젙?????ъ슜
	// 而⑤쿋?댁뼱 踰⑦듃 ?꾩뿉 ?덉쓣 ??罹먮┃?곌? 踰⑦듃 ?대룞?띾룄瑜?諛섏쁺???吏곸씪 ???ъ슜
	virtual void						OnAdjustBodyVelocity(const CharacterVirtual* inCharacter, const Body& inBody2, Vec3& ioLinearVelocity, Vec3& ioAngularVelocity) { /* Do nothing, the linear and angular velocity are already filled in */ }
	// ?뱀젙 Body? 異⑸룎 ?? True 諛섑솚
	virtual _bool						OnContactValidate(const CharacterVirtual* inCharacter, const BodyID& inBodyID2, const SubShapeID& inSubShapeID2) { return true; }
	// ?뱀젙 Character? 異⑸룎 ?? True 諛섑솚
	virtual _bool						OnCharacterContactValidate(const CharacterVirtual* inCharacter, const CharacterVirtual* inOtherCharacter, const SubShapeID& inSubShapeID2) { return true; }
	
	// Character媛 Body? 異⑸룎 Begin???몄텧
	virtual void						OnContactAdded(const CharacterVirtual* inCharacter, const BodyID& inBodyID2, const SubShapeID& inSubShapeID2, RVec3Arg inContactPosition, Vec3Arg inContactNormal, CharacterContactSettings& ioSettings) override;
	// Character媛 Body? 異⑸룎 OnGoing???몄텧
	virtual void						OnContactPersisted(const CharacterVirtual* inCharacter, const BodyID& inBodyID2, const SubShapeID& inSubShapeID2, RVec3Arg inContactPosition, Vec3Arg inContactNormal, CharacterContactSettings& ioSettings) override;
	// Character媛 Body? 異⑸룎 End???몄텧
	virtual void						OnContactRemoved(const CharacterVirtual* inCharacter, const BodyID& inBodyID2, const SubShapeID& inSubShapeID2) override;

	// Character媛 Character? 異⑸룎 Begin???몄텧
	virtual void						OnCharacterContactAdded(const CharacterVirtual* inCharacter, const CharacterVirtual* inOtherCharacter, const SubShapeID& inSubShapeID2, RVec3Arg inContactPosition, Vec3Arg inContactNormal, CharacterContactSettings& ioSettings) override;
	// Character媛 Character? 異⑸룎 OnGoing???몄텧
	virtual void						OnCharacterContactPersisted(const CharacterVirtual* inCharacter, const CharacterVirtual* inOtherCharacter, const SubShapeID& inSubShapeID2, RVec3Arg inContactPosition, Vec3Arg inContactNormal, CharacterContactSettings& ioSettings) override;
	// Character媛 Character? 異⑸룎 End???몄텧
	virtual void						OnCharacterContactRemoved(const CharacterVirtual* inCharacter, const CharacterID& inOtherCharacterID, const SubShapeID& inSubShapeID2) override;

	// Character? Body媛 異⑸룎 ?덉쓣 ?? 臾쇰━ ?곗궛 ???ㅼ뼱?ㅻ뒗 ?⑥닔
	virtual void						OnContactSolve(const CharacterVirtual* inCharacter, const BodyID& inBodyID2, const SubShapeID& inSubShapeID2, RVec3Arg inContactPosition, Vec3Arg inContactNormal, Vec3Arg inContactVelocity, const PhysicsMaterial* inContactMaterial, Vec3Arg inCharacterVelocity, Vec3& ioNewCharacterVelocity) { /* Default do nothing */ }
	// Character? Character媛 異⑸룎 ?덉쓣 ?? 臾쇰━ ?곗궛 ???ㅼ뼱?ㅻ뒗 ?⑥닔
	virtual void						OnCharacterContactSolve(const CharacterVirtual* inCharacter, const CharacterVirtual* inOtherCharacter, const SubShapeID& inSubShapeID2, RVec3Arg inContactPosition, Vec3Arg inContactNormal, Vec3Arg inContactVelocity, const PhysicsMaterial* inContactMaterial, Vec3Arg inCharacterVelocity, Vec3& ioNewCharacterVelocity) { /* Default do nothing */ }

private:
	BodyInterface*					m_pBodyInterface = { nullptr };
};

