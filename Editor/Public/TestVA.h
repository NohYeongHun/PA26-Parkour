#pragma once
#include "GameObject.h"

NS_BEGIN(Engine)
class CShader;
class CTexture;
class CVAMesh;
NS_END

NS_BEGIN(Editor)

class CTestVA : public CGameObject
{
public:
	typedef struct tagVADesc : Engine::EFFECT_DESC
	{
		_wstring		strTextureTag = {};
		_wstring		strColorTextureTag = {};
		_wstring		strMeshTag = {};

		_float			fAnimSpeed = 10.f;
		_float			fMovementScale = 0.2f;

		_int			iShaderPass = 0;
	}VA_DESC;

private:
	CTestVA(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CTestVA(const CTestVA& Prototype);
	virtual ~CTestVA() = default;

public:
	virtual		HRESULT			Initialize_Prototype();
	virtual		HRESULT			Initialize_Clone(void* pArg) override;
	virtual		void				Priority_Update(_float fTimeDelta) override;
	virtual		void				Update(_float fTimeDelta) override;
	virtual		void				Late_Update(_float fTimeDelta) override;
	virtual		void				Render() override;

public:
	virtual		void				Reset(const _fmatrix& WorldMatrix, void* pArg) override;


private:
	CShader*					m_pShaderCom = { nullptr };
	CTexture*					m_pTextureCom = { nullptr };
	CTexture*					m_pColorTextureCom = { nullptr };
	CVAMesh*					m_pVAMesh = { nullptr };
	VA_DESC						m_tDesc = {};

	//_float							m_fTimeAcc = {};					// Anim 진행 시간 누적

	_uint						m_iTest = {3};
	_uint						m_iTest2 = {};

	_float						m_fTrackPosition = {};			// Anim 진행 Pos
	_float						m_fAnimationDuration = {};		// 총 Animation 진행 시간 (HDR에서 Y Max 필요)

private:
	void						Bind_Resource();
	HRESULT						Ready_Components();

public:
	static		CTestVA* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual		CGameObject* Clone(void* pArg) override;
	virtual		void				Free() override;
};

NS_END