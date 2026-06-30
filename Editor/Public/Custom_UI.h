#pragma once

#include "Editor_Define.h"
#include "UIObject.h"

#include "VIBuffer_Rect_Instance_UI.h"



NS_BEGIN(Engine)
class CShader;
class CTexture;
class CVIBuffer_Rect;
class CVIBuffer_Rect_Instance_UI;
NS_END

NS_BEGIN(Editor)
class CAnimator_UI;
NS_END

NS_BEGIN(Editor)

class CCustom_UI final : public CUIObject
{
#pragma region enum class & struct

public:
	enum class UI_TYPE {
		NONE, BUTTON, INTERACT, END
	};

	typedef struct tagCustomUISizeDesc {
		vector<_float2>	vecSize = {};
	} UI_SIZE_DESC;

	typedef struct tagCustomUISectorDesc {
		_float2		vSectorBorder = {}; // pixel
		_float		fUIScale = {};		// ui ����
	} UI_SECTOR_DESC;



	typedef struct tagCustomUIObjectDesc : public CUIObject::UI_DESC, UI_SIZE_DESC, UI_SECTOR_DESC {
		_wstring	strFilePath = {};
		_wstring	strFileName = {};
		_uint		iNumFiles = 1;

		_wstring	strUIName = {};
		_uint		iUIType = {};			// ?⑥닚 李쎌씤吏, 踰꾪듉?몄?, 理쒖긽??援ы쁽遺?몄? 援щ텇?
		_wstring	strParentName = {};

		_bool		isInverseScreenDiscard = false;	// �׸� ���� ����
		_float		fCutout = 0.3f;					// (1:�ƾƿ� ��� ��) ���İ� ����
		_uint		iPassType = 2;			// 0 : Normal, 1 : Cutout, 2 : Transparent, 3 : SimpleGradient

		vector<_wstring> vecChildNames = {};
		CGameObject* pParentObject = nullptr;


		_bool		isInstance = false;
		vector<CVIBuffer_Rect_Instance_UI::SINGLE_INST_DESC> vecInstanceDescs = {};
	} CUSTOM_UI_DESC;


#pragma endregion


private:
	explicit				CCustom_UI(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit				CCustom_UI(const CCustom_UI& Prototype);
	virtual					~CCustom_UI() = default;

public:
	virtual HRESULT			Initialize_Prototype()					override;
	virtual HRESULT			Initialize_Clone(void* pArg)			override;
	virtual void			Priority_Update(_float fTimeDelta)		override;
	virtual void			Update(_float fTimeDelta)				override;
	virtual void			Late_Update(_float fTimeDelta)			override;
	virtual void			Render()								override;

public:
	CUSTOM_UI_DESC&			Get_UIDesc()						{ return m_tUIDesc; }
	void					Set_UIDesc(CUSTOM_UI_DESC tUIDesc)	{ m_tUIDesc = tUIDesc; }
	void					Set_CurTexIndex(_uint iIndex)		{ m_iCurTexIndex = iIndex; }

	_bool					Is_ParentActivate();

private:
	HRESULT					Ready_Prototypes(void* pArg);
	HRESULT					Ready_Components(void* pArg);
	HRESULT					Bind_Description(void* pArg);

	void					Update_CombinedMatrix();
	void					Update_CombinedDesc();

private:
	CShader*				m_pShaderCom				= { nullptr };
	CVIBuffer*				m_pVIBufferCom				= { nullptr };
	CTexture*				m_pTextureCom				= { nullptr };
	CAnimator_UI*			m_pAnimator_UICom			= { nullptr };


	CUSTOM_UI_DESC			m_tUIDesc					= {};
	_uint					m_iCurTexIndex				= {};

	_float4x4				m_CombinedWorldMatrix		= {};

public:
	static CCustom_UI*		Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject*	Clone(void* pArg) override;
	virtual void			Free() override;
};

NS_END





#pragma region json

inline void to_json(json& j, const CCustom_UI::CUSTOM_UI_DESC& d)
{
	json childNames = json::array();
	for (const auto& v : d.vecChildNames)
	{
		json data = {};
		to_json(data, WStringToString(v));
		childNames.push_back(data);
	}

	json vecImgSizes = json::array();
	for (const auto& v : d.vecSize)
	{
		json data = {};
		data = { v.x, v.y };
		vecImgSizes.push_back(data);
	}

	json vecInstanceDescs = json::array();
	for (const auto& v : d.vecInstanceDescs)
	{
		json data = {};
		to_json(data, v);
		vecInstanceDescs.push_back(data);
	}

	j = {
		{ "strFilePath", WStringToString(d.strFilePath) },
		{ "strFileName", WStringToString(d.strFileName) },
		{ "iNumFiles", d.iNumFiles },

		{ "strUIName",  WStringToString(d.strUIName) },
		{ "iUIType", d.iUIType },
		{ "strParentName", WStringToString(d.strParentName) },

		{ "isInverseScreenDiscard", d.isInverseScreenDiscard },
		{ "fCutout", d.fCutout },
		{ "iPassType", d.iPassType },

		{ "vecChildNames", childNames },
		{ "isInstance", d.isInstance },
		{ "vecInstanceDescs", vecInstanceDescs },

		// descs (size, sector)
		{ "vecSize", vecImgSizes },
		{ "vSectorBorder", {
			d.vSectorBorder.x,
			d.vSectorBorder.y
			}
		},
		{ "fUIScale", d.fUIScale },
	};
}

inline void from_json(const json& j, CCustom_UI::CUSTOM_UI_DESC& d)
{
	_string strFilePath		= j["strFilePath"].get<_string>();
	d.strFilePath			= StringToWString(strFilePath);
	_string strFileName		= j["strFileName"].get<_string>();
	d.strFileName			= StringToWString(strFileName);
	d.iNumFiles				= j["iNumFiles"];

	_string strUIName		= j["strUIName"].get<_string>();
	d.strUIName				= StringToWString(strUIName);
	d.iUIType				= j["iUIType"];
	_string strParentName	= j["strParentName"].get<_string>();
	d.strParentName			= StringToWString(strParentName);


	d.isInverseScreenDiscard= j["isInverseScreenDiscard"];
	d.fCutout				= j["fCutout"];
	d.iPassType				= j["iPassType"];

	for (const auto& element : j["vecChildNames"])
	{
		_string strChildName = element.get<_string>();
		d.vecChildNames.push_back(StringToWString(strChildName));
	}
	d.isInstance = j["isInstance"];

	for (const auto& element : j["vecInstanceDescs"])
	{
		Editor::CVIBuffer_Rect_Instance_UI::SINGLE_INST_DESC tDesc = {};
		from_json(element, tDesc);
		d.vecInstanceDescs.push_back(tDesc);
	}

	// descs
	for (const auto& element : j["vecSize"])
		d.vecSize.push_back(_float2{ element[0], element[1] });
	d.vSectorBorder			= _float2( j["vSectorBorder"][0], j["vSectorBorder"][1] );
	d.fUIScale				= j["fUIScale"];
}

inline void to_json(json& j, const vector<CCustom_UI::CUSTOM_UI_DESC>& vec)
{
	j = json::array();
	for (const auto& v : vec)
	{
		json data = {};
		to_json(data, v);
		j.push_back(data);
	}
}

inline void from_json(const json& j, vector<CCustom_UI::CUSTOM_UI_DESC>& vec)
{
	vec.clear();
	vec.reserve(j.size());

	for (const auto& element : j)
	{
		CCustom_UI::CUSTOM_UI_DESC desc = {};
		from_json(element, desc);
		vec.push_back(desc);
	}
}


#pragma endregion
