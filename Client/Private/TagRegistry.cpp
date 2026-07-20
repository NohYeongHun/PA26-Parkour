#include "ClientPch.h"
#include "TagRegistry.h"
#include "StateBlackboard.h"
#include <fstream>

using json = nlohmann::json;

HRESULT CTagRegistry::Load(const _string& strFilePath, CStateBlackboard* pBlackboard)
{
	std::ifstream File(strFilePath);
	if (!File.is_open())
	{
		MessageBoxA(nullptr, ("태그 파일을 열 수 없음: " + strFilePath).c_str(), "TagRegistry Load Failed", MB_OK);
		return E_FAIL;
	}

	try
	{
		json Root;
		File >> Root;

		if (!Root.is_object() || !Root.contains("tags") || !Root["tags"].is_array())
		{
			MessageBoxA(nullptr, "최상위 \"tags\" 배열이 없음", "TagRegistry Load Failed", MB_OK);
			return E_FAIL;
		}

		for (const auto& Tag : Root["tags"])
		{
			if (!Tag.is_string())
			{
				MessageBoxA(nullptr, "\"tags\" 원소가 문자열이 아님", "TagRegistry Load Failed", MB_OK);
				return E_FAIL;
			}
			pBlackboard->Register_Bool(Tag.get<_string>());
		}
	}
	catch (const json::exception& e)
	{
		MessageBoxA(nullptr, (_string("JSON 처리 실패: ") + e.what()).c_str(), "TagRegistry Load Failed", MB_OK);
		return E_FAIL;
	}

	return S_OK;
}
