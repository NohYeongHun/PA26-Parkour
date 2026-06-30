#pragma once
#include "Base.h"

NS_BEGIN(Editor)

struct RampEdit : public ImCurveEdit::Delegate
{
	RampEdit()
	{
		mbVisible[0] = true;
		mbVisible[1] = mbVisible[2] = false;
		mMax = ImVec2(1.f, 1.f);
		mMin = ImVec2(0.f, 0.f);
	}
	size_t GetCurveCount()
	{
		return 1;
	}

	bool IsVisible(size_t curveIndex)
	{
		return mbVisible[curveIndex];
	}
	size_t GetPointCount(size_t curveIndex)
	{
		return mPoints.size();
	}

	uint32_t GetCurveColor(size_t curveIndex)
	{
		uint32_t cols[] = { 0xFF0000FF, 0xFF00FF00, 0xFFFF0000 };
		return cols[curveIndex];
	}
	ImVec2* GetPoints(size_t curveIndex)
	{
		return mPoints.data();
	}
	virtual ImCurveEdit::CurveType GetCurveType(size_t curveIndex) const { return ImCurveEdit::CurveSmooth; }
	virtual int EditPoint(size_t curveIndex, int pointIndex, ImVec2 value)
	{
		mPoints[pointIndex] = ImVec2(value.x, value.y);
		SortValues();
		for (size_t i = 0; i < GetPointCount(0); i++)
		{
			if (mPoints[i].x == value.x)
				return (int)i;
		}
		return pointIndex;
	}
	virtual void AddPoint(size_t iType, ImVec2 value)
	{
		mPoints.push_back(value);
		if (ENUM_CLASS(ITEM_TYPE::ACTION) == iType)
		{
			if (1 == mPoints.size())
			{
				CAMERA_FRAME frame = {};
				frame.fDistance = 0.f;
				frame.fFovy = 0.f;
				frame.fStartFrame = 0.f;
				frame.isLerp = true;
				frame.vRotation = _float4(0.f, 0.f, 0.f, 1.f);
				frame.vTranslation = _float3(0.f, 0.f, 0.f);
				mTargetCameraFrames.push_back(frame);
				return;
			}

			for (size_t i = 0; i < mPoints.size(); ++i)
			{
				if (mPoints[i].x <= value.x)
					continue;
				CAMERA_FRAME frame = {};
				_int iIndex = i - 1 <= 0 ? 0 : i - 1;
				memcpy(&frame, &mTargetCameraFrames[iIndex], sizeof(CAMERA_FRAME));
				//frame.vTranslation = _float3(0.f, 0.f, 0.f);
				//frame.vRotation = _float4(0.f, 0.f, 0.f, 0.f);
				//frame.fDistance = 10.f;
				//frame.fStartFrame = 0.f;
				mTargetCameraFrames.push_back(frame);

				for (_uint j = mTargetCameraFrames.size() - 1; j > i; --j)
					swap(mTargetCameraFrames[j], mTargetCameraFrames[j - 1]);

				break;
			}

			CAMERA_FRAME frame = {};
			frame.fDistance = 0.f;
			frame.fFovy = 0.f;
			frame.fStartFrame = 0.f;
			frame.isLerp = true;
			frame.vRotation = _float4(0.f, 0.f, 0.f, 1.f);
			frame.vTranslation = _float3(0.f, 0.f, 0.f);
			mTargetCameraFrames.push_back(frame);
		}
		else if (ENUM_CLASS(ITEM_TYPE::ACTOR) == iType)
		{
			//SQ_ACTOR_DATA ActorData = {};
		}
		else if (ENUM_CLASS(ITEM_TYPE::SCENE) == iType)
		{
			SCENE_CAMERA_FRAME frame = {};
			mSQCameraDatas.push_back(frame);
		}
		else if (ENUM_CLASS(ITEM_TYPE::SOUND) == iType)
		{

		}
		else if (ENUM_CLASS(ITEM_TYPE::SFX) == iType)
		{

		}
		else if (ENUM_CLASS(ITEM_TYPE::EFFECT) == iType)
		{

		}

		SortValues();
	}
	virtual ImVec2& GetMax() { return mMax; }
	virtual ImVec2& GetMin() { return mMin; }
	virtual unsigned int GetBackgroundColor() { return 0; }

	void Update_Frame(ITEM_TYPE eType) {
		for (size_t i = 0; i < mPoints.size(); ++i)
		{
			switch (eType)
			{
			case ITEM_TYPE::ACTION:
				mTargetCameraFrames[i].fStartFrame = mPoints[i].x;
				break;
			case ITEM_TYPE::ACTOR:
				mSQActorDatas[i].fStartFrame = mPoints[i].x;
				break;
			case ITEM_TYPE::SCENE:
				mSQCameraDatas[i].fStartFrame = mPoints[i].x;
				break;
			case ITEM_TYPE::SFX:
				mSQSFXDatas[i].fStartFrame = mPoints[i].x;
				break;
			case ITEM_TYPE::SOUND:
				mSQAudioDatas[i].fStartFrame = mPoints[i].x;
				break;
			case ITEM_TYPE::EFFECT:
				mSQEffectDatas[i].fStartFrame = mPoints[i].x;
				break;
			}
		}
	}

	vector<ImVec2>		mPoints;

	vector<CAMERA_FRAME>		mTargetCameraFrames;

	vector<SQ_ACTOR_DATA>		mSQActorDatas;
	vector<SCENE_CAMERA_FRAME>	mSQCameraDatas;
	vector<SQ_SFX_DATA>			mSQSFXDatas;
	vector<SQ_AUDIO_DATA>		mSQAudioDatas;
	vector<SQ_EFFECT_DATA>		mSQEffectDatas;

	size_t			mPointCount = {};
	_bool			mbVisible[3];
	ImVec2		mMin;
	ImVec2		mMax;

	_int			miSelectCurve = { -1 };
	_int			miSelectPoint = { -1 };

private:
	void SortValues()
	{
		auto b = std::begin(mPoints);
		auto e = std::begin(mPoints) + GetPointCount(0);
		std::sort(b, e, [](ImVec2 a, ImVec2 b) { return a.x < b.x; });
	}
};

class CSequencer : public CBase, ImSequencer::SequenceInterface
{
public:
	typedef struct tagSequenceItem {
		ITEM_TYPE		eType;								// Sequence Item Type
		_int				iFrameStart{}, iFrameEnd{};		// Frame Start / End
		_bool				isExpanded{};						// Can Expand
		_char				szItemLabel[MAX_PATH] = {};
		RampEdit		mRampEdit;
		tagSequenceItem() {};
		tagSequenceItem(_int iType , _int _iFrameStart, _int _iFrameEnd, _bool _isExpanded, const _char* pLabel)
			: iFrameStart {_iFrameStart}, iFrameEnd {_iFrameEnd}, isExpanded {_isExpanded}
		{
			strcpy_s(szItemLabel, MAX_PATH, pLabel);
			switch (iType)
			{
			case ENUM_CLASS(ITEM_TYPE::ACTOR):
				eType = ITEM_TYPE::ACTOR;
				break;
			case ENUM_CLASS(ITEM_TYPE::ACTION):
				eType = ITEM_TYPE::ACTION;
				break;
			case ENUM_CLASS(ITEM_TYPE::EFFECT):
				eType = ITEM_TYPE::EFFECT;
				break;
			case ENUM_CLASS(ITEM_TYPE::SCENE):
				eType = ITEM_TYPE::SCENE;
				break;
			case ENUM_CLASS(ITEM_TYPE::SFX):
				eType = ITEM_TYPE::SFX;
				break;
			case ENUM_CLASS(ITEM_TYPE::SOUND):
				eType = ITEM_TYPE::SOUND;
				break;
			}
		}
	}SEQUENCE_ITEM;

	typedef struct tagCustomDraw {
		_int iIndex = {};
		ImRect CustomRect;
		ImRect LegendRect;
		ImRect ClippingRect;
		ImRect LegendClippingRect;
		tagCustomDraw(_int _iIndex, const ImRect& _CustomRect, const ImRect& _LegendRect, const ImRect& _ClippingRect, const ImRect& _LegendClippingRect)
			: iIndex { _iIndex }, CustomRect { _CustomRect }, LegendRect { _LegendRect }, ClippingRect { _ClippingRect }, LegendClippingRect { _LegendClippingRect }
		{}
	}CUSTOM_DRAW;

private:
	explicit CSequencer();
	virtual ~CSequencer() = default;

public:
	_int							GetFrameMin() const override { return m_iFrameMin; }
	_int							GetFrameMax() const override { return m_iFrameMax; }
	_int							GetItemCount() const override { return m_Items.size(); }
	// Item Info Get
	void							Get(_int index, _int** start, _int** end, _int* type, _uint* color) override;

	virtual void					Add(_int iType) override;
	virtual const _char*		GetItemTypeName(_int iIndex) const override;
	virtual size_t					GetCustomHeight(_int iIndex) { return m_iSelectedEntry == iIndex && m_Items[iIndex].isExpanded ? 300 : 0; }

	void							CustomDraw(RampEdit& delegate, _int iIndex, const ImRect& customRect, const ImRect& legendRect, const ImRect& clippingRect, const ImRect& legendClippingRect);
	void							CustomDrawCompact(RampEdit& delegate, _int iIndex, const ImRect& customRect, const ImRect& clippingRect);

public:
	HRESULT							Initialize();
	void								Update(_float fTimeDelta);

#pragma region Private Varation
private:
	class CGameInstance*		m_pGameInstance = { nullptr };
	ImGuiIO							io;

	// Custom Draw Label
	const _char* m_pCustomDrawLabel[3] = { "Translation", "Rotation", "Scale" };

	// Sequence Option
	_int								m_iSequenceOption = {};

	// DrawList
	ImDrawList* m_pDrawList = { nullptr };

	// Canvas
	_bool								m_isExpanded = { true };
	ImVec2							m_vCanvasPos = {};
	ImVec2							m_vCanvasSize = {};

	// Frame Min / Max
	_int								m_iFrameMin{}, m_iFrameMax{};
	_int								m_iFrameCnt = {};
	_int								m_iCurrentFrame{};
	vector<SEQUENCE_ITEM>	m_Items;
	_int								m_iFirstFrame = {};

	// Content
	ImVec2							m_vContentMin = {};
	ImVec2							m_vContentMax = {};
	ImRect							m_ContentRect = {};
	_float								m_fContentHeight = {};

	// Draw Frame
	_float								m_fFramePixelWidth = { 10.f };
	_float								m_fFramePixelWidthTarget = { 10.f };
	_int								m_iItemHeight = { 20 };						// Item 1�� �� Height
	_int								m_iLegendWidth = { 200 };					// ����(ǥ�� ����) Width

	ImVec2							m_vChildFramePos = {};
	ImVec2							m_vChildFrameSize = {};

	// Moving
	_int								m_iMovingEntry = { -1 };
	_int								m_iMovingPos = { -1 };
	_int								m_iMovingPart = { -1 };
	_bool								m_isMovingScrollBar = { false };
	_bool								m_isMovingCurrentFrame = { false };
	_bool								m_isRet = { false };

	// Panning
	_bool								m_isPanningView = { false };
	ImVec2							m_vPanningViewSource = {};
	_int								m_iPanningViewFrame = {};

	// Entry
	_int								m_iSelectedEntry = { -1 };						// Select Entry(�׸�)
	_int								m_iDelEntry = { -1 };
	_int								m_iDupEntry = { -1 };

	// PopUp
	_bool								m_isPopUp = { false };

	// Header
	_int								m_iModFrameCnt = { 10 };
	_int								m_iHalfModFrameCnt = {};
	_int								m_iFrameStep = { 1 };

	// Custom Draw
	vector<CUSTOM_DRAW>	m_CustomDraws;
	vector<CUSTOM_DRAW>	m_CompactCustomDraws;

	// Cursor
	_float								m_fCursorWidth = { 8.f };

	// ScrollBar
	_bool								m_isScrollBar = { true };
	_int								m_iVisibleFrameCnt = {};
	_float								m_fBarWidthRatio = {};
	_float								m_fBarWidthInPixels = {};
	_bool								m_isSizingRightBar = { false };
	_bool								m_isSizingLeftBar = { false };
	_float								m_fMinBarWidth = {};

#pragma endregion

private:
	_bool								m_isSave = { false };
	_bool								m_isLoad = { false };

	// Camera Action
	_bool								m_isEscape = { false };

	// Sequence System Variable
	_char								m_szSequenceTag[MAX_PATH] = {};
	_bool								m_isPlay = { false };
	_float								m_fTrackPerSec = {};
	_float								m_fTrackAcc = {};

	_bool								m_isSaveSequence = { false };
	_bool								m_isLoadSequence = { false };

private:
	// Sequence
	void								Play(_float fTimeDelta);
	void								Sequence_System(_float fTimeDelta);
	void								Save_Sequence();
	void								Load_Sequence();

	void								Save_Scene(json& Output, SEQUENCE_ITEM& item);
	void								Load_Scene(json& Input);

	// Selectable Item
	void								Selectable_Item();
	void								Sorting_Item();

	// Item SetUp
	void								SetUp_Camera(SEQUENCE_ITEM& item);
	void								SetUp_Scene(SEQUENCE_ITEM& item);

	// Point SetUp
	void								SetUp_Camera_Point(SEQUENCE_ITEM& item);
	void								SetUp_Scene_Point(SEQUENCE_ITEM& item);

	// Camera Action
	void								Save_CameraAction();
	void								Load_CameraAction();
	
	// GUI
	void								Drawing();
	void								Panning(const _int iVisibleFrameCnt);	//  (Alt + Wheel Click -> Drag => ȭ�� �¿� �̵�)
	void								Expand(_int iControllHeight);	// Canvas Expand
	void								DrawFrame();						// Sequence ������
	void								DrawLegend();						// List
	void								ItemDupDel();
	void								DrawSlot();							// Slot (Item)
	void								Moving();							// Item Duration ���� (Left, Right ��ƴ���)		
	void								Cursor();								// Cursor (Frame ȭ�� ���� ��)
	void								CopyPaste();							// Copy / Paste
	void								ScrollBar();							// ScrollBar

	// Header
	void								DrawLine(_int iFrame, _int iRegionHeight);
	void								DrawLineContent(_int iFrame, _int iRegionHeight);

private:
	_bool								SequencerAddDelButton(ImVec2 vPos, _bool isAdd = true);

public:
	static		CSequencer*	Create();
	virtual		void				Free() override;
};

NS_END