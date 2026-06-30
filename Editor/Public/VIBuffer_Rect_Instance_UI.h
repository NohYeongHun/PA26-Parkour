#pragma once
#include "VIBuffer_Instance.h"

NS_BEGIN(Editor)

class CVIBuffer_Rect_Instance_UI final : public CVIBuffer_Instance
{
#pragma region structs
public:
	typedef struct tagRectSingleInstanceDesc
	{
		// ���� �ν��Ͻ����� �ο��� ����
		_float4 vSInstRight		= { 1.f, 0.f, 0.f ,0.f };					// ���� ��ü�� Pivot �� ���� �����ǥ
		_float4 vSInstUp		= { 0.f, 1.f, 0.f ,0.f };
		_float4 vSInstLook		= { 0.f, 0.f, 1.f ,0.f };
		_float4 vSInstTrans		= { 0.f, 0.f, 0.f ,1.f };
		_float2 vSInstCoordX	= { 0.f, 1.f} ;
		_float2 vSInstCoordY	= { 0.f, 1.f} ;
		_float2 vClipTexcoordX	= { 0.f, 1.f };						// based on local space, per single instance
		_float2 vClipTexcoordY	= { 0.f, 1.f };						// based on local space, per single instance

		_float4x4 matExtraData	= {};
	}SINGLE_INST_DESC;

	typedef struct tagRectInstanceUIDesc : public CVIBuffer_Instance::INSTANCE_DESC
	{
		//_float3 vPivot = {};

	}RECT_INSTANCE_UI_DESC;
#pragma endregion

private:
	explicit CVIBuffer_Rect_Instance_UI(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CVIBuffer_Rect_Instance_UI(const CVIBuffer_Rect_Instance_UI& Prototype);
	virtual ~CVIBuffer_Rect_Instance_UI() = default;

public:
	virtual HRESULT Initialize_Prototype(const INSTANCE_DESC* pDesc) override;
	virtual HRESULT Initialize_Clone(void* pArg) override;
	// virtual HRESULT Bind_Resources() override;
	virtual HRESULT Render() override;

public:
	void Update_Instances(_float fTimeDelta, vector<SINGLE_INST_DESC>& vecDescs);

private:
	_float3					m_vPivot = {};
	//_float*				m_pSpeeds = {};
	//_bool					m_isLoop = {};

	SINGLE_INST_DESC*		m_pInstanceDesc = {};
	_uint					m_iNumAvailableInstance = 0;

public:
	static CVIBuffer_Rect_Instance_UI* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const INSTANCE_DESC* pDesc);
	virtual CComponent* Clone(void* pArg) override;
	virtual void Free() override;
};

NS_END



#pragma region json

inline void to_json(json& j, const Editor::CVIBuffer_Rect_Instance_UI::SINGLE_INST_DESC& d)
{
	j = json{
		{ "vSInstRight"		, {d.vSInstRight.x,		d.vSInstRight.y,	d.vSInstRight.z,	d.vSInstRight.w	} },
		{ "vSInstUp"		, {d.vSInstUp.x,		d.vSInstUp.y,		d.vSInstUp.z,		d.vSInstUp.w	} },
		{ "vSInstLook"		, {d.vSInstLook.x,		d.vSInstLook.y,		d.vSInstLook.z,		d.vSInstLook.w	} },
		{ "vSInstTrans"		, {d.vSInstTrans.x,		d.vSInstTrans.y,	d.vSInstTrans.z,	d.vSInstTrans.w	} },
		{ "vSInstCoordX"	, {d.vSInstCoordX.x,	d.vSInstCoordX.y} },
		{ "vSInstCoordY"	, {d.vSInstCoordY.x,	d.vSInstCoordY.y} },
		{ "vClipTexcoordX"	, {d.vClipTexcoordX.x,	d.vClipTexcoordX.y} },
		{ "vClipTexcoordY"	, {d.vClipTexcoordY.x,	d.vClipTexcoordY.y} }
	};
}

inline void from_json(const json& j, Editor::CVIBuffer_Rect_Instance_UI::SINGLE_INST_DESC& d)
{
	d.vSInstRight		= { j["vSInstRight"][0],	j["vSInstRight"][1],	j["vSInstRight"][2],	j["vSInstRight"][3]	};
	d.vSInstUp			= { j["vSInstUp"][0],		j["vSInstUp"][1],		j["vSInstUp"][2],		j["vSInstUp"][3]	};
	d.vSInstLook		= { j["vSInstLook"][0],		j["vSInstLook"][1],		j["vSInstLook"][2],		j["vSInstLook"][3]	};
	d.vSInstTrans		= { j["vSInstTrans"][0],	j["vSInstTrans"][1],	j["vSInstTrans"][2],	j["vSInstTrans"][3]	};
	d.vSInstCoordX		= { j["vSInstCoordX"][0],	j["vSInstCoordX"][1]	};
	d.vSInstCoordY		= { j["vSInstCoordY"][0],	j["vSInstCoordY"][1]	};
	d.vClipTexcoordX	= { j["vClipTexcoordX"][0], j["vClipTexcoordX"][1]  };
	d.vClipTexcoordY	= { j["vClipTexcoordY"][0], j["vClipTexcoordY"][1]  };
}

#pragma endregion
