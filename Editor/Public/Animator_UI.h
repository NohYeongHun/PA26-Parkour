#pragma once
#include "Component.h"
#include "Level_UI.h"

NS_BEGIN(Editor)

class CAnimator_UI final : public CComponent
{
public:
	enum class UI_LERPTYPE
	{
		LINEAR,
		LT,	
		RB,	
		CUBIC,
			
		END
	};

	typedef struct tagAnimatorUIDesc
	{
		CCustom_UI*		pOwner = nullptr;
	} ANIMATOR_UI_DESC;

private:
	explicit				CAnimator_UI(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit				CAnimator_UI(const CAnimator_UI& Prototype);
	virtual					~CAnimator_UI() = default;

public:
	virtual HRESULT			Initialize_Prototype()					override;
	virtual HRESULT			Initialize_Clone(void* pArg)			override;
	virtual void			Priority_Update(_float fTimeDelta);
	virtual void			Update(_float fTimeDelta);
	virtual void			Late_Update(_float fTimeDelta);
	virtual HRESULT			Render()								override;

public:
	HRESULT		Insert_Animation(CLevel_UI::UI_ANIM_DESC& Desc);
	HRESULT		Remove_Animation(_wstring strAnimName);
	HRESULT		Clear_Animation();

	HRESULT		Change_Animation(_wstring strAnimName);
	HRESULT		Change_Animation(_uint iAnimIndex);
	HRESULT		Deselect_Animation();

	CLevel_UI::UI_ANIM_DESC* Find_Animation(_wstring strAnimName);
	CLevel_UI::UI_ANIM_DESC* Find_Animation(_uint iAnimIndex);

	CLevel_UI::UI_ANIM_DESC* Get_CurAnimation() { return m_pCurAnimDesc; }

	CLevel_UI::UI_ANIM_KEYFRAME_DESC* Get_CalcedAnimKeyframeDesc() { return &m_tCalcedKeyFrameDesc; }
	CLevel_UI::UI_ANIM_KEYFRAME_DESC* Get_CurCombinedAnimKeyframeDesc() { return &m_tCombinedKeyFrameDesc; }
	void Set_CurCombinedAnimKeyframeDesc(CLevel_UI::UI_ANIM_KEYFRAME_DESC tCombinedKeyFrameDesc) { m_tCombinedKeyFrameDesc = tCombinedKeyFrameDesc; }

private:
	_float		Fix_LerpRatio(_float fIn, _uint iLerpType);				
	_float		Calc_LerpRatio(_float fStart, _float fEnd, _float Ratio);

	_float3		Calc_Lerp_Position_CMR(_uint iKeyframeIndex);			

	void		Update_Animation();

private:
	vector<CLevel_UI::UI_ANIM_DESC>		m_vecAnimationDescs = {};
	CLevel_UI::UI_ANIM_DESC*			m_pCurAnimDesc = { nullptr };

	CLevel_UI::UI_ANIM_KEYFRAME_DESC*	m_pCurKeyFrameDesc = { nullptr };

	CLevel_UI::UI_ANIM_KEYFRAME_DESC	m_tCalcedKeyFrameDesc = {};			// 실시간 desc 정보를 받아와 자식 계산에 사용하기 위함
	CLevel_UI::UI_ANIM_KEYFRAME_DESC	m_tCombinedKeyFrameDesc = {};

	CCustom_UI*							m_pOwner = { nullptr };



	_float								m_fElapsedTime = {};

public:
	static CAnimator_UI*	Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CComponent*		Clone(void* pArg) override;
	virtual void			Free() override;
};

NS_END