// UI용
#include "Engine_Shader_State.hlsli"

// ==============================
// * Global Variables
// ==============================

// Basic Variables
matrix g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;

float4 g_CamPosition;
float3 g_HolderPosition;
//Texture2D g_Texture;
//float g_AlphaStrength;

float4 g_BaseColor; // 기본 색
float4 g_HeadColor; // 시작 색
float4 g_TailColor; // 끝 색



float g_AlphaScale = 1.f;
float g_TimeElapsed = 0.f;

// ==============================
// * Vertex Shader
// ==============================

struct VS_IN_CURVE
{
    float3 vPosition        : POSITION;
    float fCurveWidth       : TEXCOORD0;
    float fCurveProgress    : TEXCOORD1; // 0~1
    //float3 vPadding         : TEXCOORD2;
};

struct VS_OUT_CURVE
{
    float4 vPosition        : SV_POSITION;
    float3 vWorldPos        : TEXCOORD0;
    float fCurveWidth       : TEXCOORD1;
    float fCurveProgress    : TEXCOORD2;
};

VS_OUT_CURVE VS_MAIN_CURVE(VS_IN_CURVE In)
{
    VS_OUT_CURVE Out;

    float4 vWorld       = mul(float4(In.vPosition, 1.0f), g_WorldMatrix);
    float4 vView        = mul(vWorld, g_ViewMatrix);                     
    Out.vPosition       = mul(vView, g_ProjMatrix);                      

    Out.vWorldPos       = vWorld.xyz;
    Out.fCurveWidth     = In.fCurveWidth;
    Out.fCurveProgress  = In.fCurveProgress;

    return Out;
}

// ==============================
// * Pixel Shader
// ==============================

struct PS_IN_CURVE
{
    float4 vPosition        : SV_POSITION;
    float3 vWorldPos        : TEXCOORD0;
    float fCurveWidth       : TEXCOORD1;
    float fCurveProgress    : TEXCOORD2;
};

struct PS_OUT
{
    float4 vColor;
};

PS_OUT PS_MAIN_CURVE(PS_IN_CURVE In) : SV_Target
{
    PS_OUT Out;
       
    // 진행도에 따른 색 그라디언트
    float4 gradColor = lerp(g_HeadColor, g_TailColor, In.fCurveProgress);

    float4 col = g_BaseColor * gradColor;

    // 좌우 가장자리 알파 페이드 (중앙 밝게, 끝은 어둡게)
    float fMaxAlpha = 0.5f;
    float alphaEdge = 1.0f - abs(In.fCurveWidth - 0.5f) * 2.0f * fMaxAlpha; // u=0.5 -> 1, u=0/1 -> 0
    if (abs(In.fCurveWidth - 0.5f) > 0.45f)
        alphaEdge = 1.f;
    alphaEdge = saturate(alphaEdge);

    col.a *= alphaEdge * g_AlphaScale;
    
    
    // 출발 타겟과 가까울 시에 투명화
    float3 vStartTargetPos = g_HolderPosition.xyz;
    float3 vFocusedPixelPos = In.vWorldPos;
    
    const float fMinLength = 2.f;   // 이보다 작으면 알파.
    const float fMaxLength = 4.f;  // 이보다 크면 그대로.
    
    
    float lengthToTarget = length(vStartTargetPos - vFocusedPixelPos);

    if (lengthToTarget < fMinLength)                                        // 너무 가까우면 투명화
        col.a = 0.f;
    else if (fMinLength <= lengthToTarget && lengthToTarget < fMaxLength)   // 적당히 가까우면 거리따라 알파 다르게.
    {
        float normalizedLength = smoothstep(fMinLength, fMaxLength, lengthToTarget);
        col.a = col.a * normalizedLength;
    }
        
    
    
    Out.vColor = col;
    return Out;
}

// ==============================
// * Technique / Pass
// ==============================

technique11 DefaultTechnique
{
    pass CurvePass
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_NoneCompare, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN_CURVE();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_CURVE();
    }
}