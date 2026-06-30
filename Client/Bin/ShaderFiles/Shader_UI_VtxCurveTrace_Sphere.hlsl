// UI용
#include "Engine_Shader_State.hlsli"

// ==============================
// * Global Variables
// ==============================

// Basic Variables
matrix g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;

float4 g_CamPosition;
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

struct VS_IN_SPHERE
{
    float3 vPosition        : POSITION;
    float2 vTexcoord        : TEXCOORD0;
};

struct VS_OUT_SPHERE
{
    float4 vPosition        : SV_POSITION;
    float2 vTexcoord        : TEXCOORD0;
    float3 vWorldPos        : TEXCOORD1; // 픽셀의 월드 위치
    float3 vNormal          : NORMAL;    // 픽셀의 월드 법선
};


VS_OUT_SPHERE VS_MAIN_SPHERE(VS_IN_SPHERE In)
{
    VS_OUT_SPHERE Out;
    
    float4 vWorld       = mul(float4(In.vPosition, 1.0f), g_WorldMatrix);     // 위치 변환
    float4 vView        = mul(vWorld, g_ViewMatrix);
    Out.vPosition       = mul(vView, g_ProjMatrix);

    Out.vWorldPos       = vWorld.xyz;
    Out.vTexcoord       = In.vTexcoord;
    
    // 구는 원점에서 뻗어나가는 방향이 곧 Normal!
    float3 vLocalNormal = normalize(In.vPosition);
    
    // World Normal로 변환 (회전만 적용하기 위해 3x3으로 캐스팅)??
    Out.vNormal = normalize(mul(vLocalNormal, (float3x3) g_WorldMatrix));
    
    return Out;
}

// ==============================
// * Pixel Shader
// ==============================

struct PS_IN_SPHERE
{
    float4 vPosition        : SV_POSITION;
    float2 vTexcoord        : TEXCOORD0;
    float3 vWorldPos        : TEXCOORD1; // 픽셀의 월드 위치
    float3 vNormal          : NORMAL;    // 픽셀의 월드 법선
};

struct PS_OUT
{
    float4 vColor;
};

PS_OUT PS_MAIN_SPHERE(PS_IN_SPHERE In) : SV_Target
{
    PS_OUT Out;

    
    //Out.vColor = float4(1.f, 0.f, 1.f, 1.f);
    //return Out;
    
    
    
    // 1. 픽셀 -> 카메라로 가는 방향을 구함
    float3 vViewDir = normalize(g_CamPosition.xyz - In.vWorldPos);
    float3 vNormal = normalize(In.vNormal);

    // 2. 내적. (1.0 ~ 0.0) / 1.0 = 정면(중앙), 0.0 = 측면(가장자리)
    float NdotV = saturate(dot(vNormal, vViewDir));

    // 3. 반전 및 제곱계산으로 지수함수화 시켜서 띠를 선명하게 유도. 프레넬 효과.(0.0 ~ 1.0)
    float fRim = pow(1.0f - NdotV, 2.f/*3.0f*/);

    // 4. 색상 및 알파 적용
    Out.vColor = g_TailColor;
    Out.vColor.a = fRim * g_TailColor.a * 0.5f; // 중앙은 투명(0), 가장자리는 불투명(1)

    return Out;
}

// ==============================
// * Technique / Pass
// ==============================

technique11 DefaultTechnique
{
    pass SpherePass
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN_SPHERE();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_SPHERE();
    }
}    