#include "Engine_Shader_Light.hlsli"
#include "Engine_Shader_Water.hlsli"

Texture2DArray<float4> g_LUT_Texture : register(t1);

const int  g_iLutIndex = 0;
float g_fLutLerpIntensity = 0.25f;
bool g_IsDynamicLUT = false;

float g_fExposure = 0.6f;

float g_fLightFar;

Texture2D g_Texture;

Texture2D g_SkinMaskTexture;

//OBJECTS
Texture2D g_DiffuseTexture; // Color
Texture2D g_NormalTexture;  // Normal
Texture2D g_DepthTexture;   // (Depth.x = Proj.z / Proj.w) , (Depth.y = Proj.w)
Texture2D g_PBRTexture;     // (PBR.x = Metallic), (PBR.y = Roughness ), (PBR.z = IsDynamic) 

//BackBuffer
Texture2D g_BackBufferTexture;

//COMBINED
Texture2D g_LightDiffuseTexture;
Texture2D g_LightSpecularTexture;
Texture2D g_LightAmbientTexture;
Texture2D g_SsaoTexture;        

//Emissive
Texture2D g_EmissiveTexture;

//Distoriton
Texture2D g_DistortionTexture;

//BLOOM
Texture2D g_BloomTexture;

//RAMP
Texture2D g_RampTexture;            // Shade Color
Texture2D g_ColorRampTexture;       // Rim Color

//FOG
Texture2D g_LutResultTexture;
Texture2D g_FogNoiseTexture;
float2 g_vFogDepthDistance;
float2 g_vFogHeightDistance;
float4 g_vFogColor;
float g_fFogTime;
Texture3D g_VoulmetricTexture;
float2 g_vFogRange;

//SSAO
Texture2D g_NoiseTexture;
float4  g_vSampleVector[16];
int     g_iSampleSize;
float   g_fSSAO_Radius;
float   g_fSSAO_MaxDistance;
float   g_fSSAO_OutDistance;

//DOF
Texture2D g_DofTexture;
Texture2D g_BlurTexture;

//Motion
Texture2D g_VelocityMap;
float g_fLimitVelocity;
float g_fLimitDepth;

//Light
uint g_iNumLight;

bool Debug_IsLight = true;

int g_DebugCSMIndex;

//SFX
float g_fEffectIntensity;

//DEBUG
bool g_IsStylized;

//WeightBlend
Texture2D g_AccumColorTexture;
Texture2D g_AccumAlphaTexture;

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

struct PS_IN
{
    float4 vPosition : SV_POSITION;
    float2 vTexcoord : TEXCOORD0;
};

struct PS_OUT_BACKBUFFER
{
    float4 vColor : SV_TARGET0;
};

PS_OUT_BACKBUFFER PS_MAIN_DRAW(PS_IN In)
{
    PS_OUT_BACKBUFFER Out = (PS_OUT_BACKBUFFER) 0;

    Out.vColor = g_Texture.Sample(DefaultSampler, In.vTexcoord);
    
    return Out;
}

PS_OUT_BACKBUFFER PS_MAIN_COMBINED(PS_IN In)
{
    PS_OUT_BACKBUFFER Out = (PS_OUT_BACKBUFFER) 0;

    float4 vPBRDesc = g_PBRTexture.Sample(DefaultSampler, In.vTexcoord);
    
    float4 vLightDiffuse = g_LightDiffuseTexture.Sample(DefaultSampler, In.vTexcoord);
    float4 vLightSpecular = g_LightSpecularTexture.Sample(DefaultSampler, In.vTexcoord);
    float4 vLightAmbient = g_LightAmbientTexture.Sample(DefaultSampler, In.vTexcoord);
    
    if (vLightDiffuse.a == 0.f)
        discard;
    
    float4 vLightColor = vLightDiffuse + vLightSpecular + vLightAmbient;
    
    Out.vColor = float4(vLightColor.xyz, 1.f);
    
    if (any(vPBRDesc.z))
    {
        return Out;
    }
    
    float fSSao = g_SsaoTexture.Sample(DefaultSampler, In.vTexcoord).r;
    Out.vColor *= fSSao;
        
    Out.vColor.a = 1.f;

    return Out;
}

struct PS_OUT_LIGHT
{
    float4 vLightDiffuse : SV_TARGET0;
    float4 vLightSpecular : SV_TARGET1;
    float4 vLightAmbient : SV_TARGET2;
};

PS_OUT_LIGHT PS_LIGHT(PS_IN In)
{
    PS_OUT_LIGHT Out = (PS_OUT_LIGHT) 0;
    
    float4 vDiffuse = g_DiffuseTexture.Sample(DefaultSampler, In.vTexcoord);
    
    if (vDiffuse.r == 1.f && vDiffuse.g == 0.f && vDiffuse.b == 1.f)
        discard;
    
    if (false == Debug_IsLight)
    {
        Out.vLightDiffuse = vDiffuse;
            
        Out.vLightDiffuse.w = 1.f;

        Out.vLightSpecular.w = 1.f;

        Out.vLightAmbient.w = 1.f;
    
        return Out;
    }
    
    float4 vViewPos = Compute_ViewPos(In.vTexcoord, g_DepthTexture);
    
    if(all(vViewPos == 0.f))
        discard;
        
    float4 vNormal = Compute_Normal(g_NormalTexture, DefaultSampler, In.vTexcoord);
    
    vector vPBRDesc = g_PBRTexture.Sample(DefaultSampler, In.vTexcoord);
    
    float4 vWorldPos = mul(vViewPos, g_ViewMatrixInv);
    
    bool IsSkin = all(g_SkinMaskTexture.Sample(DefaultSampler, In.vTexcoord).xy > 0.f);
    
    for (uint i = 0; i < g_iNumLight; ++i)
    {
        LIGHT_RESULT Result = (LIGHT_RESULT) 0;
        
        switch (g_LightDatas[i].iType)
        {
            case 0: // DIRECTIONAL
                Result = Compute_Directional(vDiffuse, vNormal, vWorldPos, vViewPos, vPBRDesc.z, vPBRDesc.y, vPBRDesc.x, IsSkin, i);
                break;
            case 1: // POINT
                Result = Compute_Point(vDiffuse, vNormal, vWorldPos, vViewPos, vPBRDesc.z, vPBRDesc.y, vPBRDesc.x, IsSkin, i);
                break;
        }
        
        Out.vLightDiffuse += float4(Result.vLightDiffuse, 1.f);
        Out.vLightSpecular += float4(Result.vLightSpecular, 1.f);
        Out.vLightAmbient += float4(Result.vLightAmbient, 1.f);
    }
    
    Out.vLightDiffuse.w = 1.f;

    Out.vLightSpecular.w = 1.f;

    Out.vLightAmbient.w = 1.f;
    
    return Out;
}

PS_OUT_BACKBUFFER PS_BLOOM(PS_IN In)
{
    PS_OUT_BACKBUFFER Out = (PS_OUT_BACKBUFFER) 0;
    
    float4 vOriginColor = g_BlurTexture.Sample(DefaultSampler, In.vTexcoord);
    
    float4 vColor = g_BloomTexture.Sample(DefaultSampler, In.vTexcoord);
    
    float4 vResult = vOriginColor + (vColor * 0.4f); //g_fExposure);
    
    //Out.vColor = vResult;
    Out.vColor = float4(ToneMap(vResult.xyz), 1.f);
    
    return Out;
}

PS_OUT_BACKBUFFER PS_DISTORTION(PS_IN In)
{
    PS_OUT_BACKBUFFER Out = (PS_OUT_BACKBUFFER) 0;
    
    float2 vTexcoord;
    float2 vWeight;
    vector vNormal;
    vector vNormalData;
    
    vNormalData = g_DistortionTexture.Sample(PointSampler, In.vTexcoord);
    
    vNormalData = vector((vNormalData.xy * 2.f) - 1.f, vNormalData.z, vNormalData.a);
    vWeight = (vNormalData.xy * vNormalData.z) * vNormalData.a;
    
    vWeight *= 0.12f;

    vTexcoord = In.vTexcoord + vWeight;
    
    vector vFinalColor = g_BackBufferTexture.Sample(ClampSampler, vTexcoord);
    
    Out.vColor = vFinalColor;
    
    return Out;
}

PS_OUT_BACKBUFFER PS_LUT(PS_IN In)
{
    PS_OUT_BACKBUFFER Out = (PS_OUT_BACKBUFFER) 0;
    
    float4 vOriginColor = g_BackBufferTexture.Sample(DefaultSampler, In.vTexcoord);
    
    bool IsDynamic = g_PBRTexture.Sample(DefaultSampler, In.vTexcoord).z;
    bool IsSky = g_DepthTexture.Sample(DefaultSampler, In.vTexcoord).x;
    
    if ((false == g_IsDynamicLUT && true == IsDynamic) || false == IsSky)
    {
        Out.vColor = vOriginColor;
    
        return Out;
    }
    
    float2 vUV;
    
    float fSpaceSize = 1.f / g_fLUT_Size;
    float fScale = (g_fLUT_Size - 1.f) / g_fLUT_Size;
    
    float fIndex = clamp(floor(vOriginColor.b * g_fLUT_Size), 0, g_fLUT_Size - 1.f);
    float fOffsetX = fIndex * (fSpaceSize);
    
    float fScaleX = vOriginColor.r * fScale;
    
    vUV.x = fScaleX * fSpaceSize + fOffsetX;
    vUV.y = vOriginColor.g;
    
    vector vLUT_Color = g_LUT_Texture.Sample(DefaultSampler, float3(vUV, g_iLutIndex));
    
    vector vFinalColor = lerp(vOriginColor, vLUT_Color, g_fLutLerpIntensity);
    
    Out.vColor = float4(vFinalColor.rgb, 1.f);

    return Out;
}

PS_OUT_BACKBUFFER PS_FOG(PS_IN In)
{
    PS_OUT_BACKBUFFER Out = (PS_OUT_BACKBUFFER) 0;
    
    float4 vViewPos = Compute_ViewPos(In.vTexcoord, g_DepthTexture);
    
    float fViewZ = vViewPos.z == 0.f ? g_vFogRange.y : clamp(vViewPos.z, 0.1f, g_vFogRange.y);;
    
    vector vOriginColor = g_BackBufferTexture.Sample(DefaultSampler, In.vTexcoord);
    
    bool IsDynamic = g_PBRTexture.Sample(DefaultSampler, In.vTexcoord).z;
    
    if (true == IsDynamic)
    {
        Out.vColor = vOriginColor;
        return Out;
    }
    
    if (fViewZ <= g_vFogRange.x)
    {
        vOriginColor = float4(ToneMap(vOriginColor.xyz * g_fExposure), 1.f);
        Out.vColor = vOriginColor;
        return Out;
    }
    
    float fZ = saturate(log(fViewZ / g_vFogRange.x) / log(g_vFogRange.y / g_vFogRange.x));
    
    float3 vUV = float3(In.vTexcoord, fZ);
    
    float4 VF = g_VoulmetricTexture.Sample(ClampSampler, vUV);
    
    float3 vFogColor = VF.xyz;
    
    //float fAlpha = VF.a;
    float fAlpha = lerp(0.4f, 1.f, saturate(VF.a));
    
    float3 vFinalColor = vOriginColor.xyz * fAlpha + vFogColor.xyz;
//    float3 vFinalColor = lerp(vFogColor.xyz, vOriginColor.xyz, fAlpha);
    vFinalColor = ToneMap(vFinalColor.xyz * g_fExposure);
    
    Out.vColor.xyz = vFinalColor;
//    Out.vColor.xyz = vOriginColor.xyz * fAlpha + vFogColor.xyz;
  
    Out.vColor.a = 1.f;
    
    return Out;
}


PS_OUT_BACKBUFFER PS_SSAO(PS_IN In)
{
    PS_OUT_BACKBUFFER Out = (PS_OUT_BACKBUFFER) 0;

    vector vViewPos = Compute_ViewPos_SSAO(In.vTexcoord, g_DepthTexture);
   
    if (vViewPos.z == 0.f || vViewPos.z >= g_fSSAO_OutDistance)
    {
        Out.vColor = 1.f;
        return Out;
    }
    
    vector vNormal = Compute_Normal(g_NormalTexture, DefaultSampler, In.vTexcoord);
    vNormal = mul(vNormal, g_CamViewMatrix);
    
    float2 vNoiseScale = float2(g_fWidth / 16.f, g_fHeight / 16.f);
    
    vector vNoiseNormal = Compute_Normal(g_NoiseTexture, PointSampler, In.vTexcoord * vNoiseScale);
    vNoiseNormal = mul(vNoiseNormal, g_CamViewMatrix);
    
    float Occlusion = 0.f;
    
    [unroll]
    for (int i = 0; i < g_iSampleSize; ++i)
    {
        Occlusion += SSAO_Factor(g_vSampleVector[i], vNoiseNormal, vNormal, vViewPos, g_fSSAO_Radius, g_fSSAO_MaxDistance, g_DepthTexture);
    }
    
    float AO = (Occlusion / g_iSampleSize);
    
    if(AO >= 0.8f)
        AO = 1.f;                            
    
 //   AO = pow(AO, 2.f);
    
    Out.vColor.xyz = AO;
    Out.vColor.w = 1.f;
    
    return Out;
}

PS_OUT_BACKBUFFER PS_DOF(PS_IN In)
{
    PS_OUT_BACKBUFFER Out = (PS_OUT_BACKBUFFER) 0;

    float4 vOriginColor = g_BackBufferTexture.Sample(DefaultSampler, In.vTexcoord);
    float4 vDofColor = g_BlurTexture.Sample(DefaultSampler, In.vTexcoord);
    
    float4 vDofData = g_DofTexture.Sample(DefaultSampler, In.vTexcoord);
    
    float fMask = smoothstep(0.3f, 1.f, vDofData.x);
    
    float4 vFinalColor = lerp(vOriginColor, vDofColor, fMask);
    
    Out.vColor = lerp(vOriginColor, vFinalColor, g_fEffectIntensity);
    
    return Out;
}

PS_OUT_BACKBUFFER PS_DOF_DEPTH(PS_IN In)
{
    PS_OUT_BACKBUFFER Out = (PS_OUT_BACKBUFFER) 0;

    float fCoc = Compute_COC(In.vTexcoord, g_DepthTexture);
    
    float4 vDepth = 0.f;
    
    vDepth.x = fCoc;
    
    vDepth.y = fCoc <= g_fFocusMinCoc ? 0.f : 1.f;
    
    vDepth.z = g_fFocusMinCoc;
    
    Out.vColor = float4(vDepth.xyz, 1.f);
    
    return Out;
}

PS_OUT_BACKBUFFER PS_BLUR(PS_IN In)
{
    PS_OUT_BACKBUFFER Out = (PS_OUT_BACKBUFFER) 0;
    
    float4 vOriginColor = g_BackBufferTexture.Sample(DefaultSampler, In.vTexcoord);
    float4 vBlurColor = g_BlurTexture.Sample(DefaultSampler, In.vTexcoord);
    
    Out.vColor = lerp(vOriginColor, vBlurColor, g_fEffectIntensity);
    
    return Out;
}

PS_OUT_BACKBUFFER PS_VELOCITY_MAP(PS_IN In)
{
    PS_OUT_BACKBUFFER Out = (PS_OUT_BACKBUFFER) 0;
    
    bool IsDyanmic = g_PBRTexture.Sample(DefaultSampler, In.vTexcoord).z;       // Dynamic Discard;
    
    if(IsDyanmic)
    {
        Out.vColor.w = 1.f;
        return Out;
    }
    
    float4 vViewPos = Compute_ViewPos(In.vTexcoord, g_DepthTexture);
    
    Out.vColor.z = vViewPos.z;          // Depth ???
    if (vViewPos.z == 0.f || vViewPos.z >= g_fLimitDepth)
    {
        Out.vColor.xy = 0.f;
    }
    else
    {
        float4 vWorldPos = mul(vViewPos, g_ViewMatrixInv);
    
        float4x4 PrevVP = mul(g_PrevCamViewMatrix, g_PrevCamProjMatrix);
    
        float4 vPrevProjPos = mul(vWorldPos, PrevVP);
        vPrevProjPos /= vPrevProjPos.w;
    
        float2 vPrevTexcoord = Compute_Texcoord(vPrevProjPos.xy);
    
        float2 vCurTexcoord = float2(In.vTexcoord.x * g_fWidth, In.vTexcoord.y * g_fHeight);
        vPrevTexcoord = float2(vPrevTexcoord.x * g_fWidth, vPrevTexcoord.y * g_fHeight);
        
        float2 vMotionVector = vPrevTexcoord - vCurTexcoord;
        
        Out.vColor.xy = vMotionVector;
    }
    
    return Out;
}


PS_OUT_BACKBUFFER PS_MOTION_BLUR(PS_IN In)
{
    PS_OUT_BACKBUFFER Out = (PS_OUT_BACKBUFFER) 0;
    
    float4 vOriginColor = g_BackBufferTexture.Sample(DefaultSampler, In.vTexcoord);
    float4 vBlurColor = g_BlurTexture.Sample(DefaultSampler, In.vTexcoord);
    float4 vVelocity = g_VelocityMap.Sample(DefaultSampler, In.vTexcoord);
  
    bool IsBlur = (vVelocity.w != 1.f && length(vVelocity) > g_fLimitVelocity) ? true : false;
   
    //float fLerpRatio = clamp(g_fEffectIntensity, 0.f, 0.5f);
   
    if(IsBlur)
        Out.vColor = lerp(vOriginColor, vBlurColor, g_fEffectIntensity);
    else
        Out.vColor = vOriginColor;
    
    return Out;
}

PS_OUT_BACKBUFFER PS_WATER(PS_IN In)
{
    PS_OUT_BACKBUFFER Out = (PS_OUT_BACKBUFFER) 0;
    
    float4 vNormal = Compute_Normal(g_NormalTexture, DefaultSampler, In.vTexcoord);
    
    float4 vViewNormal = normalize(mul(vNormal, g_CamViewMatrix));
    
    float4 vViewPos = Compute_ViewPos(In.vTexcoord, g_DepthTexture);
    
    float4 vWorldPos = mul(vViewPos, g_ViewMatrixInv);
    
    float4 vOriginColor = g_BackBufferTexture.Sample(DefaultSampler, In.vTexcoord);
   
    float4 vWaterColor = g_DiffuseTexture.Sample(DefaultSampler, In.vTexcoord);
   
    float4 vPBRDesc = g_PBRTexture.Sample(DefaultSampler, In.vTexcoord);
   
    if(vPBRDesc.w != 1.f)
    {
        Out.vColor = vOriginColor;
        return Out;
    }
   
    float4 vSceneDesc = g_SkinMaskTexture.Sample(DefaultSampler, In.vTexcoord);
   
    float4 vSceneWorldPos = 0.f;
    
    vSceneWorldPos.x = In.vTexcoord.x * 2.f - 1.f;
    vSceneWorldPos.y = In.vTexcoord.y * -2.f + 1.f;
    vSceneWorldPos.z = vSceneDesc.z;
    vSceneWorldPos.w = 1.f;
    
    vSceneWorldPos *= vSceneDesc.w;
    
    vSceneWorldPos = mul(vSceneWorldPos, g_ProjMatrixInv);
    vSceneWorldPos = mul(vSceneWorldPos, g_ViewMatrixInv);
    
    float4 vReflectColor = Compute_Reflect(vWorldPos, vViewPos, vViewNormal, vOriginColor);
    
    float4 vRefractColor = Compute_Refract(vWorldPos, vNormal, vWaterColor, g_BackBufferTexture, (vWorldPos.y - vSceneWorldPos.y), g_DepthTexture);
    
    float3 vLook = normalize(g_vCamPosition.xyz - vWorldPos.xyz);
    
    float fNdotV = saturate(dot(vNormal.xyz, vLook));
    
    float3 vF0 = 0.02f;
    float3 vFresnel = Compute_Fresnel(vF0, fNdotV);
    
    float fReflectRatio = lerp(0.5f, 0.7f, vFresnel.r);
   
    Out.vColor = lerp(vRefractColor, vReflectColor, fReflectRatio);
    
    return Out;
}

PS_OUT_BACKBUFFER PS_MAIN_DEBUG_CSM(PS_IN In)
{
    PS_OUT_BACKBUFFER Out = (PS_OUT_BACKBUFFER) 0;
    
    float fShadow = 0.f;
   
    fShadow = g_Cascade.SampleCmpLevelZero(ShadowSampler, float3(In.vTexcoord, g_DebugCSMIndex), 1.f);
    
    float4 vColor = 1.f;
   
    if (fShadow != 1.f)
    {
        switch (g_DebugCSMIndex)
        {
            case 0:
                vColor = float4(fShadow, 0.f, 0.f, 1.f);
                break;
            case 1:
            
                vColor = float4(0.f, fShadow, 0.f, 1.f);
                break;
            case 2:
            
                vColor = float4(0.f, 0.f, fShadow, 1.f);
                break;
            case 3:
                vColor = float4(fShadow, fShadow, fShadow, 1.f);
                break;
        }
    }
   
    Out.vColor = vColor;
    
    return Out;
}

PS_OUT_BACKBUFFER PS_MAIN_DEBUG_SHADOW_MAP(PS_IN In)
{
    PS_OUT_BACKBUFFER Out = (PS_OUT_BACKBUFFER) 0;
    
    float fShadow = 0.f;
   
    fShadow = g_ShadowMap.SampleCmpLevelZero(ShadowSampler, float3(In.vTexcoord, g_DebugCSMIndex), 1.f);
   
    //fShadow = g_ShadowMap.Sample(DefaultSampler, float3(In.vTexcoord, 0.f));
    
    float4 vColor = 1.f;
    
    vColor.xyz = fShadow;
    vColor.a = 1.f;
    
    if (In.vTexcoord.x == 0.5f || In.vTexcoord.y == 0.5f)
        vColor = 0.f;
   
    //if (fShadow != 1.f)
    //{
    //    vColor = float4(fShadow, 0.f, 0.f, 1.f);
    //}
   
    Out.vColor = vColor;
    
    return Out;
}

PS_OUT_BACKBUFFER PS_WeightBlend(PS_IN In)
{
    PS_OUT_BACKBUFFER Out = (PS_OUT_BACKBUFFER) 0;
    
    float4 vColor = g_AccumColorTexture.Sample(DefaultSampler, In.vTexcoord);
    float4 vAlpha = g_AccumAlphaTexture.Sample(DefaultSampler, In.vTexcoord);
    float a = vAlpha.r;
    
    float4 vAccum = float4(0.f, 0.f, 0.f, 0.f);
    
    if (a > 1e-5f)
    {
        vAccum.rgb = saturate(max(vColor.rgb / a, 0.0f));
        vAccum.a = saturate(a);
    }

    Out.vColor = float4(vAccum.rgb * vAccum.a , vAccum.a);
    
    return Out;
}


technique11 DefaultTechnique
{
    pass DebugPass // 0
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_None, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_DRAW();
    }
    
    pass CSM // 1
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_None, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_DEBUG_CSM();
    }
    
    pass SHAODW_MAP // 2
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_None, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_DEBUG_SHADOW_MAP();
    }
    
    pass CombinedPass // 3
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_None, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_COMBINED();
    }
    
    pass LightPass  //4
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_None, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_LIGHT();
    }
    
    //pass DirectionalPass // 4
    //{
    //    SetRasterizerState(RS_Default);
    //    SetDepthStencilState(DSS_None, 0);
    //    SetBlendState(BS_Blend, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

    //    VertexShader = compile vs_5_0 VS_MAIN();
    //    GeometryShader = NULL;
    //    PixelShader = compile ps_5_0 PS_LIGHT_DIRECTIONAL();
    //}
    //pass PointPass // 5
    //{
    //    SetRasterizerState(RS_Default);
    //    SetDepthStencilState(DSS_None, 0);
    //    SetBlendState(BS_Blend, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

    //    VertexShader = compile vs_5_0 VS_MAIN();
    //    GeometryShader = NULL;
    //    PixelShader = compile ps_5_0 PS_LIGHT_POINT();
    //}
    
    pass Bloom // 5
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_None, 0);
        SetBlendState(BS_Blend, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_BLOOM();
    }
    
    pass Distortion // 6
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_None, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_DISTORTION();
    }
   
    pass LUT // 7
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_None, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_LUT();
    }
    
    pass Fog // 8
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_None, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_FOG();
    }
    
    pass SSAO // 9
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_None, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_SSAO();
    }
    
    pass DOF // 10
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_None, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_DOF();
    }
    
    pass DOF_DEPTH // 11
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_None, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_DOF_DEPTH();
    }
    
    pass Blur   // 12
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_None, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_BLUR();
    }
    
    pass VelocityMap // 13
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_None, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_VELOCITY_MAP();
    }
    
    pass MotionBlur // 14
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_None, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MOTION_BLUR();
    }
    
    pass WATER // SSR   15
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_None, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_WATER(); // PS_SSR
    }

    pass WEIGHTBLEND    //16
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_None, 0);
        SetBlendState(BS_WeightBlend, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_WeightBlend();
    }
}