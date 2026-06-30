#pragma once
#include "Base.h"

NS_BEGIN(Editor)
// ImGui Animation Tool
class CAnimationTool final : public CBase
{
private:
	enum class MODE : _uint
	{
		CONVERT_GLTF_TO_DAT = 0,
		CONVERT_FBX_TO_DAT = 1, 
		LOAD_DAT = 2, 
		CREATE_ACTOR = 3,
		EDIT_ANIMATION = 4, 
		SAVE_STATE = 5,
		END
	};


private:
	explicit CAnimationTool(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CAnimationTool() = default;


#pragma region 
public:
	HRESULT	Initialize(LEVEL eLevel);
	void Update();
	void Render();
#pragma endregion

public:
	void Set_EffectContorller(class CEffect_Controller* pEffectController);
	void Export_AnimationData(class CEffect_Controller* pEffectController);



private:
	// 1 Depth Menu
	void Render_Editor();
	void Render_DebugWindow();
	void Render_Menu();
	void RenderUI_EditState();

private:
	// 2 Depth Menu
	void RenderUI_ConvertFbx();
	void RenderUI_CreateActor();
	void RenderUI_EditActor();
	void RenderUI_EditAnimation();

	void RenderUI_FromState();
	void RenderUI_ToState();
	void RenderUI_Transitions();
	void RenderUI_OptionState();
	void RenderUI_TransitionInfo();
	

private:
	// 3 Depth Menu
	void LoadDat();
	void RenderUI_ModelPrototype();
	void RenderUI_EditModel();
	void RenderUI_AnimationList();

private:
	// 4 Depth Menu
	void Render_Model_Detail();
	void Render_EditModel();
	void Render_Animation_Detail();

	

private:
	// 5Depth Menu
	void Render_Animation_ChildDetail();

	
	

private:
	LEVEL m_eCurLevel = { LEVEL::END };
	MODE m_eMode = { MODE::END };
	ID3D11Device* m_pDevice = { nullptr };
	ID3D11DeviceContext* m_pContext = { nullptr };
	class CGameInstance* m_pGameInstance = { nullptr };
	class CModelLoader* m_pLoader = { nullptr };
	class CGltfLoader* m_pGltfLoader = { nullptr };
	class CAnimNotifyTool* m_pAnimNotifyTool = { nullptr };
	class CEffect_Controller* m_pEffectController = { nullptr };

	list<_string> m_ModelNames;
	list<_string> m_ActorNames;
	list<_string> m_StateTransitions; // Transition 저장할 정보.

	typedef map<const _wstring, class CAnimationActor*> ANIMATIONACTORS;
	ANIMATIONACTORS m_AnimationActors;

	typedef map<const _wstring, const _string> MODELPATHS;
	MODELPATHS	m_ModelDirPaths;

#pragma region ANIM_MACHINE_PROPERTY
	typedef struct tagTransitionData
	{
		_string strFrom;
		_string strTo;
		//조건에 사용할 const flag변수
		_int iPriority;
		//0을 추가하기 위해 마스킹 값을 n - 1로 입력 받음(ex.입력: 4 = > 2 ^ (4 - 1) = 8이 출력 됨)
		_uint iTargetState;
		_float fTargetTrackPos;
		// 현재 애니메이션에서 변환할 수 있는 트랙위치
		_float fTransitEnablePos;
		_uint eType;
		//vector<_string> ConditionConst;
	}TRANSITION_DATA;

	class CAnimMachine* m_pAnimMachineCom = { nullptr };
	_uint m_iTransitionPriority{};
	// 0을 추가하기 위해 마스킹 값을 n-1로 입력 받음(ex. 입력: 4 => 2^(4 - 1) = 8이 출력 됨)
	_uint m_iTransitionTargetState{};
	_float m_iTransitionEnablePos{};
	_uint m_eTransitConditionType{};

	//AnimState Data
	_bool m_isRootMotion;
	_bool	m_isRootMotionRotate{};
	_bool	m_isRootMotionTranslate{};
	_bool m_isLoopCheck;
	_float m_fRootMotionRate;
	_float m_fTransitTrackPos;
	_float m_fAnimationSpeed;

	TRANSITION_DATA m_tTransitionInfo{};

	vector<TRANSITION_DATA> m_TransitionDatas;
	vector<TRANSITION_DATA> m_AnyStateTransitions;
	_int m_iTransitionInfoSelectedIndex = {-1};
	_int m_iAnyStateTransitionSelectedIndex = {-1};
	_char m_szConditionName[MAX_PATH] = {};

	_bool m_isShowImport_ST_Dialog = {};
#pragma endregion

	_wstring m_wSelected_PrototypeModelTag = {};
	_string m_Selected_PrototypeModelTag = {};

	_wstring m_wSelected_AnimActorTag = {};
	_string m_Selected_AnimActorTag = {};

	_wstring m_wPrevSelected_AnimActorTag = {};
	_string m_PrevSelected_AnimActorTag = {};


	_string m_SelectedFromStateTag = {};
	_string m_SelectedToStateTag = {};

private:
	_string m_Selected_AnimationTag = {};
	_float m_fTrackPosition = {};
	_float m_fDuration = {};
	_bool m_IsVisibleNotify = { false };
	_bool m_IsPlayAnimation = { true };
	_bool m_IsStateTransition = { false };

private:
	_float m_fEditorAlpha = { 1.f };

private:
#ifdef _DEBUG
	void Export_StateAnimationMap_ToCSV();
	void Export_StateTransition_To_CSV();
	void Import_StateTransition_From_Json();
#endif

private:
	HRESULT Add_Prototype_AnimModel(_wstring strPrototypeName, MODELTYPE eType, _fmatrix PreTransformMatrix, const _char* pFilePath);

	

public:
	static CAnimationTool* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, LEVEL eLevel);
	virtual	void Free() override;
};
NS_END

