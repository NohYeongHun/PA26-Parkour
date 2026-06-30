#pragma once
#include "VIBuffer_Instance.h"

NS_BEGIN(Engine)

class ENGINE_DLL CVIBuffer_FXMesh_Instance final : public CVIBuffer_Instance
{
public:
	enum FacingMod { OutWard, InWard, TangentCW, TangentCCW, Offset, LookAt };
	// 설정해줘야하는 값이 너무 많은데 이렇게 해야할거 같은데 ?, 방향 설정 고민 많이 해봐야할거 같음  bool로 하기에는 설정해줘야하는 방향과 값이 꽤 많을거같음.
	// 이걸 받아서, 체크해줘야할거 같은데 흠..

	typedef struct tagtMeshFXInstanceDesc : public CVIBuffer_Instance::INSTANCE_DESC
	{
		_char DatFilePath[MAX_PATH] = {};				//Dat위치 알려면 이렇게 해야되는데 여기 추가?
		//프로토타입 이름도 추가해둘까? 아니면 조합을 할까 고민해봐야할거 같음.

		_float3	vPivot;
		_float2 vSpeed;
		_float fLifeTime;
		_bool IsLoop;

		_bool IsSpawnBox = true;
		
		_bool IsSpawnRing = false;
		_bool IsRingAngle = false;
		_float fRmin = 0.f;
		_float fRmax = 0.f;
		_float2 fDegreeAngle = { 0.f, 360.f };

		//Test
		_bool IsInWard = false;
		_bool IsOutWard = false;

		_float fPitch = 0.f;

		_float fSpreadWeight = 0.f;
		_float fDropWeight = 0.f;
		_float fRotationWeight = 0.f;
		//추가 ?
	}MESH_FXINSTANCE_DESC;
private:
	explicit CVIBuffer_FXMesh_Instance(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CVIBuffer_FXMesh_Instance(const CVIBuffer_FXMesh_Instance& Prototype);
	virtual ~CVIBuffer_FXMesh_Instance() = default;

public:
	virtual HRESULT Initialize_Prototype(_fmatrix PreTransformMatrix, const _char* pFilePath, const INSTANCE_DESC* pDesc);
	virtual HRESULT Initialize_Clone(void* pArg) override;

	//컴셰로 계산처리 해야할듯
	void Bind_CSResources(class CComputeShader* pCShader, _float fTimeDelta);
	void Reset_UAV();

private:
	ID3D11Buffer* m_pCBBuffer = {};
	ID3D11Buffer* m_pSRVBuffer = {};
	ID3D11Buffer* m_pUAVBuffer = {};
	ID3D11Buffer* m_pDefaultUAVBufer = {};

	ID3D11ShaderResourceView* m_pSRV = {};
	ID3D11UnorderedAccessView* m_pUAV = {};

public:
	static CVIBuffer_FXMesh_Instance* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const _char* pFilePath, _fmatrix PreTransformMatrix, const INSTANCE_DESC* pDesc);
	virtual CComponent* Clone(void* pArg) override;
	virtual void Free() override;
};

NS_END
