#include "Engine_Shader_Defines.hlsli"


matrix g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;

Texture2D g_DiffuseTexture[2];
Texture2D g_NormalTexture[2];
Texture2D g_MaskDiffuseTexture;
Texture2D g_MetallicTexture;
vector g_vMatrlAmbient = vector(1.0f, 1.0f, 1.0f, 1.0f);
vector g_vMatrlSpecular = vector(0.4f, 0.4f, 0.4f, 0.4f);

//Texture2D   g_MaskTexture[4] : register(t8);
Texture2D g_MaskTexture[4];

matrix g_ShadowViewMatrix[4];
matrix g_ShadowProjMatrix[4];

matrix g_ShadowMapViewMatrix;
matrix g_ShadowMapProjMatrix;

float g_fOutLineRadius = 0.0005f;
float g_fOutLineRadiusZ = 0.0005f;

bool g_HasNormal = false;
bool g_HasMask = false;
bool g_HasMetallic = false;
bool g_IsDynamicObject = false;
int g_iIndex = 0;

int g_iShadowMapLayer = 0;
float4 g_vDiffuseColor = float4(0.f, 0.f, 0.f, 0.f);
float g_fRaidan;
struct VS_IN
{
    
    float3 vPosition : POSITION;
    float3 vNormal : NORMAL;
    float3 vTangent : TANGENT;
    float3 vBinormal : BINORMAL;
    float2 vTexcoord : TEXCOORD0;
    
    row_major float4x4 TransformMatrix : WORLD;
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
// 1. 월드 위치 계산 (흔들리기 전)
    float4 vWorldPos = mul(float4(In.vPosition, 1.f), In.TransformMatrix);

    // -------------------------------------------------------------------------
    // [개선 1] 랜덤 노이즈 생성 (위치 기반 해시)
    // 인접한 풀이라도 서로 다른 '랜덤 값'을 갖게 하여 박자를 쪼갭니다.
    // dot 연산 안의 숫자는 아무 소수(Prime Number)나 넣은 난수 생성 공식입니다.
    float fRandom = frac(sin(dot(vWorldPos.xz, float2(12.9898f, 78.233f))) * 43758.5453f);
    
    // -------------------------------------------------------------------------
    // [개선 2] 복합 파동 (큰 바람 + 잔떨림)
    // 단순 sin 하나가 아니라, 주파수와 속도가 다른 두 개의 파동을 섞습니다.
    
    // 파동 1: 크고 느린 바람 (전체적인 휩쓸림)
    float fMainWave = sin(g_fRaidan * 1.0f + vWorldPos.x * 0.5f + fRandom);
    
    // 파동 2: 작고 빠른 떨림 (바람 끝의 디테일) -> 3배 빠르고 3배 촘촘하게
    float fDetailWave = sin(g_fRaidan * 3.0f + vWorldPos.z * 1.5f + fRandom * 5.0f) * 0.3f;

    // 두 파동 합치기
    float fCombinedWave = fMainWave + fDetailWave;

    // -------------------------------------------------------------------------
    // [개선 3] 휨 강도 곡선 (Stiffness)
    // 단순히 y에 비례하는 게 아니라, 제곱을 하여 뿌리 부분은 단단하고 끝부분은 더 많이 휘게 합니다.
    float fSwayFactor = In.vPosition.y;
    fSwayFactor = pow(fSwayFactor, 2.0f); // 제곱 (곡선적인 휨)

    // -------------------------------------------------------------------------
    // 최종 적용
    // 바람의 방향을 (1, 0, 1)로 고정하지 않고 노이즈를 살짝 섞어주면 더 자연스럽습니다.
    float3 vWindDir = normalize(float3(1.0f, 0.f, 0.5f)); // 주 바람 방향
    vWindDir.x += (fRandom - 0.5f) * 0.5f; // 방향에 약간의 노이즈 추가

    // 최종 변위 계산
    float3 vWavedOffset = vWindDir * fCombinedWave * 0.2f * fSwayFactor;
    
    // 위치 적용
    vWorldPos.xyz += vWavedOffset;
    
    // -------------------------------------------------------------------------
    // 나머지 행렬 연산
    matrix matVP = mul(g_ViewMatrix, g_ProjMatrix);
    Out.vPosition = mul(vWorldPos, matVP);
    
    // 노말 등은 회전된 걸 반영하지 않으면 어색할 수 있으나, 미세한 흔들림이면 원본 유지도 괜찮습니다.
    // 정확하게 하려면 vWavedOffset에 따라 회전 행렬을 적용해야 하지만 보통 비용상 생략하거나 근사합니다.
    Out.vNormal = normalize(mul(float4(In.vNormal, 0.f), In.TransformMatrix));
    Out.vTangent = normalize(mul(float4(In.vTangent, 0.f), In.TransformMatrix));
    Out.vBinormal = normalize(mul(float4(In.vBinormal, 0.f), In.TransformMatrix));
    Out.vTexcoord = In.vTexcoord;
    Out.vProjPos = Out.vPosition;
    return Out;
}

VS_OUT VS_NONSHAKE(VS_IN In)
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

    return Out;
}


struct VS_OUT_SHADOW
{
    float4 vPosition : SV_POSITION;
    float2 vTexcoord : TEXCOORD0;
    float4 vProjPos : TEXCOORD1;
};

VS_OUT_SHADOW VS_SHADOW(VS_IN In)
{
    VS_OUT_SHADOW Out = (VS_OUT_SHADOW) 0;
    
    matrix matWV, matWVP;
    
    matWV = mul(g_WorldMatrix, g_ViewMatrix);
    matWVP = mul(matWV, g_ProjMatrix);
    
    Out.vPosition = mul(float4(In.vPosition, 1.f), matWVP);
    Out.vTexcoord = In.vTexcoord;
    Out.vProjPos = Out.vPosition;
    
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

struct PS_OUT_LIGHT
{
    float4 vDiffuse : SV_TARGET0;
    float4 vNormal : SV_TARGET1;
    float4 vDepth : SV_TARGET2;
    float4 vEmissive : SV_TARGET3;
    float4 vDistortion : SV_TARGET4;
    float4 vPBR : SV_TARGET5;
    float4 vSSS : SV_TARGET6;
};

struct PS_OUT_EMISSIVE
{
    float4 vDiffuse : SV_TARGET0;
    float4 vNormal : SV_TARGET1;
    float4 vSpecular : SV_TARGET3;
    float4 vAmbient : SV_TARGET4;
};

PS_OUT_LIGHT PS_MAIN_NORMAL(PS_IN In)
{
    PS_OUT_LIGHT Out = (PS_OUT_LIGHT) 0;
    
    vector vMask = g_MaskTexture[0].Sample(DefaultSampler, In.vTexcoord);
    
    vector vDiffuse = g_DiffuseTexture[0].Sample(DefaultSampler, In.vTexcoord);
    vector vMaskDiffiuse = g_DiffuseTexture[1].Sample(DefaultSampler, In.vTexcoord);

    if (g_HasMask)
    {
        Out.vDiffuse = vDiffuse * vMask.r + vDiffuse * (1.f - vMask.r);
        Out.vDiffuse = Out.vDiffuse * vMask.g + vMaskDiffiuse * (1.f - vMask.g);

    }
    else
    {
        Out.vDiffuse = vDiffuse;
    }
    if (Out.vDiffuse.a < 0.1f)
        discard;
    
    Out.vDiffuse.w = 1.f;
    
    Out.vPBR.y = g_fGlobalStaticRoughness;
    Out.vPBR.x = g_fGlobalStaticMetallic;
    
    if (g_IsDynamicObject)
    {
        Out.vDepth.z = 1.f;
        Out.vPBR.z = 1.f;
    }
    
    float4 vNormal;
    
    if (g_HasNormal)
    {
        if (g_HasMask)
        {
            vector vDefaultNormal = g_NormalTexture[0].Sample(DefaultSampler, In.vTexcoord);
        
            float4 vNormal1 = normalize(vDefaultNormal * 2.f - 1.f);
            if (vDefaultNormal.x > vDefaultNormal.z && vDefaultNormal.y > vDefaultNormal.z)
                vNormal1.z = sqrt(1.f - saturate(dot(vDefaultNormal.xy, vDefaultNormal.xy)));

            //vector vMaskNormal = g_NormalTexture[1].Sample(DefaultSampler, In.vTexcoord);
        
            //float4 vNormal2 = normalize(vMaskNormal * 2.f - 1.f);
            //if (vMaskNormal.x > vMaskNormal.z && vMaskNormal.y > vMaskNormal.z)
            //    vNormal2.z = sqrt(1.f - saturate(dot(vMaskNormal.xy, vMaskNormal.xy)));

            //if (vMask.r == 0.f && vMask.g == 0.f)
            //    vNormal = vNormal1;
            //else
            //{
            //vNormal = vNormal1 * (vMask.r) + vNormal1 * (1.f - vMask.r);
            //vNormal = vNormal * vMask.g + vNormal2 * (1.f - vMask.g);
            //}
            
            vNormal = vNormal1 * (vMask.r) + vNormal1 * (1.f - vMask.r);
        }
        else
        {
            vector vDefaultNormal = g_NormalTexture[0].Sample(DefaultSampler, In.vTexcoord);
			
            Out.vPBR.x = vDefaultNormal.b;
            Out.vPBR.y = vDefaultNormal.a;
	        
            vNormal = normalize(vDefaultNormal * 2.f - 1.f);
            if (vDefaultNormal.x > vDefaultNormal.z && vDefaultNormal.y > vDefaultNormal.z)
                vNormal.z = sqrt(1.f - saturate(dot(vDefaultNormal.xy, vDefaultNormal.xy)));
        }
        //vector vNormalDesc = vDefaultNormal * (1.f - vMask.r) + vMaskNormal * vMask.g;
        
        //vNormal = normalize(vNormalDesc * 2.f - 1.f);
        //if (vNormalDesc.x > vNormalDesc.z && vNormalDesc.y > vNormalDesc.z)
        //    vNormal.z = sqrt(1.f - saturate(dot(vNormalDesc.xy, vNormalDesc.xy)));

        //vNormal = vNormal1;  

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
    Out.vNormal = float4(vNormal.xyz, 1.f);
    Out.vDepth.x = In.vProjPos.z / In.vProjPos.w;
    Out.vDepth.y = In.vProjPos.w;
    
    Out.vSSS.z = In.vProjPos.z / In.vProjPos.w;
    Out.vSSS.w = In.vProjPos.w;
    
    Out.vDepth.w = 1.f;
    
    return Out;
}

PS_OUT_EMISSIVE PS_MAIN_TEST(PS_IN In)
{
    PS_OUT_EMISSIVE Out = (PS_OUT_EMISSIVE) 0;
    
    vector vMask = g_MaskTexture[0].Sample(DefaultSampler, In.vTexcoord);
    
    
    vector vDiffuse = g_DiffuseTexture[0].Sample(DefaultSampler, In.vTexcoord);
    vector vMaskDiffiuse = g_DiffuseTexture[1].Sample(DefaultSampler, In.vTexcoord);
    
    //Out.vDiffuse = vDiffuse * (1.f - vMask) + (vMaskDiffiuse * float4(0.1f, 0.f, 1.f, 1.f)) * vMask;
    Out.vDiffuse = vDiffuse * (1.f - vMask) + vMaskDiffiuse * vMask;
    if (Out.vDiffuse.a < 0.1f)
        discard;
    
    float3 vNormal;
    
    if (g_HasNormal)
    {
        vector vNormalDesc = g_NormalTexture[0].Sample(DefaultSampler, In.vTexcoord);
        vNormal = vNormalDesc.xyz * 2.f - 1.f;
        float3x3 WorldMatrix = float3x3(In.vTangent.xyz, In.vBinormal.xyz * -1.f, In.vNormal.xyz);
    
        vNormal = mul(vNormal, WorldMatrix);
    }
    else
        vNormal = In.vNormal.xyz;
        
    Out.vNormal = vector(vNormal * 0.5f + 0.5f, 0.f);
    Out.vSpecular = g_vMatrlSpecular;
    Out.vAmbient = g_vMatrlAmbient;
    return Out;
}

PS_OUT_LIGHT PS_MAIN_COLOR(PS_IN In)
{
    PS_OUT_LIGHT Out = (PS_OUT_LIGHT) 0;
    
    vector vMask = g_MaskTexture[0].Sample(DefaultSampler, In.vTexcoord);
    
    vector vDiffuse = g_DiffuseTexture[0].Sample(DefaultSampler, In.vTexcoord);
    vector vMaskDiffiuse = g_DiffuseTexture[1].Sample(DefaultSampler, In.vTexcoord);

    if (g_HasMask)
    {
        Out.vDiffuse = vDiffuse * vMask.r + vDiffuse * (1.f - vMask.r);
        Out.vDiffuse = Out.vDiffuse * vMask.g + vMaskDiffiuse * (1.f - vMask.g);

    }
    else
    {
        Out.vDiffuse = vDiffuse;
    }
    if (Out.vDiffuse.a < 0.1f)
        discard;
    
    Out.vDiffuse.xyz *= g_vDiffuseColor.xyz;
    Out.vDiffuse.w = 1.f;
    
    Out.vPBR.y = g_fGlobalStaticRoughness;
    Out.vPBR.x = g_fGlobalStaticMetallic;
    
    if (g_IsDynamicObject)
    {
        Out.vDepth.z = 1.f;
        Out.vPBR.z = 1.f;
    }

    float4 vNormal;
    
    if (g_HasNormal)
    {
        if (g_HasMask)
        {
            vector vDefaultNormal = g_NormalTexture[0].Sample(DefaultSampler, In.vTexcoord);
        
            float4 vNormal1 = normalize(vDefaultNormal * 2.f - 1.f);
            if (vDefaultNormal.x > vDefaultNormal.z && vDefaultNormal.y > vDefaultNormal.z)
                vNormal1.z = sqrt(1.f - saturate(dot(vDefaultNormal.xy, vDefaultNormal.xy)));
            
            vNormal = vNormal1 * (vMask.r) + vNormal1 * (1.f - vMask.r);
        }
        else
        {
            vector vDefaultNormal = g_NormalTexture[0].Sample(DefaultSampler, In.vTexcoord);
			
            Out.vPBR.x = vDefaultNormal.b;
            Out.vPBR.y = vDefaultNormal.a;
	        
            vNormal = normalize(vDefaultNormal * 2.f - 1.f);
            if (vDefaultNormal.x > vDefaultNormal.z && vDefaultNormal.y > vDefaultNormal.z)
                vNormal.z = sqrt(1.f - saturate(dot(vDefaultNormal.xy, vDefaultNormal.xy)));
        }

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
    Out.vNormal = float4(vNormal.xyz, 1.f);
    Out.vDepth.x = In.vProjPos.z / In.vProjPos.w;
    Out.vDepth.y = In.vProjPos.w;
    
    Out.vSSS.z = In.vProjPos.z / In.vProjPos.w;
    Out.vSSS.w = In.vProjPos.w;
    
    Out.vDepth.w = 1.f;
    
    return Out;
}

PS_OUT_LIGHT PS_MAIN_COLOROVERLAP(PS_IN In)
{
    PS_OUT_LIGHT Out = (PS_OUT_LIGHT) 0;
    
    vector vMask = g_MaskTexture[0].Sample(DefaultSampler, In.vTexcoord);
    
    vector vDiffuse = g_DiffuseTexture[0].Sample(DefaultSampler, In.vTexcoord);
    vector vMaskDiffiuse = g_DiffuseTexture[1].Sample(DefaultSampler, In.vTexcoord);

    if (g_HasMask)
    {
        Out.vDiffuse = vDiffuse * vMask.r + vDiffuse * (1.f - vMask.r);
        Out.vDiffuse = Out.vDiffuse * vMask.g + vMaskDiffiuse * (1.f - vMask.g);

    }
    else
    {
        Out.vDiffuse = vDiffuse;
    }
    if (Out.vDiffuse.a < 0.1f)
        discard;
    
    Out.vDiffuse.xyz = g_vDiffuseColor.xyz;
    Out.vDiffuse.w = 1.f;
    
    Out.vPBR.y = g_fGlobalStaticRoughness;
    Out.vPBR.x = g_fGlobalStaticMetallic;
    
    if (g_IsDynamicObject)
    {
        Out.vDepth.z = 1.f;
        Out.vPBR.z = 1.f;
    }

    float4 vNormal;
    
    if (g_HasNormal)
    {
        if (g_HasMask)
        {
            vector vDefaultNormal = g_NormalTexture[0].Sample(DefaultSampler, In.vTexcoord);
        
            float4 vNormal1 = normalize(vDefaultNormal * 2.f - 1.f);
            if (vDefaultNormal.x > vDefaultNormal.z && vDefaultNormal.y > vDefaultNormal.z)
                vNormal1.z = sqrt(1.f - saturate(dot(vDefaultNormal.xy, vDefaultNormal.xy)));
            
            vNormal = vNormal1 * (vMask.r) + vNormal1 * (1.f - vMask.r);
        }
        else
        {
            vector vDefaultNormal = g_NormalTexture[0].Sample(DefaultSampler, In.vTexcoord);
			
            Out.vPBR.x = vDefaultNormal.b;
            Out.vPBR.y = vDefaultNormal.a;
	        
            vNormal = normalize(vDefaultNormal * 2.f - 1.f);
            if (vDefaultNormal.x > vDefaultNormal.z && vDefaultNormal.y > vDefaultNormal.z)
                vNormal.z = sqrt(1.f - saturate(dot(vDefaultNormal.xy, vDefaultNormal.xy)));
        }

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
    Out.vNormal = float4(vNormal.xyz, 1.f);
    Out.vDepth.x = In.vProjPos.z / In.vProjPos.w;
    Out.vDepth.y = In.vProjPos.w;
    
    Out.vSSS.z = In.vProjPos.z / In.vProjPos.w;
    Out.vSSS.w = In.vProjPos.w;
    
    Out.vDepth.w = 1.f;
    
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
    pass Test // 1
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_TEST();
    }
    pass Shake // 2
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_COLOR();
    }
    pass NonShake // 3
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_NONSHAKE();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_COLOR();
    }
    pass ColorOverlapNonShake // 4
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_NONSHAKE();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_COLOROVERLAP();
    }

    pass ColorOverlapShake // 5
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_COLOROVERLAP();
    }
}