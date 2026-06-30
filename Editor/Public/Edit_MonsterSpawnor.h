#pragma once
#include "GameObject.h"
#include"Editor_Enum.h"

NS_BEGIN(Engine)
class CRigidbody;
NS_END

NS_BEGIN(Editor)
class CEdit_MonsterSpawnor : public CGameObject
{
public:
	union MyFloat4 {
		_float4 float4;
		_float arr[4];
	};
	using TriggerCallback = function<void(void*)>;
	typedef struct tagTriggerBox
	{
		_float4x4* WorldMatrix = { nullptr };
		_float3 vExtends;
		_uint iLevel = ENUM_CLASS(LEVEL::MAP);
		OBJECTTYPE eObjectType;
		_uint iTriggerIndex = {};
	}TRIGGER;

	typedef struct tagSpawnDesc {
		MyFloat4 vMonsterSpawnorPos;

		MyFloat4 vMonsterPos1;
		_char szMonsterName1[MAX_PATH] = {};

		MyFloat4 vMonsterPos2;
		_char szMonsterName2[MAX_PATH] = {};

		MyFloat4 vMonsterPos3;
		_char szMonsterName3[MAX_PATH] = {};
	}SPAWN_DESC;


private:
	CEdit_MonsterSpawnor(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CEdit_MonsterSpawnor(const CEdit_MonsterSpawnor& Prototype);
	virtual ~CEdit_MonsterSpawnor() = default;

public:
	virtual		HRESULT		Initialize_Prototype()override;
	virtual		HRESULT		Initialize_Clone(void* pArg)override;
	virtual		void			Priority_Update(_float fTimeDelta)override;
	virtual		void			Update(_float fTimeDelta)override;
	virtual		void			Late_Update(_float fTimeDelta)override;


	virtual void Set_ImGuiOption();
	void Map_Load(SPAWN_DESC& Desc);
private:
	void Ready_Components(void* pArg);

private:
	CRigidbody* m_pRigidbodyCom = { nullptr };
	class CMap_Interface* m_pMapInterface = { nullptr };

	map<_uint, SPAWN_DESC> m_MonsterSpawn;
	SPAWN_DESC m_MonsterDesc = {};

private:
	_uint m_iSpawnorIndex = {};
	_float3 m_vExtends = {};
	_uint m_iPickedSpawnNum = {};
	MyFloat4 m_vMonsterPos1 = {};
	_char m_szMonsterName1 [MAX_PATH] = {};

	MyFloat4 m_vMonsterPos2 = {};
	_char m_szMonsterName2[MAX_PATH] = {};

	MyFloat4 m_vMonsterPos3 = {};
	_char m_szMonsterName3[MAX_PATH] = {};
public:
	static CEdit_MonsterSpawnor* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg)override;
	virtual void Free()override;

};
NS_END