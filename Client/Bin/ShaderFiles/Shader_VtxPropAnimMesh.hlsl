#include "Engine_Shader_Defines.hlsli"

matrix g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;
float g_fLightFar;

Texture2D g_DiffuseTexture;
Texture2D g_SecondDiffuseTexture;
Texture2D g_NormalTexture;
Texture2D g_MaskTexture[4] : register(t8);

matrix g_ShadowViewMatrix[4];
matrix g_ShadowProjMatrix[4];

float g_fOutLineRadius = 0.001f;
float4 g_vOutLineColor = float4(0.3f, 0.15f, 0.f, 1.f);

float g_fDissolveRate = 0.f;
float g_fFlowRate = 0.f;

matrix g_BoneMatrices[512];
bool g_HasNormal = false;

cbuffer GlobalConstants
{
    int g_iNumBlendWeightsToUse = 2; 
    float g_fTime;
    float g_fEmissiveIntensity = 1.f;
    float4 g_vDissolveColor = float4(0.407f, 0.619f, 1.f, 1.f);
    float4 g_vEmissiveColor; 
    float2 g_vScrollSpeed;
    
}


struct VS_IN
{
    float3 vPosition : POSITION;
    float3 vNormal : NORMAL;
    float3 vTangent : TANGENT;
    float3 vBinormal : BINORMAL;
    uint4 vBlendIndex : BLENDINDEX;
    float4 vBlendWeight : BLENDWEIGHT;
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
    
    matrix matBone, matBW, matWV, matWVP;
    
    uint iX = max(In.vBlendIndex.x, g_iNumBlendWeightsToUse);
    uint iY = max(In.vBlendIndex.y, g_iNumBlendWeightsToUse);
    uint iZ = max(In.vBlendIndex.z, g_iNumBlendWeightsToUse);
    uint iW = max(In.vBlendIndex.w, g_iNumBlendWeightsToUse);
    
    float fWeightW = 1.f - (In.vBlendWeight.x + In.vBlendWeight.y + In.vBlendWeight.z);
    matBone =
    g_BoneMatrices[In.vBlendIndex.x] * In.vBlendWeight.x +
    g_BoneMatrices[In.vBlendIndex.y] * In.vBlendWeight.y +
    g_BoneMatrices[In.vBlendIndex.z] * In.vBlendWeight.z +
    g_BoneMatrices[In.vBlendIndex.w] * fWeightW;
    
    float4 vPosition = mul(float4(In.vPosition, 1.f), matBone);
    float4 vNormal = mul(float4(In.vNormal, 0.f), matBone);
    float4 vTangent = mul(float4(In.vTangent, 0.f), matBone);
    float4 vBinormal = mul(float4(In.vBinormal, 0.f), matBone);
    matWV = mul(g_WorldMatrix, g_ViewMatrix);
    matWVP = mul(matWV, g_ProjMatrix);
    
    Out.vPosition = mul(vPosition, matWVP);
    Out.vNormal = normalize(mul(vNormal, g_WorldMatrix));
    Out.vTangent = normalize(mul(vTangent, g_WorldMatrix));
    Out.vBinormal = normalize(mul(vBinormal, g_WorldMatrix));
    Out.vTexcoord = In.vTexcoord;
    Out.vProjPos = mul(vPosition, matWVP);

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
 
    Out.vDiffuse = g_DiffuseTexture.Sample(DefaultSampler, In.vTexcoord);
    Out.vNormal = In.vNormal * 0.5f + 0.5f;
    
    Out.vDepth.x = In.vProjPos.z / In.vProjPos.w;
    Out.vDepth.y = In.vProjPos.w;
    
    Out.vPBR.y = 0.2f;
    Out.vPBR.z = 1.f;
    
    return Out;
}

PS_OUT PS_NORMALTEX(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;

    Out.vDiffuse = g_DiffuseTexture.Sample(DefaultSampler, In.vTexcoord);
    
    vector NormalDesc = g_NormalTexture.Sample(DefaultSampler, In.vTexcoord);
    //float3 vNormal = NormalDesc.xyz * 2.f - 1.f;
    float3 vNormal = NormalDesc.xyz;
    
    float3x3 WorldMatrix = float3x3(In.vTangent.xyz, In.vBinormal.xyz * -1.f, In.vNormal.xyz);
    Out.vNormal = vector(mul(vNormal, WorldMatrix) * 0.5f + 0.5f, 0.f);
    
    Out.vDepth.x = In.vProjPos.z / In.vProjPos.w;
    Out.vDepth.y = In.vProjPos.w;
    Out.vPBR.y = 0.2f;
    Out.vPBR.z = 1.f;
    
    return Out;
}

PS_OUT PS_DEFAULT_WEAPON(PS_IN In) // Dissolve 추가.
{
    PS_OUT Out = (PS_OUT) 0;

    Out.vDiffuse = g_DiffuseTexture.Sample(DefaultSampler, In.vTexcoord);
    
    float4 vNormal = 0.f;
    
    if (g_HasNormal)
    {
        float4 vNormalDesc = g_NormalTexture.Sample(DefaultSampler, In.vTexcoord);
        vNormal = normalize(vNormalDesc * 2.f - 1.f);
        
        if (vNormalDesc.x > vNormalDesc.z && vNormalDesc.y > vNormalDesc.z)
            vNormal.z = sqrt(1.f - saturate(dot(vNormalDesc.xy, vNormalDesc.xy))); // 그대로 사용
            
        float3 vTangent = In.vTangent.xyz;
        float3 vBinormal = In.vBinormal.xyz * -1.f;
        float3 vInNormal = In.vNormal.xyz;
        
        float3x3 WorldMatrix;
        WorldMatrix = float3x3(vTangent, vBinormal, vInNormal);
        vNormal.xyz = normalize(mul(vNormal.xyz, WorldMatrix));
        
        Out.vPBR.x = vNormalDesc.b; // PBR.X = 노말 텍스처 Blue, Z 값
        Out.vPBR.y = vNormalDesc.a; // PBR.y = 노말 텍스처 Alpha 값
    }
    else
    {
        vNormal = In.vNormal;
        Out.vPBR.x = g_fGlobalDynamicMetallic; // PBR.X = 노말 텍스처 Blue, Z 값
        Out.vPBR.y = g_fGlobalDynamicRoughness; // PBR.y = 노말 텍스처 Alpha 값
    }
    
    Out.vPBR.z = 1.f; // PBR.z = STATIC = 0.f , DYNAMIC = 1.f

    vNormal.xyz = vNormal * 0.5f + 0.5f;
    
    Out.vNormal = vNormal;
    Out.vDepth.x = In.vProjPos.z / In.vProjPos.w;
    Out.vDepth.y = In.vProjPos.w;
    Out.vDepth.z = 1.f;
    
    return Out;
}

PS_OUT PS_DISSOLVE_GALBRENAWEAPON(PS_IN In) // Dissolve 추가.
{
    PS_OUT Out = (PS_OUT) 0;

    // 1. 디졸브 텍스처(g_MaskTexture[0])에서 마스크 값을 샘플링.
    float fDissolveMask = g_MaskTexture[0].Sample(DefaultSampler, In.vTexcoord).r;
    
    Out.vDiffuse = g_DiffuseTexture.Sample(DefaultSampler, In.vTexcoord);
    
    if (fDissolveMask.r - g_fDissolveRate < 0.f) // 0.f 면 Discard;
        discard;
    
    //clip(fDissolveMask.r - g_fDissolveRate);
    
    float4 vNormal = 0.f;
    
    if (g_HasNormal)
    {
        float4 vNormalDesc = g_NormalTexture.Sample(DefaultSampler, In.vTexcoord);
        vNormal = normalize(vNormalDesc * 2.f - 1.f);
        
        if (vNormalDesc.x > vNormalDesc.z && vNormalDesc.y > vNormalDesc.z)
            vNormal.z = sqrt(1.f - saturate(dot(vNormalDesc.xy, vNormalDesc.xy))); // 그대로 사용
            
        float3 vTangent = In.vTangent.xyz;
        float3 vBinormal = In.vBinormal.xyz * -1.f;
        float3 vInNormal = In.vNormal.xyz;
        
        float3x3 WorldMatrix;
        WorldMatrix = float3x3(vTangent, vBinormal, vInNormal);
        vNormal.xyz = normalize(mul(vNormal.xyz, WorldMatrix));
        
        
        Out.vPBR.x = vNormalDesc.b; // PBR.X = 노말 텍스처 Blue, Z 값
        Out.vPBR.y = vNormalDesc.a; // PBR.y = 노말 텍스처 Alpha 값
    }
    else
    {
        vNormal = In.vNormal;
        Out.vPBR.x = g_fGlobalDynamicMetallic; // PBR.X = 노말 텍스처 Blue, Z 값
        Out.vPBR.y = g_fGlobalDynamicRoughness; // PBR.y = 노말 텍스처 Alpha 값
    }
    
    
    float3 vColor = float3(0.407f, 0.619f, 1.f);
    if (fDissolveMask.r - g_fDissolveRate < 0.3f) // 0.3f 보다 작은 (사라지기 직전)
        Out.vDiffuse.rgb = vColor * 3.f; // 이러면 쨍하게 들어간다. 
    
    // Emissive 0.3f 초과인 얘들은 Emissive가 기본으로 들어가고, 0.3f 이하인 얘들은 Emissive가 지정한 색상에 더 크게 적용된다.
    float fWeight = Luminance(Out.vDiffuse.xyz);

    if (fWeight >= g_fEmissiveThreshold)
        Out.vEmissive = float4(Out.vDiffuse.xyz, 1.f);
    Out.vEmissive.xyz *= Out.vDiffuse.a;
    
    Out.vPBR.z = 1.f; // PBR.z = STATIC = 0.f , DYNAMIC = 1.f
    

    vNormal.xyz = vNormal * 0.5f + 0.5f;
    
    Out.vNormal = vNormal;
    Out.vDepth.x = In.vProjPos.z / In.vProjPos.w;
    Out.vDepth.y = In.vProjPos.w;
    Out.vDepth.z = 1.f;
    
    return Out;
}


PS_OUT PS_ENERGY_BLADE(PS_IN In) // Dissolve 추가.
{
    PS_OUT Out = (PS_OUT) 0;
    Out.vDiffuse = g_DiffuseTexture.Sample(DefaultSampler, In.vTexcoord);
    
    float4 vNormal = 0.f;
    
    if (g_HasNormal)
    {
        float4 vNormalDesc = g_NormalTexture.Sample(DefaultSampler, In.vTexcoord);
        vNormal = normalize(vNormalDesc * 2.f - 1.f);
        
        if (vNormalDesc.x > vNormalDesc.z && vNormalDesc.y > vNormalDesc.z)
            vNormal.z = sqrt(1.f - saturate(dot(vNormalDesc.xy, vNormalDesc.xy))); // 그대로 사용
            
            
        float3 vTangent = In.vTangent.xyz;
        float3 vBinormal = In.vBinormal.xyz * -1.f;
        float3 vInNormal = In.vNormal.xyz;
        
        float3x3 WorldMatrix;
        WorldMatrix = float3x3(vTangent, vBinormal, vInNormal);
        vNormal.xyz = normalize(mul(vNormal.xyz, WorldMatrix));
        
        Out.vPBR.x = vNormalDesc.b; // PBR.X = 노말 텍스처 Blue, Z 값
        Out.vPBR.y = vNormalDesc.a; // PBR.y = 노말 텍스처 Alpha 값
    }
    else
    {
        vNormal = In.vNormal;
        Out.vPBR.x = g_fGlobalDynamicMetallic; // PBR.X = 노말 텍스처 Blue, Z 값
        Out.vPBR.y = g_fGlobalDynamicRoughness; // PBR.y = 노말 텍스처 Alpha 값
    }
    
    float fBladeMask = step(0.88f, In.vTexcoord.y);
    
    // 검날 부분을 제외한 부분은 밝기를 제어.
    float fGlowRatio = lerp(0.05f, 1.f, fBladeMask);
    
    // 최종 발광 색상 계산.
    
    float3 vColor = g_vEmissiveColor.rgb;
    float fIntensity = g_fEmissiveIntensity; // 블룸먹도록 증폭.
    
    //float fWeight = Luminance(vColor);
    //if (fWeight >= g_fEmissiveThreshold)
    //    Out.vEmissive = float4(vColor * fIntensity * fGlowRatio, 1.0f);
    Out.vEmissive = float4(vColor * fIntensity * fGlowRatio, 1.0f);

    vNormal.xyz = vNormal * 0.5f + 0.5f;
    Out.vNormal = vNormal;
    
    Out.vDepth.x = In.vProjPos.z / In.vProjPos.w;
    Out.vDepth.y = In.vProjPos.w;
    Out.vDepth.z = 1.f;
    
    Out.vPBR.z = 1.f; // PBR.z = STATIC = 0.f , DYNAMIC = 1.f
    
    return Out;
}


PS_OUT PS_AUGUSTA_HEADPROP(PS_IN In) // Dissolve 추가.
{
    PS_OUT Out = (PS_OUT) 0;
    Out.vDiffuse = g_DiffuseTexture.Sample(DefaultSampler, In.vTexcoord);
    
    float4 vNormal = 0.f;
    
    if (g_HasNormal)
    {
        float4 vNormalDesc = g_NormalTexture.Sample(DefaultSampler, In.vTexcoord);
        vNormal = normalize(vNormalDesc * 2.f - 1.f);
        
        if (vNormalDesc.x > vNormalDesc.z && vNormalDesc.y > vNormalDesc.z)
            vNormal.z = sqrt(1.f - saturate(dot(vNormalDesc.xy, vNormalDesc.xy))); // 그대로 사용

        float3 vTangent = In.vTangent.xyz;
        float3 vBinormal = In.vBinormal.xyz * -1.f;
        float3 vInNormal = In.vNormal.xyz;
        
        float3x3 WorldMatrix;
        WorldMatrix = float3x3(vTangent, vBinormal, vInNormal);
        vNormal.xyz = normalize(mul(vNormal.xyz, WorldMatrix));
        
        Out.vPBR.x = vNormalDesc.b;
        Out.vPBR.y = vNormalDesc.a;
    }
    else
    {
        vNormal = In.vNormal;
        Out.vPBR.x = g_fGlobalDynamicMetallic;  
        Out.vPBR.y = g_fGlobalDynamicRoughness; 
    }
    
    
    float3 vEmissiveColor = g_vEmissiveColor.rgb;
    float3 vWhiteColor = float3(1.0f, 1.0f, 1.0f); // 가장 밝은 부분 흰색주기.
    float fIntensity = g_fEmissiveIntensity; // 블룸먹도록 증폭.
    //Out.vEmissive = float4(vEmissiveColor * fIntensity, 1.f);
    
    // 1. 텍스쳐 밝기 구하기.
    float fLuminance = dot(Out.vDiffuse.rgb, float3(0.299, 0.587, 0.114));
    
    // 2. 색상 혼합.
    float3 vFinalColor = lerp(vEmissiveColor, vWhiteColor, pow(fLuminance, 2.0f));
    
    // 3. 발광 
    float3 vEmissiveResult = Out.vDiffuse.rgb * vFinalColor * fIntensity;
    
    //float3 vFinalEmissive = Out.vDiffuse.rgb * vEmissiveColor * fIntensity * step(0.5f, fLuminance);
    Out.vEmissive = float4(vEmissiveResult, 1.0f);
    
    
    vNormal.xyz = vNormal * 0.5f + 0.5f;
    Out.vNormal = vNormal;
    
    Out.vDepth.x = In.vProjPos.z / In.vProjPos.w;
    Out.vDepth.y = In.vProjPos.w;
    Out.vDepth.z = 1.f;
    Out.vPBR.z = 1.f;
    
    return Out;
}

PS_OUT PS_DISSOLVE_AUGUSTAWEAPON(PS_IN In) // Dissolve 추가.
{
    PS_OUT Out = (PS_OUT) 0;

    // 1. 디졸브 텍스처(g_MaskTexture[0])에서 마스크 값을 샘플링.
    float fDissolveMask = g_MaskTexture[0].Sample(DefaultSampler, In.vTexcoord).r;
    
    Out.vDiffuse = g_DiffuseTexture.Sample(DefaultSampler, In.vTexcoord);
    
    if (fDissolveMask.r - g_fDissolveRate < 0.f) // 0.f 면 Discard;
        discard;
    
    float4 vNormal = 0.f;
    
    if (g_HasNormal)
    {
        float4 vNormalDesc = g_NormalTexture.Sample(DefaultSampler, In.vTexcoord);
        vNormal = normalize(vNormalDesc * 2.f - 1.f);
        
        if (vNormalDesc.x > vNormalDesc.z && vNormalDesc.y > vNormalDesc.z)
            vNormal.z = sqrt(1.f - saturate(dot(vNormalDesc.xy, vNormalDesc.xy))); // 그대로 사용
            
        float3 vTangent = In.vTangent.xyz;
        float3 vBinormal = In.vBinormal.xyz * -1.f;
        float3 vInNormal = In.vNormal.xyz;
        
        float3x3 WorldMatrix;
        WorldMatrix = float3x3(vTangent, vBinormal, vInNormal);
        vNormal.xyz = normalize(mul(vNormal.xyz, WorldMatrix));
        
        
        Out.vPBR.x = vNormalDesc.b; // PBR.X = 노말 텍스처 Blue, Z 값
        Out.vPBR.y = vNormalDesc.a; // PBR.y = 노말 텍스처 Alpha 값
    }
    else
    {
        vNormal = In.vNormal;
        Out.vPBR.x = g_fGlobalDynamicMetallic; // PBR.X = 노말 텍스처 Blue, Z 값
        Out.vPBR.y = g_fGlobalDynamicRoughness; // PBR.y = 노말 텍스처 Alpha 값
    }
    
    
    float3 vColor = g_vDissolveColor.rgb;
    if (fDissolveMask.r - g_fDissolveRate < 0.3f) // 0.3f 보다 작은 (사라지기 직전)
        Out.vDiffuse.rgb = vColor * 3.f; // 이러면 쨍하게 들어간다. 
    
    // Emissive 0.3f 초과인 얘들은 Emissive가 기본으로 들어가고, 0.3f 이하인 얘들은 Emmisive가 지정한 색상에 더 크게 적용된다.
    float fWeight = Luminance(Out.vDiffuse.xyz);

    if (fWeight >= g_fEmissiveThreshold)
        Out.vEmissive = float4(Out.vDiffuse.xyz, 1.f);
    Out.vEmissive.xyz *= Out.vDiffuse.a;
    
    Out.vPBR.z = 1.f; // PBR.z = STATIC = 0.f , DYNAMIC = 1.f
    

    vNormal.xyz = vNormal * 0.5f + 0.5f;
    
    Out.vNormal = vNormal;
    Out.vDepth.x = In.vProjPos.z / In.vProjPos.w;
    Out.vDepth.y = In.vProjPos.w;
    Out.vDepth.z = 1.f;
    
    return Out;
}

PS_OUT PS_DISSOLVE_ROVERWEAPON(PS_IN In) // Dissolve 추가.
{
    PS_OUT Out = (PS_OUT) 0;

    // 1. 디졸브 텍스처(g_MaskTexture[0])에서 마스크 값을 샘플링.
    float fDissolveMask = g_MaskTexture[0].Sample(DefaultSampler, In.vTexcoord).r;
    
    Out.vDiffuse = g_DiffuseTexture.Sample(DefaultSampler, In.vTexcoord);
    
    if (fDissolveMask.r - g_fDissolveRate < 0.f) // 0.f 면 Discard;
        discard;
    
    float4 vNormal = 0.f;
    
    if (g_HasNormal)
    {
        float4 vNormalDesc = g_NormalTexture.Sample(DefaultSampler, In.vTexcoord);
        vNormal = normalize(vNormalDesc * 2.f - 1.f);
        
        if (vNormalDesc.x > vNormalDesc.z && vNormalDesc.y > vNormalDesc.z)
            vNormal.z = sqrt(1.f - saturate(dot(vNormalDesc.xy, vNormalDesc.xy))); // 그대로 사용
            
        float3 vTangent = In.vTangent.xyz;
        float3 vBinormal = In.vBinormal.xyz * -1.f;
        float3 vInNormal = In.vNormal.xyz;
        
        float3x3 WorldMatrix;
        WorldMatrix = float3x3(vTangent, vBinormal, vInNormal);
        vNormal.xyz = normalize(mul(vNormal.xyz, WorldMatrix));
        
        
        Out.vPBR.x = vNormalDesc.b; // PBR.X = 노말 텍스처 Blue, Z 값
        Out.vPBR.y = vNormalDesc.a; // PBR.y = 노말 텍스처 Alpha 값
    }
    else
    {
        vNormal = In.vNormal;
        Out.vPBR.x = g_fGlobalDynamicMetallic; // PBR.X = 노말 텍스처 Blue, Z 값
        Out.vPBR.y = g_fGlobalDynamicRoughness; // PBR.y = 노말 텍스처 Alpha 값
    }
    
    
    float3 vColor = g_vDissolveColor.rgb;
    float fIntensity = g_fEmissiveIntensity;
    if (fDissolveMask.r - g_fDissolveRate < 0.3f) // 0.3f 보다 작은 (사라지기 직전)
        Out.vDiffuse.rgb = vColor * fIntensity; // 이러면 쨍하게 들어간다. 
    
    // Emissive 0.3f 초과인 얘들은 Emissive가 기본으로 들어가고, 0.3f 이하인 얘들은 Emmisive가 지정한 색상에 더 크게 적용된다.
    float fWeight = Luminance(Out.vDiffuse.xyz);

    if (fWeight >= g_fEmissiveThreshold)
        Out.vEmissive = float4(Out.vDiffuse.xyz, 1.f);
    Out.vEmissive.xyz *= Out.vDiffuse.a;
    
    Out.vPBR.z = 1.f; // PBR.z = STATIC = 0.f , DYNAMIC = 1.f
    

    vNormal.xyz = vNormal * 0.5f + 0.5f;
    
    Out.vNormal = vNormal;
    Out.vDepth.x = In.vProjPos.z / In.vProjPos.w;
    Out.vDepth.y = In.vProjPos.w;
    Out.vDepth.z = 1.f;
    
    return Out;
}

PS_OUT PS_AUGUSTA_BURSTWEAPON_EFFECT(PS_IN In) // Dissolve 추가.
{
    PS_OUT Out = (PS_OUT) 0;
    
    // 1. 메인 텍스쳐 샘플링.
    float4 vDiffuse = g_DiffuseTexture.Sample(DefaultSampler, In.vTexcoord);
    Out.vDiffuse = vDiffuse;
    
    // 2. 투명도 처리.
    clip(vDiffuse.a - 0.1f);
    
    float4 vNormal = 0.f;
    
    if (g_HasNormal)
    {
        float4 vNormalDesc = g_NormalTexture.Sample(DefaultSampler, In.vTexcoord);
        vNormal = normalize(vNormalDesc * 2.f - 1.f);
        
        if (vNormalDesc.x > vNormalDesc.z && vNormalDesc.y > vNormalDesc.z)
            vNormal.z = sqrt(1.f - saturate(dot(vNormalDesc.xy, vNormalDesc.xy))); // 그대로 사용

        float3 vTangent = In.vTangent.xyz;
        float3 vBinormal = In.vBinormal.xyz * -1.f;
        float3 vInNormal = In.vNormal.xyz;
        
        float3x3 WorldMatrix;
        WorldMatrix = float3x3(vTangent, vBinormal, vInNormal);
        vNormal.xyz = normalize(mul(vNormal.xyz, WorldMatrix));
        
        Out.vPBR.x = vNormalDesc.b;
        Out.vPBR.y = vNormalDesc.a;
    }
    else
    {
        vNormal = In.vNormal;
        Out.vPBR.x = g_fGlobalDynamicMetallic;
        Out.vPBR.y = g_fGlobalDynamicRoughness;
    }
    
    // Emissive (발광) 색상 처리
    
    float3 vBaseTextureColor = vDiffuse.rgb;
    float3 vColor = g_vEmissiveColor.rgb;
    float fIntensity = g_fEmissiveIntensity; // 블룸먹도록 증폭.
    
    float3 vFinalEmissive = vBaseTextureColor * vColor * fIntensity;
    
    Out.vEmissive = float4(vFinalEmissive, 1.0f);
    
    
    vNormal.xyz = vNormal * 0.5f + 0.5f;
    Out.vNormal = vNormal;
    
    Out.vDepth.x = In.vProjPos.z / In.vProjPos.w;
    Out.vDepth.y = In.vProjPos.w;
    Out.vDepth.z = 1.f;
    Out.vPBR.z = 1.f;
    
    return Out;
}

PS_OUT PS_AUGUSTA_BURSTWEAPON(PS_IN In) // Dissolve 추가.
{
    PS_OUT Out = (PS_OUT) 0;

    Out.vDiffuse = g_DiffuseTexture.Sample(DefaultSampler, In.vTexcoord);
    
    float4 vMask = g_MaskTexture[0].Sample(DefaultSampler, In.vTexcoord);
    
    float3 vBaseColor = Out.vDiffuse.rgb;
    float fGlowMask = vMask.r;
    float fBaseRatio = 0.3f;
    float fFinalFactor = lerp(fBaseRatio, 1.0f, fGlowMask);
    
    float3 vFinalEmissive = vBaseColor * fFinalFactor * g_fEmissiveIntensity;
    Out.vEmissive = float4(vFinalEmissive, 1.0f);
    
    float4 vNormal = 0.f;
    
    if (g_HasNormal)
    {
        float4 vNormalDesc = g_NormalTexture.Sample(DefaultSampler, In.vTexcoord);
        vNormal = normalize(vNormalDesc * 2.f - 1.f);
        
        if (vNormalDesc.x > vNormalDesc.z && vNormalDesc.y > vNormalDesc.z)
            vNormal.z = sqrt(1.f - saturate(dot(vNormalDesc.xy, vNormalDesc.xy))); // 그대로 사용
            
        float3 vTangent = In.vTangent.xyz;
        float3 vBinormal = In.vBinormal.xyz * -1.f;
        float3 vInNormal = In.vNormal.xyz;
        
        float3x3 WorldMatrix;
        WorldMatrix = float3x3(vTangent, vBinormal, vInNormal);
        vNormal.xyz = normalize(mul(vNormal.xyz, WorldMatrix));
        
        Out.vPBR.x = vNormalDesc.b; // PBR.X = 노말 텍스처 Blue, Z 값
        Out.vPBR.y = vNormalDesc.a; // PBR.y = 노말 텍스처 Alpha 값
    }
    else
    {
        vNormal = In.vNormal;
        Out.vPBR.x = g_fGlobalDynamicMetallic; // PBR.X = 노말 텍스처 Blue, Z 값
        Out.vPBR.y = g_fGlobalDynamicRoughness; // PBR.y = 노말 텍스처 Alpha 값
    }
    
    Out.vPBR.z = 1.f; // PBR.z = STATIC = 0.f , DYNAMIC = 1.f

    vNormal.xyz = vNormal * 0.5f + 0.5f;
    
    Out.vNormal = vNormal;
    Out.vDepth.x = In.vProjPos.z / In.vProjPos.w;
    Out.vDepth.y = In.vProjPos.w;
    Out.vDepth.z = 1.f;
    
    return Out;
}

PS_OUT PS_YUNO_WEAPON(PS_IN In) // Dissolve 추가.
{
    PS_OUT Out = (PS_OUT) 0;

    Out.vDiffuse = g_DiffuseTexture.Sample(DefaultSampler, In.vTexcoord);
    
    float4 vNormal = 0.f;
    
    if (g_HasNormal)
    {
        float4 vNormalDesc = g_NormalTexture.Sample(DefaultSampler, In.vTexcoord);
        vNormal = normalize(vNormalDesc * 2.f - 1.f);
        
        if (vNormalDesc.x > vNormalDesc.z && vNormalDesc.y > vNormalDesc.z)
            vNormal.z = sqrt(1.f - saturate(dot(vNormalDesc.xy, vNormalDesc.xy))); // 그대로 사용
            
        float3 vTangent = In.vTangent.xyz;
        float3 vBinormal = In.vBinormal.xyz * -1.f;
        float3 vInNormal = In.vNormal.xyz;
        
        float3x3 WorldMatrix;
        WorldMatrix = float3x3(vTangent, vBinormal, vInNormal);
        vNormal.xyz = normalize(mul(vNormal.xyz, WorldMatrix));
        
        Out.vPBR.x = vNormalDesc.b; // PBR.X = 노말 텍스처 Blue, Z 값
        Out.vPBR.y = vNormalDesc.a; // PBR.y = 노말 텍스처 Alpha 값
    }
    else
    {
        vNormal = In.vNormal;
        Out.vPBR.x = g_fGlobalDynamicMetallic; // PBR.X = 노말 텍스처 Blue, Z 값
        Out.vPBR.y = g_fGlobalDynamicRoughness; // PBR.y = 노말 텍스처 Alpha 값
    }
    
    Out.vPBR.z = 1.f; // PBR.z = STATIC = 0.f , DYNAMIC = 1.f

    vNormal.xyz = vNormal * 0.5f + 0.5f;
    
    Out.vNormal = vNormal;
    Out.vDepth.x = In.vProjPos.z / In.vProjPos.w;
    Out.vDepth.y = In.vProjPos.w;
    Out.vDepth.z = 1.f;
    
    return Out;
}

/*------------------------------------------------SHADOW BEGIN------------------------------------------------*/

struct VS_OUT_SHADOW
{
    float4 vPosition : POSITION;
};

VS_OUT_SHADOW VS_SHADOW(VS_IN In)
{
    VS_OUT_SHADOW Out = (VS_OUT_SHADOW) 0;
    
    uint iX = max(In.vBlendIndex.x, g_iNumBlendWeightsToUse);
    uint iY = max(In.vBlendIndex.y, g_iNumBlendWeightsToUse);
    uint iZ = max(In.vBlendIndex.z, g_iNumBlendWeightsToUse);
    uint iW = max(In.vBlendIndex.w, g_iNumBlendWeightsToUse);
    
    float fWeightW = 1.f - (In.vBlendWeight.x + In.vBlendWeight.y + In.vBlendWeight.z);
    matrix matBone =
    g_BoneMatrices[In.vBlendIndex.x] * In.vBlendWeight.x +
    g_BoneMatrices[In.vBlendIndex.y] * In.vBlendWeight.y +
    g_BoneMatrices[In.vBlendIndex.z] * In.vBlendWeight.z +
    g_BoneMatrices[In.vBlendIndex.w] * fWeightW;
    
    float4 vPosition = mul(float4(In.vPosition, 1.f), matBone);
    
    Out.vPosition = mul(vPosition, g_WorldMatrix);

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
/*------------------------------------------------SHADOW END------------------------------------------------*/


/*------------------------------------------------OULTINE BEGIN------------------------------------------------*/

struct VS_OUT_OUTLINE
{
    float4 vPosition : SV_POSITION;
    bool IsDraw : TEXCOORD0;
    float4 vProjPos : TEXCOORD1;
};

VS_OUT_OUTLINE VS_OUTLINE(VS_IN In)
{
    VS_OUT_OUTLINE Out = (VS_OUT_OUTLINE) 0;
    
    float fWeightW = 1.f - (In.vBlendWeight.x + In.vBlendWeight.y + In.vBlendWeight.z);
    matrix matBone =
    g_BoneMatrices[In.vBlendIndex.x] * In.vBlendWeight.x +
    g_BoneMatrices[In.vBlendIndex.y] * In.vBlendWeight.y +
    g_BoneMatrices[In.vBlendIndex.z] * In.vBlendWeight.z +
    g_BoneMatrices[In.vBlendIndex.w] * fWeightW;
    
    matrix matWV;
    
    matWV = mul(g_WorldMatrix, g_ViewMatrix);
   
    float4 vPosition = mul(float4(In.vPosition, 1.f), matBone);
    float4 vNormal = mul(float4(In.vNormal, 0.f), matBone);
    
    vector vViewPos = mul(vPosition, matWV);
    
    vector vViewNormal = normalize(mul(vNormal, matWV));
   
    if(vViewNormal.z < 0.f)
    {
        vViewNormal.z *= -1.f;
    }
    
    vViewNormal = normalize(float4(vViewNormal.x, vViewNormal.y, vViewNormal.z * 0.01f, 0.f));
    
    vector vOutLinePos = vViewPos + (vViewNormal * g_fOutLineRadius);
    
    Out.vPosition = mul(float4(vOutLinePos), g_ProjMatrix);
    Out.IsDraw = true;
    Out.vProjPos = Out.vPosition;
    
    return Out;
}

struct PS_IN_OUTLINE
{
    float4 vPosition : SV_POSITION;
    bool IsDraw : TEXCOORD0;
    float4 vProjPos : TEXCOORD1;
};

struct PS_OUT_OUTLINE
{
    float4 vColor : SV_TARGET0;
    float4 vDepth : SV_TARGET1;
    float4 vPBR : SV_TARGET2;
};

PS_OUT_OUTLINE PS_OUTLINE(PS_IN_OUTLINE In)
{
    PS_OUT_OUTLINE Out = (PS_OUT_OUTLINE) 0;

    if (In.IsDraw)
    {
        Out.vColor = g_vOutLineColor;
        Out.vDepth.x = In.vProjPos.z / In.vProjPos.w;
        Out.vDepth.y = In.vProjPos.w;
        Out.vDepth.z = 1.f;
        Out.vPBR.z = 1.f;
    }
    else
        discard;
        
    return Out;
}

/*------------------------------------------------OULTINE END------------------------------------------------*/

technique11 DefaultTechnique
{
    pass DefaultNormal // 0
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN();
    }

    pass NormalTexture // 1
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_NORMALTEX();
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
 
    pass Outline // 3
    {
        SetRasterizerState(RS_Cull_Front);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_OUTLINE();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_OUTLINE();
    }

    pass DefaultWeapon // 4
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_DEFAULT_WEAPON();
    }

    pass DissolveGalbrenaWeapon // 5
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_DISSOLVE_GALBRENAWEAPON();
    }

    pass EnergyBlade // 6
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_ENERGY_BLADE();
    }

    pass AugustaHeadProp // 7
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_AUGUSTA_HEADPROP();
    }
    
    pass DissolveAugustaWeapon // 8
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_DISSOLVE_AUGUSTAWEAPON();
    }

    pass DissolveRoverWeapon // 9
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_DISSOLVE_ROVERWEAPON();
    }

    pass AugustaBurstWeaponEffect // 10
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_AUGUSTA_BURSTWEAPON_EFFECT();
    }

    pass AugustaBurstWeapon // 11
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_AUGUSTA_BURSTWEAPON();
    }


    pass YunoWeapon // 12
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_YUNO_WEAPON();
    }

}