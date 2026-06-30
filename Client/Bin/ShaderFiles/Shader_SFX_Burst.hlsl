#include "Engine_Shader_Defines.hlsli"

static float PI = 3.1415926535f;

matrix g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;

float g_fIntensity;

float2 g_vScreenSize;

float3 g_vColor;
float g_fCircleRadius;
float g_fCircleWidth;

float g_fMaxRadian;

bool g_IsReverse = false;

Texture2D g_SceneTexture;
Texture2D g_MaskTexture;
Texture2D g_NoiseTexture;
Texture2D g_NoiseMaskTexture;

#define RADIUS 16

cbuffer RadialData : register(b0)
{
    float fMinDistance;
    float fMaxDistance;
    float fLengthScale;
    float fPadding0;
    float2 vPivot;
    float2 fPadding1;
};

struct VS_IN
{
    float3 vPosition : POSITION;
    float2 vTexcoord : TEXCOORD0;
};

struct VS_OUT
{
    float4 vPosition : SV_POSITION;
    float2 vTexcoord : TEXCOORD0;
};

VS_OUT VS_MAIN(VS_IN In)
{
    VS_OUT Out = (VS_OUT)0;
    
    matrix matWV, matWVP;
    
    matWV = mul(g_WorldMatrix, g_ViewMatrix);
    matWVP = mul(matWV, g_ProjMatrix);
    Out.vPosition = mul(float4(In.vPosition, 1.f), matWVP);
    Out.vTexcoord = In.vTexcoord;

    return Out;
}

struct PS_IN
{
    float4 vPosition : SV_POSITION;
    float2 vTexcoord : TEXCOORD0;
};

struct PS_OUT_SFX
{
    float4 vColor : SV_TARGET0;
    float3 vEmissive : SV_TARGET1;
    float4 vDistortion : SV_TARGET2;
};

struct PS_OUT_POST_SFX
{
    float4 vColor : SV_TARGET0;
};

PS_OUT_SFX PS_SLASH(PS_IN In)
{
    PS_OUT_SFX Out = (PS_OUT_SFX) 0;
    
    float4 vMask = g_MaskTexture.Sample(DefaultSampler, In.vTexcoord);
    float4 vNoise = g_NoiseMaskTexture.Sample(DefaultSampler, In.vTexcoord);
    float4 vSFXColor = 0;
    
    vSFXColor.xyz = g_vColor * (vMask.xyz);
    vSFXColor.a = max(max(vSFXColor.r, vSFXColor.g), vSFXColor.b);
    
    Out.vColor = vSFXColor;
    
    Out.vEmissive = Out.vColor;
    
    return Out;
}

float Hash21(float2 vTexcoord)
{
    float2 vInput = vTexcoord;
    vInput = frac(vInput * float2(123.32, 456.21));
    vInput += dot(vInput, vInput + 45.32);
    return frac(vInput.x * vInput.y);
}

float4 SFX_Radial_BlurMax(float2 vTexcoord, float2 vRadialScale, uint iSampleCount)
{
    float fMax = 1.f;
    
    if (vRadialScale.x > 0.f)
    {
        float fX = (1.f - vTexcoord.x) / (vRadialScale.x);
        fMax = min(fMax, fX);
    }
    else if (vRadialScale.x < 0.f)
    {
        float fX = (vTexcoord.x) / (vRadialScale.x * -1.f);
        fMax = min(fMax, fX);
    }
    
    if (vRadialScale.y > 0.f)
    {
        float fY = (1.f - vTexcoord.y) / (vRadialScale.y);
        fMax = min(fMax, fY);
    }
    else if (vRadialScale.y < 0.f)
    {
        float fY = (vTexcoord.y) / (vRadialScale.y * -1.f);
        fMax = min(fMax, fY);
    }
    
    vRadialScale *= fMax;
    
    float4 vColor = 0.f;
    float4 vFinalColor = 0.f;
    float fTotalWeight = 0.f;
   
    float Jitter = lerp(0.5f, 1.f, Hash21(vTexcoord));
    
    for (int i = 0; i < iSampleCount; ++i)
    {
        float fRatio = ((float) i + Jitter) / (float) iSampleCount;

        float2 vOffset = vRadialScale * fRatio;
        
        float2 vOffsetTex = vTexcoord + vOffset;

        float4 vSampleColor = g_SceneTexture.Sample(ClampSampler, vOffsetTex);

        float fWeight = exp2(-fRatio * 3.f);
        
//        vColor += vSampleColor * fWeight;
        vColor = max(vSampleColor, vColor);
        fTotalWeight += fWeight;
    }
    
    vFinalColor = vColor;
    
    return vFinalColor;
}

PS_OUT_POST_SFX PS_SLASH_BLUR(PS_IN In)
{
    PS_OUT_POST_SFX Out = (PS_OUT_POST_SFX) 0;
    
    float2 vDir = In.vTexcoord - vPivot;
    float fLength = length(vDir);
    
    float fScale = smoothstep(fMaxDistance, fMinDistance, fLength);
    
    float2 vNormalDir = normalize(vDir);
    
    float fAngle = atan2(vNormalDir.y, vNormalDir.x) / (2 * PI) + 0.5f;
    
    float fNoise = g_NoiseTexture.Sample(DefaultSampler, float2(fAngle * 30.f, 0.5f));
    
    float2 vRadialScale = vDir * fLengthScale * fNoise * fScale;
    
    int iSampleCount = clamp(RADIUS * fScale, 4, RADIUS);
    
    float4 vFinalColor = 0.f;
    
    vFinalColor = SFX_Radial_BlurMax(In.vTexcoord, vRadialScale, iSampleCount);
    //vFinalColor = g_SceneTexture.Sample(DefaultSampler, In.vTexcoord);

    Out.vColor = vFinalColor;
    
    return Out;
}


float4 SFX_Radial_Blur(float2 vTexcoord, float2 vRadialScale, uint iSampleCount)
{
    float fMax = 1.f;
    
    if (vRadialScale.x > 0.f)
    {
        float fX = (1.f - vTexcoord.x) / (vRadialScale.x);
        fMax = min(fMax, fX);
    }
    else if (vRadialScale.x < 0.f)
    {
        float fX = (vTexcoord.x) / (vRadialScale.x * -1.f);
        fMax = min(fMax, fX);
    }
    
    if (vRadialScale.y > 0.f)
    {
        float fY = (1.f - vTexcoord.y) / (vRadialScale.y);
        fMax = min(fMax, fY);
    }
    else if (vRadialScale.y < 0.f)
    {
        float fY = (vTexcoord.y) / (vRadialScale.y * -1.f);
        fMax = min(fMax, fY);
    }
    
    vRadialScale *= fMax;
    
    float4 vColor = 0.f;
    float4 vFinalColor = 0.f;
    float fTotalWeight = 0.f;
   
    float Jitter = lerp(0.5f, 1.f, Hash21(vTexcoord));
    
    for (int i = 0; i < iSampleCount; ++i)
    {
        float fRatio = ((float) i + Jitter) / (float) iSampleCount;

        float2 vOffset = vRadialScale * fRatio;
        
        float2 vOffsetTex = vTexcoord + vOffset;

        float4 vSampleColor = g_SceneTexture.Sample(ClampSampler, vOffsetTex);

        float fWeight = exp2(-fRatio * 3.f);
        
        vColor += vSampleColor * fWeight;
//        vColor = max(vSampleColor, vColor);
        fTotalWeight += fWeight;
    }
    
    vFinalColor = vColor / (fTotalWeight / 2.f);
    
    return vFinalColor;
}


PS_OUT_SFX PS_GALBRENA_Circle(PS_IN In)
{
    PS_OUT_SFX Out = (PS_OUT_SFX) 0;
    
    float2 vTexel = In.vTexcoord * g_vScreenSize;
    
    float2 vCenter = float2(0.5f, 0.5f) * g_vScreenSize;
    
    float fDistance = length(vTexel - vCenter);
    
    float2 vRange = float2(g_fCircleRadius, g_fCircleRadius + g_fCircleWidth);
    
    if(fDistance >= vRange.x && fDistance <= vRange.y)
    {
        Out.vColor = float4(g_vColor, 1.f);
    }
    else
        discard;
    
    return Out;
}

PS_OUT_POST_SFX PS_GALBRENA_BLUR(PS_IN In)
{
    PS_OUT_POST_SFX Out = (PS_OUT_POST_SFX) 0;

    float2 vDir = In.vTexcoord - vPivot;
    float fLength = length(vDir);
    
    float2 vNormalDir = normalize(vDir);
    
    float fScale = smoothstep(fMinDistance, fMaxDistance, fLength);
    
    float fAngle = atan2(vNormalDir.y, vNormalDir.x) / (2 * PI) + 0.5f;
    
    float fNoise = g_NoiseTexture.Sample(DefaultSampler, float2(fAngle * 30.f, 0.5f));
    
    float2 vRadialScale = vDir * fLengthScale * fNoise * fScale;
    
    int iSampleCount = clamp(RADIUS * fScale, 4, RADIUS);
    
    float4 vFinalColor = 0.f;
    
    vFinalColor = SFX_Radial_Blur(In.vTexcoord, vRadialScale, iSampleCount);
    
    if (g_IsReverse)
    {
        vFinalColor.xyz = 1.f - vFinalColor.xyz;
    }
    
    Out.vColor = vFinalColor;
    
    return Out;
}

cbuffer SlashData : register(b1)
{
    float2 vSlashPoint0;
    float2 vSlashPoint1;
    float2 vScreenSize;
    float fOffset;
    float fIntensity;
};

PS_OUT_POST_SFX PS_EXCUTE(PS_IN In)
{
    PS_OUT_POST_SFX Out = (PS_OUT_POST_SFX) 0;
    
    float4 vFinalColor = g_SceneTexture.Sample(ClampSampler, In.vTexcoord);
    
    float2 vSlashDir = normalize(vSlashPoint1 - vSlashPoint0);
    float2 vSlashNormal = float2(-vSlashDir.y, vSlashDir.x);
    
    float2 vTexel = In.vTexcoord * vScreenSize;
    
    float2 vDir = vTexel - vSlashPoint0;
    float fDot = dot(vDir, vSlashNormal);
    
    float fDist = abs(fDot);

    float2 vOffset = 0.f;    
    
    vSlashDir = fDot > 0.f ? vSlashDir * -1.f : vSlashDir;
    
    float fUVIntensity = fDot > 0.f ? smoothstep(1.f, 0.f, In.vTexcoord) : smoothstep(0.f, 1.f, In.vTexcoord);
    
    vOffset = vSlashDir * (fOffset / vScreenSize) * fIntensity * fUVIntensity;
    
    Out.vColor = g_SceneTexture.Sample(ClampSampler, In.vTexcoord + vOffset);
    
    return Out;
}

technique11 DefaultTechnique
{
    pass AugustaSlash // 0
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_SLASH();
    }
    
    pass AugustaSlashBlur
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_SLASH_BLUR();
    }
 
    pass GalbrenaCircle
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Blend, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_GALBRENA_Circle();
    }
    
    pass GalbrenaBlur
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_GALBRENA_BLUR();
    }

    pass Excute
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_EXCUTE();
    }
}

/*

cbuffer g_SlashData : register(b0)
{
    float2 vSlashPoint0;
    float2 vSlashPoint1;
    float fOffset;
    float fIntensity;
    float2 Padding;
};

    float2 vSlashDir = normalize(vSlashPoint1- vSlashPoint0);
    float2 vSlashNormal = float2(-vSlashDir.y, vSlashDir.x);m
    
    float2 vTexel = In.vTexcoord * vScreenSize;
    
    float2 vDir = vTexel - vSlashPoint0;
    float fDot = dot(vDir, vSlashNormal);
    
    float fDist = abs(fDot);
    
    if (fDist <= 3.f)
    {
        Out.vColor = float4(1.f, 0.f, 0.f, 1.f);
        return Out;
    }
    
    float2 vOffset = 0.f;
    
    
    vSlashDir = fDot > 0.f ? vSlashDir * -1.f : vSlashDir;
    
    //vSlashDir = fDot > 0.f ? float2(1.f, 0.f) : float2(-1.f, 0.f);
    
    
    float fUVIntensity = fDot > 0.f ? smoothstep(1.f, 0.f, In.vTexcoord) : smoothstep(0.f, 1.f, In.vTexcoord);
    
    vOffset = vSlashDir * (fOffset / vScreenSize) * fIntensity * fUVIntensity;
    
    Out.vColor = g_SceneTexture.Sample(ClampSampler, In.vTexcoord + vOffset);
    
*/