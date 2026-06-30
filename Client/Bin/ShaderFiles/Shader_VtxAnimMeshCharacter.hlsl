#include "Engine_Shader_Defines.hlsli"

matrix g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;
matrix g_ViewMatrixInv;

float g_fLightFar;

Texture2D g_DiffuseTexture;
Texture2D g_SecondDiffuseTexture;
Texture2D g_NormalTexture;
Texture2D g_MaskTexture[4] : register(t8);

matrix g_ShadowViewMatrix[4];
matrix g_ShadowProjMatrix[4];

float g_fOutLineRadius = 0.001f;
float4 g_vOutLineColor;

float g_fDissolveRate = 0.f;
float g_fFlowRate = 0.f;

matrix g_BoneMatrices[512];
bool g_HasNormal = false;
bool g_HasSkinMask = false;

float4 g_vBaseColor = 1.f;
float3 g_vCamPosition;
float g_fMaxTime = 1.f;
float g_fCurrentTime = 0.f;

float g_fNoiseTime;

// 렌더링 파이프 라인으로 넘겨질 최종 정점 정보.
struct OutputVertex
{
    float3 vPosition;
    float3 vNormal;
};


cbuffer GlobalConstants
{
    int g_iNumBlendWeightsToUse = 2; 
    uint g_iGalbrenaMaskIndex = 1;
    float g_fEmissiveIntensity = 0.5f;
    float g_fGalbrenaEyeAlpha = 0.6f;
    float4 g_vEmissiveColor = float4(1.f, 1.f, 1.f, 1.f);
    float4 g_vDissolveColor = float4(1.f, 1.f, 1.f, 1.f);
}

// Delta 데이터를 담은 구조화 버퍼 C++에서 SRV로 바인딩 => 메시별로 바인딩 됩니다.
StructuredBuffer<OutputVertex> g_MorphedVertices : register(t20);
//StructuredBuffer<float3> g_MorphDeltaPositions : register(t20);
//StructuredBuffer<float3> g_MorphDeltaNormals : register(t21);

struct VS_IN
{
    float3 vPosition : POSITION;
    float3 vNormal : NORMAL;
    float3 vTangent : TANGENT;
    float3 vBinormal : BINORMAL;
    uint4 vBlendIndex : BLENDINDEX;
    float4 vBlendWeight : BLENDWEIGHT;
    float2 vTexcoord : TEXCOORD0;
    uint iVertexID : SV_VERTEXID; // AutoMatically VertexID
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

    // 정점 정보를 교체합니다.
    OutputVertex MorphedVert = g_MorphedVertices[In.iVertexID];
    
    // 뼈대 계산 전에 얼굴부터 변형시킵니다.
    float3 vMorphedPos = MorphedVert.vPosition;
    float3 vMorphedNormal = MorphedVert.vNormal;

    // 스키닝 행렬 생성
    matrix matBone = (matrix) 0;
    matBone += g_BoneMatrices[In.vBlendIndex.x] * In.vBlendWeight.x;
    matBone += g_BoneMatrices[In.vBlendIndex.y] * In.vBlendWeight.y;
    matBone += g_BoneMatrices[In.vBlendIndex.z] * In.vBlendWeight.z;
    matBone += g_BoneMatrices[In.vBlendIndex.w] * In.vBlendWeight.w;

    // 애니메이션 행렬 적용 (vMorphedPos 사용)
    vector vPosition = mul(float4(vMorphedPos, 1.f), matBone);
    vector vNormal = mul(float4(vMorphedNormal, 0.f), matBone);
    vector vTangent = mul(float4(In.vTangent, 0.f), matBone); 
    vector vBinormal = mul(float4(In.vBinormal, 0.f), matBone);

    // [WVP Transformation] 화면 좌표로 변환
    matrix matWVP = mul(g_WorldMatrix, g_ViewMatrix);
    matWVP = mul(matWVP, g_ProjMatrix);
    
    Out.vPosition = mul(vPosition, matWVP);
    Out.vNormal = normalize(mul(vNormal, g_WorldMatrix));
    Out.vTangent = normalize(mul(vTangent, g_WorldMatrix));
    Out.vBinormal = normalize(mul(vBinormal, g_WorldMatrix));
    Out.vTexcoord = In.vTexcoord;
    Out.vProjPos = mul(vPosition, matWVP);

    return Out;
}

VS_OUT VS_NONMORPH_MAIN(VS_IN In)
{
    VS_OUT Out = (VS_OUT) 0;

    matrix matBone = (matrix) 0;
    matBone += g_BoneMatrices[In.vBlendIndex.x] * In.vBlendWeight.x;
    matBone += g_BoneMatrices[In.vBlendIndex.y] * In.vBlendWeight.y;
    matBone += g_BoneMatrices[In.vBlendIndex.z] * In.vBlendWeight.z;
    matBone += g_BoneMatrices[In.vBlendIndex.w] * In.vBlendWeight.w;

    // 애니메이션 행렬 적용 (vMorphedPos 사용!)
    vector vPosition = mul(float4(In.vPosition, 1.f), matBone);
    vector vNormal = mul(float4(In.vNormal, 0.f), matBone);
    vector vTangent = mul(float4(In.vTangent, 0.f), matBone); // Tangent는 Morph 안 했으므로 In.vTangent 사용 (약식)
    vector vBinormal = mul(float4(In.vBinormal, 0.f), matBone);

    // 3. [WVP Transformation] 화면 좌표로 변환
    matrix matWVP = mul(g_WorldMatrix, g_ViewMatrix);
    matWVP = mul(matWVP, g_ProjMatrix);
    
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
    float4 vSSS : SV_TARGET6;
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

PS_OUT PS_AUGUSTA(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;

    Out.vDiffuse = g_DiffuseTexture.Sample(DefaultSampler, In.vTexcoord);
    
    // 
    float4 vNormal = 0.f;
    
    if (g_HasNormal)
    {
        float4 vNormalDesc = g_NormalTexture.Sample(DefaultSampler, In.vTexcoord);
        vNormal.xyz = vNormalDesc.xyz * 2.f - 1.f;
        vNormal.z = sqrt(1.f - saturate(dot(vNormalDesc.xy, vNormalDesc.xy))); // 그대로 사용
        
        //vNormal = normalize(vNormalDesc * 2.f - 1.f);
        
        //if (vNormalDesc.x > vNormalDesc.z && vNormalDesc.y > vNormalDesc.z)
        //    vNormal.z = sqrt(1.f - saturate(dot(vNormalDesc.xy, vNormalDesc.xy))); // 그대로 사용
            
        float3 vTangent = In.vTangent.xyz;
        float3 vBinormal = In.vBinormal.xyz * -1.f;
        float3 vInNormal = In.vNormal.xyz;
        
        float3x3 WorldMatrix;
        WorldMatrix = float3x3(vTangent, vBinormal, vInNormal);
        vNormal.xyz = normalize(mul(vNormal.xyz, WorldMatrix));
        
        //Out.vPBR.x = g_fGlobalDynamicMetallic; // PBR.X = 노말 텍스처 Blue, Z 값
        //Out.vPBR.y = g_fGlobalDynamicRoughness; // PBR.y = 노말 텍스처 Alpha 값
        
        Out.vPBR.x = vNormalDesc.b; // PBR.X = 노말 텍스처 Blue, Z 값
        Out.vPBR.y = vNormalDesc.a; // PBR.y = 노말 텍스처 Alpha 값
    }
    else
    {
        vNormal = In.vNormal;
        Out.vPBR.x = g_fGlobalDynamicMetallic; // PBR.X = 노말 텍스처 Blue, Z 값
        Out.vPBR.y = g_fGlobalDynamicRoughness; // PBR.y = 노말 텍스처 Alpha 값
    }
    
    if (g_HasSkinMask)
    {
        Out.vSSS = g_MaskTexture[0].Sample(DefaultSampler, In.vTexcoord);
    }
    
    Out.vPBR.z = 1.f; // PBR.z = STATIC = 0.f , DYNAMIC = 1.f
    
    vNormal.xyz = vNormal * 0.5f + 0.5f;
    
    Out.vNormal = vNormal;
    Out.vDepth.x = In.vProjPos.z / In.vProjPos.w;
    Out.vDepth.y = In.vProjPos.w;
    Out.vDepth.z = 1.f;
    
    Out.vSSS.z = In.vProjPos.z / In.vProjPos.w;
    Out.vSSS.w = In.vProjPos.w;
    
    
    return Out;
}

PS_OUT PS_ROVER(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;

    Out.vDiffuse = g_DiffuseTexture.Sample(DefaultSampler, In.vTexcoord);
    
    // 
    float4 vNormal = 0.f;
    
    if (g_HasNormal)
    {
        float4 vNormalDesc = g_NormalTexture.Sample(DefaultSampler, In.vTexcoord);
        vNormal.xyz = vNormalDesc.xyz * 2.f - 1.f;
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
    if (g_HasSkinMask)
    {
        Out.vSSS = g_MaskTexture[1].Sample(DefaultSampler, In.vTexcoord);
    }
    Out.vPBR.z = 1.f; // PBR.z = STATIC = 0.f , DYNAMIC = 1.f
    

    vNormal.xyz = vNormal * 0.5f + 0.5f;
    
    Out.vNormal = vNormal;
    Out.vDepth.x = In.vProjPos.z / In.vProjPos.w;
    Out.vDepth.y = In.vProjPos.w;
    Out.vDepth.z = 1.f;
    
    Out.vSSS.z = In.vProjPos.z / In.vProjPos.w;
    Out.vSSS.w = In.vProjPos.w;
    
    return Out;
}

PS_OUT PS_GALBRENA(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;

    Out.vDiffuse = g_DiffuseTexture.Sample(DefaultSampler, In.vTexcoord);
    
    // 
    float4 vNormal = 0.f;
    
    if (g_HasNormal)
    {
        float4 vNormalDesc = g_NormalTexture.Sample(DefaultSampler, In.vTexcoord);
        vNormal.xyz = vNormalDesc.xyz * 2.f - 1.f;
        vNormal.z = sqrt(1.f - saturate(dot(vNormalDesc.xy, vNormalDesc.xy))); // 그대로 사용
        
        //vNormal = normalize(vNormalDesc * 2.f - 1.f);
        
        //if (vNormalDesc.x > vNormalDesc.z && vNormalDesc.y > vNormalDesc.z)
        //    vNormal.z = sqrt(1.f - saturate(dot(vNormalDesc.xy, vNormalDesc.xy))); // 그대로 사용
        
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
    if (g_HasSkinMask)
    {
        Out.vSSS = g_MaskTexture[1].Sample(DefaultSampler, In.vTexcoord);
    }
    Out.vPBR.z = 1.f; // PBR.z = STATIC = 0.f , DYNAMIC = 1.f
    
    vNormal.xyz = vNormal * 0.5f + 0.5f;
    
    Out.vNormal = vNormal;
    Out.vDepth.x = In.vProjPos.z / In.vProjPos.w;
    Out.vDepth.y = In.vProjPos.w;
    Out.vDepth.z = 1.f;
    
    Out.vSSS.z = In.vProjPos.z / In.vProjPos.w;
    Out.vSSS.w = In.vProjPos.w;
    
    return Out;
}

PS_OUT PS_LOGO_ROVER(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;

    Out.vDiffuse = g_DiffuseTexture.Sample(DefaultSampler, In.vTexcoord);
    
    // 
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
    
    //Test
    Out.vPBR.a = 1.f;

    vNormal.xyz = vNormal * 0.5f + 0.5f;
    
    Out.vNormal = vNormal;
    Out.vDepth.x = In.vProjPos.z / In.vProjPos.w;
    Out.vDepth.y = In.vProjPos.w;
    Out.vDepth.z = 1.f;
    
    
    return Out;
}

PS_OUT PS_GALBRENABACK(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;

    // 1. 마스크 텍스쳐 샘플링.
    float4 vMaskColor;
    //vMaskColor = g_MaskTexture[1].Sample(DefaultSampler, In.vTexcoord);
    if (1 == g_iGalbrenaMaskIndex)
        vMaskColor = g_MaskTexture[1].Sample(DefaultSampler, In.vTexcoord);
    if (2 == g_iGalbrenaMaskIndex)
        vMaskColor = g_MaskTexture[2].Sample(DefaultSampler, In.vTexcoord);
    
    // 2. 알파 클리핑.
    // 텍스쳐 알파 r값이 0.1보다 작으면 해당 픽셀 렌더링 포기.
    clip(vMaskColor.r - 0.1f);
    
    // 3. Diffuse 색상 결정.
    Out.vDiffuse = float4(vMaskColor.rgb, 1.f);
    
    
    float3 vColor = g_vEmissiveColor.rgb;
    float fIntensity = g_fEmissiveIntensity;
    
    Out.vEmissive = float4(vColor * fIntensity, 1.f);
    
    
    float4 vNormal = 0.f;
    vNormal = In.vNormal;
    Out.vPBR.x = g_fGlobalDynamicMetallic; // PBR.X = 노말 텍스처 Blue, Z 값
    Out.vPBR.y = g_fGlobalDynamicRoughness; // PBR.y = 노말 텍스처 Alpha 값
    Out.vPBR.z = 1.f; // PBR.z = STATIC = 0.f , DYNAMIC = 1.f
    
    vNormal.xyz = vNormal * 0.5f + 0.5f;
    
    Out.vNormal = vNormal;
    Out.vDepth.x = In.vProjPos.z / In.vProjPos.w;
    Out.vDepth.y = In.vProjPos.w;
    Out.vDepth.z = 1.f;
    
    Out.vSSS.z = In.vProjPos.z / In.vProjPos.w;
    Out.vSSS.w = In.vProjPos.w;
    
    return Out;
}

PS_OUT PS_DISSOLVE_CHARACTER(PS_IN In)
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
    //if (g_HasSkinMask)
    //{
    //    Out.vSSS = g_MaskTexture[1].Sample(DefaultSampler, In.vTexcoord);
    //}
    
    
    // Dissolve 진행도가 0.3f 보다 초과인 얘들은 Emissive가 기본으로 들어가고 0.3f 이하인 얘들은 Emissive가 지정한 색상에 더 크게 작용.
    float3 vColor = g_vDissolveColor.rgb;
    if (fDissolveMask.r - g_fDissolveRate < 0.3f) // 0.3f 보다 작은 (사라지기 직전)
        Out.vDiffuse.rgb = vColor * g_fEmissiveIntensity; // 이러면 쨍하게 들어간다. 
    
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
    
    

    //Out.vSSS.z = In.vProjPos.z / In.vProjPos.w;
    //Out.vSSS.w = In.vProjPos.w;
    
    
    
    return Out;
}

PS_OUT PS_LEVI_BEHIT(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;

     // need value : g_vCamPosition, g_fMaxTime, g_fCurrentTime
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
        
        //Out.vPBR.x = g_fGlobalDynamicMetallic; // PBR.X = 노말 텍스처 Blue, Z 값
        //Out.vPBR.y = g_fGlobalDynamicRoughness; // PBR.y = 노말 텍스처 Alpha 값
        
        Out.vPBR.x = vNormalDesc.b; // PBR.X = 노말 텍스처 Blue, Z 값
        Out.vPBR.y = vNormalDesc.a; // PBR.y = 노말 텍스처 Alpha 값
    }
    else
    {
        vNormal = In.vNormal;
        Out.vPBR.x = g_fGlobalDynamicMetallic; // PBR.X = 노말 텍스처 Blue, Z 값
        Out.vPBR.y = g_fGlobalDynamicRoughness; // PBR.y = 노말 텍스처 Alpha 값
    }
    
    if (g_HasSkinMask)
    {
        Out.vSSS = g_MaskTexture[0].Sample(DefaultSampler, In.vTexcoord);
    }
    
    Out.vPBR.z = 1.f; // PBR.z = STATIC = 0.f , DYNAMIC = 1.f
    
    vNormal.xyz = vNormal * 0.5f + 0.5f;
    
    Out.vNormal = vNormal;
    Out.vDepth.x = In.vProjPos.z / In.vProjPos.w;
    Out.vDepth.y = In.vProjPos.w;
    Out.vDepth.z = 1.f;
    
    Out.vSSS.z = In.vProjPos.z / In.vProjPos.w;
    Out.vSSS.w = In.vProjPos.w;
    
    
    return Out;
}


PS_OUT PS_LEVI_FX(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;
    
    float2 vNoiseTex = float2(In.vTexcoord.x + g_fNoiseTime, In.vTexcoord.y);
    
    float vNoise = g_NormalTexture.Sample(DefaultSampler, vNoiseTex).r;
    
    float2 vTexcoord = In.vTexcoord + (vNoise * 0.12f);
    
    float4 vDiffuse = g_DiffuseTexture.Sample(DefaultSampler, vTexcoord);
    
    float3 vNormal = In.vNormal.xyz;
    
    vNormal = vNormal * 0.5f + 0.5f;
    
    
    Out.vDiffuse = float4(vDiffuse.xyz, 1.f);
    Out.vNormal = float4(vNormal, 0.f);
    Out.vDepth.x = In.vProjPos.z / In.vProjPos.w;
    Out.vDepth.y = In.vProjPos.w;
    Out.vDepth.z = 1.f;
    
    Out.vPBR.x = g_fGlobalDynamicMetallic;
    Out.vPBR.y = g_fGlobalDynamicRoughness;
    Out.vPBR.z = 1.f;
    
    Out.vSSS.z = In.vProjPos.z / In.vProjPos.w;
    Out.vSSS.w = In.vProjPos.w;
    
    Out.vEmissive = float4(vDiffuse.xyz * 0.3f, 1.f);
    
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

PS_OUT PS_GALBRENA_EYE(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;

    Out.vDiffuse = g_DiffuseTexture.Sample(DefaultSampler, In.vTexcoord);
    float4 vDiffuseColor = Out.vDiffuse;
    
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
    if (g_HasSkinMask)
    {
        Out.vSSS = g_MaskTexture[1].Sample(DefaultSampler, In.vTexcoord);
    }
    
    // 1. 마스크 생성
    float fEyeMask = step(g_fGalbrenaEyeAlpha, Out.vDiffuse.a);
    
    // 2. 발광 비율 결정.
    float fGlowRatio = lerp(0.05f, 1.f, fEyeMask);
    
    // 3. 최종 Emissive 계산.
    float3 vColor = g_vEmissiveColor.rgb;
    float fIntensity = g_fEmissiveIntensity; // 블룸먹도록 증폭.
    Out.vEmissive = float4(vColor * fIntensity * fGlowRatio, 1.0f);
    
    Out.vPBR.z = 1.f; // PBR.z = STATIC = 0.f , DYNAMIC = 1.f
    
    vNormal.xyz = vNormal * 0.5f + 0.5f;
    
    Out.vNormal = vNormal;
    Out.vDepth.x = In.vProjPos.z / In.vProjPos.w;
    Out.vDepth.y = In.vProjPos.w;
    Out.vDepth.z = 1.f;
    
    Out.vSSS.z = In.vProjPos.z / In.vProjPos.w;
    Out.vSSS.w = In.vProjPos.w;
    
    return Out;
}

PS_OUT PS_ROVERMASK(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;

    Out.vDiffuse = g_DiffuseTexture.Sample(DefaultSampler, In.vTexcoord);
    
    if (Out.vDiffuse.g < 0.01f && Out.vDiffuse.b < 0.01f)
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
    if (g_HasSkinMask)
    {
        Out.vSSS = g_MaskTexture[1].Sample(DefaultSampler, In.vTexcoord);
    }
    Out.vPBR.z = 1.f; // PBR.z = STATIC = 0.f , DYNAMIC = 1.f
    

    vNormal.xyz = vNormal * 0.5f + 0.5f;
    
    Out.vNormal = vNormal;
    Out.vDepth.x = In.vProjPos.z / In.vProjPos.w;
    Out.vDepth.y = In.vProjPos.w;
    Out.vDepth.z = 1.f;
    
    Out.vSSS.z = In.vProjPos.z / In.vProjPos.w;
    Out.vSSS.w = In.vProjPos.w;
    
    return Out;
}

// Dissovle 반대로 나오게.
PS_OUT PS_UNDISSOLVE_CHARACTER(PS_IN In)
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
    //if (g_HasSkinMask)
    //{
    //    Out.vSSS = g_MaskTexture[1].Sample(DefaultSampler, In.vTexcoord);
    //}
    
    
    // Dissolve 진행도가 0.3f 보다 초과인 얘들은 Emissive가 기본으로 들어가고 0.3f 이하인 얘들은 Emissive가 지정한 색상에 더 크게 작용.
    float3 vColor = g_vDissolveColor.rgb;
    if (fDissolveMask.r - g_fDissolveRate < 0.3f) // 0.3f 보다 작은 (사라지기 직전)
        Out.vDiffuse.rgb = vColor * g_fEmissiveIntensity; // 이러면 쨍하게 들어간다. 
    
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
    
    //Out.vSSS.z = In.vProjPos.z / In.vProjPos.w;
    //Out.vSSS.w = In.vProjPos.w;
    return Out;
}


PS_OUT PS_CHARACTER_EYE(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;

    Out.vDiffuse = g_DiffuseTexture.Sample(DefaultSampler, In.vTexcoord);

    Out.vDiffuse.xyz *= 1.5f;
    
    float4 vNormal = 0.f;
    
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
    
    Out.vSSS.z = In.vProjPos.z / In.vProjPos.w;
    Out.vSSS.w = In.vProjPos.w;
    
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
    float4 vOutlinePos : TEXCOORD2;
    float4 vViewPos : TEXCOORD3;
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
   
    bool IsDraw = true;
    
    if(vViewNormal.z < 0.f)
    {
        IsDraw = false;
    }
    
    vViewNormal = normalize(float4(vViewNormal.x, vViewNormal.y, vViewNormal.z * 0.01f, 0.f));
    
    
    float fTimeRatio = saturate(1.f - (g_fCurrentTime / g_fMaxTime));
    
    vector vOutLinePos = vViewPos + (vViewNormal * (g_fOutLineRadius * fTimeRatio));
    
    Out.vPosition = mul(float4(vOutLinePos), g_ProjMatrix);
    Out.IsDraw = IsDraw;
    Out.vProjPos = Out.vPosition;
    Out.vOutlinePos = vOutLinePos;
    Out.vViewPos = vViewPos;
    
    return Out;
}

float Hash21(float2 ID)
{
    float2 vInput = ID;
    vInput = frac(vInput * float2(123.32, 456.21));
    vInput += dot(vInput, vInput + 45.32);
    return frac(vInput.x * vInput.y);
}


VS_OUT_OUTLINE VS_BOSS_OUTLINE(VS_IN In)
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
   
    bool IsDraw = true;
    
    float JitterRatio = lerp(1.f, 2.f, Hash21(vNormal.xy + vViewPos.xy));
    
    float JitterLength = lerp(0.3f, 0.8f, Hash21(vNormal.xy + vViewPos.xy));
    
    float2 vNormalJitter = float2(vViewNormal.x < 0.f ? JitterLength * -1.f : JitterLength, vViewNormal.y < 0.f ? JitterLength * -1.f : JitterLength);
    
    vViewNormal.xy += vNormalJitter;
    vViewNormal.xy *= JitterRatio;
    
    //vViewNormal = normalize(float4(vViewNormal.x, vViewNormal.y, vViewNormal.z * 0.01f, 0.f));
    
    if (vViewNormal.z < 0.f)
    {
        IsDraw = false;
    }
    
    float fTimeRatio = saturate(1.f - (g_fCurrentTime / g_fMaxTime));
    
    vector vOutLinePos = vViewPos + ((vViewNormal * g_fOutLineRadius) * fTimeRatio);
    
    Out.vPosition = mul(float4(vOutLinePos), g_ProjMatrix);
    Out.IsDraw = IsDraw;
    Out.vProjPos = Out.vPosition;
    Out.vOutlinePos = vOutLinePos;
    Out.vViewPos = vViewPos;
    
    return Out;
}


struct PS_IN_OUTLINE
{
    float4 vPosition : SV_POSITION;
    bool IsDraw : TEXCOORD0;
    float4 vProjPos : TEXCOORD1;
    float4 vOutlinePos : TEXCOORD2;
    float4 vViewPos : TEXCOORD3;
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


PS_OUT_OUTLINE PS_BOSS_OUTLINE(PS_IN_OUTLINE In)
{
    PS_OUT_OUTLINE Out = (PS_OUT_OUTLINE) 0;

    if (In.IsDraw)
    {
        float4 vOutlineWorldPos = mul(In.vOutlinePos, g_ViewMatrixInv);
        
        float fViewLength = length(In.vOutlinePos.xyz - In.vViewPos.xyz);
        
        float fGradiant = pow(saturate(fViewLength / (5.2f * g_fOutLineRadius)), 2.f);
        
        bool IsLine = fmod(abs(vOutlineWorldPos.y), 0.05f) > 0.025f;
        Out.vColor = IsLine ? g_vOutLineColor : 0.f;
        
        if (all(Out.vColor == 0.f))
            discard;
        
        Out.vColor.xyz *= fGradiant;
        Out.vColor.xyz *= 3.f;
        
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

    pass Augusta // 2
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_AUGUSTA();
    }

    pass Shadow // 3
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_SHADOW();
        GeometryShader = compile gs_5_0 GS_SHADOW();
        PixelShader = compile ps_5_0 PS_SHADOW();
    }
 
    pass Outline //4
    {
        SetRasterizerState(RS_Cull_Front);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_OUTLINE();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_OUTLINE();
    }
 

    pass Rover // 5
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_ROVER();
    }

    pass Galbrena // 6
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_GALBRENA();
    }

    pass NormalYellow // 7
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_NORMALTEX();
    }

    pass LogoRover // 8
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_LOGO_ROVER();
    }

    pass GalbrenaBack // 9
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_GALBRENABACK();
    }

    pass DissolveCharacter // 10
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_DISSOLVE_CHARACTER();
    }

    pass GalbrenaEye // 11
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_GALBRENA_EYE();
    }

    pass RoverMask // 12
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_ROVERMASK();
    }

    pass NonMorphCharacter // 13
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_NONMORPH_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_GALBRENA();
    }

    pass Leviatan_Behit // 14
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_LEVI_BEHIT();
    }
    pass LeviFX // 15
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_LEVI_FX();
    }   
    pass UnDissolveCharacter // 16
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_UNDISSOLVE_CHARACTER();
    }

    pass CharacterEye       // 17
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_CHARACTER_EYE();
    }
    
    pass BossOutLine // 18
    {
        SetRasterizerState(RS_Cull_Front);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_BOSS_OUTLINE();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_BOSS_OUTLINE();
    }
}