#include "EnginePch.h"
#include "Resource_Manager.h"

CResource_Manager::CResource_Manager(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: m_pDevice { pDevice }, m_pContext { pContext }
{
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pContext);
}

void CResource_Manager::Load_Resource(const _char* pFolderPath)
{
	if (nullptr == pFolderPath)
		CRASH("Folder Path");

	for (const auto& Entry : filesystem::recursive_directory_iterator(pFolderPath))
	{
		if (false == Entry.is_regular_file())
			continue;
		if (Entry.path().extension() != ".dds" && Entry.path().extension() != ".png")
			continue;

			// Folder
		//if (true == Entry.is_directory())
		//{
		//	Load_Resource(Entry.path().string().c_str());
		//}
		//// File
		//else 
		//if (Entry.path().extension() != ".dds" || Entry.path().extension() != ".png")
		//	continue;
		if(true == Entry.is_regular_file())
		{
			_string strFilePath = Entry.path().string();
			_string strResourceTag = Entry.path().stem().string();
			if (Entry.path().string().find("Default") != string::npos)
				int a = 0;
			// 이미 존재할 경우 CRASH
			if (nullptr != Find_Resource(strResourceTag))
				continue;

			HRESULT hr;
			ID3D11ShaderResourceView* pSRV = { nullptr };
			// dds
			if (strFilePath.find(".dds") != std::string::npos)
				hr = CreateDDSTextureFromFile(m_pDevice, StringToWString(strFilePath).c_str(), nullptr, &pSRV);
			else if (strFilePath.find(".png") != std::string::npos)
				hr = CreateWICTextureFromFile(m_pDevice, StringToWString(strFilePath).c_str(), nullptr, &pSRV);
			else
				continue;

			m_Resources.emplace(strResourceTag, pSRV);
		}
	}
}

ID3D11ShaderResourceView* CResource_Manager::Get_Resource(const _string& strResourceTag)
{
	ID3D11ShaderResourceView* pInstance = Find_Resource(strResourceTag);
	ASSERT_CRASH(pInstance);

	Safe_AddRef(pInstance);

	return pInstance;
}

void CResource_Manager::Clear_Resource()
{
	for (auto& Pair : m_Resources)
		Safe_Release(Pair.second);
	m_Resources.clear();
}

ID3D11ShaderResourceView* CResource_Manager::Find_Resource(const _string& strResourceTag)
{
	auto iter = m_Resources.find(strResourceTag);

	if (iter == m_Resources.end())
		return nullptr;

	return iter->second;
}

CResource_Manager* CResource_Manager::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	return new CResource_Manager(pDevice, pContext);
}

void CResource_Manager::Free()
{
	__super::Free();

	for (auto& Pair : m_Resources)
	{
		if (nullptr != Pair.second)
			Safe_Release(Pair.second);
	}

	Safe_Release(m_pDevice);
	Safe_Release(m_pContext);
}
