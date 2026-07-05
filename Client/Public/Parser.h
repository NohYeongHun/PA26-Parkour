#pragma once
#include "Base.h"
NS_BEGIN(Client)

class CParser final : public CBase
{
public:
	typedef struct tagSpawnDesc {
		_float4 vMonsterSpawnorPos;

		_float4 vMonsterPos1;
		_char szMonsterName1[MAX_PATH] = {};

		_float4 vMonsterPos2;
		_char szMonsterName2[MAX_PATH] = {};

		_float4 vMonsterPos3;
		_char szMonsterName3[MAX_PATH] = {};
	}SPAWN_DESC;

	typedef struct tagEffectTag {
		_uint iEffectTag;
		_float4 vEffectPos;
	}MAPEFFECT;
private:
	explicit CParser(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CParser() = default;

public:
#pragma region MAP
	// Loader에서 Prototype 객체 생성
	void							Ready_Prototype_Map(const _char* pFilePath, LEVEL eLevel, const _char* pModelFilePath);
	// Level에서 Clone 객체 생성
	void							Clone_MapObjects(LEVEL eLevel);
#pragma endregion

private:
	void							Read_Map_Prototype(const _string pDataFilePath, LEVEL eLevel, const _char* pModelFilePath);
	void							Read_Map_Dat(LEVEL eLevel, const _string pFilePath);


public:
	HRESULT						Initialize();

private:
	class CGameInstance* m_pGameInstance = { nullptr };
	ID3D11Device* m_pDevice = { nullptr };
	ID3D11DeviceContext* m_pContext = { nullptr };

	vector<vector<_string>> m_Data;
	vector<MAPEFFECT> m_MapEffects;
	unordered_map<LEVEL, vector<const _char*>> m_LoadingMap;
	unordered_map<LEVEL, vector<SPAWN_DESC>> m_MonsterDesc;
public:
	static		CParser* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual		void				Free() override;
};

NS_END