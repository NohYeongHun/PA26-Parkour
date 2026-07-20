#include "ClientPch.h"
#include "ParkourTuningTable.h"
#include <fstream>

using json = nlohmann::json;

HRESULT CParkourTuningTable::Load(const _string& strFilePath)
{
	PARKOUR_TUNING NewTuning{};
	_string strError;
	if (FAILED(Parse(strFilePath, NewTuning, strError)))
	{
		MessageBoxA(nullptr, strError.c_str(), "ParkourTuningTable Load Failed", MB_OK);
		return E_FAIL;
	}

	m_strFilePath = strFilePath;
	m_Tuning = move(NewTuning);
	return S_OK;
}

void CParkourTuningTable::Reload()
{
	PARKOUR_TUNING NewTuning{};
	_string strError;
	if (FAILED(Parse(m_strFilePath, NewTuning, strError)))
	{
		MessageBoxA(nullptr, strError.c_str(), "ParkourTuningTable Reload Failed (기존 값 유지)", MB_OK);
		return;
	}

	m_Tuning = move(NewTuning);
	++m_iVersion;
}

HRESULT CParkourTuningTable::Parse(const _string& strFilePath, PARKOUR_TUNING& OutTuning, _string& strOutError)
{
	OutTuning.Priority = {
		PARKOUR_ACTION::LOW_VAULT, PARKOUR_ACTION::HIGH_VAULT,
		PARKOUR_ACTION::MANTLE,    PARKOUR_ACTION::WALL_RUN,
		PARKOUR_ACTION::CLIMB,     PARKOUR_ACTION::HANG };

	try
	{
		std::ifstream File(strFilePath);
		if (!File.is_open()) { strOutError = "파일 열기 실패: " + strFilePath; return E_FAIL; }
		json Root; File >> Root;

		auto GetF = [](const json& j, const char* k, _float fDef) -> _float {
			return j.contains(k) ? j[k].get<_float>() : fDef;
		};

		if (Root.contains("vault")) {
			OutTuning.Vault.fMinApproachDot       = GetF(Root["vault"], "minApproachDot", OutTuning.Vault.fMinApproachDot);
			OutTuning.Vault.fHighVaultHeightRatio = GetF(Root["vault"], "highVaultHeightRatio", OutTuning.Vault.fHighVaultHeightRatio);
		}
		if (Root.contains("mantle")) {
			OutTuning.Mantle.fMinDepthMult   = GetF(Root["mantle"], "minDepthMult", OutTuning.Mantle.fMinDepthMult);
			OutTuning.Mantle.fMinWidthMult   = GetF(Root["mantle"], "minWidthMult", OutTuning.Mantle.fMinWidthMult);
			OutTuning.Mantle.fMinApproachDot = GetF(Root["mantle"], "minApproachDot", OutTuning.Mantle.fMinApproachDot);
		}
		if (Root.contains("climb"))
			OutTuning.Climb.fMaxHeightRatio = GetF(Root["climb"], "maxHeightRatio", OutTuning.Climb.fMaxHeightRatio);
		if (Root.contains("wallrun")) {
			OutTuning.WallRun.fMinApproachDot   = GetF(Root["wallrun"], "minApproachDot", OutTuning.WallRun.fMinApproachDot);
			OutTuning.WallRun.fMaxNormalY       = GetF(Root["wallrun"], "maxNormalY", OutTuning.WallRun.fMaxNormalY);
			OutTuning.WallRun.fMaxStartDistMult = GetF(Root["wallrun"], "maxStartDistMult", OutTuning.WallRun.fMaxStartDistMult);
		}
		if (Root.contains("hang")) {
			OutTuning.Hang.fMinTopHeightMult  = GetF(Root["hang"], "minTopHeightMult",  OutTuning.Hang.fMinTopHeightMult);
			OutTuning.Hang.fMaxTopHeightMult  = GetF(Root["hang"], "maxTopHeightMult",  OutTuning.Hang.fMaxTopHeightMult);
			OutTuning.Hang.fMinApproachDot    = GetF(Root["hang"], "minApproachDot",    OutTuning.Hang.fMinApproachDot);
			OutTuning.Hang.fMaxNormalY        = GetF(Root["hang"], "maxNormalY",        OutTuning.Hang.fMaxNormalY);
			OutTuning.Hang.fHangOffsetMult    = GetF(Root["hang"], "hangOffsetMult",    OutTuning.Hang.fHangOffsetMult);
			OutTuning.Hang.fWallOffset        = GetF(Root["hang"], "wallOffset",        OutTuning.Hang.fWallOffset);
			OutTuning.Hang.fSnapTime          = GetF(Root["hang"], "snapTime",          OutTuning.Hang.fSnapTime);
			OutTuning.Hang.fHopDistLR         = GetF(Root["hang"], "hopDistLR",         OutTuning.Hang.fHopDistLR);
			OutTuning.Hang.fHopDistUp         = GetF(Root["hang"], "hopDistUp",         OutTuning.Hang.fHopDistUp);
			OutTuning.Hang.fHopDistDown       = GetF(Root["hang"], "hopDistDown",       OutTuning.Hang.fHopDistDown);
			OutTuning.Hang.fHopProbeRadius    = GetF(Root["hang"], "hopProbeRadius",    OutTuning.Hang.fHopProbeRadius);
			OutTuning.Hang.fStartClearance    = GetF(Root["hang"], "startClearance",    OutTuning.Hang.fStartClearance);
			OutTuning.Hang.fMinNormalDot      = GetF(Root["hang"], "minNormalDot",      OutTuning.Hang.fMinNormalDot);
			OutTuning.Hang.fTopStandDepthMult = GetF(Root["hang"], "topStandDepthMult", OutTuning.Hang.fTopStandDepthMult);
			OutTuning.Hang.fStandProbeUp      = GetF(Root["hang"], "standProbeUp",      OutTuning.Hang.fStandProbeUp);
			OutTuning.Hang.fStandProbeInset   = GetF(Root["hang"], "standProbeInset",   OutTuning.Hang.fStandProbeInset);
			OutTuning.Hang.fStandMaxRise      = GetF(Root["hang"], "standMaxRise",      OutTuning.Hang.fStandMaxRise);
		}
		if (Root.contains("cooldowns"))
			OutTuning.fWallRunCooldown = GetF(Root["cooldowns"], "WALL_RUN", OutTuning.fWallRunCooldown);
		if (Root.contains("priority")) {
			static const map<_string, PARKOUR_ACTION> NameMap = {
				{"LOW_VAULT",  PARKOUR_ACTION::LOW_VAULT},
				{"HIGH_VAULT", PARKOUR_ACTION::HIGH_VAULT},
				{"MANTLE",     PARKOUR_ACTION::MANTLE},
				{"CLIMB",      PARKOUR_ACTION::CLIMB},
				{"HANG",       PARKOUR_ACTION::HANG},
				{"WALL_RUN",   PARKOUR_ACTION::WALL_RUN} };
			OutTuning.Priority.clear();
			for (const auto& Name : Root["priority"]) {
				auto it = NameMap.find(Name.get<_string>());
				if (it == NameMap.end()) { strOutError = "미지의 액션 이름: " + Name.get<_string>(); return E_FAIL; }
				OutTuning.Priority.push_back(it->second);
			}
		}
		return S_OK;
	}
	catch (const json::exception& e) { strOutError = _string("JSON 처리 실패: ") + e.what(); return E_FAIL; }
}

CParkourTuningTable* CParkourTuningTable::Create(const _string& strFilePath)
{
	CParkourTuningTable* pInstance = new CParkourTuningTable();
	if (FAILED(pInstance->Load(strFilePath)))
	{
		Safe_Release(pInstance);
		MSG_BOX("Failed to Create : CParkourTuningTable");
		return nullptr;
	}
	return pInstance;
}

void CParkourTuningTable::Free()
{
	__super::Free();
}
