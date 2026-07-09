#pragma once
#include "Component.h"

NS_BEGIN(Engine)
typedef struct tagShapeKeyInfo
{
	_uint iMeshIndex; // 어떤 메쉬에 속해있는지?
	class CShapeKey* pShapeKey; // 실제 Shape Key 객체 포인터.
}SHAPEKEYINFO;

class ENGINE_DLL CModel final : public CComponent
{
public:
	enum BUFFER
	{
		BUFFER_KEY_FRAME = 0,
		BUFFER_ANIM_INFO = 1,
		BUFFER_FINAL_BONEMATRIX = 2,
		BUFFER_ANIM_INFOCB = 3, // constant
		BUFFER_ANIM_INFOFLYCB = 4, // constant
		BUFFER_STAGING_0 = 5,
		BUFFER_STAGING_1 = 6,
		BUFFER_BONE_CHANNEL = 7,
		BUFFER_MORPH_WEIGHT = 8,
		BUFFER_END
	};


	enum SRV
	{
		SRV_KEY_FRAME = 0,
		SRV_ANIM_INFO = 1,
		SRV_BONE_CHANNEL = 2,
		SRV_FINAL_BONEMATRIX = 3,
		SRV_MORPH_WEIGHT = 4,
		SRV_END
	};

	enum UAV
	{
		UAV_FINAL_BONEMATRIX = 0,
		UAV_END
	};

private:
	explicit CModel(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CModel(const CModel& Prototype);
	virtual ~CModel() = default;

public:
	const ROOT_MOTION_DELTA&			Get_RootMotionDelta();
	_uint								Get_NumBones(_uint iMeshIndex);
	void								Copy_BoneMatrices(_float4x4* pOutMatrices, _uint iMeshIndex);
	const vector<class CBone*>			Get_Bones() { return m_Bones; }
	_uint								Get_NumMesh() { return m_iNumMeshes; }
	void								Sync_RootNode(class CTransform* pOwnerTransform, _float fTimeDelta);
	const _float4x4* Get_BoneMatrixPtr(const _char* pBoneName);
	const vector<_uint>& Get_Indices(_uint iIndex);
	const vector<_float3>& Get_VerticesPos(_uint iIndex);
	// View 용도.
	const vector<_string>& Get_AllShapeKeyNames() const { return m_ShapeKeyNames; }
	// 제어용도
	void Set_ShapeKeyWeight(const _string& strKeyName, _float fWeight);
	_uint Get_NumShapeKeys() { return static_cast<_uint>(m_ShapeKeyNames.size()); }
	void Set_TrackPosition(const _string& strAnimName, const _float fTrackPosition);
	_float3 Get_RootMotionTotalDisplacement(const _string& strAnimationName);


#ifdef _DEBUG
	const vector<_string>& Get_AnimationNames() const { return m_AnimationNames; }
	_float* Get_TrackPositionPtr(const _string& strAnimName);
	_float								Get_Duration(const _string& strAnimName);

	HRESULT Bind_Bone_to_GUI(_int& iBoneIndex, _fmatrix TransformMatrix);
	void Render_Gizmo(_fmatrix TransformMatrix);

	_bool Find_Animation(const _string& strAnimName);

	void Print_ShapeKeyWeights();
#endif

public:
	void								Register_Notify(const _string& strFilePath, const vector<function<void()>>& Functions);
	void								Register_AllNotifies(const _string& strNotifyFolderPath, function<void(const _wstring&, _bool)> ColliderCallback, function<void(const _wstring&)> EffectCallback, function<void(const _wstring&)> ObjectCallback);

#ifdef _DEBUG
	void								Clear_AllNotifies(); // Debug 용도
#endif // _DEBUG



public:
	virtual		HRESULT				Initialize_Prototype(MODELTYPE eType, _fmatrix PreTransformMatrix, const _char* pFilePath);
	virtual		HRESULT				Initialize_Clone(void* pArg);
	HRESULT							Render(_uint iMeshIndex);
	HRESULT							Render(_uint iMeshIndex, ID3D11DeviceContext* pDC);

#ifdef _DEBUG
	_bool								Is_Picked(const _fvector& vRayPos, const _fvector& vRayDir, _float* pDistance);
#endif
public:
	HRESULT							Bind_Materials(class CShader* pShader, const _char* pConstantName, _uint iMeshIndex, TEXTURETYPE eTextureType, _uint iTextureIndex);
	HRESULT							Bind_Materials(class CShader* pShader, const _char* pConstantName, _uint iMeshIndex, TEXTURETYPE eTextureType);
	HRESULT							Bind_Materials(class CDeferredShader* pShader, const _char* pConstantName, _uint iMeshIndex, TEXTURETYPE eTextureType, _uint iTextureIndex, ID3DX11Effect* pEffect);
	HRESULT							Bind_Materials(class CDeferredShader* pShader, const _char* pConstantName, _uint iMeshIndex, TEXTURETYPE eTextureType, ID3DX11Effect* pEffect);
	HRESULT							Bind_BoneMatrices(class CShader* pShader, const _char* pConstantName, _uint iMeshIndex);
	HRESULT							Bind_MorphedResult(class CShader* pShader, _uint iMeshIndex, const _char* pConstantName); // Mesh의 Morph 연산을 바인딩합니다.
	HRESULT							Clear_Materials(class CDeferredShader* pShader, const _char* pConstanceName, _uint iMeshIndex, TEXTURETYPE eTextureType, ID3DX11Effect* pEffect);

	_bool	Play_Animation_CPU(const _string& strAnimationName, _float fTimeDelta, _float* pTrackPosition, _bool isBlend = true, _bool isRootMotion = true, _bool IsRootMotionRotate = true, _bool IsRootMotionTranslate = true, _float fRootMotionRate = 0.1f);
	_bool	Play_Animation_CPU(const ANIMATION_PLAY_DESC& playDesc, const ROOTMOTION_DESC& rootMotionDesc, _float fTimeDelta);
	_bool	Play_BlendSpace_CPU(const BLENDSPACE_1D_DESC& desc, const ROOTMOTION_DESC& rootMotionDesc, _float fTimeDelta);
	
	// Compute Shader
	_bool	Play_Animation_GPU(class CComputeShader* pComputeShaderCom, const ANIMATION_PLAY_DESC& playDesc, const ROOTMOTION_DESC& rootMotionDesc, _float fTimeDelta);
	_bool	Play_Animation_GPU(class CComputeShader* pComputeShaderCom, class CComputeShader* pMorphComputeShaderCom, const ANIMATION_PLAY_DESC& playDesc, const ROOTMOTION_DESC& rootMotionDesc, _float fTimeDelta);
	_bool	Play_NonRibAnimation_GPU(class CComputeShader* pComputeShaderCom, const ANIMATION_PLAY_DESC& playDesc, const ROOTMOTION_DESC& rootMotionDesc, _float fTimeDelta);
	_bool	Play_FlyAnimation_GPU(class CComputeShader* pComputeShaderCom, const ANIMATION_PLAY_DESC& playDesc, const ROOTMOTION_DESC& rootMotionDesc, const GPU_BLEND_INFO& gpuBlendInfo, _float fTimeDelta);
	_bool	Play_FlyAnimation_GPU(class CComputeShader* pComputeShaderCom, CComputeShader* pMorphComputeShaderCom, const ANIMATION_PLAY_DESC& playDesc, const ROOTMOTION_DESC& rootMotionDesc, const GPU_BLEND_INFO& gpuBlendInfo, _float fTimeDelta);
	void	Clear_Animation(const _string& strAnimationName, _float fTrackPosition = 0.f);

	_uint								Get_BoneSize() { return  static_cast<_uint>(m_Bones.size()); }
	const _float4x4* Get_BoneMatrixPtr(_uint iBoneIndex);
	void								Update_BoneMatrix_Map();
private:
	MODELTYPE							m_eType = { MODELTYPE::NONANIM };

	_uint									m_iNumMeshes = {};
	vector<class CMesh*>					m_Meshes;

	_uint									m_iNumMaterials = {};
	vector<class CMeshMaterial*>			m_Materials;

	_float4x4								m_PreTransformMatrix = {};

	_float4									m_vPreRootRotation = {};
	_float4									m_vPreRootPosition = {};
	_matrix									m_RootMatrix = {};
	_uint									m_iRootBoneIndex = {};
	vector<class CBone*>					m_Bones;

	_uint									m_iNumAnimations = {};
	

	map<_string, class CAnimation*>			m_Animations;
	map<_string, _uint>						m_AnimationNameToIndex; // Compute Shader


	ROOT_MOTION_DELTA m_RootMotionDelta;

	_bool									m_isChangeAnimation = { false };

private:
	// Blend
	_float					m_fBlendElapsed = 0.f;
	_float					m_fBlendWeight = 0.f;
	_bool					m_isBlending = { false };

	_string					m_strPreAnimation;
	_string					m_strCurAnimation;

	// BlendSpace
	_float					m_fBlendSpacePhase = 0.f;

private:
	_float								m_fPreScale = {}; // RootMotionRate에 곱해줄 값.

	BoundingBox* m_pBoundingBox = { nullptr };

	static const _string kRibPrefix;

#ifdef _DEBUG
	vector<_string>					m_AnimationNames;
	_uint m_iSelectIndex = { 0 };
#endif

#pragma region FACIAL
	vector<_string> 		   m_ShapeKeyNames;
	vector<_float>			   m_ShapeKeyWeights;
	map<_string, _uint>		   m_ShapeKeyIndices;

	_float4x4				   m_ConversionMatrix = {};

#pragma endregion


#pragma region 더블 버퍼링
	_uint m_iCurStagingFlip = { 0 };		// 이번 프레임에 GPU가 복사할 버퍼 인덱스
	_bool m_bIsStagingFilled = { false }; // 최소한 한 프레임이 지나서 읽을 데이터가 있는지 확인합니다.
	vector <_float4x4> m_vLocalMatrices;
#pragma endregion





#pragma region Compute Shader
private:
	void FetchLocalMatrices_FromCompute(class CComputeShader* pComputeShaderCom, _float fTrackPosition, const _string& strAnimationName);
	void FetchLocalMatrices_FromComputeFly(class CComputeShader* pComputeShaderCom, _float fTrackPosition, const _string& strAnimationName, const GPU_BLEND_INFO& gpuBlendInfo);
	void FetchLocalMatrices_FromComputeNonRib(class CComputeShader* pComputeShaderCom, _float fTrackPosition, const _string& strAnimationName);


private:
	vector<ID3D11Buffer*> m_Buffers = {};
	vector<ID3D11ShaderResourceView*> m_SRVs = {};
	vector<ID3D11UnorderedAccessView*> m_UAVs = {};

	_bool m_isRibAnimation = { false };

#pragma endregion



private:
	CAnimation*						Get_AnimationOrNull(const string& name);
	void							Compute_RootAnimation(_float fRootMotionRate, _bool IsRootMotionRotation = true, _bool IsRootMotionTranslate = true, _bool IsRootMotionEnable = true);
	void							HandleAnimationChange(const _string& strAnimationName);
	void							Update_MorphAnimation(CAnimation* pAnimation, CComputeShader* pMorphComputeShaderCom, _float fTimeDelta, _bool isFacial);
	_bool							Update_TrackPosition(CAnimation* pAnimation, _float* pTrackPosition,  _float fTimeDelta);

	void							Update_NonRibAnimConstantBuffer(const _string& strAnimationName, _float fTrackPosition);
	void 							Update_AnimConstantBuffer(const _string& strAnimationName, _float fTrackPosition);
	void 							Update_FlyAnimConstantBuffer(const GPU_BLEND_INFO& gpuBlendInfo, const _string& strAnimationName, _float fTrackPosition);
	void							Bind_AnimationResource(CComputeShader* pComputeShaderCom);
	void							Bind_FlyAnimationResource(CComputeShader* pComputeShaderCom);
	void							Readback_BoneMatrices();
	_uint							GetSafeIndex(const _string& strAnimName);

private:
	HRESULT							Ready_NonAnimModel(_fmatrix PreTransformMatrix, const _char* pFilePath, ifstream& InputFile);
	HRESULT							Ready_AnimModel(_fmatrix PreTransformMatrix, const _char* pFilePath, ifstream& InputFile);
	HRESULT							Ready_CharacterModel(_fmatrix PreTransformMatrix, const _char* pFilePath, ifstream& InputFile);
	HRESULT							Ready_EchoModel(_fmatrix PreTransformMatrix, const _char* pFilePath, ifstream& InputFile);



private:
	HRESULT							Ready_Bone(ifstream& InputFile, _int iParentIndex);
	HRESULT							Ready_Mesh(ifstream& InputFile);
	HRESULT							Ready_ShapeKeyMesh(ifstream& InputFile);

private:
	HRESULT							Organize_ShapeKeyIndices();
	HRESULT							Ready_Mesh_MorphBuffers();

	HRESULT							Ready_Material(const _char* pFilePath);
	HRESULT							Ready_Animation(const _char* pFilePath);
	HRESULT							Ready_MorphAnimation(const _char* pFilePath);


	HRESULT							Ready_Shared_Buffers();
	HRESULT							Ready_Instance_Buffers();

private:
	HRESULT							Ready_MorphInstance_Buffers();





public:
	static		CModel* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, MODELTYPE eType, _fmatrix PreTransformMatrix, const _char* pFilePath);
	virtual		CComponent* Clone(void* pArg);
	virtual		void					Free() override;
};

NS_END