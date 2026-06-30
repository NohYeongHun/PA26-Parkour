#include "Engine_Shader_Defines.hlsli"

matrix g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;

Texture2D g_DiffuseTexture;
Texture2D g_NormalTexture;
Texture2D g_MaskTexture;

float g_rotationSpeed = 2.0f; // 전체 회전 속도 (양수: 시계 방향 효과*)
float g_swirlStrength = 3.0f; // 휘어짐 강도 (클수록 많이 꼬임)
float g_suctionSpeed = 0.5f; // 빨려 들어가는 속도 (텍스처 줌인 효과)
float g_TotalTime;
float4 vColor = float4(1.f, 1.f, 1.f, 1.f);


//TEST
float g_Alpha = 1.f;
float g_MaskSpeed = 1.f;
float g_Time;
float g_ColorGamma = 1.f;
float g_ColorGain = 1.f;


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

struct VS_OUT_POTAL
{
    float4 vPosition : SV_POSITION;
    float2 vTexcoord : TEXCOORD0;
    float2 vTempTexcoord : TEXCOORD1;
};

struct PS_IN
{
    float4 vPosition : SV_POSITION;
    float2 vTexcoord : TEXCOORD0;
};

struct PS_OUT
{
    float4 vColor : SV_TARGET0;
};

struct PS_SPECTRUMOUT
{
    float4 vColor : SV_TARGET0;
    float4 vEmissive : SV_TARGET1;
};

PS_OUT PS_MAIN(PS_IN In)
{
    PS_OUT Out = (PS_OUT)0;

    Out.vColor = g_DiffuseTexture.Sample(DefaultSampler, In.vTexcoord);
    
    return Out;
}

PS_OUT PS_MOUSE(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;

    Out.vColor = g_DiffuseTexture.Sample(DefaultSampler, In.vTexcoord);
    
    if (0.5f >= Out.vColor.a)
        discard;
    
    if(0.f == Out.vColor.r)
        discard;
    
    return Out;
}

struct PS_IN_POTAL
{
    float4 vPosition : SV_POSITION;
    float2 vTexcoord : TEXCOORD0;
    float2 vTempTexcoord : TEXCOORD1;
};
struct PS_OUT_POTAL
{
    float4 vBackBuffer : SV_TARGET0;
    float4 vEmissive : SV_TARGET1;
    float4 vDistortion : SV_TARGET2;
};

PS_OUT_POTAL PS_POTAL2(PS_IN In)
{
    PS_OUT_POTAL Out = (PS_OUT_POTAL) 0;
    float2 CenterUV = In.vTexcoord - float2(0.5f, 0.5f);
    

    float Theta = (g_TotalTime * 1.f);
    float C = cos(Theta);
    float S = sin(Theta);
    float2 NewTexcoord;
    NewTexcoord.x = CenterUV.x * C - CenterUV.y * S;
    NewTexcoord.y = CenterUV.x * S + CenterUV.y * C;
    NewTexcoord += float2(0.5f, 0.5f);
    
    vector vDiffuse = g_DiffuseTexture.Sample(DefaultSampler, NewTexcoord);
    vector vMask = g_MaskTexture.Sample(DefaultSampler, NewTexcoord);
    
    Out.vDistortion = float4(vMask.rgb, 0.6f);
    float Alpha = g_TotalTime / 5.f;
    if (Alpha >= 1.f)
        Alpha = 1.f;
    Out.vBackBuffer = float4(0.5f, 0.5f, 0.f, Alpha);
    Out.vBackBuffer *= vMask;
    Out.vEmissive = Out.vBackBuffer;

    float Dist = distance(In.vTexcoord, float2(0.5f, 0.5f));
    if (Dist >= 0.5f)
        discard;
    
    return Out;
}

PS_SPECTRUMOUT PS_SPUCTRUM(PS_IN In)
{
    PS_SPECTRUMOUT Out = (PS_SPECTRUMOUT) 0;
    
    float2 UV = In.vTexcoord;
    
    float Radial = frac(UV.y + g_MaskSpeed * g_Time);

    float2 FlowUV;
    FlowUV.x = UV.x;
    FlowUV.y = Radial;
    
    float MaskR = g_MaskTexture.Sample(DefaultSampler, float2(FlowUV.y, FlowUV.x)).r;
    
    if (MaskR < 0.25f)
        discard;

    float4 vColor;

    vColor = g_DiffuseTexture.Sample(DefaultSampler, float2(saturate(In.vTexcoord.x), saturate(In.vTexcoord.y)));
    
    Out.vColor = float4(vColor.rgb, 1.f);
    
    Out.vColor.rgb = saturate(vColor.rgb);
    Out.vColor.rgb = pow(vColor.rgb, g_ColorGamma);
    Out.vColor.rgb *= g_ColorGain;

    float fWeight = Luminance(Out.vColor.xyz);

    if (fWeight >= g_fEmissiveThreshold)
        Out.vEmissive = float4(Out.vColor.xyz, 1.f);

    Out.vColor.a *= g_Alpha;
    
    Out.vEmissive.xyz *= Out.vColor.a;
    
    return Out;
}

PS_SPECTRUMOUT PS_SPUCTRUM_DG(PS_IN In)
{
    PS_SPECTRUMOUT Out = (PS_SPECTRUMOUT) 0;
    
    float2 UV = In.vTexcoord;
    
    float MaskR = g_MaskTexture.Sample(DefaultSampler, float2(UV.y, UV.x)).r;
    
    if (MaskR < 0.25f)
        discard;

    float4 vColor;

    vColor = g_DiffuseTexture.Sample(DefaultSampler, float2(saturate(In.vTexcoord.x), saturate(In.vTexcoord.y)));
    
    Out.vColor = float4(vColor.rgb, 1.f);
    
    Out.vColor.rgb = saturate(vColor.rgb);
    Out.vColor.rgb = pow(vColor.rgb, g_ColorGamma);
    Out.vColor.rgb *= g_ColorGain;

    float fWeight = Luminance(Out.vColor.xyz);

    if (fWeight >= g_fEmissiveThreshold)
        Out.vEmissive = float4(Out.vColor.xyz, 1.f);

    Out.vColor.a *= g_Alpha;
    
    Out.vEmissive.xyz *= Out.vColor.a;
    
    return Out;
}

technique11 DefaultTechnique
{
    pass DefaultPass // 0
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN();
    }

    pass Mouse // 1
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MOUSE();
    }

    pass Potal // 2
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_POTAL2();
    }

    pass Spectrum //3
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_SPUCTRUM();
    }

    pass Spectrum_DG //4
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_SPUCTRUM_DG();
    }
}