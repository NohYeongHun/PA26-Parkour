#pragma once
#include "Base.h"
#include "Client_Define.h"

NS_BEGIN(Client)

struct VAULT_TUNING
{
	_float fMinApproachDot       = 0.9f;
	_float fHighVaultHeightRatio = 0.8f;
};

struct MANTLE_TUNING
{
	_float fMinDepthMult   = 1.0f;
	_float fMinWidthMult   = 2.0f;
	_float fMinApproachDot = 0.5f;
};

struct CLIMB_TUNING
{
	_float fMaxHeightRatio = 1.5f;
};

struct WALLRUN_TUNING
{
	_float fMinApproachDot   = 0.85f;
	_float fMaxNormalY       = 0.1f;
	_float fMaxStartDistMult = 1.1f;
};

struct PARKOUR_TUNING
{
	VAULT_TUNING   Vault;
	MANTLE_TUNING  Mantle;
	CLIMB_TUNING   Climb;
	WALLRUN_TUNING WallRun;
	vector<PARKOUR_ACTION> Priority; // populated by CParkourTuningTable::Load
	_float fWallRunCooldown = 0.3f;  // TraceurGroundMove::FWALLRUN_COOLDOWN
};

class CParkourTuningTable final : public CBase
{
private:
	explicit CParkourTuningTable() = default;
	virtual ~CParkourTuningTable() = default;

public:
	HRESULT Load(const _string& strFilePath);
	void    Reload();
	_uint   Get_Version() const { return m_iVersion; }
	const PARKOUR_TUNING& Get() const { return m_Tuning; }

private:
	HRESULT Parse(const _string& strFilePath, PARKOUR_TUNING& OutTuning, _string& strOutError);

private:
	_string        m_strFilePath;
	PARKOUR_TUNING m_Tuning{};
	_uint          m_iVersion = 1;

public:
	static CParkourTuningTable* Create(const _string& strFilePath);
	virtual void Free() override;
};

NS_END
