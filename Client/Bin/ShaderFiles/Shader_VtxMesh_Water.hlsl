#include "Engine_Shader_Defines.hlsli"

matrix g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;

float4 g_GrassColor = float4(0.6f, 0.564136f, 0.48f, 1.f);
float4 g_LogoWaterColor = float4(0.2627f, 0.3373f, 0.3725f, 1.f);
//float4 g_HeavenWaterColor = float4(0.1922f, 0.0235f, 0.2902, 1.f);
//float4 g_HeavenWaterColor = float4(0.0961f, 0.0117f, 0.1451f, 1.f);
float4 g_HeavenWaterColor = float4(0.1020f, 0.0235f, 0.1725f, 1.f);

Texture2D   g_DiffuseTexture[4];
Texture2D   g_NormalTexture[4];
Texture2D   g_MaskDiffuseTexture;
Texture2D g_MaskTexture[4];
Texture2D g_MaskSprite;

bool g_HasNormal = false;
bool g_HasMask = false;
bool g_HasMetallic = false;
bool g_IsDynamicObject = false;
int g_iIndex = 0;

int g_iShadowMapLayer = 0;
float4 g_vDiffuseColor = float4(1.f, 1.f, 1.f, 1.f);
float g_fXOffset = 0.f;
float g_fYOffset = 0.f;

float g_fTime = 0.f;

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

struct VS_HEAVEN
{
    float4 vPosition : SV_POSITION;
    float4 vNormal : NORMAL;
    float4 vTangent : TANGENT;
    float4 vBinormal : BINORMAL;
    float2 vTexcoord : TEXCOORD0;
    float4 vProjPos : TEXCOORD1;
    float4 vWorldPos : TEXCOORD2;
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

VS_HEAVEN VS_NONREFLECT(VS_IN In)
{
    VS_HEAVEN Out = (VS_HEAVEN) 0;
    
    matrix matWV, matWVP;
    
    matWV = mul(g_WorldMatrix, g_ViewMatrix);
    matWVP = mul(matWV, g_ProjMatrix);
    Out.vPosition = mul(float4(In.vPosition, 1.f), matWVP);
    Out.vNormal = normalize(mul(float4(In.vNormal, 0.f), g_WorldMatrix));
    Out.vTangent = normalize(mul(float4(In.vTangent, 0.f), g_WorldMatrix));
    Out.vBinormal = normalize(mul(float4(In.vBinormal, 0.f), g_WorldMatrix));
    Out.vTexcoord = In.vTexcoord;
    Out.vProjPos = mul(float4(In.vPosition, 1.f), matWVP);
    Out.vWorldPos = mul(float4(In.vPosition, 1.f), g_WorldMatrix);

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

struct PS_IN_HEAVEN
{
    float4 vPosition : SV_POSITION;
    float4 vNormal : NORMAL;
    float4 vTangent : TANGENT;
    float4 vBinormal : BINORMAL;
    float2 vTexcoord : TEXCOORD0;
    float4 vProjPos : TEXCOORD1;
    float4 vWorldPos : TEXCOORD2;
};

struct PS_OUT_LIGHT
{
    float4 vDiffuse : SV_TARGET0;
    float4 vNormal : SV_TARGET1;
    float4 vDepth : SV_TARGET2;
    float4 vPBR : SV_TARGET3;
};

PS_OUT_LIGHT PS_MAIN_NORMAL(PS_IN In)
{
    /*
    Fmodel : Client-Content-Aki-Map-Launch-LaunchScene.uasset에 로고 물 생긴 거 + 텍스쳐 정보 있음.
    아니면 LaunchScene 검색해서 Mou 있는 폴더에 Wat 큰 거.
*/
    
    PS_OUT_LIGHT Out = (PS_OUT_LIGHT) 0;
    
    vector vMask = g_MaskTexture[0].Sample(DefaultSampler, In.vTexcoord);
    
    Out.vDiffuse = g_DiffuseTexture[0].Sample(DefaultSampler, In.vTexcoord);
    
    Out.vPBR.w = 1.f;       // Water Masking
    
    float4 vNormal;

    vector vDefaultNormal = g_NormalTexture[0].Sample(DefaultSampler, float2(In.vTexcoord.x + g_fXOffset*0.01f, In.vTexcoord.y + g_fYOffset*0.01f));
		
    vNormal = normalize(vDefaultNormal * 2.f - 1.f);
    if (vDefaultNormal.x > vDefaultNormal.z && vDefaultNormal.y > vDefaultNormal.z)
        vNormal.z = sqrt(1.f - saturate(dot(vDefaultNormal.xy, vDefaultNormal.xy)));

    float3 vTangent = In.vTangent.xyz;
    float3 vBinormal = In.vBinormal.xyz * -1.f;
    float3 vInNormal = In.vNormal.xyz;

    float3x3 WorldMatrix;
    WorldMatrix = float3x3(vTangent, vBinormal, vInNormal);
        
    vNormal.xyz = normalize(mul(vNormal.xyz, WorldMatrix));
    vNormal.xyz = vNormal.xyz * 0.5f + 0.5f;
    
    Out.vNormal = float4(vNormal.xyz, 1.f);
    Out.vDepth.x = In.vProjPos.z / In.vProjPos.w;
    Out.vDepth.y = In.vProjPos.w;
    
    Out.vDepth.w = 1.f;
    
    return Out;
}

PS_OUT_LIGHT PS_MAIN_NORMAL_ALPHA(PS_IN In)
{
    PS_OUT_LIGHT Out = (PS_OUT_LIGHT) 0;
    
    //Out.vDiffuse = g_DiffuseTexture.Sample(DefaultSampler, In.vTexcoord);
    Out.vDiffuse = float4(1.f, 1.f, 1.f, 1.f);
    
    Out.vNormal = In.vNormal * 0.5f + 0.5f;
    
    Out.vDepth.x = In.vProjPos.z / In.vProjPos.w;
    Out.vDepth.y = In.vProjPos.w;
    Out.vDepth.z = 0.f;
    
    Out.vDepth.w = 1.f;
    
    return Out;
}

PS_OUT_LIGHT PS_MAIN_NORMAL_FOCUS(PS_IN In)
{
    PS_OUT_LIGHT Out = (PS_OUT_LIGHT) 0;
    
    vector vMask = g_MaskTexture[0].Sample(DefaultSampler, In.vTexcoord);
    
    vector vDiffuse = g_DiffuseTexture[0].Sample(DefaultSampler, In.vTexcoord);
    vector vMaskDiffiuse = g_DiffuseTexture[1].Sample(DefaultSampler, In.vTexcoord);
    
    Out.vDiffuse = vDiffuse * (1.f - vMask.r) + vMaskDiffiuse * vMask.g;
    Out.vDiffuse *= float4(0.7f, 1.f, 0.7f, 1.f);
    
    float3 vNormal;
    
    if(g_HasNormal)
    {
        vector vDefaultNormal = g_NormalTexture[0].Sample(DefaultSampler, In.vTexcoord);
        vector vMaskNormal = g_NormalTexture[1].Sample(DefaultSampler, In.vTexcoord);
        
        vector vNormalDesc = vDefaultNormal * (1.f - vMask.r) + vMaskNormal * vMask.g;
        
        vNormal = normalize(vNormalDesc * 2.f - 1.f);
        if (vNormalDesc.x > vNormalDesc.z && vNormalDesc.y > vNormalDesc.z)
            vNormal.z = sqrt(1.f - saturate(dot(vNormalDesc.xy, vNormalDesc.xy)));

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
        vNormal = In.vNormal.xyz;
        vNormal = vNormal * 0.5f + 0.5f;
    }
    
    Out.vNormal = float4(vNormal, 1.f);
    Out.vDepth.x = In.vProjPos.z / In.vProjPos.w;
    Out.vDepth.y = In.vProjPos.w;
    
    Out.vDepth.w = 1.f;
    
    return Out;
}

struct PS_OUT_EMISSIVE
{
    float4 vDiffuse : SV_TARGET0;
    float4 vEmissive : SV_TARGET1;
};

PS_OUT_EMISSIVE PS_EMISSIVE(PS_IN In)
{
    PS_OUT_EMISSIVE Out = (PS_OUT_EMISSIVE) 0;

    float4 vColor = g_DiffuseTexture[0].Sample(DefaultSampler, In.vTexcoord);
    
    Out.vDiffuse = float4(vColor.xyz, 1.f);
    
    float fWeight = Luminance(vColor.xyz);
    
    if(fWeight >= g_fEmissiveThreshold)
        Out.vEmissive = float4(vColor.xyz, 1.f);
        
    return Out;
}

PS_OUT_LIGHT PS_TEST(PS_IN In)
{
    PS_OUT_LIGHT Out = (PS_OUT_LIGHT) 0;
    
    float4 vGrassDiffuse = g_DiffuseTexture[0].SampleLevel(DefaultSampler, (In.vTexcoord * 10.f), 0.f);
    float4 vRockDiffuse = g_DiffuseTexture[1].SampleLevel(DefaultSampler, In.vTexcoord, 0.f);
    
    float4 vMainNormal = g_NormalTexture[0].SampleLevel(DefaultSampler, In.vTexcoord, 0.f);
    float4 vGrasNormal = g_NormalTexture[1].SampleLevel(DefaultSampler, In.vTexcoord * 10.f, 0.f);
    float4 vDetailNormal = g_NormalTexture[2].SampleLevel(DefaultSampler, In.vTexcoord, 0.f);
    
    float4 vAlphaMask = g_MaskTexture[0].SampleLevel(DefaultSampler, In.vTexcoord, 0.f);
    float4 vMask = g_MaskTexture[1].SampleLevel(DefaultSampler, In.vTexcoord, 0.f);
    
    float4 vDiffuse = lerp(vRockDiffuse, (vGrassDiffuse * g_GrassColor), vMask.r);
    
    float3 vNormalDesc = lerp(lerp(vDetailNormal, vMainNormal, vAlphaMask.a), vGrasNormal, vMask.r);
    
    float3 vNormal;
	        
    vNormal = vNormalDesc * 2.f - 1.f;
    
    vNormal.z = sqrt(1.f - saturate(dot(vNormalDesc.xy, vNormalDesc.xy)));
    
    float3 vTangent = In.vTangent.xyz;
    float3 vBinormal = In.vBinormal.xyz * -1.f;
    float3 vInNormal = In.vNormal.xyz;

    float3x3 WorldMatrix;
    WorldMatrix = float3x3(vTangent, vBinormal, vInNormal);
        
    vNormal = normalize(mul(vNormal, WorldMatrix));
    vNormal = vNormal * 0.5f + 0.5f;
    
    Out.vDiffuse.xyz = vDiffuse.xyz;
    Out.vDiffuse.w = 1.f;
    
    Out.vNormal = float4(vNormal, 1.f);
    
    Out.vDepth.x = In.vProjPos.z / In.vProjPos.w;
    Out.vDepth.y = In.vProjPos.w;
    
    Out.vPBR.y = g_fGlobalStaticRoughness;
    Out.vPBR.x = g_fGlobalStaticMetallic;
    
    return Out;
    
}

PS_OUT_LIGHT PS_LOGO(PS_IN In)
{
    PS_OUT_LIGHT Out = (PS_OUT_LIGHT) 0;
 
    float4 vColor = g_LogoWaterColor;

    float2 vTexcoord = float2(In.vTexcoord.x, In.vTexcoord.y);
    
    vector vNormalDesc = g_DiffuseTexture[0].Sample(DefaultSampler, vTexcoord);
    
    float3 vMainNormal = 0.f;

    vMainNormal = vNormalDesc.xyz * 2.f - 1.f;
    vMainNormal.z = sqrt(1.f - saturate(dot(vNormalDesc.xy, vNormalDesc.xy)));
    

    float3 vTangent = In.vTangent.xyz;
    float3 vBinormal = In.vBinormal.xyz * -1.f;
    float3 vInNormal = In.vNormal.xyz;

    float3x3 WorldMatrix;
    WorldMatrix = float3x3(vTangent, vBinormal, vInNormal);
        
    vMainNormal = mul(vMainNormal, WorldMatrix);
    
    float3 vNormal = normalize(vMainNormal.xyz * 0.2f);
    
    vNormal = vNormal * 0.5f + 0.5f;
    
    Out.vDiffuse = vColor;
    Out.vNormal = float4(vNormal, 1.f);
    Out.vDepth.x = In.vProjPos.z / In.vProjPos.w;
    Out.vDepth.y = In.vProjPos.w;
    
    Out.vDepth.w = 1.f;
    Out.vPBR.w = 1.f; // Water Masking
    
    return Out;
}

PS_OUT_LIGHT PS_NONREFLECT(PS_IN_HEAVEN In)
{
    PS_OUT_LIGHT Out = (PS_OUT_LIGHT) 0;
 
    // Wolrd 기반 UV
    float2 vUV = In.vWorldPos.xz * 0.02f;
    
    vector vMask = g_MaskSprite.Sample(DefaultSampler, vUV);
    float fAlpha = max(vMask.r, max(vMask.g, vMask.b));
    
    vector vHeavenWaterColor = g_DiffuseTexture[0].Sample(DefaultSampler, In.vTexcoord) * 0.5f;
    vHeavenWaterColor.xyz = lerp(float3(0.f, 0.f, 0.8f), vHeavenWaterColor.xyz, 0.4f);
    
    float3 vMaskColor = lerp(vHeavenWaterColor.xyz, vMask.xyz, 0.2f);
    float3 vColor = lerp(g_HeavenWaterColor.xyz, vMaskColor, fAlpha);
    //vColor = lerp(float3(0.f, 0.f, 0.2f), vColor, 0.35f);
    
    Out.vDiffuse = float4(vColor, 1.f);
    Out.vNormal = In.vNormal;
    Out.vDepth.x = In.vProjPos.z / In.vProjPos.w;
    Out.vDepth.y = In.vProjPos.w;
    
    Out.vDepth.w = 1.f;
    Out.vPBR.w = 0.f; // Water Masking
    
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

    pass AlphaNotDiscard // 1
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_NORMAL_ALPHA();
    }

    pass SelectedObject // 2
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_NORMAL_FOCUS();
    }

    pass Emissive            // 3
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_EMISSIVE();
    }
    
    pass LogoWater        // 4
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_LOGO();
    }

    pass HeavenWater      // 5
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_NONREFLECT();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_NONREFLECT();
    }
}