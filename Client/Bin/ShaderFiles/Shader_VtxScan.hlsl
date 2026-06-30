#include "Engine_Shader_Defines.hlsli"

matrix g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;

matrix g_WorldMatrixInv;
matrix g_ViewMatrixInv;
matrix g_ProjMatrixInv;

Texture2D g_DepthTexture;
Texture2D g_MaskTexture;

float g_fCircleRadius;
float g_fCircleWidth;
float g_fBlendRadius;
float g_fMaxRadius;

float4 g_vScanColor;

struct VS_IN
{
    float3 vPosition : POSITION;
};

struct VS_OUT
{
    float4 vPosition : SV_POSITION;
    float4 vProjPos : TEXCOORD0;
};

VS_OUT VS_MAIN(VS_IN In)
{
    VS_OUT Out = (VS_OUT)0;
    
    matrix matWV, matWVP;
    
    matWV = mul(g_WorldMatrix, g_ViewMatrix);
    matWVP = mul(matWV, g_ProjMatrix);
    
    Out.vPosition = mul(float4(In.vPosition, 1.f), matWVP);
    Out.vProjPos = Out.vPosition;

    return Out;
}

struct PS_IN
{
    float4 vPosition : SV_POSITION;
    float4 vProjPos : TEXCOORD0;
};

struct PS_OUT
{
    float4 vDiffuse : SV_TARGET0;
    float4 vNormal : SV_TARGET1;
    float4 vEmissive : SV_TARGET2;
};

PS_OUT PS_MAIN(PS_IN In)
{
    PS_OUT Out = (PS_OUT)0;
    
    float2 vNDC = In.vProjPos.xy / In.vProjPos.w;

    float2 vTexcoord = 0.f;
     
    vTexcoord.x = vNDC.x * 0.5f + 0.5f;
    vTexcoord.y = vNDC.y * -0.5f + 0.5f;
        
    float4 vWorldPos = 0.f;
    
    vector vDepthDesc = g_DepthTexture.Sample(PointSampler, vTexcoord);
    
    if(vDepthDesc.y == 0.f)
        discard;
    
    vWorldPos.x = vTexcoord.x * 2.f - 1.f;
    vWorldPos.y = vTexcoord.y * -2.f + 1.f;
    vWorldPos.z = vDepthDesc.x;
    vWorldPos.w = 1.f;
    
    vWorldPos *= vDepthDesc.y;
    vWorldPos = mul(vWorldPos, g_ProjMatrixInv);
    
    vWorldPos = mul(float4(vWorldPos.xyz, 1.f), g_ViewMatrixInv);
    
    if (false == all(vWorldPos.xyz))
        discard;

    float4 vLocalPos = mul(float4(vWorldPos.xyz, 1.f), g_WorldMatrixInv);
    
    float3 vLocalVolume = abs(vLocalPos.xyz);
        
    if (any(vLocalVolume.xz > 0.5f))
        discard;
    
    float2 vCenterPos = g_WorldMatrix[3].xz;
    
    float fDistance = length(vWorldPos.xz - vCenterPos);
    
    float2 vRange = float2(g_fCircleRadius, g_fCircleRadius + g_fCircleWidth);
    
    float fAlpha = g_fCircleRadius < g_fBlendRadius ? 1.f : (1.f - ((g_fCircleRadius - g_fBlendRadius) / max((g_fMaxRadius - g_fBlendRadius), 1e-5f)));
    
    if (fDistance >= vRange.y)
        discard;
    else if(fDistance <= vRange.x)
    {
        float fDistanceWeight = lerp(0.f, 0.5f, smoothstep(0.f, vRange.x, fDistance));
        Out.vDiffuse = float4(0.f, 0.f, 0.f, min(fDistanceWeight, fAlpha));
        return Out;
    }
    
    float2 vDecalUV = clamp((vLocalPos.xz) + 0.5f, 0.f, 1.f); // Decal ¹üÀ§ -0.5~0.5;
    
    float4 vMask = 0.f;
    
    vMask = g_MaskTexture.Sample(DefaultSampler, vDecalUV * (floor(g_fCircleRadius) * 5.f));
    
    float fMaskAlpha = max(max(vMask.r, vMask.b), vMask.g);
    

    float4 vColor = float4(vMask.xyz * g_vScanColor.xyz, min(fMaskAlpha, fAlpha));
    
    Out.vDiffuse = vColor;
    
    return Out;
}

technique11 DefaultTechnique
{
    pass DefaultPass // 0
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_None, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN();
    }
}