#include "Engine_Shader_Defines.hlsli"

matrix g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;

Texture2D g_DiffuseTexture;

struct VS_IN
{
    
    float3 vPosition : POSITION;
    float3 vNormal : NORMAL;
    float3 vTangent : TANGENT;
    float3 vBinormal : BINORMAL;
    float2 vTexcoord : TEXCOORD0;
    
    row_major float4x4 TransformMatrix : WORLD;
    float2 vLifeTime : TEXCOORD1;
};

struct VS_OUT
{
    float4 vPosition : SV_POSITION;
    float4 vNormal : NORMAL;
    float4 vTangent : TANGENT;
    float4 vBinormal : BINORMAL;
    float2 vTexcoord : TEXCOORD0;
    float4 vProjPos : TEXCOORD1;
    float2 vLifeTime : TEXCOORD2;
};

VS_OUT VS_MAIN(VS_IN In)
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
    Out.vLifeTime = In.vLifeTime;
    
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
    float2 vLifeTime : TEXCOORD2;
};

struct PS_OUT
{
    float4 vColor : SV_TARGET0;             //지금은 그냥 그리기
    //float4 vDiffuse : SV_TARGET0;
    //float4 vNormal : SV_TARGET1;
    //float4 vDepth : SV_TARGET2;
    //float4 vEmissive : SV_TARGET3;
    //float4 vDistortion : SV_TARGET4;
};

PS_OUT PS_MAIN(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;
    
    Out.vColor = g_DiffuseTexture.Sample(DefaultSampler, In.vTexcoord);
   
    if(Out.vColor.a < 0.3f)
        discard;
    
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
        PixelShader = compile ps_5_0 PS_MAIN();
    }
}