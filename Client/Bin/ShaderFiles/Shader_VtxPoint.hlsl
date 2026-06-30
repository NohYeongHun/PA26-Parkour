#include "Engine_Shader_Defines.hlsli"

matrix g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;

Texture2D g_DepthTexture;
Texture2D g_MaskTexture;
Texture2D g_DiffuseTexture;

float g_fRange;

struct VS_IN
{
    float3 vPosition : POSITION;
};

struct VS_OUT
{
    float4 vPosition : POSITION;
};

VS_OUT VS_MAIN(VS_IN In)
{
    VS_OUT Out = (VS_OUT)0;
    
    matrix matWV, matWVP;
    
    Out.vPosition = mul(float4(In.vPosition, 1.f), g_WorldMatrix);
    return Out;
}
struct GS_IN
{
    float4 vPosition : POSITION;
};

    struct GS_OUT
{
    float4 vPosition : SV_POSITION;
    float2 vTexcoord : TEXCOORD0;
    float4 vWorldPos : TEXCOORD1;
};

[maxvertexcount(6)]
void GS_MAIN(point GS_IN In[1], inout TriangleStream<GS_OUT> Vertices)
{
    GS_OUT Out[4];
    
    vector vRight, vUp, vLook;
    vRight = float4(g_fRange, 0.f, 0.f, 0.f);
    vUp = float4(0.f, 1.f, 0.f, 0.f);
    vLook = float4(0.f, 0.f, g_fRange, 0.f);
    
    matrix matVP = mul(g_ViewMatrix, g_ProjMatrix);
    
    Out[0].vWorldPos = In[0].vPosition + vRight + vLook;
    Out[0].vPosition = mul(Out[0].vWorldPos, matVP);
    Out[0].vTexcoord = float2(0.f, 0.f);
    
    Out[1].vWorldPos = In[0].vPosition - vRight + vLook;
    Out[1].vPosition = mul(Out[1].vWorldPos, matVP);
    Out[1].vTexcoord = float2(1.f, 0.f);
    
    Out[2].vWorldPos = In[0].vPosition - vRight - vLook;
    Out[2].vPosition = mul(Out[2].vWorldPos, matVP);
    Out[2].vTexcoord = float2(1.f, 1.f);
    
    Out[3].vWorldPos = In[0].vPosition + vRight - vLook;
    Out[3].vPosition = mul(Out[3].vWorldPos, matVP);
    Out[3].vTexcoord = float2(0.f, 1.f);
    
    Vertices.Append(Out[0]);
    Vertices.Append(Out[1]);
    Vertices.Append(Out[2]);
    Vertices.RestartStrip();
    
    Vertices.Append(Out[0]);
    Vertices.Append(Out[2]);
    Vertices.Append(Out[3]);
    Vertices.RestartStrip();
    float4 vDepth = g_DepthTexture.Sample(DefaultSampler, float2(0, 0));
}

struct PS_IN
{
    float4 vPosition : SV_POSITION;
    float2 vTexcoord : TEXCOORD0;
    float4 vWorldPos : TEXCOORD1;
};



struct PS_OUT
{
    float4 vColor : SV_TARGET0;
};

PS_OUT PS_MAIN(PS_IN In)
{
    PS_OUT Out = (PS_OUT)0;
    
    float4 vDepth = g_DepthTexture.Sample(DefaultSampler, In.vTexcoord);
    Out.vColor = g_DiffuseTexture.Sample(DefaultSampler, In.vTexcoord);
    
    if(vDepth.w == 0)
        discard;
    
    float3 PixelWorldPos = In.vWorldPos.xyz;
    
    float Dist = distance(PixelWorldPos.xz, g_WorldMatrix[3].xz);
    
    if(Dist < g_fRange)
    {
        //float4 vMask = g_MaskTexture.Sample(DefaultSampler, In.vTexcoord);
        Out.vColor = float4(0.7f, 1.f, 0.7f, 1.f);
    }
    
    return Out;
}

technique11 DefaultTechnique
{
    pass DefaultPass // 0
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = compile gs_5_0 GS_MAIN();
        PixelShader = compile ps_5_0 PS_MAIN();
    }
}