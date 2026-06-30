#pragma once
#include "Base.h"

NS_BEGIN(Engine)

class CPipeLine final : public CBase
{
private:
	explicit CPipeLine();
	virtual ~CPipeLine() = default;

public:
	const _float4x4*				Get_TransformState_Float4x4(D3DTS eState) const;
	_matrix							Get_TransformState_Matrix(D3DTS eState) const;

	const _float4x4*				Get_TransformState_Float4x4_Inv(D3DTS eState) const;
	_matrix							Get_TransformState_Matrix_Inv(D3DTS eState) const;

	const _float4x4*				Get_PrevTransformState_Float4x4(D3DTS eState) const;
	_matrix							Get_PrevTransformState_Matrix(D3DTS eState) const;

	void							Set_TransformState(D3DTS eState, _fmatrix Matrix);
	void							Set_TransformState(D3DTS eState, const _float4x4& Matrix);
	
	void							Set_PrevTransformState(D3DTS eState, _fmatrix Matrix);
	void							Set_PrevTransformState(D3DTS eState, const _float4x4& Matrix);


	const _float4*					Get_CamPos() const { return &m_vCamPos; }

	_float							Compute_Distance(class CGameObject* pObject);

public:
	HRESULT							Initialize();
	void							Update();

private:

	_float4x4						m_TransformMatrixes[ENUM_CLASS(D3DTS::END)] = {};
	_float4x4						m_TransformMatrixes_Inv[ENUM_CLASS(D3DTS::END)] = {};
	_float4x4						m_PrevTransformMatrixes[ENUM_CLASS(D3DTS::END)] = {};
	_float4							m_vCamPos = {};


public:
	static		CPipeLine*		Create();
	virtual		void				Free() override;
};

NS_END