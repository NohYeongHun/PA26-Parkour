#include "Engine_Shader_Defines.hlsli"

matrix g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;

Texture2D g_DiffuseTexture;
Texture2D g_NormalTexture;
Texture2D g_NoiseTexture;

bool g_HasNormal = false;
float g_fRatio = 0.f;
float4 g_vBaseColor = 1.f;

float g_fFxTime;

matrix g_ShadowViewMatrix[4];
matrix g_ShadowProjMatrix[4];

struct VS_IN
{
    float3 vPosition : POSITION;
    float3 vNormal : NORMAL;
    float3 vTangent : TANGENT;
    float3 vBinormal : BINORMAL;
    float2 vTexcoord : TEXCOORD0;
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
    
    matrix matWV, matWVP;
    
    matWV = mul(g_WorldMatrix, g_ViewMatrix);
    matWVP = mul(matWV, g_ProjMatrix);
    Out.vPosition = mul(float4(In.vPosition, 1.f), matWVP);
    Out.vNormal = normalize(mul(float4(In.vNormal, 0.f), g_WorldMatrix));
    Out.vTangent = normalize(mul(float4(In.vTangent, 0.f), g_WorldMatrix));
    Out.vBinormal = normalize(mul(float4(In.vBinormal, 0.f), g_WorldMatrix));
    Out.vTexcoord = In.vTexcoord;
    Out.vProjPos = mul(float4(In.vPosition, 1.f), matWVP);

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
};

PS_OUT PS_MAIN(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;

    vector vDiffuse = g_DiffuseTexture.Sample(DefaultSampler, In.vTexcoord);

    float4 vNormal;
    
    if (g_HasNormal)
    {
        vector vDefaultNormal = g_NormalTexture.Sample(DefaultSampler, In.vTexcoord);
        vNormal = normalize(vDefaultNormal * 2.f - 1.f);
        if (vDefaultNormal.x > vDefaultNormal.z && vDefaultNormal.y > vDefaultNormal.z)
            vNormal.z = sqrt(1.f - saturate(dot(vDefaultNormal.xy, vDefaultNormal.xy)));
        
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
    
    Out.vDiffuse = vDiffuse * g_vBaseColor;
    Out.vNormal = vNormal;
    Out.vDepth.x = In.vProjPos.z / In.vProjPos.w;
    Out.vDepth.y = In.vProjPos.w;
    Out.vPBR.y = g_fGlobalDynamicRoughness;
    Out.vPBR.x = g_fGlobalDynamicMetallic;
    return Out;
}

PS_OUT PS_SHUI(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;

    float2 vTexcoord = float2(In.vTexcoord.x, In.vTexcoord.y + g_fFxTime);

    float4 vDiffuse = g_DiffuseTexture.Sample(DefaultSampler, vTexcoord);
    
    Out.vDiffuse = vDiffuse;
   
    float4 vNormal = In.vNormal;
    vNormal = vNormal * 0.5f + 0.5f;
    
    Out.vNormal = vNormal;
    Out.vDepth.x = In.vProjPos.z / In.vProjPos.w;
    Out.vDepth.y = In.vProjPos.w;
    Out.vPBR.y = g_fGlobalDynamicRoughness;
    Out.vPBR.x = g_fGlobalDynamicMetallic;
    
    return Out;
}


struct VS_OUT_SHADOW
{
    float4 vPosition : POSITION;
};

VS_OUT_SHADOW VS_SHADOW(VS_IN In)
{
    VS_OUT_SHADOW Out = (VS_OUT_SHADOW) 0;
    
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
    uint iIndex : SV_RenderTargetArrayIndex;
};

[maxvertexcount(12)]
void GS_SHADOW(triangle GS_IN In[3], inout TriangleStream<GS_OUT> Vertices)
{
    for (int Face = 0; Face < 4; Face++)
    {
        GS_OUT Out = (GS_OUT) 0;
        Out.iIndex = Face;
        
        matrix matVP;
        matVP = mul(g_ShadowViewMatrix[Face], g_ShadowProjMatrix[Face]);

        for (int i = 0; i < 3; i++)
        {
            Out.vPosition = mul(In[i].vPosition, matVP);
            Vertices.Append(Out);
        }
        Vertices.RestartStrip();
    }
}

struct PS_IN_SHADOW
{
    float4 vPosition : SV_POSITION;
};

void PS_SHADOW(PS_IN_SHADOW In)
{
    if (In.vPosition.z >= 1.f)
        discard;
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

    pass ShuiPass // 1
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_SHUI();
    }
    
    pass Shadow // 2
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_SHADOW();
        GeometryShader = compile gs_5_0 GS_SHADOW();
        PixelShader = compile ps_5_0 PS_SHADOW();
    }
    //pass NormalTex // 1
    //{
    //    SetRasterizerState(RS_Default);
    //    SetDepthStencilState(DSS_Default, 0);
    //    SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);
    //
    //    VertexShader = compile vs_5_0 VS_MAIN();
    //    GeometryShader = NULL;
    //    PixelShader = compile ps_5_0 PS_MAIN();
    //}
}