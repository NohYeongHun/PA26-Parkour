#pragma once
#include "Level.h"
#include "Custom_UI.h"

NS_BEGIN(Editor)

class CLevel_UI final : public CLevel
{
#pragma region Structs

public:
	// for Editor
	typedef struct tagHierarchyObjectDesc
	{
		_wstring				strObjName	= {};
		CCustom_UI*				pCustomUI	= nullptr;
	} HIERARCHY_OBJ_DESC;


	// for Output
	// ?섏쨷??*_Struct.h 濡???꺼??????
	typedef struct tagUIAnimKeyFrameDesc
	{
		_uint			iKeyframeIndex = {};			// ?뺣낫媛 ?닿만 ?ㅽ봽?덉엫 ?뺣낫
		_uint			iLerpType = {};

		_uint			iTexIndex = {};
		_float			fAlpha = {};			// 0 ~ 1
		_float3			vPos = {};
		_float3			vRot = {};			// Euler
		_float3			vSca = {};


		_float2			vScreenLT = {};			// ǥ�õ� ȭ����� ��ǥ ����. (��� 0, 0 / ���� ȭ��ũ��)
		_float2			vScreenRB = { g_iWinSizeX, g_iWinSizeY };

		_float4			vBlendToOuterWidth = {};

	} UI_ANIM_KEYFRAME_DESC;

	typedef struct tagUIAnimDesc
	{
		CCustom_UI::CUSTOM_UI_DESC		tUIDesc = {};	// FilePath, FileName, NumTex (� �ؽ��Ŀ������� ����)

		// ?ㅽ봽?덉엫, ?ㅽ봽?덉엫蹂??됰젹?뺣낫, 蹂닿컙諛⑸쾿, 湲몄씠 ??.
		_wstring				strAnimName = {};
		//_uint					iNumKeyFrame = {};

		vector<UI_ANIM_KEYFRAME_DESC>	vecKeyFrames = {};

		//_uint					iLerpType = {};
		_bool					isLoop = false;
	} UI_ANIM_DESC;

	
	typedef struct tagUIInfoDesc
	{
		CCustom_UI::CUSTOM_UI_DESC	tUIDesc = {};	// FilePath, FileName, NumTex

		_float3						vPos = {};
		_float3						vRot = {};	// Euler
		_float3						vSca = {};
	} UI_INFO_DESC;

	typedef struct tagCustomUITreeDesc {
		wstring					strTreeName = {};

		vector<UI_INFO_DESC>	vecUIInfoDescs = {};
	} CUSTOM_UITREE_DESC;

#pragma endregion

private:
	explicit CLevel_UI(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CLevel_UI() = default;

public:
	virtual HRESULT		Initialize() override;
	virtual void		Update(_float fTimeDelta) override;
	virtual void		Render() override;

private:
	void				Update_Picking();

	void				Update_MenuWindow();
	void				Update_Hierarchy();
	void				Update_Hierarchy_CheckTree(CCustom_UI* pParentUI, ImGuiTreeNodeFlags flags);
   
	void				Update_SaveLoad();
	void				Update_Inspector();
	void				Update_AnimEditor(_float fTimeDelta);
	void				Update_InstanceEditor();

	void				Update_ObjectParents();
	void				Update_ObjectChilds();

	void				Update_SelectedKeyframeDesc();

private:
	class CGameObject*			m_pCurObj = { nullptr };
	class CGameObject*			m_pPreObj = { nullptr };

	_float3						m_vCurObjPos = {};
	_float3						m_vCurObjRot = {};	// Euler
	_float3						m_vCurObjSca = {};



	// for Update_SaveLoad
	_bool						m_isOn_SaveLoad = true;


	// for Update_Hierarchy
	vector<HIERARCHY_OBJ_DESC>	m_vecCustomUIs = {};


	// for Update_AnimEditor
	// ��AnimEditor
	_bool						m_isOn_AnimEdit = true;
	vector<UI_ANIM_DESC>		m_vecUIAnims = {};

	vector<UI_ANIM_KEYFRAME_DESC> m_vecUIKeyFrameDescs = {};	// Temp Keyframe
	UI_ANIM_KEYFRAME_DESC*		m_pSelectedKeyFrameDesc = { nullptr };
	_int						m_iLerpType = 0;
	_bool						m_isAnimLoop = false;

	// ?퀮nimList
	_bool						m_isPlayAnimation = false;
	UI_ANIM_DESC*				m_pSelectedUIAnim = { nullptr };

	// ��Instance
	CVIBuffer_Rect_Instance_UI::SINGLE_INST_DESC*	m_pSelectedInstance = { nullptr };

public:
	static CLevel_UI*		Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual void			Free() override;
};

NS_END

#pragma region json

inline json vec_to_json(const std::vector<_float3>& vec)
{
	json j = json::array();
	for (const auto& v : vec)
		j.push_back({ {"x", v.x}, {"y", v.y}, {"z", v.z} });
	return j;
}


inline void to_json(json& j, const CLevel_UI::UI_ANIM_KEYFRAME_DESC& d)
{
	j = json{
		{ "iKeyframeIndex", d.iKeyframeIndex },

		{ "iLerpType", d.iLerpType },
		{ "iTexIndex", d.iTexIndex },
		{ "fAlpha", d.fAlpha },
		{ "vecPos", { d.vPos.x, d.vPos.y, d.vPos.z } },
		{ "vecRot", { d.vRot.x, d.vRot.y, d.vRot.z } },
		{ "vecSca", { d.vSca.x, d.vSca.y, d.vSca.z } },

		{ "vScreenLT", { d.vScreenLT.x, d.vScreenLT.y } },
		{ "vScreenRB", { d.vScreenRB.x, d.vScreenRB.y } },

		{ "vBlendToOuterWidth", { d.vBlendToOuterWidth.x, d.vBlendToOuterWidth.y, d.vBlendToOuterWidth.z, d.vBlendToOuterWidth.w} }
	};
}

inline void from_json(const json& j, CLevel_UI::UI_ANIM_KEYFRAME_DESC& d)
{
	d.iKeyframeIndex	= j["iKeyframeIndex"];
	
	d.iLerpType			= j["iLerpType"];
	d.iTexIndex			= j["iTexIndex"];
	d.fAlpha			= j["fAlpha"];

	d.vPos = { j["vecPos"][0], j["vecPos"][1], j["vecPos"][2] };
	d.vRot = { j["vecRot"][0], j["vecRot"][1], j["vecRot"][2] };
	d.vSca = { j["vecSca"][0], j["vecSca"][1], j["vecSca"][2] };

	d.vScreenLT = _float2(j["vScreenLT"][0], j["vScreenLT"][1]);
	d.vScreenRB = _float2(j["vScreenRB"][0], j["vScreenRB"][1]);

	d.vBlendToOuterWidth = _float4(
		j["vBlendToOuterWidth"][0], j["vBlendToOuterWidth"][1], 
		j["vBlendToOuterWidth"][2], j["vBlendToOuterWidth"][3]
	);
}

inline void to_json(json& j, const vector<CLevel_UI::UI_ANIM_KEYFRAME_DESC>& vec)
{
	j = json::array();
	for (const auto& v : vec)
	{
		json data = {};
		to_json(data, v);
		j.push_back(data);
	}
}

inline void from_json(const json& j, vector<CLevel_UI::UI_ANIM_KEYFRAME_DESC>& vec)
{
	vec.clear();
	vec.reserve(j.size());

	for (const auto& element : j)
	{
		CLevel_UI::UI_ANIM_KEYFRAME_DESC desc = {};
		from_json(element, desc);
		vec.push_back(desc);
	}
}

inline void to_json(json& j, const CLevel_UI::UI_ANIM_DESC& d)
{
	json vecKeyFrames = {};
	to_json(vecKeyFrames, d.vecKeyFrames);

	json j_tUIDesc = {};
	to_json(j_tUIDesc, d.tUIDesc);

	j = json{
		{ "tUIDesc", j_tUIDesc },
		{ "strAnimName", WStringToString(d.strAnimName) },
		{ "iNumKeyFrame", d.vecKeyFrames.size()},
		{ "vecKeyFrames", vecKeyFrames },
		//{ "iLerpType", d.iLerpType },
		{ "isLoop", d.isLoop }
	};
}

inline void from_json(const json& j, CLevel_UI::UI_ANIM_DESC& d)
{
	from_json(j["vecKeyFrames"], d.vecKeyFrames);
	from_json(j["tUIDesc"], d.tUIDesc);

	//d.iLerpType		= j["iLerpType"];
	d.isLoop		= j["isLoop"];
	_string strAnimName = j["strAnimName"].get<_string>();
	d.strAnimName	= StringToWString(strAnimName);
}

inline void to_json(json& j, const CLevel_UI::UI_INFO_DESC& d)
{
	json j_tUIDesc = {};
	to_json(j_tUIDesc, d.tUIDesc);

	j = json{
		{ "tUIDesc", j_tUIDesc },
		{ "vPos", {d.vPos.x, d.vPos.y, d.vPos.z} },
		{ "vRot", {d.vRot.x, d.vRot.y, d.vRot.z} },
		{ "vSca", {d.vSca.x, d.vSca.y, d.vSca.z} }
	};
}

inline void from_json(const json& j, CLevel_UI::UI_INFO_DESC& d)
{
	from_json(j["tUIDesc"], d.tUIDesc);

	d.vPos = {j["vPos"][0], j["vPos"][1], j["vPos"][2]};
	d.vRot = {j["vRot"][0], j["vRot"][1], j["vRot"][2]};
	d.vSca = {j["vSca"][0], j["vSca"][1], j["vSca"][2]};
}

inline void to_json(json& j, const vector<CLevel_UI::UI_INFO_DESC>& vec)
{
	j = json::array();
	for (const auto& v : vec)
	{
		json data = {};
		to_json(data, v);
		j.push_back(data);
	}
}

inline void from_json(const json& j, vector<CLevel_UI::UI_INFO_DESC>& vec)
{
	vec.clear();
	vec.reserve(j.size());

	for (const auto& element : j)
	{
		CLevel_UI::UI_INFO_DESC desc = {};
		from_json(element, desc);
		vec.push_back(desc);
	}
}

inline void to_json(json& j, const CLevel_UI::CUSTOM_UITREE_DESC& d)
{
	json vecUIInfoDescs = {};
	to_json(vecUIInfoDescs, d.vecUIInfoDescs);

	j = {
		{ "strTreeName", WStringToString(d.strTreeName) },
		//{ "iNumUIDescs", d.vecUIDescs.size() },
		{ "vecUIInfoDescs", vecUIInfoDescs }
	};
}

inline void from_json(const json& j, CLevel_UI::CUSTOM_UITREE_DESC& d)
{
	_string strTreeName = j["strTreeName"].get<_string>();
	d.strTreeName = StringToWString(strTreeName);
	from_json(j["vecUIInfoDescs"], d.vecUIInfoDescs);
}

#pragma endregion
