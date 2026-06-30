#include"EnginePch.h"
#include "Model_Manager.h"
#include"GameInstance.h"
#include"Model_Streaming.h"
#include"StaticObject.h"

#define INVALID_OFFSET UINT_MAX

CModel_Manager::CModel_Manager(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:m_pDevice(pDevice), m_pContext(pContext),m_pGameInstance(CGameInstance::GetInstance())
{
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pContext); 
	Safe_AddRef(m_pGameInstance);
}

HRESULT CModel_Manager::Initialize(_uint iMaxLevel)
{
	m_iMaxLevel = iMaxLevel;
	m_ModelPrototypes = new unordered_map<_string, class CModel_Streaming*>[iMaxLevel];
	_float fSize = 0.01f;
	XMStoreFloat4x4(&m_PreTransformMatrix, XMMatrixScaling(fSize, fSize, fSize));
#ifndef _DEBUG
	m_pBufferPool[0] = CBufferPool::Create(m_pDevice, m_pContext, 70, 20);
	m_pBufferPool[1] = CBufferPool::Create(m_pDevice, m_pContext, 55, 10);
	m_pBufferPool[2] = CBufferPool::Create(m_pDevice, m_pContext, 35, 6);
	m_pBufferPool[3] = CBufferPool::Create(m_pDevice, m_pContext, 20, 3);
#else
		m_pBufferPool[0] = CBufferPool::Create(m_pDevice, m_pContext, 256, sizeof(VTXMESH));
	m_pBufferPool[1] = CBufferPool::Create(m_pDevice, m_pContext, 128, sizeof(VTXMESH));
	m_pBufferPool[2] = CBufferPool::Create(m_pDevice, m_pContext, 64, sizeof(VTXMESH));
	m_pBufferPool[3] = CBufferPool::Create(m_pDevice, m_pContext, 64, sizeof(VTXMESH));
#endif
	D3D11_BUFFER_DESC StagingDesc = {};
	StagingDesc.ByteWidth = 1024 * 1024 * 64;
	StagingDesc.Usage = D3D11_USAGE_STAGING;
	StagingDesc.BindFlags = 0;
	StagingDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	if (FAILED(m_pDevice->CreateBuffer(&StagingDesc, nullptr, &m_pStagingBuffer)))
		CRASH("Failed");
	m_iSearchIndex = m_ModelPrototypes[0].begin();

	m_DelayedNotice.reserve(200);
	return S_OK;
}

void CModel_Manager::Update(_float fTimeDelta)
{
	m_fTotalPlayTime = m_pGameInstance->Get_PlayTime();
	m_iCurrentLoadCnt = 0;

	for (_uint i = 0; i < m_DeleteList.size(); ++i)
	{
		auto& Data = m_DeleteList[i];
		Data.iLifeCount--;
		if (Data.iLifeCount <= 0)
		{
			m_pBufferPool[Data.iLODIndex]->FreeMemory_Vertex(Data.VertexOffset, Data.VertexSize);
			m_pBufferPool[Data.iLODIndex]->FreeMemory_Index(Data.IndexOffset, Data.IndexSize);
			if (i != m_DeleteList.size() - 1)
				m_DeleteList[i] = m_DeleteList.back();
			m_DeleteList.pop_back();
		}
		else
			i++;
	}

	if (!m_DelayedNotice.empty())
	{
		for (_uint i = 0; i < m_DelayedNotice.size();)
		{
			if (m_DelayedNotice[i].iDelayFrame >= 2)
			{
				m_DelayedNotice[i].pModel->Get_MeshState(m_DelayedNotice[i].iLODIndex).store(LOADSTATE::LOADED);
				m_DelayedNotice[i].pModel = nullptr;
				
				m_DelayedNotice[i] = m_DelayedNotice.back();
				m_DelayedNotice.pop_back();
			}
			else
			{
				m_DelayedNotice[i].iDelayFrame++;
				i++;
			}
		}
	}

	if (!m_StagingData.empty())
	{
		vector<MODEL_DATA> pTempVector;
		{
			lock_guard<mutex> lock(m_StagingMutex);
			pTempVector = move(m_StagingData);
		}

		for (auto& Data : pTempVector)
		{
			//Data의 Data.LoadData 개수가 메쉬의 개수.
			vector< SHARED_DATA_DESC>* pData = Data.pModel->Get_MeshDesc(Data.iLODIndex);
			pData->clear();
			if (Data.LoadData.empty())
				continue;
			for (_uint i = 0; i < Data.LoadData.size(); ++i)
			{
				_uint VertexSize = Data.LoadData[i].VertexData.size() * sizeof(VTXMESH);
				_uint VertexOffset = m_pBufferPool[Data.iLODIndex]->Allocate_Vertex(VertexSize);

				if (VertexOffset == INVALID_OFFSET)
					CRASH("Failed");

				D3D11_BOX PoolBox{};
				PoolBox.left = VertexOffset;
				PoolBox.top = 0;
				PoolBox.front = 0;
				PoolBox.right = VertexOffset + VertexSize;
				PoolBox.bottom = 1;
				PoolBox.back = 1;

				m_pContext->UpdateSubresource(m_pBufferPool[Data.iLODIndex]->Get_VertexBuffer(), 0, &PoolBox, Data.LoadData[i].VertexData.data(), 0, 0);

				D3D11_MAPPED_SUBRESOURCE StagingDesc{};

				_uint IndexSize = Data.LoadData[i].IndexData.size() * sizeof(_uint);
				_uint IndexOffSet = m_pBufferPool[Data.iLODIndex]->Allocate_Index(IndexSize);

				if (IndexOffSet == INVALID_OFFSET)
					CRASH("Failed");

				PoolBox.left = IndexOffSet;
				PoolBox.top = 0;
				PoolBox.front = 0;
				PoolBox.right = IndexOffSet + IndexSize;
				PoolBox.bottom = 1;
				PoolBox.back = 1;

				m_pContext->UpdateSubresource(m_pBufferPool[Data.iLODIndex]->Get_IndexBuffer(), 0, &PoolBox, Data.LoadData[i].IndexData.data(), 0, 0);

				SHARED_DATA_DESC Desc{};
				Desc.IndexOffset = IndexOffSet / sizeof(_uint);
				Desc.IndexSize = IndexSize;
				Desc.NumIndices = Data.LoadData[i].iNumIndices;
				Desc.VertexOffset = VertexOffset / sizeof(VTXMESH);
				Desc.VertexSize = VertexSize;

				pData->push_back(Desc);
			}
			PEDDING_DATA PeddingData{};
			PeddingData.pModel = Data.pModel;
			PeddingData.iLODIndex = Data.iLODIndex;
			m_DelayedNotice.push_back(PeddingData);
		}
		Release_Vector(pTempVector);
	}

	if (m_ModelPrototypes[m_iCurrentLevel].empty())
		return;

	if (m_iSearchIndex == m_ModelPrototypes[m_iCurrentLevel].end())
		m_iSearchIndex = m_ModelPrototypes[m_iCurrentLevel].begin();
	_uint iCheckCount = { 0 };
	while (iCheckCount < m_iCheckPerFrame && m_iSearchIndex != m_ModelPrototypes[m_iCurrentLevel].end())
	{
		CModel_Streaming* pModel = m_iSearchIndex->second;
		for (_uint i = 0; i < 3; ++i)
		{
			if (pModel->Is_RenderTimeOver(i) && pModel->Get_MeshState(i) == LOADSTATE::LOADED)
			{
				if (pModel->Get_MeshDesc(0)[i].empty())
					continue;
				vector<SHARED_DATA_DESC>* pMeshVector = pModel->Get_MeshDesc(i);
				if (pMeshVector->empty())
					continue;
				for (auto& pDesc : *pMeshVector)
				{
					DELETE_DATA Data{};
					Data.iLifeCount = 3;
					Data.iLODIndex = i;
					Data.IndexOffset = pDesc.IndexOffset * sizeof(_uint);
					Data.IndexSize = pDesc.IndexSize;
					Data.VertexOffset = pDesc.VertexOffset * sizeof(VTXMESH);
					Data.VertexSize = pDesc.VertexSize;

					m_DeleteList.push_back(Data);
				}
				pModel->Get_MeshState(i).store(LOADSTATE::NOTLOADED);
			}
		}
		m_iSearchIndex++;
		iCheckCount++;
	}

}

HRESULT CModel_Manager::RegisterPrototype(const _char* pFilePath, CModel_Streaming* pModel)
{
	if (!pModel)
		CRASH("Failed");

	m_ModelPrototypes[m_iCurrentLevel].emplace(pFilePath, pModel);
	Safe_AddRef(pModel);

	return S_OK;
}

void CModel_Manager::RequestData(CModel_Streaming* pModel, const _string& pFilePath, _uint iLODIndex)
{
	atomic<LOADSTATE>& LoadState = pModel->Get_MeshState(iLODIndex);
	
	if (LoadState != LOADSTATE::NOTLOADED)
		return;

	if (m_iCurrentLoadCnt > 5)
		return;

	LOADSTATE ExpectedState = LOADSTATE::NOTLOADED;
	if (LoadState.compare_exchange_strong(ExpectedState, LOADSTATE::LOADING))
	{
		m_iCurrentLoadCnt++;
		m_pGameInstance->Add_Work([=, lModel = pModel, lFilePath = pFilePath, liLODIndex = iLODIndex, Matrix = XMLoadFloat4x4(&m_PreTransformMatrix)]() {
			LoadData(lModel, lFilePath, liLODIndex, Matrix);
			});
	}
}

void CModel_Manager::SetUp_Data(CModel_Streaming* pModel, const _string& pFilePath, _uint iLODIndex)
{
	atomic<LOADSTATE>& LoadState = pModel->Get_MeshState(iLODIndex);

	if (LoadState != LOADSTATE::NOTLOADED)
		return;

	LOADSTATE ExpectedState = LOADSTATE::NOTLOADED;
	if (LoadState.compare_exchange_strong(ExpectedState, LOADSTATE::LOADING))
	{
		m_pGameInstance->Add_Work([=, lModel = pModel, lFilePath = pFilePath, liLODIndex = iLODIndex, Matrix = XMLoadFloat4x4(&m_PreTransformMatrix)]() {
			LoadData(lModel, lFilePath, liLODIndex, Matrix, true);
			});
	}
}

void CModel_Manager::LoadData(CModel_Streaming* pModel, const _string& pFilePath, _uint iLODIndex,_fmatrix PreMatrix, _bool IsInitialize)
{
	ifstream File(pFilePath + to_string(iLODIndex) + ".dat", ios::binary);
	if (!File.is_open())
		CRASH("Failed to Open File");

	_uint iNumMeshes = 0;
	File.read(reinterpret_cast<_char*>(&iNumMeshes), sizeof(_uint));

	if (iNumMeshes > 1000 || iNumMeshes == 0)
		CRASH("Invalid Mesh Count: Memory Corruption Suspected");

	MODEL_DATA Datas = Acquire_Vector();
	Datas.pModel = pModel;
	Datas.LoadData.resize(iNumMeshes);
	Datas.iLODIndex = iLODIndex;

	for (_uint i = 0; i < iNumMeshes; ++i)
	{
		_uint iNumVertices = {};
		vector<VTXMESH>& pVertices = Datas.LoadData[i].VertexData;
		vector<_uint>& pIndices = Datas.LoadData[i].IndexData;
		_uint iNumMaterialIndex = {};
		File.read(reinterpret_cast<_char*>(&iNumVertices), sizeof(_uint));

		File.read(reinterpret_cast<_char*>(&Datas.LoadData[i].iNumIndices), sizeof(_uint));
		Datas.LoadData[i].iNumIndices = Datas.LoadData[i].iNumIndices * 3;

		File.read(reinterpret_cast<_char*>(&iNumMaterialIndex), sizeof(_uint));

		Datas.LoadData[i].VertexData.resize(iNumVertices);
		File.read(reinterpret_cast<_char*>(Datas.LoadData[i].VertexData.data()), sizeof(VTXMESH) * iNumVertices);

		Datas.LoadData[i].IndexData.resize(Datas.LoadData[i].iNumIndices);
		File.read(reinterpret_cast<_char*>(Datas.LoadData[i].IndexData.data()), sizeof(_uint) * Datas.LoadData[i].iNumIndices);

		vector<_float3> vVertexPos;
		for (size_t j = 0; j < iNumVertices; ++j)
		{
			XMStoreFloat3(&Datas.LoadData[i].VertexData[j].vPosition, XMVector3TransformCoord(XMLoadFloat3(&Datas.LoadData[i].VertexData[j].vPosition), PreMatrix));
			vVertexPos.push_back(Datas.LoadData[i].VertexData[j].vPosition);
			XMStoreFloat3(&Datas.LoadData[i].VertexData[j].vNormal, XMVector3TransformNormal(XMLoadFloat3(&Datas.LoadData[i].VertexData[j].vNormal), PreMatrix));
			XMStoreFloat3(&Datas.LoadData[i].VertexData[j].vTangent, XMVector3TransformNormal(XMLoadFloat3(&Datas.LoadData[i].VertexData[j].vTangent), PreMatrix));
			XMStoreFloat3(&Datas.LoadData[i].VertexData[j].vBinormal, XMVector3TransformNormal(XMLoadFloat3(&Datas.LoadData[i].VertexData[j].vBinormal), PreMatrix));
		}
		if (iLODIndex == 0 && IsInitialize)
		{
			Datas.pModel->Set_RigidData(vVertexPos, Datas.LoadData[i].IndexData, i);
		}
	}

	{
		lock_guard<mutex> lock(m_StagingMutex);
		m_StagingData.push_back(move(Datas));
	}
}

CModel_Manager::MODEL_DATA CModel_Manager::Acquire_Vector()
{
	{
		lock_guard<mutex> lock(m_DataPoolMutex);
		if (m_DataPool.empty())
			return MODEL_DATA();

		MODEL_DATA Data = move(m_DataPool.back());
		m_DataPool.pop_back();
		return Data;
	}
}

void CModel_Manager::Release_Vector(vector<MODEL_DATA>& data)
{
	for (auto& Data : data)
		Data.pModel = nullptr;

	{
		lock_guard<mutex> lock(m_DataPoolMutex);
		if (m_DataPool.empty())
			m_DataPool = move(data);
		else
			m_DataPool.insert(m_DataPool.end(), make_move_iterator(data.begin()), make_move_iterator(data.end()));
	}
}

void CModel_Manager::Bind_SharedBuffer(_uint iLODIndex, ID3D11DeviceContext** pDC, _uint iNumThread)
{
	for (_uint i = 0; i < iNumThread; ++i)
		m_pBufferPool[iLODIndex]->Bind_BufferPool(pDC[i]);
}

void CModel_Manager::Bind_SharedBuffer(_uint iLODIndex, ID3D11DeviceContext* pDC)
{
	m_pBufferPool[iLODIndex]->Bind_BufferPool(pDC);
}

void CModel_Manager::Destroy_RigidData()
{
	for (auto& pModel : m_ModelPrototypes[m_iCurrentLevel])
		pModel.second->Destroy_RigidData();
}

void CModel_Manager::Clear_Resource(_uint iLevel)
{
	for (auto& pPair : m_ModelPrototypes[iLevel])
		Safe_Release(pPair.second);
	m_ModelPrototypes[iLevel].clear();
	m_pBufferPool[0]->Clear_Resource();
	m_pBufferPool[1]->Clear_Resource();
	m_pBufferPool[2]->Clear_Resource();
	m_pBufferPool[3]->Clear_Resource();
}

void CModel_Manager::Change_Level(_uint iLevel)
{
	m_iCurrentLevel = iLevel;
	m_iSearchIndex = m_ModelPrototypes[iLevel].begin();
}

void CModel_Manager::LoadLastLOD()
{
	for (auto& pModel : m_ModelPrototypes[m_iCurrentLevel])
	{
		pModel.second->RequestModel();
		for (_uint i = 0; i < 4; ++i)
			pModel.second->Get_SharedBuffers(i, m_pBufferPool[i]->Get_VertexBuffer(), m_pBufferPool[i]->Get_IndexBuffer());
	}

	m_pGameInstance->Wait_Thread_End();
	for (auto& Data : m_StagingData)
	{
		//Data의 Data.LoadData 개수가 메쉬의 개수.
		vector< SHARED_DATA_DESC>* pData = Data.pModel->Get_MeshDesc(Data.iLODIndex);
		pData->clear();
		for (_uint i = 0; i < static_cast<_uint>(Data.LoadData.size()); ++i)
		{
			_uint VertexSize = static_cast<_uint>(Data.LoadData[i].VertexData.size()) * sizeof(VTXMESH);
			_uint VertexOffset = m_pBufferPool[Data.iLODIndex]->Allocate_Vertex(VertexSize);

			if (VertexOffset == INVALID_OFFSET)
				CRASH("Failed");
			
			D3D11_MAPPED_SUBRESOURCE StagingDesc{};
			D3D11_BOX PoolBox{};
			PoolBox.left = VertexOffset;
			PoolBox.top = 0;
			PoolBox.front = 0;
			PoolBox.right = VertexOffset + VertexSize;
			PoolBox.bottom = 1;
			PoolBox.back = 1;

			m_pContext->UpdateSubresource(m_pBufferPool[Data.iLODIndex]->Get_VertexBuffer(), 0, &PoolBox, Data.LoadData[i].VertexData.data(), 0, 0);


			_uint IndexSize = static_cast<_uint>(Data.LoadData[i].IndexData.size()) * sizeof(_uint);
			_uint IndexOffSet = m_pBufferPool[Data.iLODIndex]->Allocate_Index(IndexSize);

			if (IndexOffSet == INVALID_OFFSET)
				CRASH("Failed");

			PoolBox.left = IndexOffSet;
			PoolBox.top = 0;
			PoolBox.front = 0;
			PoolBox.right = IndexOffSet + IndexSize;
			PoolBox.bottom = 1;
			PoolBox.back = 1;

			m_pContext->UpdateSubresource(m_pBufferPool[Data.iLODIndex]->Get_IndexBuffer(), 0, &PoolBox, Data.LoadData[i].IndexData.data(), 0, 0);

			SHARED_DATA_DESC Desc{};
			Desc.IndexOffset = IndexOffSet / sizeof(_uint);
			Desc.IndexSize = IndexSize;
			Desc.NumIndices = Data.LoadData[i].iNumIndices;
			Desc.VertexOffset = VertexOffset / sizeof(VTXMESH);
			Desc.VertexSize = VertexSize;


			pData->push_back(Desc);
		}
		Data.pModel->Get_MeshState(Data.iLODIndex).store(LOADSTATE::LOADED);
	}
	Release_Vector(m_StagingData);
	m_StagingData.clear();
	m_DataPool.clear();
	m_DataPool.shrink_to_fit();
}

CModel_Manager* CModel_Manager::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext,_uint iMaxLevel)
{
	CModel_Manager* pInstance = new CModel_Manager(pDevice, pContext);

	if (FAILED(pInstance->Initialize(iMaxLevel)))
	{
		MSG_BOX("Failed to Create : Model_Manager");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CModel_Manager::Free()
{
	__super::Free();
	Safe_Release(m_pDevice);
	Safe_Release(m_pContext);
	Safe_Release(m_pGameInstance);

	for (_uint i = 0; i < m_iMaxLevel; ++i)
	{
		for (auto& pPair : m_ModelPrototypes[i])
			Safe_Release(pPair.second);
		m_ModelPrototypes[i].clear();
	}
	Safe_Delete_Array(m_ModelPrototypes);

	for (_uint i = 0; i < 4; ++i)
		Safe_Release(m_pBufferPool[i]);
	Safe_Release(m_pStagingBuffer);

	//m_DeleteList.clear();
	//m_StagingData.clear();
	for (auto& pData : m_DataPool)
		pData.pModel = nullptr;
	//m_DataPool.clear();
}