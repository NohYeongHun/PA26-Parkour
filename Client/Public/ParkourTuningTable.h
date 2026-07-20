#pragma once
#include "Base.h"
#include "Client_Define.h"

NS_BEGIN(Client)

struct VAULT_TUNING
{
	_float fMinApproachDot       = 0.9f;
	_float fHighVaultHeightRatio = 0.8f;
};

struct HIGH_MANTLE_TUNING
{
	_float fMinDepthMult   = 1.0f;
	_float fMinWidthMult   = 2.0f;
	_float fMinApproachDot = 0.5f;
};

struct LOW_MANTLE_TUNING
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

struct HANG_TUNING
{
	// 판정 (Judge_Hang)
	_float fMinTopHeightMult  = 1.1f;
	_float fMaxTopHeightMult  = 1.5f;
	_float fMinApproachDot    = 0.5f;
	_float fMaxNormalY        = 0.2f;
	// 행 포즈 (pose)
	_float fHangOffsetMult    = 1.25f;
	_float fWallOffset        = 0.1f; 
	_float fSnapTime          = 0.15f;
	// Hop 탐색 (probe)
	_float fHopDistLR         = 2.5f;
	_float fHopDistUp         = 2.0f;
	_float fHopDistDown       = 2.5f;
	_float fHopProbeRadius    = 0.3f;
	_float fStartClearance    = 0.3f;
	_float fMinNormalDot      = 0.7f;
	// 올라서기 (HopUp 폴백)
	_float fTopStandDepthMult = 1.0f;
	_float fStandProbeUp      = 1.0f;
	_float fStandProbeInset   = 0.3f;
	_float fStandMaxRise      = 0.7f;
};

struct PARKOUR_TUNING
{
	VAULT_TUNING   Vault;
	HIGH_MANTLE_TUNING  HIGH_MANTLE;
	LOW_MANTLE_TUNING   LOW_MANTLE;
	CLIMB_TUNING   Climb;
	WALLRUN_TUNING WallRun;
	HANG_TUNING    Hang;
	vector<PARKOUR_ACTION> Priority;
	_float fWallRunCooldown = 0.3f;
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
