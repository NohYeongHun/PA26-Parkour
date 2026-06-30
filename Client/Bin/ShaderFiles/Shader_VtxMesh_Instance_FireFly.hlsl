#include "Engine_Shader_Defines.hlsli"


matrix g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;

Texture2D g_DiffuseTexture[2];
Texture2D g_NormalTexture[2];
Texture2D g_MaskDiffuseTexture;
Texture2D g_MetallicTexture;
vector g_vMatrlAmbient = vector(1.0f, 1.0f, 1.0f, 1.0f);
vector g_vMatrlSpecular = vector(0.4f, 0.4f, 0.4f, 0.4f);

//Texture2D   g_MaskTexture[4] : register(t8);
Texture2D g_MaskTexture[4];

matrix g_ShadowViewMatrix[4];
matrix g_ShadowProjMatrix[4];

matrix g_ShadowMapViewMatrix;
matrix g_ShadowMapProjMatrix;

float g_fOutLineRadius = 0.0005f;
float g_fOutLineRadiusZ = 0.0005f;

bool g_HasNormal = false;
bool g_HasMask = false;
bool g_HasMetallic = false;
bool g_IsDynamicObject = false;
int g_iIndex = 0;

int g_iShadowMapLayer = 0;
float4 g_vDiffuseColor = float4(0.f, 0.f, 0.f, 0.f);
float g_fRaidan;
struct VS_IN
{
    
    float3 vPosition : POSITION;
    float3 vNormal : NORMAL;
    float3 vTangent : TANGENT;
    float3 vBinormal : BINORMAL;
    float2 vTexcoord : TEXCOORD0;
    
    row_major float4x4 TransformMatrix : WORLD;
    float3 vPerMove : TEXCOORD1;
    float  vRange    : TEXCOORD2;
};

struct VS_OUT
{
    float4 vPosition : SV_POSITION;
    float4 vNormal : NORMAL;
    float4 vTangent : TANGENT;
    float4 vBinormal : BINORMAL;
    float2 vTexcoord : TEXCOORD0;
    float4 vProjPos : TEXCOORD1;
};

VS_OUT VS_MAIN(VS_IN In)
{
    VS_OUT Out = (VS_OUT) 0;
    
    float4 vWorldPos = mul(float4(In.vPosition, 1.f), In.TransformMatrix);
    
    matrix matVP = mul(g_ViewMatrix, g_ProjMatrix);
    float xOffset = sin(g_fRaidan * In.vPerMove.x) * In.vRange;
    float yOffset = cos(g_fRaidan * In.vPerMove.y) * In.vRange;
    float zOffset = sin(g_fRaidan * In.vPerMove.z) * In.vRange;

    vWorldPos.xyz = vWorldPos.xyz + float3(xOffset, yOffset, zOffset);
    Out.vPosition = mul(vWorldPos, matVP);

    Out.vNormal = normalize(mul(float4(In.vNormal, 0.f), In.TransformMatrix));
    Out.vTangent = normalize(mul(float4(In.vTangent, 0.f), In.TransformMatrix));
    Out.vBinormal = normalize(mul(float4(In.vBinormal, 0.f), In.TransformMatrix));
    Out.vTexcoord = In.vTexcoord;
    Out.vProjPos = Out.vPosition;
    return Out;
}

VS_OUT VS_NONSHAKE(VS_IN In)
{
    VS_OUT Out = (VS_OUT) 0;
    
    matrix matVP;
    matVP = mul(g_ViewMatrix, g_ProjMatrix);
    float4 vPos = mul(float4(In.vPosition, 1.f), In.TransformMatrix);
 
    Out.vPosition = mul(vPos, matVP);
    Out.vNormal = normalize(mul(float4(In.vNormal, 0.f), In.TransformMatrix));
    Out.vTangent = normalize(mul(float4(In.vTangent, 0.f), In.TransformMatrix));
    Out.vBinormal = normalize(mul(float4(In.vBinormal, 0.f), In.TransformMatrix));
    Out.vTexcoord = In.vTexcoord;
    Out.vProjPos = mul(vPos, matVP);

    return Out;
}


struct VS_OUT_SHADOW
{
    float4 vPosition : SV_POSITION;
    float2 vTexcoord : TEXCOORD0;
    float4 vProjPos : TEXCOORD1;
};

VS_OUT_SHADOW VS_SHADOW(VS_IN In)
{
    VS_OUT_SHADOW Out = (VS_OUT_SHADOW) 0;
    
    matrix matWV, matWVP;
    
    matWV = mul(g_WorldMatrix, g_ViewMatrix);
    matWVP = mul(matWV, g_ProjMatrix);
    
    Out.vPosition = mul(float4(In.vPosition, 1.f), matWVP);
    Out.vTexcoord = In.vTexcoord;
    Out.vProjPos = Out.vPosition;
    
    return Out;
}

struct PS_IN
{
    float4 vPosition : SV_POSITION;
    float4 vNormal : NORMAL;
    float4 vTangent : TANGENT;
    float4 vBinormal : BINORMAL;
    float2 vTexcoord : TEXCOORD0;
    float4 vProjPos : TEXCOORD1;
};

struct PS_OUT_LIGHT
{
    float4 vDiffuse : SV_TARGET0;
    float4 vNormal : SV_TARGET1;
    float4 vDepth : SV_TARGET2;
    float4 vEmissive : SV_TARGET3;
    float4 vDistortion : SV_TARGET4;
    float4 vPBR : SV_TARGET5;
    float4 vSSS : SV_TARGET6;
};

struct PS_OUT_EMISSIVE
{
    float4 vDiffuse : SV_TARGET0;
    float4 vNormal : SV_TARGET1;
    float4 vSpecular : SV_TARGET3;
    float4 vAmbient : SV_TARGET4;
};

PS_OUT_LIGHT PS_MAIN_NORMAL(PS_IN In)
{
    PS_OUT_LIGHT Out = (PS_OUT_LIGHT) 0;
    
    vector vMask = g_MaskTexture[0].Sample(DefaultSampler, In.vTexcoord);
    
    vector vDiffuse = g_DiffuseTexture[0].Sample(DefaultSampler, In.vTexcoord);
    vector vMaskDiffiuse = g_DiffuseTexture[1].Sample(DefaultSampler, In.vTexcoord);

    if (g_HasMask)
    {
        Out.vDiffuse = vDiffuse * vMask.r + vDiffuse * (1.f - vMask.r);
        Out.vDiffuse = Out.vDiffuse * vMask.g + vMaskDiffiuse * (1.f - vMask.g);

    }
    else
    {
        Out.vDiffuse = vDiffuse;
    }
    if (Out.vDiffuse.a < 0.1f)
        discard;
    Out.vEmissive = float4(0.75f, 1.0f, 0.2f, 1.f);
    
    Out.vDiffuse.w = 1.f;
    
    Out.vPBR.y = g_fGlobalStaticRoughness;
    Out.vPBR.x = g_fGlobalStaticMetallic;
    
    if (g_IsDynamicObject)
    {
        Out.vDepth.z = 1.f;
        Out.vPBR.z = 1.f;
    }
    
    float4 vNormal;
    
    if (g_HasNormal)
    {
        if (g_HasMask)
        {
            vector vDefaultNormal = g_NormalTexture[0].Sample(DefaultSampler, In.vTexcoord);
        
            float4 vNormal1 = normalize(vDefaultNormal * 2.f - 1.f);
            if (vDefaultNormal.x > vDefaultNormal.z && vDefaultNormal.y > vDefaultNormal.z)
                vNormal1.z = sqrt(1.f - saturate(dot(vDefaultNormal.xy, vDefaultNormal.xy)));

            //vector vMaskNormal = g_NormalTexture[1].Sample(DefaultSampler, In.vTexcoord);
        
            //float4 vNormal2 = normalize(vMaskNormal * 2.f - 1.f);
            //if (vMaskNormal.x > vMaskNormal.z && vMaskNormal.y > vMaskNormal.z)
            //    vNormal2.z = sqrt(1.f - saturate(dot(vMaskNormal.xy, vMaskNormal.xy)));

            //if (vMask.r == 0.f && vMask.g == 0.f)
            //    vNormal = vNormal1;
            //else
            //{
            //vNormal = vNormal1 * (vMask.r) + vNormal1 * (1.f - vMask.r);
            //vNormal = vNormal * vMask.g + vNormal2 * (1.f - vMask.g);
            //}
            
            vNormal = vNormal1 * (vMask.r) + vNormal1 * (1.f - vMask.r);
        }
        else
        {
            vector vDefaultNormal = g_NormalTexture[0].Sample(DefaultSampler, In.vTexcoord);
			
            Out.vPBR.x = vDefaultNormal.b;
            Out.vPBR.y = vDefaultNormal.a;
	        
            vNormal = normalize(vDefaultNormal * 2.f - 1.f);
            if (vDefaultNormal.x > vDefaultNormal.z && vDefaultNormal.y > vDefaultNormal.z)
                vNormal.z = sqrt(1.f - saturate(dot(vDefaultNormal.xy, vDefaultNormal.xy)));
        }
        //vector vNormalDesc = vDefaultNormal * (1.f - vMask.r) + vMaskNormal * vMask.g;
        
        //vNormal = normalize(vNormalDesc * 2.f - 1.f);
        //if (vNormalDesc.x > vNormalDesc.z && vNormalDesc.y > vNormalDesc.z)
        //    vNormal.z = sqrt(1.f - saturate(dot(vNormalDesc.xy, vNormalDesc.xy)));

        //vNormal = vNormal1;  

        float3 vTangent = In.vTangent.xyz;
        float3 vBinormal = In.vBinormal.xyz * -1.f;
        float3 vInNormal = In.vNormal.xyz;

        float3x3 WorldMatrix;
        WorldMatrix = float3x3(vTangent, vBinormal, vInNormal);
        
        vNormal.xyz = normalize(mul(vNormal.xyz, WorldMatrix));
        vNormal.xyz = vNormal * 0.5f + 0.5f;
    }
    else
    {
        vNormal = In.vNormal;
        vNormal = vNormal * 0.5f + 0.5f;
    }
    Out.vNormal = float4(vNormal.xyz, 1.f);
    Out.vDepth.x = In.vProjPos.z / In.vProjPos.w;
    Out.vDepth.y = In.vProjPos.w;
    
    Out.vSSS.z = In.vProjPos.z / In.vProjPos.w;
    Out.vSSS.w = In.vProjPos.w;
    
    Out.vDepth.w = 1.f;
    
    return Out;
}

technique11 DefaultTechnique
{
    pass DefaultPass // 0
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_NORMAL();
    }
}