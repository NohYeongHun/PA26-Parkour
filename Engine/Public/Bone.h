#pragma once
#include "Base.h"

NS_BEGIN(Engine)

class CBone final : public CBase
{
private:
	explicit CBone();
	explicit CBone(const CBone& Copy);
	virtual ~CBone() = default;

public:
	_int Get_ParentIndex() const { return m_iParentBoneIndex; }
	const _char*				Get_Name() { return m_szName; }
	const _float4x4*		Get_CombinedTransformationMatrix() { return &m_CombinedTransformationMatrix; }
	const _float4x4*		Get_TransformationMatrix() { return &m_TransformationMatrix; }
	void					Set_TransformationMatrix(const _fmatrix& Matrix) {
		XMStoreFloat4x4(&m_TransformationMatrix, Matrix);
	}

	void					Set_CombinedTransformationMatrix(const _fmatrix& Matrix) {
		XMStoreFloat4x4(&m_CombinedTransformationMatrix, Matrix);
	}
public:
	HRESULT					Initialize(const _char* pBoneName, const _fmatrix& TransformationMatrix, _int iParentBoneIndex);
	void						Update_CombinedTransformationMatrix(const _fmatrix& PreTransformationMatrix, const vector<CBone*>& Bones);
	void						Update_RibCombinedTransformationMatrix(const _fmatrix& PreTransformationMatrix, const vector<CBone*>& Bones);
	void						Update_CombinedTransformationMatrix(const _fmatrix& PreTransformationMatrix, const _float4x4* pBoneMatrix, const vector<CBone*>& Bones);

private:
	_float4x4					m_TransformationMatrix = {};
	_float4x4					m_CombinedTransformationMatrix = {};

	_int						m_iParentBoneIndex = {};
	_char						m_szName[MAX_PATH] = {};

public:
	static		CBone*		Create(const _char* pBoneName, const _fmatrix& TransformationMatrix, _int iParentBoneIndex);
	CBone*	Clone();
	virtual		void			Free() override;
};

NS_END