#pragma once
#include "GameObject.h"

NS_BEGIN(Engine)
class CModel;
class CTransform;
class CShader;
NS_END

NS_BEGIN(Client)

class CMotionTrail final : public CGameObject
{
public:
	typedef struct tagMotionTrailDesc {
		CTransform* pTransform;
		CModel* pModel;
		_float fDuration;
		_float fInterval;
		_float fMotionLifeTime;
		_float4 vColor;
		_uint iShaderPassIndex;
	}MOTION_TRAIL_DESC;

private:
	typedef struct tagMotionTrailData{
		_float4x4 WorldMatrix;
		vector<vector<_float4x4>> BoneMatrices;
		_float2 vMotionLifeTime;
		_float4 vColor;
	}MOTION_TRAIL_DATA;

private:
	CMotionTrail(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CMotionTrail(const CMotionTrail& Prototype);
	virtual ~CMotionTrail() = default;

public:
	virtual		HRESULT		Initialize_Prototype() override;
	virtual		HRESULT		Initialize_Clone(void* pArg) override;
	virtual		void		Priority_Update(_float fTimeDelta) override;
	virtual		void		Update(_float fTimeDelta) override;
	virtual		void		Late_Update(_float fTimeDelta) override;
	virtual		void		Render_OutLine() override;
	virtual		void		Reset(const _fmatrix& WorldMatrix, void* pArg) override;

private:
	_uint						m_iShaderPassIndex = {};
	_uint						m_iNumMeshes = {};			// Owner Model Mesh
	vector<_uint>				m_iNumBones;			// Owner Model Bones

	_uint						m_iNumTrails = {};

	_float						m_fInterval = {};
	_float						m_fCurInterval = {};

	_float						m_fDuration = {};
	_float						m_fCurDuration = {};

	_float						m_fMotionLifeTime = {};
	_float4						m_vColor = {};

	deque<MOTION_TRAIL_DATA>	m_Datas;

	CShader*					m_pShader = { nullptr };
	CModel*						m_pOwnerModel = { nullptr };
	CTransform*					m_pOwnerTransform = { nullptr };

private:
	HRESULT					Ready_Components();

	void					Add_MotionTrail();
	void					Update_Datas(_float fTimeDelta);

	HRESULT					Bind_BoneMatrices(_uint iTrailIndex, _uint iMeshIndex);

public:
	static CMotionTrail*	Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject*	Clone(void* pArg) override;
	virtual void			Free() override;
};

NS_END