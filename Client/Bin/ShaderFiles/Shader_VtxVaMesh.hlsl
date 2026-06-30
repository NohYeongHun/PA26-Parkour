#include "Engine_Shader_Defines.hlsli"

matrix g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;

Texture2D g_VatTexture;
Texture2D g_DiffuseTexture;

float g_fTrackPosition;
float g_fAnimationDuration;
float g_fMovementScale;

struct VS_IN
{
    float3 vPosition : POSITION;
    float3 vNormal : NORMAL;
    float3 vTangent : TANGENT;
    float3 vBinormal : BINORMAL;
    float2 vTexcoord : TEXCOORD0;
    float2 vVATcoord : TEXCOORD1;
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
    
    float fTexelSize = 1.f / g_fAnimationDuration;
    
    float fVatCoorX = In.vVATcoord.x;
    float fVatCoordY = (g_fTrackPosition / g_fAnimationDuration);
    
    float4 vMovement = 0.f;
  
    float2 vVatCoord = float2(fVatCoorX, fVatCoordY);
//    float2 vVatCoord = float2(In.vVATcoord.x, fVatCoordY);

    int iCurentTexelCount = (int) (fVatCoordY / fTexelSize);
    if ((iCurentTexelCount + 1) >= (int) g_fAnimationDuration)
    {
        float2 vVatCoord = float2(In.vVATcoord.x, fVatCoordY);
        
        vMovement = g_VatTexture.SampleLevel(DefaultSampler, vVatCoord, 0);
    }
    else
    {
        int iNextTexelCount = iCurentTexelCount + 1;
        float fStartY = fTexelSize * iCurentTexelCount;
        float fEndY = fTexelSize * iNextTexelCount;
        
        float4 vStartMovement = g_VatTexture.SampleLevel(DefaultSampler, float2(In.vVATcoord.x, fStartY), 0);
        float4 vNextMovement = g_VatTexture.SampleLevel(DefaultSampler, float2(In.vVATcoord.x, fEndY), 0);
        
        float fRatio = 1.f - ((fEndY - fVatCoordY) / fTexelSize);
        vMovement = lerp(vStartMovement, vNextMovement, fRatio);
    }
    
    vMovement *= g_fMovementScale;
    
    float3 vPosition = In.vPosition + vMovement.xyz;
    
    matrix matWV, matWVP;
    
    matWV = mul(g_WorldMatrix, g_ViewMatrix);
    matWVP = mul(matWV, g_ProjMatrix);
    Out.vPosition = mul(float4(vPosition, 1.f), matWVP);
    Out.vNormal = normalize(mul(float4(In.vNormal, 0.f), g_WorldMatrix));
    Out.vTangent = normalize(mul(float4(In.vTangent, 0.f), g_WorldMatrix));
    Out.vBinormal = normalize(mul(float4(In.vBinormal, 0.f), g_WorldMatrix));
    Out.vTexcoord = In.vTexcoord;
    Out.vProjPos = mul(float4(vPosition, 1.f), matWVP);

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

struct PS_OUT
{
    float4 vDiffuse : SV_TARGET0;
    float4 vNormal : SV_TARGET1;
    float4 vDepth : SV_TARGET2;
    float4 vEmissive : SV_TARGET3;
    float4 vDistortion : SV_TARGET4;
    float4 vPBR : SV_TARGET5;
    float4 vSSS : SV_TARGET6;
};

PS_OUT PS_VA(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;
    
    float4 vDiffuse = g_DiffuseTexture.Sample(DefaultSampler, In.vTexcoord);
    
    Out.vDiffuse = vDiffuse;

    Out.vNormal = In.vNormal;
    Out.vDepth.x = In.vProjPos.z / In.vProjPos.w;
    Out.vDepth.y = In.vProjPos.w;
   
    return Out;
}

technique11 DefaultTechnique
{
    pass VA // 0
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_VA();
    }
}
