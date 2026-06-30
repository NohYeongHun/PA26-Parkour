#pragma once
#include "Level.h"

NS_BEGIN(Engine)
class CRigidbody;
NS_END

NS_BEGIN(Editor)

class CLevel_Camera final : public CLevel
{
private:
	explicit CLevel_Camera(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CLevel_Camera() = default;

public:
	virtual		HRESULT		Initialize() override;
	virtual		void			Update(_float fTimeDelta) override;
	virtual		void			Render() override;

private:
	// Interface
	class CMap_Interface*		m_pMapInterface = { nullptr };
	class CCamera_Interface*	m_pCameraInterface = { nullptr };
	class CAnimationTool*		m_pAnimationTool = { nullptr };

	// Sequencer
	class CSequencer*				m_pSequencer = { nullptr };

private:
	_bool						m_isMapInterface = { false };

	// Test�� Ground
	CRigidbody*			m_pGround = { nullptr };

private:
	void						Ready_Prototype();
	void						Ready_Light();
	void						Ready_Dummy();
	void						Ready_Ground();
	//void						Ready_Sequencer();

public:
	static		CLevel_Camera* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual		void			Free() override;
};

NS_END