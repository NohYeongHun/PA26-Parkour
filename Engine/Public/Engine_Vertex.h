#ifndef Engine_Vertex_h__
#define Engine_Vertex_h__

#include "Engine_Typedef.h"

namespace Engine
{
	typedef struct tagVertexPosition
	{
		_float3		vPosition;

		static const _uint iNumElements = { 1 };
		static constexpr D3D11_INPUT_ELEMENT_DESC Elements[] = {
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0}
		};
	}VTXPOS;

	typedef struct tagVertexPositionColor
	{
		_float3		vPosition;
		_float4		vColor;

		static const _uint iNumElements = { 2 };
		static constexpr D3D11_INPUT_ELEMENT_DESC Elements[] = {
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
		};
	}VTXPOSCOL;

	typedef struct tagVertexPositionTexture
	{
		_float3		vPosition;
		_float2		vTexcoord;

		static const _uint iNumElements = { 2 };
		static constexpr D3D11_INPUT_ELEMENT_DESC Elements[] = {
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
		};
	}VTXPOSTEX;

	typedef struct tagVertexTrail
	{
		_float3		vPosition;
		_float2		vTexcoord;
		_float2		vLifeTime;

		static const _uint iNumElements = { 3 };
		static constexpr D3D11_INPUT_ELEMENT_DESC Elements[] = {
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{ "TEXCOORD", 1, DXGI_FORMAT_R32G32_FLOAT, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0},
		};
	}VTXTRAIL;

	typedef struct tagVertexNormalTexture
	{
		_float3		vPosition;
		_float3		vNormal;
		_float2		vTexcoord;

		static const _uint iNumElements = { 3 };
		static constexpr D3D11_INPUT_ELEMENT_DESC Elements[] = {
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0},
		};
	}VTXNORTEX;

	typedef struct tagVertexMesh
	{
		_float3		vPosition;
		_float3		vNormal;
		_float3		vTangent;
		_float3		vBinormal;
		_float2		vTexcoord;

		static const _uint iNumElements = { 5 };
		static constexpr D3D11_INPUT_ELEMENT_DESC Elements[] = {
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{ "BINORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 36, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 48, D3D11_INPUT_PER_VERTEX_DATA, 0},
		};
	}VTXMESH;

	typedef struct tagVAMesh
	{
		_float3 vPosition;
		_float3 vNormal;
		_float3 vTangent;
		_float3 vBinormal;
		_float2 vTexcoord;
		_float2 vVATcoord;

		static const _uint iNumElements = { 6 };
		static constexpr D3D11_INPUT_ELEMENT_DESC Elements[] = {
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{ "BINORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 36, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 48, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{ "TEXCOORD", 1, DXGI_FORMAT_R32G32_FLOAT, 0, 56, D3D11_INPUT_PER_VERTEX_DATA, 0}
		};
	}VTX_VAMESH;


	typedef struct tagVertexAnimMesh
	{
		_float3		vPosition;
		_float3		vNormal;
		_float3		vTangent;
		_float3		vBinormal;
		_uint4		vBlendIndex;
		_float4		vBlendWeight;
		_float2		vTexcoord;

		static const _uint iNumElements = { 7 };
		static constexpr D3D11_INPUT_ELEMENT_DESC Elements[] = {
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{ "BINORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 36, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{ "BLENDINDEX", 0, DXGI_FORMAT_R32G32B32A32_UINT, 0, 48, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{ "BLENDWEIGHT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 64, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 80, D3D11_INPUT_PER_VERTEX_DATA, 0},
		};
	}VTXANIMMESH;

	typedef struct tagVertexInstanceMesh
	{
		_float4		vRight;
		_float4		vUp;
		_float4		vLook;
		_float4		vTranslation;
	}VTXINSTANCE_MESH;

	typedef struct tagVertexInstanceMeshFireFly
	{
		_float4		vRight;
		_float4		vUp;
		_float4		vLook;
		_float4		vTranslation;

		_float3     vPerMove;
		_float		vRange;
	}VTXINSTANCE_MESH_FIREFLY;

	typedef struct tagVertexInstanceFXMesh
	{
		_float4		vRight;
		_float4		vUp;
		_float4		vLook;
		_float4		vTranslation;
		_float2		vLifeTime;
	}VTXINSTACNE_FXMESH;

	typedef struct tagVertexInstanceParticle
	{
		_float4		vRight;
		_float4		vUp;
		_float4		vLook;
		_float4		vTranslation;

		_float2		vLifeTime;
		_float2		fDelay = { 0.f, 0.f };

		_float4		vVelTail = { 0.f, 0.f, 0.f, 0.f};			//x,y,z = Vel / w = TailLen

		_float		fPhase = 0.f;
		_float		_pad0[3];
	}VTXINSTANCE_PARTICLE;

	typedef struct tagVertexParticle
	{
		static const _uint iNumElements = { 7 };
		static constexpr D3D11_INPUT_ELEMENT_DESC Elements[] = {
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
			
			{ "TEXCOORD", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1},
			{ "TEXCOORD", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 16, D3D11_INPUT_PER_INSTANCE_DATA, 1},
			{ "TEXCOORD", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 32, D3D11_INPUT_PER_INSTANCE_DATA, 1},
			{ "TEXCOORD", 4, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 48, D3D11_INPUT_PER_INSTANCE_DATA, 1},
			{ "TEXCOORD", 5, DXGI_FORMAT_R32G32_FLOAT, 1, 64, D3D11_INPUT_PER_INSTANCE_DATA, 1},
		};
	}VTXPARTICLE;

	typedef struct tagVertexPointParticle
	{
		static const _uint iNumElements = { 9 };
		static constexpr D3D11_INPUT_ELEMENT_DESC Elements[] = {
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},

			{ "WORLD", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1},
			{ "WORLD", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 16, D3D11_INPUT_PER_INSTANCE_DATA, 1},
			{ "WORLD", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 32, D3D11_INPUT_PER_INSTANCE_DATA, 1},
			{ "WORLD", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 48, D3D11_INPUT_PER_INSTANCE_DATA, 1},
			
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 1, 64, D3D11_INPUT_PER_INSTANCE_DATA, 1},				//vLifeTime
			{ "TEXCOORD", 1, DXGI_FORMAT_R32G32_FLOAT, 1, 72, D3D11_INPUT_PER_INSTANCE_DATA, 1 },				//Delay

			{ "TEXCOORD", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 80, D3D11_INPUT_PER_INSTANCE_DATA, 1 },			//vVelTail

			{ "TEXCOORD", 3, DXGI_FORMAT_R32_FLOAT, 1, 96, D3D11_INPUT_PER_INSTANCE_DATA, 1 },					//fPhase
		};
	}VTXPOINTPARTICLE;

	typedef struct tagVertexMeshInstance
	{
		static const _uint iNumElements = { 9 };
		static constexpr D3D11_INPUT_ELEMENT_DESC Elements[] = {
			{ "POSITION",	0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0},
			{ "NORMAL",		0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{ "TANGENT",	0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{ "BINORMAL",	0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 36, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{ "TEXCOORD",	0, DXGI_FORMAT_R32G32_FLOAT,	0, 48, D3D11_INPUT_PER_VERTEX_DATA, 0},

			{ "WORLD", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0,  D3D11_INPUT_PER_INSTANCE_DATA, 1},
			{ "WORLD", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 16, D3D11_INPUT_PER_INSTANCE_DATA, 1},
			{ "WORLD", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 32, D3D11_INPUT_PER_INSTANCE_DATA, 1},
			{ "WORLD", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 48, D3D11_INPUT_PER_INSTANCE_DATA, 1},
		};
	}VTXMESHINSTANCE;

	typedef struct tagVertexMeshInstance_FireFly
	{
		static const _uint iNumElements = { 11 };
		static constexpr D3D11_INPUT_ELEMENT_DESC Elements[] = {
			{ "POSITION",	0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0},
			{ "NORMAL",		0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{ "TANGENT",	0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{ "BINORMAL",	0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 36, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{ "TEXCOORD",	0, DXGI_FORMAT_R32G32_FLOAT,	0, 48, D3D11_INPUT_PER_VERTEX_DATA, 0},

			{ "WORLD",  0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0,  D3D11_INPUT_PER_INSTANCE_DATA, 1},
			{ "WORLD",  1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 16, D3D11_INPUT_PER_INSTANCE_DATA, 1},
			{ "WORLD",  2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 32, D3D11_INPUT_PER_INSTANCE_DATA, 1},
			{ "WORLD",  3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 48, D3D11_INPUT_PER_INSTANCE_DATA, 1},
			{"TEXCOORD",1, DXGI_FORMAT_R32G32B32_FLOAT,    1, 64, D3D11_INPUT_PER_INSTANCE_DATA, 1},
			{"TEXCOORD",2, DXGI_FORMAT_R32_FLOAT,          1, 76, D3D11_INPUT_PER_INSTANCE_DATA, 1},
		};
	}VTXMESHINSTANCE_FIREFLY;

	typedef struct tagVertexInstanceDecal
	{
		_float4		vRight;
		_float4		vUp;
		_float4		vLook;
		_float4		vTranslation;

		_float4		vRightInv;
		_float4		vUpInv;
		_float4		vLookInv;
		_float4		vTranslationInv;

		_float		fAlpha;
		_float4		vColor;

		_float		fEmissiveIntensity;
	}VTXINSTANCE_DECAL;

	typedef struct tagVertexDecal
	{
		static const _uint iNumElements = { 12 };
		static constexpr D3D11_INPUT_ELEMENT_DESC Elements[] = {
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },

			{ "WORLD", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "WORLD", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 16, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "WORLD", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 32, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "WORLD", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 48, D3D11_INPUT_PER_INSTANCE_DATA, 1 },

			{ "INVWORLD", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 64, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "INVWORLD", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 80, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "INVWORLD", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 96, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "INVWORLD", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 112, D3D11_INPUT_PER_INSTANCE_DATA, 1 },

			{ "TEXCOORD", 0, DXGI_FORMAT_R32_FLOAT, 1, 128, D3D11_INPUT_PER_INSTANCE_DATA, 1 },				//LifeTime
			{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 132, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "TEXCOORD", 1, DXGI_FORMAT_R32_FLOAT, 1, 148,D3D11_INPUT_PER_INSTANCE_DATA, 1 }
		};
	}VTX_DECAL;

	typedef struct tagVertexInstanceRect
	{
		_float4		vRight;
		_float4		vUp;
		_float4		vLook;
		_float4		vTranslation;
	}VTXINSTANCE_RECT;

	typedef struct tagVertexRectInstance
	{
		static const _uint iNumElements = { 6 };

		static constexpr D3D11_INPUT_ELEMENT_DESC Elements[] = {
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},

			{ "WORLD", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "WORLD", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 16, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "WORLD", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 32, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "WORLD", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 48, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		};
	}VTXRECTINSTANCE;

typedef struct tagVertexInstanceAnimMesh
	{
		_float4		vRight;
		_float4		vUp;
		_float4		vLook;
		_float4		vTranslation;
		_uint		iBaseIndex;		//인스턴스의 뼈 팔레트 시작 패딩 인덱스
		//_uint		iTexIndex;		//임시: 텍스처 매핑 인덱스(얼굴)
	}VTXINSTANCE_ANIMMESH;

	typedef struct tagVertexAnimMeshInstance
	{
		static const _uint iNumElements = { 12 };
		static constexpr D3D11_INPUT_ELEMENT_DESC Elements[] = {
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{ "BINORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 36, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{ "BLENDINDEX", 0, DXGI_FORMAT_R32G32B32A32_UINT, 0, 48, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{ "BLENDWEIGHT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 64, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 80, D3D11_INPUT_PER_VERTEX_DATA, 0},

			{ "WORLD", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "WORLD", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 16, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "WORLD", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 32, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "WORLD", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 48, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "INSTANCEID", 0, DXGI_FORMAT_R32_UINT, 1, 64, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		};
	}VTXANIMMESH_INSTANCE;
	
	// ==============================
	// * for UI Instancing
	// ==============================
	typedef struct tagVertexUI
	{
		_float3		vPosition;
		_float2		vTexcoord;

		_float4		vSInstRight;
		_float4		vSInstUp;
		_float4		vSInstLook;
		_float4		vSInstTrans;

		_float2		vSInstCoordX;
		_float2		vSInstCoordY;
		_float2		vClipTexcoordX;
		_float2		vClipTexcoordY;

		_float4x4	mExtraData;

		static const _uint iNumElements = { 14 };
		static constexpr D3D11_INPUT_ELEMENT_DESC Elements[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,		0, 0,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,			0, 12,	D3D11_INPUT_PER_VERTEX_DATA, 0 },

			{ "TEXCOORD", 1, DXGI_FORMAT_R32G32B32A32_FLOAT,	1, 0,	D3D11_INPUT_PER_INSTANCE_DATA, 1 },	// vSInstRight
			{ "TEXCOORD", 2, DXGI_FORMAT_R32G32B32A32_FLOAT,	1, 16,	D3D11_INPUT_PER_INSTANCE_DATA, 1 },	// vSInstUp
			{ "TEXCOORD", 3, DXGI_FORMAT_R32G32B32A32_FLOAT,	1, 32,	D3D11_INPUT_PER_INSTANCE_DATA, 1 },	// vSInstLook
			{ "TEXCOORD", 4, DXGI_FORMAT_R32G32B32A32_FLOAT,	1, 48,	D3D11_INPUT_PER_INSTANCE_DATA, 1 },	// vSInstTrans

			{ "TEXCOORD", 5, DXGI_FORMAT_R32G32_FLOAT,			1, 64,	D3D11_INPUT_PER_INSTANCE_DATA, 1 },	// vTexcoordX
			{ "TEXCOORD", 6, DXGI_FORMAT_R32G32_FLOAT,			1, 72,	D3D11_INPUT_PER_INSTANCE_DATA, 1 },	// vTexcoordY
			{ "TEXCOORD", 7, DXGI_FORMAT_R32G32_FLOAT,			1, 80,	D3D11_INPUT_PER_INSTANCE_DATA, 1 },	// vClipTexcoordX
			{ "TEXCOORD", 8, DXGI_FORMAT_R32G32_FLOAT,			1, 88,	D3D11_INPUT_PER_INSTANCE_DATA, 1 },	// vClipTexcoordY

			{ "TEXCOORD", 9,  DXGI_FORMAT_R32G32B32A32_FLOAT,	1, 96,	D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "TEXCOORD", 10, DXGI_FORMAT_R32G32B32A32_FLOAT,	1, 112,	D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "TEXCOORD", 11, DXGI_FORMAT_R32G32B32A32_FLOAT,	1, 128,	D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "TEXCOORD", 12, DXGI_FORMAT_R32G32B32A32_FLOAT,	1, 144,	D3D11_INPUT_PER_INSTANCE_DATA, 1 }
		};
	}VTXUIINSTANCE;

	typedef struct tagVertexUIText
	{
		_float2		vPosition;
		_float2		vTexcoord;

		static const _uint iNumElements = { 2 };
		static constexpr D3D11_INPUT_ELEMENT_DESC Elements[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,		0, 0,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,			0, 8,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};
	}VTXUITEXT;

	//typedef struct tagVertexCurveTrace
	//{
	//	_float3		vPosition;
	//	_float		fWidth;;
	//	_float		fCurve;
	//
	//	static const _uint iNumElements = { 3 };
	//	static constexpr D3D11_INPUT_ELEMENT_DESC Elements[] =
	//	{
	//		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,		0, 0,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
	//		{ "TEXCOORD", 0, DXGI_FORMAT_R32_FLOAT,				0, 12,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
	//		{ "TEXCOORD", 1, DXGI_FORMAT_R32_FLOAT,				0, 16,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
	//	};
	//}VTXUICURVE;

	typedef struct tagVertexCurveTrace
	{
		_float3 vPosition;
		_float  fWidth;   
		_float  fCurve;   
		_float3 vPadding; // padding

		static const _uint iNumElements = 3;
		static constexpr D3D11_INPUT_ELEMENT_DESC Elements[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32_FLOAT,       0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 1, DXGI_FORMAT_R32_FLOAT,       0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};
	} VTXUICURVE;


	// ==============================

	typedef struct tagVertexFXMeshInstance
	{
		static const _uint iNumElements = { 10 };
		static constexpr D3D11_INPUT_ELEMENT_DESC Elements[] = {
			{ "POSITION",	0, DXGI_FORMAT_R32G32B32_FLOAT,		0, 0,  D3D11_INPUT_PER_VERTEX_DATA,		0},
			{ "NORMAL",		0, DXGI_FORMAT_R32G32B32_FLOAT,		0, 12, D3D11_INPUT_PER_VERTEX_DATA,		0},
			{ "TANGENT",	0, DXGI_FORMAT_R32G32B32_FLOAT,		0, 24, D3D11_INPUT_PER_VERTEX_DATA,		0},
			{ "BINORMAL",	0, DXGI_FORMAT_R32G32B32_FLOAT,		0, 36, D3D11_INPUT_PER_VERTEX_DATA,		0},
			{ "TEXCOORD",	0, DXGI_FORMAT_R32G32_FLOAT,		0, 48, D3D11_INPUT_PER_VERTEX_DATA,		0},

			{ "WORLD",		0, DXGI_FORMAT_R32G32B32A32_FLOAT,	1, 0,  D3D11_INPUT_PER_INSTANCE_DATA,	1},
			{ "WORLD",		1, DXGI_FORMAT_R32G32B32A32_FLOAT,	1, 16, D3D11_INPUT_PER_INSTANCE_DATA,	1},
			{ "WORLD",		2, DXGI_FORMAT_R32G32B32A32_FLOAT,	1, 32, D3D11_INPUT_PER_INSTANCE_DATA,	1},
			{ "WORLD",		3, DXGI_FORMAT_R32G32B32A32_FLOAT,	1, 48, D3D11_INPUT_PER_INSTANCE_DATA,	1},

			{ "TEXCOORD",	1, DXGI_FORMAT_R32G32_FLOAT,		1, 64, D3D11_INPUT_PER_INSTANCE_DATA, 1},
		};
	}VTXFXMESHINSTANCE;
}


#endif // Engine_Vertex_h__
