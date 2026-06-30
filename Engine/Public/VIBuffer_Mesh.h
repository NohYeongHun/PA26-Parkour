#pragma once
#include "VIBuffer.h"

#include "VIBuffer_Instance.h"


NS_BEGIN(Engine)

class ENGINE_DLL CVIBuffer_Mesh final : public CVIBuffer
{

public:
	enum FacingMod { OutWard, InWard, TangentCW, TangentCCW, Offset, LookAt };
	// ?ㅼ젙?댁쨾?쇳븯??媛믪씠 ?덈Т 留롮????대젃寃??댁빞?좉굅 媛숈????, 諛⑺뼢 ?ㅼ젙 怨좊? 留롮씠 ?대킄?쇳븷嫄?媛숈쓬  bool濡??섍린?먮뒗 ?ㅼ젙?댁쨾?쇳븯??諛⑺뼢怨?媛믪씠 苑?留롮쓣嫄곌컳??
	// ?닿구 諛쏆븘?? 泥댄겕?댁쨾?쇳븷嫄?媛숈?????.

	typedef struct tagtMeshFXInstanceDesc : public CVIBuffer_Instance::INSTANCE_DESC
	{
		_char DatFilePath[MAX_PATH] = {};				//Dat?꾩튂 ?뚮젮硫??대젃寃??댁빞?섎뒗???ш린 異붽??
		//?꾨줈?좏????대쫫??異붽??대몮源? ?꾨땲硫?議고빀???좉퉴 怨좊??대킄?쇳븷嫄?媛숈쓬.

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
		//異붽? ?
	}MESH_FXINSTANCE_DESC;

private:
	explicit CVIBuffer_Mesh(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CVIBuffer_Mesh(const CVIBuffer_Mesh& Prototype);
	virtual ~CVIBuffer_Mesh() = default;

public:
	virtual HRESULT Initialize_Prototype(_fmatrix PreTransformMatrix, const _char* pFilePath);
	virtual HRESULT Initialize_Clone(void* pArg) override;


	//而댁뀺濡?怨꾩궛泥섎━ ?댁빞?좊벏
	void Bind_CSResources(class CComputeShader* pCShader, _float fTimeDelta);

private:
	//ID3D11Buffer* m_pCBBuffer = {};
	//ID3D11Buffer* m_pSRVBuffer = {};
	//ID3D11Buffer* m_pUAVBuffer = {};
	//ID3D11ShaderResourceView* m_pSRV = {};
	//ID3D11UnorderedAccessView* m_pUAV = {};

public:
	static CVIBuffer_Mesh* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const _char* pFilePath, _fmatrix PreTransformMatrix);
	virtual CComponent* Clone(void* pArg) override;
	virtual void Free() override;
};

NS_END
