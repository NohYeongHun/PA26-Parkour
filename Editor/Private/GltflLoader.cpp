#include "EditorPch.h"
#include "GltfLoader.h"

CGltfLoader::CGltfLoader()
{
}

HRESULT CGltfLoader::Initialize()
{
	return S_OK;
}

void CGltfLoader::Update()
{
	ImGui::Begin("GLTF Model Load");

	if (ImGui::RadioButton("Anim", m_iAnim == 1)) m_iAnim = 1;
	if (ImGui::RadioButton("Character", m_iAnim == 2)) m_iAnim = 2;

	if(1 == m_iAnim) m_eType = MODELTYPE::ANIM;
	else if(2 == m_iAnim) m_eType = MODELTYPE::CHARACTER;

	if (ImGui::Button("Load Gltf"))
		m_isShowLoadFile = !m_isShowLoadFile;
	ImGui::SameLine();
	if (ImGui::Button("Save Model"))
		m_isShowSaveFile = !m_isShowSaveFile;

	if(true == m_isShowLoadFile)
		Load_File();
	if (true == m_isShowSaveFile)
		Save_File();

	// GLTF Model Info
	Show_Info();

	ImGui::End();
}

HRESULT CGltfLoader::Save_Dat_Anim(const _char* pFileName)
{
	if (nullptr == m_pAIScene)
		return E_FAIL;

	ofstream file(pFileName, ios::binary);

	if (false == file.is_open())
	{
		MSG_BOX("Model Save Fail");
		return E_FAIL;
	}

#pragma region BONE
	aiNode* pRoot = m_pAIScene->mRootNode;
	if (FAILED(Save_Bone(file, pRoot)))
		return E_FAIL;
#pragma endregion

#pragma region MESH
	file.write(reinterpret_cast<const _char*>(&m_pAIScene->mNumMeshes), sizeof(_uint));

	for (size_t i = 0; i < m_pAIScene->mNumMeshes; ++i)
	{
		aiMesh* pMesh = m_pAIScene->mMeshes[i];

		file.write(reinterpret_cast<const _char*>(&pMesh->mNumVertices), sizeof(_uint));
		file.write(reinterpret_cast<const _char*>(&pMesh->mNumFaces), sizeof(_uint));
		file.write(reinterpret_cast<const _char*>(&pMesh->mMaterialIndex), sizeof(_uint));
		file.write(reinterpret_cast<const _char*>(&pMesh->mNumBones), sizeof(_uint));

		
		VTXANIMMESH* Vertices = new VTXANIMMESH[pMesh->mNumVertices];
		ZeroMemory(Vertices, sizeof(VTXANIMMESH) * pMesh->mNumVertices);

		for (size_t j = 0; j < pMesh->mNumVertices; ++j)
		{
			memcpy(&Vertices[j].vPosition, &pMesh->mVertices[j], sizeof(_float3));
			memcpy(&Vertices[j].vNormal, &pMesh->mNormals[j], sizeof(_float3));
			memcpy(&Vertices[j].vTangent, &pMesh->mTangents[j], sizeof(_float3));
			memcpy(&Vertices[j].vBinormal, &pMesh->mBitangents[j], sizeof(_float3));
			memcpy(&Vertices[j].vTexcoord, &pMesh->mTextureCoords[0][j], sizeof(_float2));
		}

		for (size_t j = 0; j < pMesh->mNumBones; ++j)
		{
			aiBone* pBone = pMesh->mBones[j];
			aiString strBoneName = pBone->mName;
			_uint iLength = strBoneName.length;
			file.write(reinterpret_cast<const _char*>(&iLength), sizeof(_uint));
			file.write(reinterpret_cast<const _char*>(strBoneName.data), iLength);
			file.write(reinterpret_cast<const _char*>(&pBone->mOffsetMatrix), sizeof(_float4x4));
			// Bone Weight
			for (size_t k = 0; k < pBone->mNumWeights; ++k)
			{
				aiVertexWeight VertexWeight = pBone->mWeights[k];
				_uint iVertexID = VertexWeight.mVertexId;
				if (0 == Vertices[iVertexID].vBlendWeight.x)
				{
					Vertices[iVertexID].vBlendIndex.x = j;
					Vertices[iVertexID].vBlendWeight.x = VertexWeight.mWeight;
				}
				else if (0 == Vertices[iVertexID].vBlendWeight.y)
				{
					Vertices[iVertexID].vBlendIndex.y = j;
					Vertices[iVertexID].vBlendWeight.y = VertexWeight.mWeight;
				}
				else if (0 == Vertices[iVertexID].vBlendWeight.z)
				{
					Vertices[iVertexID].vBlendIndex.z = j;
					Vertices[iVertexID].vBlendWeight.z = VertexWeight.mWeight;
				}
				else if (0 == Vertices[iVertexID].vBlendWeight.w)
				{
					Vertices[iVertexID].vBlendIndex.w = j;
					Vertices[iVertexID].vBlendWeight.w = VertexWeight.mWeight;
				}
			}
		}

		file.write(reinterpret_cast<const _char*>(Vertices), sizeof(VTXANIMMESH) * pMesh->mNumVertices);
		Safe_Delete_Array(Vertices);

		_uint* Indices = new _uint[pMesh->mNumFaces * 3];
		_uint iIndex = {};
		for (size_t j = 0; j < pMesh->mNumFaces; ++j)
		{
			Indices[iIndex++] = pMesh->mFaces[j].mIndices[0];
			Indices[iIndex++] = pMesh->mFaces[j].mIndices[1];
			Indices[iIndex++] = pMesh->mFaces[j].mIndices[2];
		}
		file.write(reinterpret_cast<const _char*>(Indices), sizeof(_uint) * pMesh->mNumFaces * 3);
		Safe_Delete_Array(Indices);
	}

	file.close();
#pragma endregion

	return S_OK;
}


HRESULT CGltfLoader::Save_Dat_Character(const _char* pFileName)
{
	if (nullptr == m_pAIScene)
		return E_FAIL;

	ofstream file(pFileName, ios::binary);

	if (false == file.is_open())
	{
		MSG_BOX("Model Save Fail");
		return E_FAIL;
	}

#pragma region BONE 계층 구조 저장
	aiNode* pRoot = m_pAIScene->mRootNode;
	if (FAILED(Save_Bone(file, pRoot)))
		return E_FAIL;
#pragma endregion

#pragma region MESH 및 Shape key 데이터 저장.
	file.write(reinterpret_cast<const _char*>(&m_pAIScene->mNumMeshes), sizeof(_uint)); // 전체 메쉬 개수.

	for (size_t i = 0; i < m_pAIScene->mNumMeshes; ++i)
	{
		aiMesh* pMesh = m_pAIScene->mMeshes[i];

		// 1. 기본 메쉬 정보 저장.
		file.write(reinterpret_cast<const _char*>(&pMesh->mNumVertices), sizeof(_uint));
		file.write(reinterpret_cast<const _char*>(&pMesh->mNumFaces), sizeof(_uint));
		file.write(reinterpret_cast<const _char*>(&pMesh->mMaterialIndex), sizeof(_uint));
		file.write(reinterpret_cast<const _char*>(&pMesh->mNumBones), sizeof(_uint));

		// 2. Shape Key (Morph Target) 정보 저장 시작 =. Shape Key (Morph Target) 개수 저장.
		file.write(reinterpret_cast<const _char*>(&pMesh->mNumAnimMeshes), sizeof(_uint)); 

		// 최적화 1 루프 밖에서 임시 버퍼 미리 할당 (최대 버텍스 개수만큼)
		_float3* pDeltaPosBuffer = new _float3[pMesh->mNumVertices];
		_float3* pDeltaNormalBuffer = new _float3[pMesh->mNumVertices];

		for (size_t k = 0; k < pMesh->mNumAnimMeshes; ++k)
		{
			aiAnimMesh* pAnimMesh = pMesh->mAnimMeshes[k];
			
			// 1. 이름
			aiString strShapeName = pAnimMesh->mName;
			_uint iNameLen = strShapeName.length;
			file.write(reinterpret_cast<const _char*>(&iNameLen), sizeof(_uint));
			file.write(reinterpret_cast<const _char*>(strShapeName.data), iNameLen);

			// 2. 정점 개수
			_uint iNumVertices = pAnimMesh->mNumVertices;
			file.write(reinterpret_cast<const _char*>(&iNumVertices), sizeof(_uint));

			// 3. Delta Position (변화량 위치만 저장)
			// 델타 값을 담을 임시 배열 할당.
			for (_uint v = 0; v < iNumVertices; ++v)
			{
				_float fDeltaX = pAnimMesh->mVertices[v].x - pMesh->mVertices[v].x; // x 변화량만 저장
				_float fDeltaY = pAnimMesh->mVertices[v].y - pMesh->mVertices[v].y; // y 변화량만 저장
				_float fDeltaZ = pAnimMesh->mVertices[v].z - pMesh->mVertices[v].z; // z 변화량만 저장

				pDeltaPosBuffer[v].x = fDeltaX;
				pDeltaPosBuffer[v].y = fDeltaY;
				pDeltaPosBuffer[v].z = fDeltaZ;
			}

			// 계산된 Delta 값 저장.
			file.write(reinterpret_cast<const _char*>(pDeltaPosBuffer), sizeof(_float3) * iNumVertices);

			// 4. Delta Normal에 대한 존재 확인하기.
			_bool bHasNormal = (pAnimMesh->mNormals != nullptr);
			file.write(reinterpret_cast<const _char*>(&bHasNormal), sizeof(_bool));

			// 5. Normal이 존재한다면?
			if (bHasNormal)
			{
				const aiVector3D* pBaseNorm = pMesh->mNormals;
				const aiVector3D* pTargetNorm = pAnimMesh->mNormals;

				for (_uint v = 0; v < iNumVertices; ++v)
				{
					// Delta Normal 계산
					pDeltaNormalBuffer[v].x = pTargetNorm[v].x - pBaseNorm[v].x;
					pDeltaNormalBuffer[v].y = pTargetNorm[v].y - pBaseNorm[v].y;
					pDeltaNormalBuffer[v].z = pTargetNorm[v].z - pBaseNorm[v].z;
				}
				// 5. 계산된 Delta Normal 일괄 저장
				file.write(reinterpret_cast<const _char*>(pDeltaNormalBuffer), sizeof(_float3) * iNumVertices);
			}
				
		}
		// 루프가 다 끝난 뒤 메모리 해제
		Safe_Delete_Array(pDeltaPosBuffer);
		Safe_Delete_Array(pDeltaNormalBuffer);

		// --- Shape Key 저장 끝 ---


		// 정점(Vertex) 데이터 생성 및 저장
		VTXANIMMESH* Vertices = new VTXANIMMESH[pMesh->mNumVertices];
		ZeroMemory(Vertices, sizeof(VTXANIMMESH) * pMesh->mNumVertices);

		for (size_t j = 0; j < pMesh->mNumVertices; ++j)
		{
			memcpy(&Vertices[j].vPosition, &pMesh->mVertices[j], sizeof(_float3));
			memcpy(&Vertices[j].vNormal, &pMesh->mNormals[j], sizeof(_float3));
			memcpy(&Vertices[j].vTangent, &pMesh->mTangents[j], sizeof(_float3));
			memcpy(&Vertices[j].vBinormal, &pMesh->mBitangents[j], sizeof(_float3));
			memcpy(&Vertices[j].vTexcoord, &pMesh->mTextureCoords[0][j], sizeof(_float2));
		}

		// Bone Weight 계산 ( 기존과 동일 )
		for (size_t j = 0; j < pMesh->mNumBones; ++j)
		{
			aiBone* pBone = pMesh->mBones[j];
			aiString strBoneName = pBone->mName;
			_uint iLength = strBoneName.length;
			file.write(reinterpret_cast<const _char*>(&iLength), sizeof(_uint));
			file.write(reinterpret_cast<const _char*>(strBoneName.data), iLength);
			file.write(reinterpret_cast<const _char*>(&pBone->mOffsetMatrix), sizeof(_float4x4));

			// Bone Weight
			for (size_t k = 0; k < pBone->mNumWeights; ++k)
			{
				aiVertexWeight VertexWeight = pBone->mWeights[k];
				_uint iVertexID = VertexWeight.mVertexId;
				if (0 == Vertices[iVertexID].vBlendWeight.x)
				{
					Vertices[iVertexID].vBlendIndex.x = j;
					Vertices[iVertexID].vBlendWeight.x = VertexWeight.mWeight;
				}
				else if (0 == Vertices[iVertexID].vBlendWeight.y)
				{
					Vertices[iVertexID].vBlendIndex.y = j;
					Vertices[iVertexID].vBlendWeight.y = VertexWeight.mWeight;
				}
				else if (0 == Vertices[iVertexID].vBlendWeight.z)
				{
					Vertices[iVertexID].vBlendIndex.z = j;
					Vertices[iVertexID].vBlendWeight.z = VertexWeight.mWeight;
				}
				else if (0 == Vertices[iVertexID].vBlendWeight.w)
				{
					Vertices[iVertexID].vBlendIndex.w = j;
					Vertices[iVertexID].vBlendWeight.w = VertexWeight.mWeight;
				}
			}
		}

		// 정점 및 인덱스 버퍼 쓰기.
		file.write(reinterpret_cast<const _char*>(Vertices), sizeof(VTXANIMMESH) * pMesh->mNumVertices);
		Safe_Delete_Array(Vertices);

		_uint* Indices = new _uint[pMesh->mNumFaces * 3];
		_uint iIndex = {};
		for (size_t j = 0; j < pMesh->mNumFaces; ++j)
		{
			Indices[iIndex++] = pMesh->mFaces[j].mIndices[0];
			Indices[iIndex++] = pMesh->mFaces[j].mIndices[1];
			Indices[iIndex++] = pMesh->mFaces[j].mIndices[2];
		}
		file.write(reinterpret_cast<const _char*>(Indices), sizeof(_uint) * pMesh->mNumFaces * 3);
		Safe_Delete_Array(Indices);
	}

#pragma endregion

	return S_OK;


}

HRESULT CGltfLoader::Save_Animation(const _char* pFileName)
{
	if (nullptr == m_pAIScene)
		return E_FAIL;

	_char szDirPath[MAX_PATH] = {};
	_char szFileName[MAX_PATH] = {};
	_splitpath_s(pFileName, nullptr, 0, szDirPath, MAX_PATH, szFileName, MAX_PATH, nullptr, 0);

	_char szAnimFilePath[MAX_PATH] = {};
	strcpy_s(szAnimFilePath, szDirPath);
	strcat_s(szAnimFilePath, "Animation/");
	strcat_s(szAnimFilePath, szFileName);
	strcat_s(szAnimFilePath, "_Anim.dat");

	filesystem::path dir = filesystem::path(szAnimFilePath).parent_path();
	if (!dir.empty() && !filesystem::exists(dir))
		filesystem::create_directories(dir);

	ofstream file(szAnimFilePath, ios::binary);

	if (false == file.is_open())
	{
		MSG_BOX("Animation Save Fail");
		return E_FAIL;
	}

	_uint iNumAnimations = m_pAIScene->mNumAnimations;
	// Num Animation
	file.write(reinterpret_cast<const _char*>(&iNumAnimations), sizeof(_uint));

	for (size_t i = 0; i < iNumAnimations; ++i)
	{
		aiAnimation* pAnimation = m_pAIScene->mAnimations[i];
		aiString strName = pAnimation->mName;
		_uint iLength = strName.length;
		// Animation Name
		file.write(reinterpret_cast<const _char*>(&iLength), sizeof(_uint));
		file.write(strName.data, iLength);

		_float fDuration = pAnimation->mDuration;
		// Animation Duration
		file.write(reinterpret_cast<const _char*>(&fDuration), sizeof(_float));

		_float fTickPerSecond = pAnimation->mTicksPerSecond;
		// Animation TickPerSecond
		file.write(reinterpret_cast<const _char*>(&fTickPerSecond), sizeof(_float));

		_uint iNumChannels = pAnimation->mNumChannels;
		// Num Channel
		file.write(reinterpret_cast<const _char*>(&iNumChannels), sizeof(_uint));

		for (size_t j = 0; j < iNumChannels; ++j)
		{
			aiNodeAnim* pChannel = pAnimation->mChannels[j];
			aiString strChannelName = pChannel->mNodeName;
			_uint iChannelNameLength = strChannelName.length;
			// Channel(Bone) Name
			file.write(reinterpret_cast<const _char*>(&iChannelNameLength), sizeof(_uint));
			file.write(strChannelName.data, iChannelNameLength);

			_uint iNumKeyFrame = max(pChannel->mNumPositionKeys, max(pChannel->mNumRotationKeys, pChannel->mNumScalingKeys));
			// Num KeyFrame
			file.write(reinterpret_cast<const _char*>(&iNumKeyFrame), sizeof(_uint));

			_float3 vScale = {};
			_float4 vRotation = {};
			_float3 vTranslation = {};

			for (size_t k = 0; k < iNumKeyFrame; ++k)
			{
				KEYFRAME KeyFrame = {};
				if (k < pChannel->mNumScalingKeys)
				{
					KeyFrame.fTrackPosition = pChannel->mScalingKeys[k].mTime;
					memcpy(&vScale, &pChannel->mScalingKeys[k].mValue, sizeof(_float3));
				}
				if (k < pChannel->mNumRotationKeys)
				{
					KeyFrame.fTrackPosition = pChannel->mRotationKeys[k].mTime;
					{
						vRotation.x = pChannel->mRotationKeys[k].mValue.x;
						vRotation.y = pChannel->mRotationKeys[k].mValue.y;
						vRotation.z = pChannel->mRotationKeys[k].mValue.z;
						vRotation.w = pChannel->mRotationKeys[k].mValue.w;
					}
				}
				if (k < pChannel->mNumPositionKeys)
				{
					KeyFrame.fTrackPosition = pChannel->mPositionKeys[k].mTime;
					memcpy(&vTranslation, &pChannel->mPositionKeys[k].mValue, sizeof(_float3));
				}
				KeyFrame.vScale = vScale;
				KeyFrame.vRotation = vRotation;
				KeyFrame.vTranslation = vTranslation;
				file.write(reinterpret_cast<const _char*>(&KeyFrame), sizeof(KEYFRAME));
			}
		}
	}

	file.close();

	return S_OK;
}

HRESULT CGltfLoader::Save_Animation_Character(const _char* pFileName)
{
	if (nullptr == m_pAIScene)
		return E_FAIL;

	// 1. 경로 및 파일 생성 설정
	_char szDirPath[MAX_PATH] = {};
	_char szFileName[MAX_PATH] = {};
	_splitpath_s(pFileName, nullptr, 0, szDirPath, MAX_PATH, szFileName, MAX_PATH, nullptr, 0);

	_char szAnimFilePath[MAX_PATH] = {};
	strcpy_s(szAnimFilePath, szDirPath);
	strcat_s(szAnimFilePath, "Animation/");
	strcat_s(szAnimFilePath, szFileName);
	strcat_s(szAnimFilePath, "_MorphAnim.dat");

	filesystem::path dir = filesystem::path(szAnimFilePath).parent_path();
	if (!dir.empty() && !filesystem::exists(dir))
		filesystem::create_directories(dir);

	ofstream file(szAnimFilePath, ios::binary);
	if (false == file.is_open())
	{
		MSG_BOX("Animation Save Fail");
		return E_FAIL;
	}

	// 2. 애니메이션 분류 (Main / Morph)
	map<_string, ANIM_SET> mapAnimGroups;
	for (size_t i = 0; i < m_pAIScene->mNumAnimations; ++i)
	{
		aiAnimation* pAnim = m_pAIScene->mAnimations[i];
		string strFullName = pAnim->mName.C_Str();
		string strBaseName = strFullName;

		_bool isMorphOnly = false;
		size_t findRes = strFullName.find("_Curves");
		if (findRes != string::npos)
		{
			strBaseName = strFullName.substr(0, findRes);
			isMorphOnly = true;
		}

		if (isMorphOnly)
			mapAnimGroups[strBaseName].pMorphAnim = pAnim;
		else
		{
			mapAnimGroups[strBaseName].pMainAnim = pAnim;
			if (pAnim->mNumMorphMeshChannels > 0 && mapAnimGroups[strBaseName].pMorphAnim == nullptr)
				mapAnimGroups[strBaseName].pMorphAnim = pAnim;
		}
	}

	_uint iNumMergedAnims = static_cast<_uint>(mapAnimGroups.size());
	file.write(reinterpret_cast<const _char*>(&iNumMergedAnims), sizeof(_uint));


	// 3. 데이터 저장 루프
	for (auto& Pair : mapAnimGroups)
	{
		// TimeScale 계산을 공통 영역으로 이동
		// Main이 없으면 Morph에서라도 기본 정보를 가져와야 함.
		aiAnimation* pBaseAnim = Pair.second.pMainAnim ? Pair.second.pMainAnim : Pair.second.pMorphAnim;
		if (!pBaseAnim) continue; // 둘 다 없으면 스킵

		_float fTargetFPS = 24.0f;
		_float fSourceFPS = static_cast<_float>(pBaseAnim->mTicksPerSecond);
		if (fSourceFPS <= 0.1f) fSourceFPS = 1000.0f; // glTF는 보통 1000(ms) 또는 0(Assimp default)

		_float fTimeScale = fTargetFPS / fSourceFPS;
		_float fDurationTick = static_cast<_float>(pBaseAnim->mDuration);
		_float fDurationSecond = fDurationTick / fSourceFPS;

		// 실제 재생 시간 * TargetFPS = 총 프레임 수 (올림 처리 혹은 +1)
		_uint iTotalFrameCount = static_cast<_uint>(fDurationSecond * fTargetFPS) + 1;
		_float fDurationConverted = static_cast<_float>(iTotalFrameCount); // Duration을 프레임 단위로 저장한다고 가정

#pragma region 2. Main Animation 저장 (Resampling 적용)
		if (nullptr != Pair.second.pMainAnim)
		{
			aiAnimation* pAnimation = Pair.second.pMainAnim;
			aiString strName = pAnimation->mName;
			_uint iLength = strName.length;

			file.write(reinterpret_cast<const _char*>(&iLength), sizeof(_uint));
			file.write(strName.data, iLength);

			// Duration과 FPS 저장
			file.write(reinterpret_cast<const _char*>(&fDurationConverted), sizeof(_float));
			file.write(reinterpret_cast<const _char*>(&fTargetFPS), sizeof(_float));

			_uint iNumChannels = pAnimation->mNumChannels;
			file.write(reinterpret_cast<const _char*>(&iNumChannels), sizeof(_uint));

			for (uint j = 0; j < iNumChannels; ++j)
			{
				aiNodeAnim* pChannel = pAnimation->mChannels[j];
				aiString strChannelName = pChannel->mNodeName;
				_uint iChannelNameLength = strChannelName.length;

				file.write(reinterpret_cast<const _char*>(&iChannelNameLength), sizeof(_uint));
				file.write(strChannelName.data, iChannelNameLength);

				//  키프레임 개수를 '전체 프레임 수'로 고정합니다.
				// 이제 원본 키 개수와 상관없이 24FPS로 꽉 채운 데이터를 씁니다.
				file.write(reinterpret_cast<const _char*>(&iTotalFrameCount), sizeof(_uint));

				// 인덱스(k)가 아니라 0프레임부터 끝 프레임까지 시간을 순회합니다.
				for (_uint iFrame = 0; iFrame < iTotalFrameCount; ++iFrame)
				{
					KEYFRAME KeyFrame = {};

					// 1. 현재 저장하려는 프레임 번호
					KeyFrame.fTrackPosition = static_cast<_float>(iFrame);

					// 2. 현재 프레임이 원본 애니메이션의 어느 시간(Tick)에 해당하는지 역계산
					// 공식: (현재프레임 / 목표FPS) * 원본TPS
					_float fCurrentTick = (static_cast<_float>(iFrame) / static_cast<_float>(fTargetFPS)) * fSourceFPS;

					// 3. 해당 시간(dCurrentTick)의 값을 보간해서 가져옴 (인덱스 참조 X)
					KeyFrame.vScale = GetScaleAtTime(pChannel, fCurrentTick);
					KeyFrame.vRotation = GetRotationAtTime(pChannel, fCurrentTick);
					KeyFrame.vTranslation = GetPositionAtTime(pChannel, fCurrentTick);

					file.write(reinterpret_cast<const _char*>(&KeyFrame), sizeof(KEYFRAME));
				}
			}
		}
		else
			continue;
#pragma endregion

#pragma region 3. Morph Animation 저장
		map<string, vector<KEYFRAME_CURVE>> mapMorphCurves;

		if (nullptr != Pair.second.pMorphAnim)
		{
			aiAnimation* pMorphAnimation = Pair.second.pMorphAnim;
			if (pMorphAnimation->mNumMorphMeshChannels > 0)
			{
				for (size_t i = 0; i < pMorphAnimation->mNumMorphMeshChannels; ++i)
				{
					aiMeshMorphAnim* pMeshMorphAnim = pMorphAnimation->mMorphMeshChannels[i];

					// FindMeshByMorphChannelName 함수가 있다고 가정
					aiMesh* pTargetMesh = FindMeshByMorphChannelName(pMeshMorphAnim->mName);
					// if (nullptr == pTargetMesh && m_pAIScene->mNumMeshes > 0) pTargetMesh = m_pAIScene->mMeshes[0]; // Fallback
					if (nullptr == pTargetMesh) continue;

					for (_uint keyIdx = 0; keyIdx < pMeshMorphAnim->mNumKeys; ++keyIdx)
					{
						const aiMeshMorphKey& MorphKey = pMeshMorphAnim->mKeys[keyIdx];

						for (unsigned int v = 0; v < MorphKey.mNumValuesAndWeights; ++v)
						{
							_uint iShapeIdx = MorphKey.mValues[v];
							if (iShapeIdx >= pTargetMesh->mNumAnimMeshes) continue;

							aiAnimMesh* pAnimMesh = pTargetMesh->mAnimMeshes[iShapeIdx];
							string strShapeKeyName = pAnimMesh->mName.C_Str();

							if (strShapeKeyName == "Basis" || strShapeKeyName.empty()) continue;

							_float fWeight = static_cast<_float>(MorphKey.mWeights[v]);

							KEYFRAME_CURVE KeyFrame = {};
							// Morph는 기존 로직대로 키값만 저장 (TimeScale만 적용)
							// fTimeScale은 이제 위에서 안전하게 계산됨
							KeyFrame.fTrackPosition = static_cast<_float>(MorphKey.mTime) * fTimeScale;
							KeyFrame.fValue = fWeight;

							mapMorphCurves[strShapeKeyName].push_back(KeyFrame);
						}
					}
				}
			}
		}

		_uint iTotalCurves = static_cast<_uint>(mapMorphCurves.size());
		file.write(reinterpret_cast<const _char*>(&iTotalCurves), sizeof(_uint));

		for (auto& Pair : mapMorphCurves)
		{
			string strCurveName = Pair.first;
			auto& vecKeys = Pair.second;

			_uint iNameLen = static_cast<_uint>(strCurveName.length());
			file.write(reinterpret_cast<const _char*>(&iNameLen), sizeof(_uint));
			file.write(strCurveName.data(), iNameLen);

			_uint iNumKeys = static_cast<_uint>(vecKeys.size());
			file.write(reinterpret_cast<const _char*>(&iNumKeys), sizeof(_uint));
			file.write(reinterpret_cast<const _char*>(vecKeys.data()), sizeof(KEYFRAME_CURVE) * iNumKeys);
		}
#pragma endregion
	}

	file.close();
	return S_OK;
}

HRESULT CGltfLoader::Save_Material(const _char* pFileName)
{
	if (nullptr == m_pAIScene)
		return E_FAIL;

	_char szDirPath[MAX_PATH] = {};
	_char szFileName[MAX_PATH] = {};
	_splitpath_s(pFileName, nullptr, 0, szDirPath, MAX_PATH, szFileName, MAX_PATH, nullptr, 0);

	_char szMatFilePath[MAX_PATH] = {};
	strcpy_s(szMatFilePath, szDirPath);
	strcat_s(szMatFilePath, "Mat/");
	strcat_s(szMatFilePath, szFileName);
	strcat_s(szMatFilePath, ".json");

	filesystem::path dir = filesystem::path(szMatFilePath).parent_path();
	if (!dir.empty() && !filesystem::exists(dir))
		filesystem::create_directories(dir);

	ofstream file(szMatFilePath);
	if (false == file.is_open())
	{
		MSG_BOX("Material Save Fail");
		return E_FAIL;
	}

	json OutData;
	OutData["NumMaterial"] = m_pAIScene->mNumMaterials;
	OutData["Materials"] = json::array();
	for (size_t i = 0; i < m_pAIScene->mNumMaterials; ++i)
	{
		aiMaterial* pMaterial = m_pAIScene->mMaterials[i];
		json MaterialData;
		Save_Texture(MaterialData, pMaterial, aiTextureType::aiTextureType_DIFFUSE);
		Save_Texture(MaterialData, pMaterial, aiTextureType::aiTextureType_NORMALS);
		OutData["Materials"].push_back(MaterialData);
	}

	file << OutData.dump(4);

	file.close();

	return S_OK;
}

void CGltfLoader::Load_File()
{
	IGFD::FileDialogConfig config;

	config.path = "../../Editor/Bin/Resource/Player/";
	config.flags = ImGuiFileDialogFlags_ReadOnlyFileNameField;

	ImGuiFileDialog::Instance()->OpenDialog("GLTF File Load", "Import File", ".gltf", config);

	ImVec2 vMinSize = ImVec2(600, 400);
	ImVec2 vMaxSize = ImVec2(800, 400);

	if (ImGuiFileDialog::Instance()->Display(
		"GLTF File Load", ImGuiWindowFlags_NoCollapse
		, vMinSize
		, vMaxSize)) {
		if (ImGuiFileDialog::Instance()->IsOk()) {
			_string strFilePath = ImGuiFileDialog::Instance()->GetFilePathName();
			m_strModelName = ImGuiFileDialog::Instance()->GetCurrentFileName();

			// 임포트한 파일의 폴더 경로만 추출해서 저장.
			filesystem::path pathObj(strFilePath);
			m_strImportDir = pathObj.parent_path().string();

			_uint iFlag = { aiProcess_ConvertToLeftHanded | aiProcessPreset_TargetRealtime_Fast };
			m_pAIScene = m_Importer.ReadFile(strFilePath.c_str(), iFlag);
			if (nullptr == m_pAIScene)
			{
				MSG_BOX("AIScene Not Found");
				return;
			}
		}
		ImGuiFileDialog::Instance()->Close();
		m_isShowLoadFile = false;
	}
}

void CGltfLoader::Save_File()
{
	if (nullptr == m_pAIScene)
		return;

	IGFD::FileDialogConfig config;

	config.path = "../../Client/Bin/Resource/Model/Player/";
	config.flags = ImGuiFileDialogFlags_ConfirmOverwrite;

	ImGuiFileDialog::Instance()->OpenDialog("Save Model", "Export File", ".dat", config);

	ImVec2 vMinSize = ImVec2(600, 400); 
	ImVec2 vMaxSize = ImVec2(800, 400); 

	if (ImGuiFileDialog::Instance()->Display("Save Model"
		, ImGuiWindowFlags_NoCollapse
		, vMinSize
		, vMaxSize
	)) {
		if (ImGuiFileDialog::Instance()->IsOk()) {
			_string strFilePath = ImGuiFileDialog::Instance()->GetFilePathName();

			if (MODELTYPE::ANIM == m_eType)
			{
				Save_Dat_Anim(strFilePath.c_str());
				Save_Animation(strFilePath.c_str());
			}
			else if (MODELTYPE::CHARACTER == m_eType)
			{
				Save_Dat_Character(strFilePath.c_str()); // 캐릭터 전용 포맷(쉐이프키 포함) 저장
				Save_Animation_Character(strFilePath.c_str()); // 애니메이션 데이터도 필요하다면 저장 
			}
			Save_Material(strFilePath.c_str());
		}
		ImGuiFileDialog::Instance()->Close();
		m_isShowSaveFile = false;
	}
}

void CGltfLoader::Show_Info()
{
	if (nullptr == m_pAIScene)
		return;

	_uint iNumMesh = m_pAIScene->mNumMeshes;
	_uint iNumMat = m_pAIScene->mNumMaterials;
	_uint iNumAnim = m_pAIScene->mNumAnimations;

	_char szInfo[MAX_PATH] = {};
	sprintf_s(szInfo, "Mesh : %d / Mat : %d / Anim : %d", iNumMesh, iNumMat, iNumAnim);

	ImGui::PushID(1000);
	ImGui::Text(m_strModelName.c_str());
	ImGui::PopID();
	ImGui::PushID(1001);
	ImGui::Text(szInfo);
	ImGui::PopID();
}

aiMesh* CGltfLoader::FindMeshByMorphChannelName(const aiString& strMorphChannelName)
{
	string strNodeName = strMorphChannelName.C_Str();

	// 1. *0 제거
	size_t starPos = strNodeName.find('*');
	if (starPos != std::string::npos)
		strNodeName = strNodeName.substr(0, starPos);

	// 2. 타겟 노드 찾기.
	aiNode* pTargetNode = Find_Node(m_pAIScene->mRootNode, strNodeName);


	// 3. 노드를 못 찾음 (이름 불일치 등)
	if (nullptr == pTargetNode)
		return nullptr;

	// 4. 찾은 Node에 연결된 메쉬 인덱스들을 순회하며 Shape Key가 있는 메쉬를 찾음
	//    (Blender의 Mesh 오브젝트 하나가 Material 개수만큼 쪼개져 있으므로 순회 필요)
	for (unsigned int i = 0; i < m_pAIScene->mNumMeshes; ++i)
	{
		_uint iMeshIndex = pTargetNode->mMeshes[i];
		aiMesh* pMesh = m_pAIScene->mMeshes[iMeshIndex];


		// Shape Key(AnimMeshes) 데이터가 존재하는지 확인
		if (pMesh->mNumAnimMeshes > 0)
			return pMesh; // 진짜 데이터를 가진 메쉬 반환
	}

	return nullptr;
}

aiNode* CGltfLoader::Find_Node(aiNode* pNode, const _string& strNodeName)
{
	if (!pNode) return nullptr;

	// 현재 노드 이름과 찾는 이름이 같으면 반환
	if (strNodeName == pNode->mName.C_Str())
		return pNode;

	// 자식 노드들 순회
	for (_uint i = 0; i < pNode->mNumChildren; ++i)
	{
		aiNode* pResult = Find_Node(pNode->mChildren[i], strNodeName);
		if (pResult) return pResult;
	}

	return nullptr;
}



_float4 CGltfLoader::GetRotationAtTime(aiNodeAnim* pNodeAnim, _float fTime)
{
	if (pNodeAnim->mNumRotationKeys == 0) return _float4(0.f, 0.f, 0.f, 1.f);
	if (pNodeAnim->mNumRotationKeys == 1) {
		auto val = pNodeAnim->mRotationKeys[0].mValue;
		return _float4(val.x, val.y, val.z, val.w);
	}

	_uint iIndex = 0;
	for (_uint i = 0; i < pNodeAnim->mNumRotationKeys - 1; ++i) {
		if (fTime < pNodeAnim->mRotationKeys[i + 1].mTime) {
			iIndex = i;
			break;
		}
	}

	if (fTime >= pNodeAnim->mRotationKeys[pNodeAnim->mNumRotationKeys - 1].mTime) {
		auto val = pNodeAnim->mRotationKeys[pNodeAnim->mNumRotationKeys - 1].mValue;
		return _float4(val.x, val.y, val.z, val.w);
	}

	aiQuatKey& KeyA = pNodeAnim->mRotationKeys[iIndex];
	aiQuatKey& KeyB = pNodeAnim->mRotationKeys[iIndex + 1];

	_float fDelta = KeyB.mTime - KeyA.mTime;
	_float fFactor = (fDelta > 0.0) ? static_cast<float>((fTime - KeyA.mTime) / fDelta) : 0.0f;
	fFactor = max(0.0f, min(fFactor, 1.0f));

	aiQuaternion out;
	aiQuaternion::Interpolate(out, KeyA.mValue, KeyB.mValue, fFactor);
	return _float4(out.x, out.y, out.z, out.w);
}

_float3 CGltfLoader::GetPositionAtTime(aiNodeAnim* pNodeAnim, _float fTime)
{
	if (pNodeAnim->mNumPositionKeys == 0) return _float3(0.f, 0.f, 0.f);
	if (pNodeAnim->mNumPositionKeys == 1) {
		auto val = pNodeAnim->mPositionKeys[0].mValue;
		return _float3(val.x, val.y, val.z);
	}

	// 현재 시간(dTime)이 위치한 인덱스 찾기
	_uint iIndex = 0;
	for (_uint i = 0; i < pNodeAnim->mNumPositionKeys - 1; ++i) {
		if (fTime < pNodeAnim->mPositionKeys[i + 1].mTime) {
			iIndex = i;
			break;
		}
	}

	// 마지막 키 이후라면 마지막 값 반환
	if (fTime >= pNodeAnim->mPositionKeys[pNodeAnim->mNumPositionKeys - 1].mTime) {
		auto val = pNodeAnim->mPositionKeys[pNodeAnim->mNumPositionKeys - 1].mValue;
		return _float3(val.x, val.y, val.z);
	}

	// 보간 계산
	aiVectorKey& KeyA = pNodeAnim->mPositionKeys[iIndex];
	aiVectorKey& KeyB = pNodeAnim->mPositionKeys[iIndex + 1];

	_float fDelta = KeyB.mTime - KeyA.mTime;
	_float fFactor = (fDelta > 0.0) ? static_cast<float>((fTime - KeyA.mTime) / fDelta) : 0.0f;
	fFactor = max(0.0f, min(fFactor, 1.0f));

	aiVector3D out = KeyA.mValue + (KeyB.mValue - KeyA.mValue) * fFactor;
	return _float3(out.x, out.y, out.z);
}

_float3 CGltfLoader::GetScaleAtTime(aiNodeAnim* pNodeAnim, _float fTime)
{
	if (pNodeAnim->mNumScalingKeys == 0) return _float3(1.f, 1.f, 1.f);
	if (pNodeAnim->mNumScalingKeys == 1) {
		auto val = pNodeAnim->mScalingKeys[0].mValue;
		return _float3(val.x, val.y, val.z);
	}

	_uint iIndex = 0;
	for (_uint i = 0; i < pNodeAnim->mNumScalingKeys - 1; ++i) {
		if (fTime < pNodeAnim->mScalingKeys[i + 1].mTime) {
			iIndex = i;
			break;
		}
	}

	if (fTime >= pNodeAnim->mScalingKeys[pNodeAnim->mNumScalingKeys - 1].mTime) {
		auto val = pNodeAnim->mScalingKeys[pNodeAnim->mNumScalingKeys - 1].mValue;
		return _float3(val.x, val.y, val.z);
	}

	aiVectorKey& KeyA = pNodeAnim->mScalingKeys[iIndex];
	aiVectorKey& KeyB = pNodeAnim->mScalingKeys[iIndex + 1];

	_float fDelta = KeyB.mTime - KeyA.mTime;
	_float fFactor = (fDelta > 0.0) ? static_cast<float>((fTime - KeyA.mTime) / fDelta) : 0.0f;
	fFactor = max(0.0f, min(fFactor, 1.0f));

	aiVector3D out = KeyA.mValue + (KeyB.mValue - KeyA.mValue) * fFactor;
	return _float3(out.x, out.y, out.z);
}


HRESULT CGltfLoader::Save_Texture(json& MaterialData, const aiMaterial* pMaterial, aiTextureType eType)
{
	json TypeData;
	_uint iTextureCnt = pMaterial->GetTextureCount(eType);
	TypeData["TextureCnt"] = iTextureCnt;
	TypeData["FileName"] = json::array();
	for (size_t k = 0; k < iTextureCnt; ++k)
	{
		aiString strFilePath = {};
		pMaterial->GetTexture(eType, k, &strFilePath);

		// 1. 파일명 추출.
		_char szName[MAX_PATH] = {};
		_char szExt[MAX_PATH] = {};
		_splitpath_s(strFilePath.data, nullptr, 0, nullptr, 0, szName, MAX_PATH, szExt, MAX_PATH);
		strcat_s(szName, szExt);
		TypeData["FileName"].push_back(szName);

	}
	if(aiTextureType::aiTextureType_DIFFUSE == eType)
		MaterialData["Diffuse"] = TypeData;
	if (aiTextureType::aiTextureType_NORMALS == eType)
		MaterialData["Normal"] = TypeData;

	return S_OK;
}

HRESULT CGltfLoader::Save_Bone(ofstream& OutPut, const aiNode* pNode)
{
	OutPut.write(reinterpret_cast<const _char*>(&pNode->mNumChildren), sizeof(_uint));
	aiString strName = pNode->mName;
	_uint iLength = strName.length;
	OutPut.write(reinterpret_cast<const _char*>(&iLength), sizeof(_uint));
	OutPut.write(strName.data, iLength);
	OutPut.write(reinterpret_cast<const _char*>(&pNode->mTransformation), sizeof(_float4x4));
	//OutPut.write(reinterpret_cast<const _char*>(&pNode->mOffsetMatrix), sizeof(_float4x4));
	for (size_t i = 0; i < pNode->mNumChildren; ++i)
	{
		if (FAILED(Save_Bone(OutPut, pNode->mChildren[i])))
			return E_FAIL;
	}

	return S_OK;
}

CGltfLoader* CGltfLoader::Create()
{
	CGltfLoader* pInstance = new CGltfLoader();

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX("Failed to Create : ModelLoader");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CGltfLoader::Free()
{
	__super::Free();
	m_Importer.FreeScene();
}
