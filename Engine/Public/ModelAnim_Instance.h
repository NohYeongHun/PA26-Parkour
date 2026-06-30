#pragma once
#include "Component.h"

NS_BEGIN(Engine)

class ENGINE_DLL CModelAnim_Instance final : public CComponent
{
public:
	enum BUFFER
	{
		BUFFER_KEY_FRAME = 0,
		BUFFER_ANIM_INFO = 1,
		BUFFER_BONE_PARENT = 2,
		BUFFER_FINAL_BONEMATRIX = 3,
		BUFFER_ANIM_INFOCB = 4, // constant
		BUFFER_STAGING = 5,
		BUFFER_BONE_CHANNEL = 6,
		BUFFER_ANIM_LOCALMATRIX = 7,
		BUFFER_INSTANCECB = 8,
		BUFFER_PRETRANSFORM = 9,
		BUFFER_END
	};


	enum SRV
	{
		SRV_KEY_FRAME = 0,
		SRV_ANIM_INFO = 1,
		SRV_BONE_CHANNEL = 2,
		SRV_BONE_PARENT = 3,
		SRV_ANIM_INFOCB = 4,
		SRV_ANIM_LOCALMATRIX = 5,
		SRV_FINAL_BONEMATRIX = 6,
		SRV_END
	};

	enum UAV
	{
		UAV_FINAL_BONEMATRIX = 0,
		UAV_ANIM_LOCALMATRIX = 1,
		UAV_END
	};
	typedef struct tagInstanceCB
	{
		_uint			iNumBones;
		_uint			iNumInstance;
		_uint			iMaxDepth;
		_float			fTempFloat;
	}INSTANCECB;

private:
	explicit CModelAnim_Instance(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CModelAnim_Instance(const CModelAnim_Instance& Prototype);
	virtual ~CModelAnim_Instance() = default;

public:
	_uint								Get_NumMesh() { return m_iNumMeshes; }
	void								Sync_RootNode(class CTransform* pOwnerTransform, class CNavigation* pOwnerNavigation, _float fTimeDelta);
	void								Sync_RootNode(class CTransform* pOwnerTransform, _float fTimeDelta);
	const _float4x4*					Get_BoneMatrixPtr(const _char* pBoneName);
	const vector<_float3>&				Get_VerticesPos(_uint iIndex);
	const vector<_uint>&				Get_Indices(_uint iIndex);
	const vector<_uint>&				Get_MeshOffset() { return m_MeshTypeOffsets; };
	const _uint							Get_NumInstance() const { return m_iNumInstance; }

#ifdef _DEBUG
	const vector<_string>&				Get_AnimationNames() const { return m_AnimationNames; }
	_float								Get_Duration(const _string& strAnimName);

	_bool Find_Animation(const _string& strAnimName);
#endif

public:
	void								Register_Notify(const _string& strFilePath, const vector<function<void()>>& Functions);
	void								Register_AllNotifies(const _string& strNotifyFolderPath, function<void(const _wstring&, _bool)> ColliderCallback, function<void(const _wstring&)> EffectCallback, function<void(const _wstring&)> ObjectCallback);

public:
	virtual		HRESULT					Initialize_Prototype(MODELTYPE eType, _fmatrix PreTransformMatrix, _uint iNumInstance, const _char* pFilePath, vector<_string>* strMeshTypes);
	virtual		HRESULT					Initialize_Clone(void* pArg);
	HRESULT								Render(_uint iMeshIndex);
	HRESULT								Render(_uint iMeshIndex, ID3D11DeviceContext* pDC);

#ifdef _DEBUG
	_bool								Is_Picked(const _fvector& vRayPos, const _fvector& vRayDir, _float* pDistance);
#endif
public:
	HRESULT								Bind_Materials(class CShader* pShader, const _char* pConstantName, _uint iMeshIndex, TEXTURETYPE eTextureType, _uint iTextureIndex);
	HRESULT								Bind_Materials(class CShader* pShader, const _char* pConstantName, _uint iMeshIndex, TEXTURETYPE eTextureType);
	HRESULT								Bind_Materials(class CDeferredShader* pShader, const _char* pConstantName, _uint iMeshIndex, TEXTURETYPE eTextureType, _uint iTextureIndex, ID3DX11Effect* pEffect);
	HRESULT								Bind_Materials(class CDeferredShader* pShader, const _char* pConstantName, _uint iMeshIndex, TEXTURETYPE eTextureType, ID3DX11Effect* pEffect);
	HRESULT								Bind_BoneMatrices(class CShader* pShader, const _char* pConstantName);
	HRESULT								Bind_OffsetMatrices(class CShader* pShader, const _char* pConstantName, _uint iMeshIndex);
	HRESULT								Bind_ConstantBuffers(class CShader* pShader);
	HRESULT								Clear_Materials(class CDeferredShader* pShader, const _char* pConstanceName, _uint iMeshIndex, TEXTURETYPE eTextureType, ID3DX11Effect* pEffect);
	//_bool								Play_Animation_CPU(const _string& strAnimationName, _float fTimeDelta, _float* pTrackPosition, _bool isBlend = true, _bool isRootMotion = true, _bool IsRootMotionRotate = true, _bool IsRootMotionTranslate = true, _float fRootMotionRate = 0.1f);
	
	//루트모션 & 인스턴스 객체 track 갱신
	_bool								Update_RootMotion(const _string& strAnimationName, class CTransform* pTransform, _float fTimeDelta, _float* pTrackPosition, _bool isRootMotion = true, _bool IsRootMotionRotate = true, _bool IsRootMotionTranslate = true, _float fRootMotionRate = 0.1f);
	//충돌 상호작용 이후 최종 매트릭스 업데이트
	void								Update_AnimationState(const _string& strAnimationName, _fmatrix WorldMatrix, _uint iInstanceIndex, _float* pTrackPosition, _uint* pPaddingIndices = nullptr, _uint iExtra = 0);
	// Compute Shader
	void								Play_NonRibAnimation_GPU(class CComputeShader* pComputeShaderCom);
	void								FetchModelMatrices_FromCompute(class CComputeShader* pComputeShaderCom);
	void								Update_WorldInstances();
	void								Clear_Animation(const _string& strAnimationName, _float fTrackPosition = 0.f);
	

	_uint								Get_BoneSize() { return  static_cast<_uint>(m_Bones.size()); }
	const _float4x4*					Get_BoneMatrixPtr(_uint iBoneIndex);
	void								Update_BoneMatrix_Map();
private:
	MODELTYPE								m_eType = { MODELTYPE::ANIM };

	_uint									m_iNumMeshes = {};
	vector<class CMeshAnim_Instance*>		m_Meshes;

	_uint									m_iNumMaterials = {};
	vector<class CMeshMaterial*>			m_Materials;

	_float4x4								m_PreTransformMatrix = {};

	_float4									m_vPreRootRotation = {};
	_float4									m_vPreRootPosition = {};
	_matrix									m_RootMatrix = {};
	_uint									m_iRootBoneIndex = {};
	_uint									m_iNumBones = {};
	vector<class CBone*>					m_Bones;

	_uint									m_iNumAnimations = {};
	_string									m_strPreAnimation;
	map<_string, class CAnimation_Inst*>		m_Animations;
	map<_string, _uint>						m_AnimationNameToIndex; // Compute Shader

	_bool									m_isBlend = { false };
	_bool									m_isChangeAnimation = { false };

	_float								m_fPreScale = { 0.01f }; // RootMotionRate에 곱해줄 값.

	_uint								m_iNumInstance{};

#ifdef _DEBUG
	vector<_string>					m_AnimationNames;
	_uint m_iSelectIndex = { 0 };
#endif
	


#pragma region Compute Shader
private:
	void ApplyComputeResults_ToBones();
	void FetchLocalMatrices_FromComputeNonRib(class CComputeShader* pComputeShaderCom);

private:
	vector<ID3D11Buffer*> m_Buffers = {};
	vector<ID3D11ShaderResourceView*> m_SRVs = {};
	vector<ID3D11UnorderedAccessView*> m_UAVs = {};
	vector<ANIMATION_CBINFO> m_AnimCBInfos = {};

	vector<VTXINSTANCE_ANIMMESH>			m_VtxInstanceDatas = {};
	//인스턴스에 사용되는 매쉬 부위 개수
	_uint									m_iNumMeshType = {};
	//메쉬 종류에 따른 패딩 위치
	vector<_uint>							m_MeshTypeOffsets;
	vector<vector<VTXINSTANCE_ANIMMESH>>		m_pVtxInstanceDatas;
	_bool m_isRibAnimation = { false };

#pragma endregion

private:
	void							Compute_RootAnimation(_float fRootMotionRate, _bool IsRootMotionRotation = true, _bool IsRootMotionTranslate = true);

private:
	HRESULT							Ready_Bone(ifstream& InputFile, _int iParentIndex, _bool isFirst, vector<CBone*>& Temp);
	HRESULT							Ready_Mesh(ifstream& InputFile);
	HRESULT							Ready_Parts(const _char* pFolderPath, vector<_string>* strMeshTypes, vector<CBone*>& Temp);
	HRESULT							Ready_Material(const _char* pFilePath);
	HRESULT							Ready_Animation(const _char* pFilePath);

	HRESULT							Ready_Shared_Buffers();
	HRESULT							Ready_Instance_Buffers();


public:
	//주의사항: 모델이 갖고있는 매쉬가 하나의 파일에서 여러 mesh가 있는 경우는 strMeshTypes = nullptr, 여러 모델 파일을 가져오는 경우 strMeshTypes 각 모델 폴더명 기재
	static		CModelAnim_Instance*	Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, MODELTYPE eType, _fmatrix PreTransformMatrix, _uint iNumInstance, const _char* pFilePath, vector<_string>* strMeshTypes = nullptr);
	virtual		CComponent*				Clone(void* pArg);
	virtual		void					Free() override;
};

NS_END