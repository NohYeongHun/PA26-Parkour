#include "Engine_Shader_Defines.hlsli"

matrix g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;

texture2D g_DiffuseTexture;
texture2D g_MaskTexture;
texture2D g_DissolveTexture;
texture2D g_DistortionTexture;

//색상
float g_Sweep;
float g_SweepSpeed;
float g_SweepWitdh;
float g_Soft = 0.3f; //툴에서 받아올 수 있게 해주자.

//공용
int g_Dir; 
float g_Time;
float g_Alpha;
float2 g_LifeTime;
int g_MaskFlag; // 0이면 R로, 1이면 알파로

float g_DistortionWeight;

//
float g_ColorGain; // 밝기 스케일 0~1
float g_ColorGamma; // 톤 커버, (1 == 그대로, >1 어두워지게)

//마스크
float2 g_MaskSweep;
float2 g_MaskOffset;
float g_MaskMix;
float g_MaskSoft;

//임시
float g_MaskSpeed = 1.f;
float g_ColorSpeed;

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
    float4 vEmissive : SV_TARGET1;
    float4 vDistortion : SV_TARGET2;
    float4 vAccumColor : SV_TARGET3;     //TEST
    float4 vAccumAlpha : SV_TARGET4;
};

struct PS_DISTORTION_OUT
{
    float4 vDistortion : SV_TARGET0;
};

PS_OUT PS_MAIN(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;

    Out.vDiffuse = g_DiffuseTexture.Sample(DefaultSampler, In.vTexcoord);

    if (Out.vDiffuse.a < 0.3f)
        discard;

    float fWeight = Luminance(Out.vDiffuse.xyz);

    if (fWeight >= g_fEmissiveThreshold)
        Out.vEmissive = float4(Out.vDiffuse.xyz, 1.f);

    Out.vDistortion.a = 0; //?

    return Out;
}

PS_OUT PS_TrailDefault(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;

//float2 UV = In.vTexcoord;

//float MaskR = g_MaskTexture.Sample(ClampSampler, UV).r;

//if( MaskR < 0.2f)
//    discard;

//float fY = 1.f - g_MaskSweep;
//float fVisibleY;

//fVisibleY = 1.f - step(fY, UV.y);

//float fVisibleX;

//float fTailFad = smoothstep(g_Sweep - g_SweepWitdh, g_Sweep - g_SweepWitdh + g_Soft, In.vTexcoord.x);

//float fHeadFad = 1 - smoothstep(g_Sweep - g_Soft, g_Sweep, In.vTexcoord.x);

//fVisibleX = fTailFad * fHeadFad;

//float2 ColorUV = In.vTexcoord;
//ColorUV -= g_Sweep;
//float4 vColor = g_DiffuseTexture.Sample(DefaultSampler, ColorUV);

//float fAlpha = fVisibleY * fVisibleX * MaskR;

//if(fAlpha < 0.2f)
//    discard;

//Out.vDiffuse = float4(vColor.arb, fAlpha);

//float fWeight = Luminance(Out.vDiffuse.xyz);

//if (fWeight >= g_fEmissiveThreshold)
//    Out.vEmissive = float4(Out.vDiffuse.xyz, 1.f);l

    float2 MaskUV = In.vTexcoord;

    MaskUV -= g_Sweep;

    float4 vMask = g_MaskTexture.Sample(ClampSampler, MaskUV);

    float MaskAlpha;

    if (g_MaskFlag == 1)
    {
        if (vMask.a < 0.3f)
            discard;

        MaskAlpha = vMask.a;

    }
    else
    {
        if (vMask.r < 0.35f)
            discard;

        MaskAlpha = vMask.r;
    }

    float2 ColorUV = In.vTexcoord;
    ColorUV -= g_Sweep; //X로 긴 텍스처니까 색상 움직이듯 보여질려면 이렇게 해야하나?
    float4 vColor = g_DiffuseTexture.Sample(DefaultSampler, ColorUV);

     vColor.rgb = saturate(vColor.rgb);
     vColor.rgb = pow(vColor.rgb, g_ColorGamma);
     vColor.rgb *= g_ColorGain;

    float alpha;

    if (g_Dir == 1)
    {
        float fTailFad = smoothstep(g_Sweep - g_SweepWitdh, g_Sweep - g_SweepWitdh + g_Soft, In.vTexcoord.x);

        float fHeadFad = 1 - smoothstep(g_Sweep - g_Soft, g_Sweep, In.vTexcoord.x);

        float fVisible = fTailFad * fHeadFad;

        alpha = fVisible * MaskAlpha;
    }
    else
    {
        float fTailFad = smoothstep(g_Sweep - g_SweepWitdh, g_Sweep - g_SweepWitdh + g_Soft, 1 - In.vTexcoord.x);

        float fHeadFad = 1 - smoothstep(g_Sweep - g_Soft, g_Sweep, 1 - In.vTexcoord.x);

        float fVisible = fTailFad * fHeadFad;

        alpha = fVisible * MaskAlpha;
    }

    if (alpha < 0.3f)
        discard;

    Out.vDiffuse = float4(vColor.rgb, alpha);
    
    Out.vDiffuse.a *= g_Alpha;

    float fWeight = Luminance(Out.vDiffuse.xyz);

    if (fWeight >= g_fEmissiveThreshold)
        Out.vEmissive = float4(Out.vDiffuse.xyz, 1.f);
    
    Out.vEmissive.xyz *= Out.vDiffuse.a;
    
    float z = In.vProjPos.z / In.vProjPos.w;
    
    float Weight = max(1e-5, exp(-z * g_WeightBlend));
    
    Out.vAccumColor = float4(Out.vDiffuse.rgb * Out.vDiffuse.a, 0.0f) * Weight;
    Out.vAccumAlpha.r = Out.vDiffuse.a * Weight;
    Out.vDiffuse = float4(0.f, 0.f, 0.f, 0.f);
    return Out;
}

PS_OUT PS_TraillRightTail(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;

    float2 UV = In.vTexcoord;

    float MaskR = g_MaskTexture.Sample(ClampSampler, UV).r;

    if (MaskR < 0.35f)
        discard;

    float fY = 1.f - g_MaskSweep;
    float fVisibleY;

    fVisibleY = 1.f - step(fY, UV.y);

    float fVisibleX;

    float fTailFad = smoothstep(g_Sweep - g_SweepWitdh, g_Sweep - g_SweepWitdh + g_Soft, 1.f - In.vTexcoord.x);

    float fHeadFad = 1.f - smoothstep(g_Sweep - g_Soft, g_Sweep, 1 - In.vTexcoord.x);

    fVisibleX = fTailFad * fHeadFad;

    float4 vColor = g_DiffuseTexture.Sample(DefaultSampler, float2(0.5f, saturate(In.vTexcoord.y)));

    vColor.rgb = saturate(vColor.rgb);
    vColor.rgb = pow(vColor.rgb, g_ColorGamma);
    vColor.rgb *= g_ColorGain;

    float fAlpha = fVisibleY * fVisibleX * MaskR;

    if (fAlpha < 0.2f)
        discard;

    Out.vDiffuse = float4(vColor.rgb, fAlpha);

    float fWeight = Luminance(Out.vDiffuse.xyz);

    if (fWeight >= g_fEmissiveThreshold)
        Out.vEmissive = float4(Out.vDiffuse.xyz, 1.f);

    Out.vDiffuse *= g_Alpha;
    
    Out.vEmissive.xyz *= Out.vDiffuse.a;

//float4 vMask = g_MaskTexture.Sample(ClampSampler, In.vTexcoord);

////마스크 알파 값으로 잘라내기 처리, 만약 검정색이면 r로 해도 될듯함.
//if (vMask.r < 0.35f)
//    discard;

//// 1 - x
//float fTailFad = smoothstep(g_Sweep - g_SweepWitdh, g_Sweep - g_SweepWitdh + g_Soft, 1 - In.vTexcoord.x);

//float fHeadFad = 1 - smoothstep(g_Sweep - g_Soft, g_Sweep, 1 - In.vTexcoord.x);

//float fVisible = fTailFad * fHeadFad;

//float4 vColor = g_DiffuseTexture.Sample(DefaultSampler, float2(0.5f, saturate(In.vTexcoord.y)));

//float fAlpha = fVisible * vMask.a;

//Out.vDiffuse = float4(vColor.rgb * fAlpha, fAlpha);

//if (Out.vDiffuse.r < 0.35f)      //테스트
//    discard;

//float fWeight = Luminance(Out.vDiffuse.xyz);

//if (fWeight >= g_fEmissiveThreshold)
//    Out.vEmissive = float4(Out.vDiffuse.xyz, 1.f);
    
    float z = In.vProjPos.z / In.vProjPos.w;
    float Weight = max(1e-5, exp(-z * g_WeightBlend));
    
    Out.vAccumColor = float4(Out.vDiffuse.rgb * Out.vDiffuse.a, 0.0f) * Weight;
    Out.vAccumAlpha.r = Out.vDiffuse.a * Weight;
    Out.vDiffuse = float4(0.f, 0.f, 0.f, 0.f);
   
    return Out;
}

PS_OUT PS_Y_OUT(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;

    float2 UV = In.vTexcoord;

    float2 MaskUV = UV;

    MaskUV.x = frac(MaskUV.x + g_MaskSpeed * g_Time);

    float MaskR = g_MaskTexture.Sample(ClampSampler, MaskUV).r;

    if (MaskR < 0.3f)
        discard;

    float fY = 1.f - g_MaskSweep;
    float fVisibleY;

    fVisibleY = 1.f - step(fY, UV.y);

    float fVisibleX;

    float fTailFad = smoothstep(g_Sweep - g_SweepWitdh, g_Sweep - g_SweepWitdh + g_Soft, 1.f - In.vTexcoord.y);

    float fHeadFad = 1.f - smoothstep(g_Sweep - g_Soft, g_Sweep, 1.f - In.vTexcoord.y);

    fVisibleX = fTailFad * fHeadFad;

    float4 vColor;
    
    if(g_Dir == 0)
        vColor = g_DiffuseTexture.Sample(DefaultSampler, float2(0.5f, saturate(In.vTexcoord.y)));
    else
        vColor = g_DiffuseTexture.Sample(DefaultSampler, float2(0.5f, saturate(In.vTexcoord.x)));

    vColor.rgb = saturate(vColor.rgb);
    vColor.rgb = pow(vColor.rgb, g_ColorGamma);
    vColor.rgb *= g_ColorGain;

    float fAlpha = fVisibleY * fVisibleX * MaskR;

    if (fAlpha < 0.2f)
        discard;

    Out.vDiffuse = float4(vColor.rgb, fAlpha);
    
    float fWeight = Luminance(Out.vDiffuse.xyz);

    if (fWeight >= g_fEmissiveThreshold)
        Out.vEmissive = float4(Out.vDiffuse.xyz, 1.f);

    Out.vDiffuse.a *= g_Alpha;
    
    Out.vEmissive.xyz *= Out.vDiffuse.a;
    
    float z = In.vProjPos.z / In.vProjPos.w;
    
    float Weight = max(1e-5, exp(-z * g_WeightBlend));
    
    Out.vAccumColor = float4(Out.vDiffuse.rgb * Out.vDiffuse.a, 0.0f) * Weight;
    Out.vAccumAlpha.r = Out.vDiffuse.a * Weight;
    Out.vDiffuse = float4(0.f, 0.f, 0.f, 0.f);
    return Out;
}

PS_OUT PS_Y_IN(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;

    float2 UV = In.vTexcoord;

    float MaskR = g_MaskTexture.Sample(ClampSampler, UV).r;

    if (MaskR < 0.35f)
        discard;

    float fY = 1.f - g_MaskSweep;
    float fVisibleY;

    fVisibleY = 1.f - step(fY, UV.y);

    float fVisibleX;

    float fTailFad = smoothstep(g_Sweep - g_SweepWitdh, g_Sweep - g_SweepWitdh + g_Soft, In.vTexcoord.y);

    float fHeadFad = 1.f - smoothstep(g_Sweep - g_Soft, g_Sweep, In.vTexcoord.y);

    fVisibleX = fTailFad * fHeadFad;

    float4 vColor;
    
    if (g_Dir == 0)
        vColor = g_DiffuseTexture.Sample(DefaultSampler, float2(0.5f, saturate(In.vTexcoord.y)));
    else
        vColor = g_DiffuseTexture.Sample(DefaultSampler, float2(0.5f, saturate(In.vTexcoord.x)));
    
    
    vColor.rgb = saturate(vColor.rgb);
    vColor.rgb = pow(vColor.rgb, g_ColorGamma);
    vColor.rgb *= g_ColorGain;

    float fAlpha = fVisibleY * fVisibleX * MaskR;

    if (fAlpha < 0.2f)
        discard;

    Out.vDiffuse = float4(vColor.rgb, fAlpha);

    float fWeight = Luminance(Out.vDiffuse.xyz);

    if (fWeight >= g_fEmissiveThreshold)
        Out.vEmissive = float4(Out.vDiffuse.xyz, 1.f);

    Out.vDiffuse.a *= g_Alpha;
    
    Out.vEmissive.xyz *= Out.vDiffuse.a;
    
    
    float z = In.vProjPos.z / In.vProjPos.w;
    
    float Weight = max(1e-5, exp(-z * g_WeightBlend));
    
    Out.vAccumColor = float4(Out.vDiffuse.rgb * Out.vDiffuse.a, 0.0f) * Weight;
    Out.vAccumAlpha.r = Out.vDiffuse.a * Weight;
    Out.vDiffuse = float4(0.f, 0.f, 0.f, 0.f);
    
    
    return Out;
}

PS_OUT PS_TraillLeftTail(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;

    float2 UV = In.vTexcoord;

    float MaskR = g_MaskTexture.Sample(ClampSampler, UV).r;

    if (MaskR < 0.35f)
        discard;

    float fY = 1.f - g_MaskSweep;
    float fVisibleY;

    fVisibleY = 1.f - step(fY, UV.y);

    float fVisibleX;

    float fTailFad = smoothstep(g_Sweep - g_SweepWitdh, g_Sweep - g_SweepWitdh + g_Soft, In.vTexcoord.x);

    float fHeadFad = 1.f - smoothstep(g_Sweep - g_Soft, g_Sweep, In.vTexcoord.x);

    fVisibleX = fTailFad * fHeadFad;

    float4 vColor;
    
    if (g_Dir == 0)
        vColor = g_DiffuseTexture.Sample(DefaultSampler, float2(0.5f, saturate(In.vTexcoord.y)));
    else
        vColor = g_DiffuseTexture.Sample(DefaultSampler, float2(0.5f, saturate(In.vTexcoord.x)));

    float fAlpha = fVisibleY * fVisibleX * MaskR;

    vColor.rgb = saturate(vColor.rgb);
    vColor.rgb = pow(vColor.rgb, g_ColorGamma);
    vColor.rgb *= g_ColorGain;

    if (fAlpha < 0.2f)
        discard;

    Out.vDiffuse = float4(vColor.rgb, fAlpha);

    float fWeight = Luminance(Out.vDiffuse.xyz);

    if (fWeight >= g_fEmissiveThreshold)
        Out.vEmissive = float4(Out.vDiffuse.xyz, 1.f);

//float4 vMask = g_MaskTexture.Sample(ClampSampler, In.vTexcoord);

////마스크 알파 값으로 잘라내기 처리, 만약 검정색이면 r로 해도 될듯함.
//if (vMask.r < 0.35f)
//    discard;

//// 1x
//float fTailFad = smoothstep(g_Sweep - g_SweepWitdh, g_Sweep - g_SweepWitdh + g_Soft, In.vTexcoord.x);

//float fHeadFad = 1 - smoothstep(g_Sweep - g_Soft, g_Sweep, In.vTexcoord.x);

//float fVisible = fTailFad * fHeadFad;

//float4 vColor = g_DiffuseTexture.Sample(DefaultSampler, float2(0.5f, saturate(In.vTexcoord.y)));

//float fAlpha = fVisible * vMask.a;

//Out.vDiffuse = float4(vColor.rgb * fAlpha, fAlpha);

//if (Out.vDiffuse.r < 0.35f)      //테스트
//    discard;

//float fWeight = Luminance(Out.vDiffuse.xyz);

//if (fWeight >= g_fEmissiveThreshold)
//    Out.vEmissive = float4(Out.vDiffuse.xyz, 1.f);

    Out.vDiffuse.a *= g_Alpha;
    
    Out.vEmissive.xyz *= Out.vDiffuse.a;
    
    float z = In.vProjPos.z / In.vProjPos.w;
    
    float Weight = max(1e-5, exp(-z * g_WeightBlend));
    
    Out.vAccumColor = float4(Out.vDiffuse.rgb * Out.vDiffuse.a, 0.0f) * Weight;
    Out.vAccumAlpha.r = Out.vDiffuse.a * Weight;
    Out.vDiffuse = float4(0.f, 0.f, 0.f, 0.f);
    
    return Out;
}

PS_OUT PS_TraillDissolve(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;

    float2 UV = In.vTexcoord;

    float4 Mask = g_MaskTexture.Sample(DefaultSampler, UV);

    float MaskR = max(max(Mask.r, Mask.g), Mask.b);

    if (MaskR < 0.15f)
        discard;

    float Dissolve = g_DissolveTexture.Sample(DefaultSampler, UV).r;
    
    float fRatio = g_LifeTime.x / g_LifeTime.y;

    if (Dissolve - fRatio < 0.f)
        discard;

    float4 vColor = g_DiffuseTexture.Sample(DefaultSampler, float2(0.5f, saturate(In.vTexcoord.y)));

    vColor.rgb = saturate(vColor.rgb);
    vColor.rgb = pow(vColor.rgb, g_ColorGamma);
    vColor.rgb *= g_ColorGain;

    Out.vDiffuse = vColor;

    if (Dissolve - fRatio <= 0.1f)
    {
        Out.vDiffuse.rgb *= 3.f;
    }
    
    float fWeight = Luminance(Out.vDiffuse.xyz);
    
    if (fWeight >= g_fEmissiveThreshold)
        Out.vEmissive = float4(Out.vDiffuse.xyz, 1.f);

    Out.vDiffuse.a *= g_Alpha;
    Out.vEmissive.xyz *= Out.vDiffuse.a;
    
    
    float z = In.vProjPos.z / In.vProjPos.w;
    
    float Weight = max(1e-5, exp(-z * g_WeightBlend));
    
    Out.vAccumColor = float4(Out.vDiffuse.rgb * Out.vDiffuse.a, 0.0f) * Weight;
    Out.vAccumAlpha.r = Out.vDiffuse.a * Weight;
    Out.vDiffuse = float4(0.f, 0.f, 0.f, 0.f);
    
    return Out;
}

// ==Test==
PS_OUT PS_TraillDesh(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;

    float2 UV = In.vTexcoord;

    UV.y -= g_MaskSpeed * g_Sweep; //타임
    UV.y = frac(UV.y);

    float4 mask = g_MaskTexture.Sample(ClampSampler, UV);

    if (mask.r < 0.35f)
        discard;

    Out.vDiffuse = mask * 0.6f;

    if (Out.vDiffuse.r < 0.2f)      //테스트
        discard;
   

    float fWeight = Luminance(Out.vDiffuse.xyz);

    if (fWeight >= g_fEmissiveThreshold)
        Out.vEmissive = float4(Out.vDiffuse.xyz, 1.f);

    Out.vDiffuse.rgb *= Out.vDiffuse.a;
    
    float z = In.vProjPos.z / In.vProjPos.w;
    
    float Weight = max(1e-5, exp(-z * g_WeightBlend));
    
    Out.vAccumColor = float4(Out.vDiffuse.rgb * Out.vDiffuse.a, 0.0f) * Weight;
    Out.vAccumAlpha.r = Out.vDiffuse.a * Weight;
    Out.vDiffuse = float4(0.f, 0.f, 0.f, 0.f);
    
    return Out;
}

PS_OUT PS_TraillDeshB(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;

    float2 UV = In.vTexcoord;

    UV.y += g_MaskSpeed * g_Sweep; //타임
    UV.y = frac(UV.y);

    float4 mask = g_MaskTexture.Sample(ClampSampler, UV);

    if (mask.r < 0.35f)
        discard;

    Out.vDiffuse = mask * 0.6f;

    if (Out.vDiffuse.r < 0.2f)      //테스트
        discard;
 
    float fWeight = Luminance(Out.vDiffuse.xyz);

    if (fWeight >= g_fEmissiveThreshold)
        Out.vEmissive = float4(Out.vDiffuse.xyz, 1.f);

    Out.vDiffuse.rgb *= Out.vDiffuse.a;
    
    float z = In.vProjPos.z / In.vProjPos.w;
    
    float Weight = max(1e-5, exp(-z * g_WeightBlend));
    
    Out.vAccumColor = float4(Out.vDiffuse.rgb * Out.vDiffuse.a, 0.0f) * Weight;
    Out.vAccumAlpha.r = Out.vDiffuse.a * Weight;
    Out.vDiffuse = float4(0.f, 0.f, 0.f, 0.f);
    
    return Out;
}

PS_DISTORTION_OUT PS_DistortionWave(PS_IN In)
{
    PS_DISTORTION_OUT Out = (PS_DISTORTION_OUT) 0;

    float2 UV = In.vTexcoord;

    float waveFreq = 6.0f; // 줄무늬 얼마나 촘촘한지
    float waveAmp = 0.01f; // 얼마나 흔들릴지
    float waveSpeed = 1.5f; // 얼마나 빨리 움직일지

    float wave = sin(UV.y * waveFreq + g_Time * waveSpeed);

    float2 duv;
    duv.x = wave * waveAmp;
    duv.y = 0.0f;

    float4 vDist = g_DistortionTexture.Sample(DefaultSampler, UV + duv);

    Out.vDistortion = vDist;
    Out.vDistortion.a = g_DistortionWeight;

//speed , freq, time
// wave , x y 기준,

    return Out;
}

PS_DISTORTION_OUT PS_DistortionPotal(PS_IN In)
{
    PS_DISTORTION_OUT Out = (PS_DISTORTION_OUT) 0;

    float2 UV = In.vTexcoord;

    float2 Center = float2(0.5f, 0.5f);

    float2 Dir = Center - UV;

    float Dist = length(Dir); //거리에 따라 강도 쎄게

    float Weight = saturate(1 - Dist);

    float2 DirN = normalize(Dir);

    float SpinSpeed = 0.3f; //임시값

    float SpinFreq = 1.f;

    float AngleWave = sin(g_Time * SpinSpeed + Dist * SpinFreq);

    float2 OffsetDir = float2(-DirN.y, DirN.x);

    float2 Offset = OffsetDir * (g_DistortionWeight * Weight * AngleWave);

    float4 vDist = g_DistortionTexture.Sample(DefaultSampler, UV + Offset);

    Out.vDistortion = vDist;

    Out.vDistortion.a = g_DistortionWeight;
    

    return Out;
}

PS_OUT PS_TrailAlphaLeft(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;

    float2 UV = In.vTexcoord;

    float2 MaskUV = UV;

    MaskUV.x = 1 - frac(MaskUV.x + g_MaskSpeed * g_Time);

    float4 Mask = g_MaskTexture.Sample(ClampSampler, MaskUV);

    float MaskR = max(max(Mask.r, Mask.g), Mask.b);
    
    if (MaskR < 0.2f)
        discard;

    if (UV.x > g_Sweep)
        discard;

    float Sweep = g_Sweep - UV.x;; //0이면 머리,

    float FadeRange = 0.8;

    float fAge = saturate(1.f - Sweep / FadeRange);

    float4 vColor;
    
    if (g_Dir == 0)
        vColor = g_DiffuseTexture.Sample(DefaultSampler, float2(0.5f, saturate(In.vTexcoord.y)));
    else
        vColor = g_DiffuseTexture.Sample(DefaultSampler, float2(0.5f, saturate(In.vTexcoord.x)));

    vColor.rgb = saturate(vColor.rgb);
    vColor.rgb = pow(vColor.rgb, g_ColorGamma);
    vColor.rgb *= g_ColorGain;

    Out.vDiffuse = float4(vColor.rgb, fAge);
    Out.vDiffuse.a *= g_Alpha;

    if (Out.vDiffuse.a < 0.1f)
        discard;

   float fWeight = Luminance(Out.vDiffuse.xyz);

   if (fWeight >= g_fEmissiveThreshold)
       Out.vEmissive = float4(Out.vDiffuse.xyz, 1.f);
  
    Out.vEmissive.xyz *= Out.vDiffuse.a;
    
    float z = In.vProjPos.z / In.vProjPos.w;
    
    float Weight = max(1e-5, exp(-z * g_WeightBlend));
    
    Out.vAccumColor = float4(Out.vDiffuse.rgb * Out.vDiffuse.a, 0.0f) * Weight;
    Out.vAccumAlpha.r = Out.vDiffuse.a * Weight;
    Out.vDiffuse = float4(0.f, 0.f, 0.f, 0.f);
    
    return Out;
}

PS_OUT PS_TrailAlphaRight(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;

    float2 UV = In.vTexcoord;

    float2 MaskUV = UV;

    MaskUV.x = frac(MaskUV.x + g_MaskSpeed * g_Time);

    float4 Mask = g_MaskTexture.Sample(ClampSampler, MaskUV);

    float MaskR = max(max(Mask.r, Mask.g), Mask.b);
    
    if (MaskR < 0.2f)
        discard;

    if (UV.x < 1.f - g_Sweep)
        discard;

    float Sweep = UV.x - (1.f - g_Sweep); //0이면 머리,

    float FadeRange = 0.8;

    float fAge = saturate(1.f - Sweep / FadeRange);

    float4 vColor;
    
    if (g_Dir == 0)
        vColor = g_DiffuseTexture.Sample(DefaultSampler, float2(0.5f, saturate(In.vTexcoord.y)));
    else
        vColor = g_DiffuseTexture.Sample(DefaultSampler, float2(0.5f, saturate(In.vTexcoord.x)));

    vColor.rgb = saturate(vColor.rgb);
    vColor.rgb = pow(vColor.rgb, g_ColorGamma);
    vColor.rgb *= g_ColorGain;

    Out.vDiffuse = float4(vColor.rgb, fAge);
    Out.vDiffuse.a *= g_Alpha;
    
    if (Out.vDiffuse.a < 0.1f)
        discard;

    float fWeight = Luminance(Out.vDiffuse.xyz);

    if (fWeight >= g_fEmissiveThreshold)
        Out.vEmissive = float4(Out.vDiffuse.xyz, 1.f);
   
    Out.vEmissive.xyz *= Out.vDiffuse.a;
    
    float z = In.vProjPos.z / In.vProjPos.w;
    
    float Weight = max(1e-5, exp(-z * g_WeightBlend));
    
    Out.vAccumColor = float4(Out.vDiffuse.rgb * Out.vDiffuse.a, 0.0f) * Weight;
    Out.vAccumAlpha.r = Out.vDiffuse.a * Weight;
    Out.vDiffuse = float4(0.f, 0.f, 0.f, 0.f);
    
    return Out;
}

PS_OUT PS_DefaultMeshRender(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;
    
    //그냥 팡 하고 나타나는거 어색할거 같으니, 임시로 스윕처리?
    //스윕을 a에 박아줄까.
    
    float fAlpha = saturate(g_Sweep);
    
    float2 UV = In.vTexcoord;
    
    float Dissolve = g_DissolveTexture.Sample(DefaultSampler, UV).r;
    
    float fRatio = g_LifeTime.x / g_LifeTime.y;

    if (Dissolve - fRatio < 0.f)
        discard;

    float4 vColor;
    
    if (g_Dir == 0)
        vColor = g_DiffuseTexture.Sample(DefaultSampler, float2(0.5f, saturate(In.vTexcoord.y)));
    else
        vColor = g_DiffuseTexture.Sample(DefaultSampler, float2(0.5f, saturate(In.vTexcoord.x)));

    vColor.a = fAlpha;
    
    vColor.rgb = saturate(vColor.rgb);
    vColor.rgb = pow(vColor.rgb, g_ColorGamma);
    vColor.rgb *= g_ColorGain;

    if (Dissolve - fRatio <= 0.1f)
    {
        vColor.rgb *= 3.f;
    }
    
    float fWeight = Luminance(vColor.xyz);

    if (fWeight >= g_fEmissiveThreshold)
        Out.vEmissive = float4(vColor.xyz, 1.f); 
    
    Out.vDiffuse = vColor;
    
    Out.vDiffuse.a *= g_Alpha;
    
    
    // 단순 매쉬랜더는 웨이트블랜드 적용 안하는게 나을지도
    //float z = In.vProjPos.z / In.vProjPos.w;
    
    //float Weight = max(1e-5, exp(-z * g_WeightBlend));
    
    //Out.vAccumColor = float4(vColor.rgb * vColor.a, vColor.a) * Weight;
    //Out.vAccumAlpha.r = vColor.a * Weight;
    
    return Out;
}

PS_OUT PS_X_OUT(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;

    float2 UV = In.vTexcoord;

    float MaskR = g_MaskTexture.Sample(ClampSampler, UV).r;

    if (MaskR < 0.2f)
        discard;

    float fX = 1.f - g_MaskSweep;
    float fVisibleX;

    fVisibleX = 1.f - step(fX, UV.x);

    float fVisibleY;

    float fTailFad = smoothstep(g_Sweep - g_SweepWitdh, g_Sweep - g_SweepWitdh + g_Soft, In.vTexcoord.x);

    float fHeadFad = 1.f - smoothstep(g_Sweep - g_Soft, g_Sweep, In.vTexcoord.x);

    fVisibleY = fTailFad * fHeadFad;

    float4 vColor;
    
    if (g_Dir == 0)
        vColor = g_DiffuseTexture.Sample(DefaultSampler, float2(0.5f, saturate(In.vTexcoord.x)));
    else
        vColor = g_DiffuseTexture.Sample(DefaultSampler, float2(0.5f, saturate(In.vTexcoord.y)));
    
    
    vColor.rgb = saturate(vColor.rgb);
    vColor.rgb = pow(vColor.rgb, g_ColorGamma);
    vColor.rgb *= g_ColorGain;

    float fAlpha = fVisibleY * fVisibleX * MaskR;

    if (fAlpha < 0.2f)
        discard;

    Out.vDiffuse = float4(vColor.rgb, fAlpha);

    float fWeight = Luminance(Out.vDiffuse.xyz);

    if (fWeight >= g_fEmissiveThreshold)
        Out.vEmissive = float4(Out.vDiffuse.xyz, 1.f);

    Out.vDiffuse.a *= g_Alpha;
    
    Out.vEmissive.xyz *= Out.vDiffuse.a;
    
    
    float z = In.vProjPos.z / In.vProjPos.w;
    
    float Weight = max(1e-5, exp(-z * g_WeightBlend));
    
    Out.vAccumColor = float4(Out.vDiffuse.rgb * Out.vDiffuse.a, 0.0f) * Weight;
    Out.vAccumAlpha.r = Out.vDiffuse.a * Weight;
    Out.vDiffuse = float4(0.f, 0.f, 0.f, 0.f);
    
    
    return Out;
}

PS_OUT PS_SkyTrail(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;

    float2 UV = In.vTexcoord;

    float2 MaskUV = UV;
    MaskUV.x = frac(MaskUV.x + g_MaskSpeed * g_Time);
    
    float MaskR = g_MaskTexture.Sample(ClampSampler, MaskUV).r;

    if (MaskR < 0.3f)
        discard;

    float fVisibleY;

    fVisibleY = 1.f - step(g_Sweep, UV.y);

    float4 vColor;
    
    if (g_Dir == 0)
        vColor = g_DiffuseTexture.Sample(DefaultSampler, float2(0.5f, saturate(In.vTexcoord.y)));
    else
        vColor = g_DiffuseTexture.Sample(DefaultSampler, float2(0.5f, saturate(In.vTexcoord.x)));
    
    
    vColor.rgb = saturate(vColor.rgb);
    vColor.rgb = pow(vColor.rgb, g_ColorGamma);
    vColor.rgb *= g_ColorGain;

    float fAlpha = fVisibleY * MaskR;

    if (fAlpha < 0.2f)
        discard;

    float fRatio = g_LifeTime.x / g_LifeTime.y;
    
    if(1.f - fRatio <= 0.4f)
        fAlpha *= 1.f - fRatio;
    
    Out.vDiffuse = float4(vColor.rgb, fAlpha);

    float fWeight = Luminance(Out.vDiffuse.xyz);

    if (fWeight >= g_fEmissiveThreshold)
        Out.vEmissive = float4(Out.vDiffuse.xyz, 1.f);

    Out.vDiffuse.a *= g_Alpha;
    
    Out.vEmissive.xyz *= Out.vDiffuse.a;
    
    float z = In.vProjPos.z / In.vProjPos.w;
    
    float Weight = max(1e-5, exp(-z * g_WeightBlend));
    
    Out.vAccumColor = float4(Out.vDiffuse.rgb * Out.vDiffuse.a, 0.0f) * Weight;
    Out.vAccumAlpha.r = Out.vDiffuse.a * Weight;
    Out.vDiffuse = float4(0.f, 0.f, 0.f, 0.f);
    
    return Out;
}

PS_OUT PS_SkyTrailColor(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;

    float2 UV = In.vTexcoord;

    //float2 MaskUV = UV;
    //MaskUV.x = frac(MaskUV.x + g_MaskSpeed * g_Time);
    
    float Center = 0.5f;
    float Dx = UV.y - Center;
    float Dist = abs(Dx); // 중심에서 거리

    float Radial = frac(Dist + g_MaskSpeed * g_Time);

    float2 FlowUV;
    FlowUV.x = UV.x;
    FlowUV.y = Radial;
    
    float fVisibleY;

    fVisibleY = step(1.f - g_Sweep, UV.y);

    float4 vColor;
    
    if (g_Dir == 0)
        vColor = g_DiffuseTexture.Sample(DefaultSampler, float2(FlowUV.x, saturate(FlowUV.y)));
    else
        vColor = g_DiffuseTexture.Sample(DefaultSampler, float2(FlowUV.y, saturate(FlowUV.x)));
    
    vColor.rgb = saturate(vColor.rgb);
    vColor.rgb = pow(vColor.rgb, g_ColorGamma);
    vColor.rgb *= g_ColorGain;

    vColor.a = fVisibleY;

    if (vColor.a < 0.2f)
        discard;

    float fRatio = g_LifeTime.x / g_LifeTime.y;
    
    if (1.f - fRatio <= 0.4f)
        vColor.a *= 1.f - fRatio;
    
    Out.vDiffuse = vColor;

    float fWeight = Luminance(Out.vDiffuse.xyz);

    if (fWeight >= g_fEmissiveThreshold)
        Out.vEmissive = float4(Out.vDiffuse.xyz, 1.f);

    Out.vDiffuse.a *= g_Alpha;
    
    Out.vEmissive.xyz *= Out.vDiffuse.a;
    
    float z = In.vProjPos.z / In.vProjPos.w;
    
    float Weight = max(1e-5, exp(-z * g_WeightBlend));
    
    Out.vAccumColor = float4(Out.vDiffuse.rgb * Out.vDiffuse.a, 0.0f) * Weight;
    Out.vAccumAlpha.r = Out.vDiffuse.a * Weight;
    Out.vDiffuse = float4(0.f, 0.f, 0.f, 0.f);
    
    return Out;
}

PS_OUT PS_SkyTrailMask_X(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;

    float2 UV = In.vTexcoord;
    float Radial = frac(UV.x + g_MaskSpeed * g_Time);

    float2 FlowUV;
    FlowUV.x = Radial;
    FlowUV.y = UV.y;
    
    float MaskR = g_MaskTexture.Sample(ClampSampler, FlowUV).r;
    
    if(MaskR < 0.3f)
        discard;
    
    float fVisibleY;

    float Sweep = min(1.f, g_Sweep);
    
    fVisibleY = step(1.f - Sweep, UV.y);

    float fVisibleX;

    float fTailFad = smoothstep(Sweep - g_SweepWitdh, Sweep - g_SweepWitdh + g_Soft, 1.f - FlowUV.x);

    float fHeadFad = 1.f - smoothstep(Sweep - g_Soft, Sweep, 1.f - FlowUV.x);

    fVisibleX = fTailFad * fHeadFad;

    float4 vColor;

    if (g_Dir == 0)
        vColor = g_DiffuseTexture.Sample(DefaultSampler, float2(0.5f, saturate(In.vTexcoord.y)));
    else
        vColor = g_DiffuseTexture.Sample(DefaultSampler, float2(0.5f, saturate(In.vTexcoord.x)));
    
    
    vColor.rgb = saturate(vColor.rgb);
    vColor.rgb = pow(vColor.rgb, g_ColorGamma);
    vColor.rgb *= g_ColorGain;

    float fAlpha = saturate(fVisibleY * fVisibleX * MaskR);

    if (fAlpha < 0.2f)
        discard;

    float fRatio = g_LifeTime.x / g_LifeTime.y;
    
    if (1.f - fRatio <= 0.4f)
        fAlpha *= 1.f - fRatio;
    
    Out.vDiffuse = float4(vColor.rgb, fAlpha);

    float fWeight = Luminance(Out.vDiffuse.xyz);

    if (fWeight >= g_fEmissiveThreshold)
        Out.vEmissive = float4(Out.vDiffuse.xyz, 1.f);

    Out.vDiffuse.a *= g_Alpha;
    
    Out.vEmissive.xyz *= Out.vDiffuse.a;
    
    float z = In.vProjPos.z / In.vProjPos.w;
    
    float Weight = max(1e-5, exp(-z * g_WeightBlend));
    
    Out.vAccumColor = float4(Out.vDiffuse.rgb * Out.vDiffuse.a, 0.0f) * Weight;
    Out.vAccumAlpha.r = Out.vDiffuse.a * Weight;
    Out.vDiffuse = float4(0.f, 0.f, 0.f, 0.f);
    
    return Out;
}

PS_OUT PS_SkyTrailMask_Y(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;

    float2 UV = In.vTexcoord;
    float Radial = frac(UV.y + g_MaskSpeed * g_Time);

    float2 FlowUV;
    FlowUV.x = UV.x;
    FlowUV.y = Radial;
    
    float MaskR = g_MaskTexture.Sample(ClampSampler, FlowUV).r;
    
    if (MaskR < 0.3f)
        discard;
    
    float fVisible;

    float Sweep = min(1.f, g_Sweep);
    
    fVisible = step(1.f - Sweep, UV.y);

    float fVisibleY;

    float fTailFad = smoothstep(Sweep - g_SweepWitdh, Sweep - g_SweepWitdh + g_Soft, 1.f - FlowUV.y);

    float fHeadFad = 1.f - smoothstep(Sweep - g_Soft, Sweep, 1.f - FlowUV.y);

    fVisibleY = fTailFad * fHeadFad;

    float4 vColor;

    if (g_Dir == 0)
        vColor = g_DiffuseTexture.Sample(DefaultSampler, float2(0.5f, saturate(In.vTexcoord.y)));
    else
        vColor = g_DiffuseTexture.Sample(DefaultSampler, float2(0.5f, saturate(In.vTexcoord.x)));
    
    
    vColor.rgb = saturate(vColor.rgb);
    vColor.rgb = pow(vColor.rgb, g_ColorGamma);
    vColor.rgb *= g_ColorGain;

    float fAlpha = saturate(fVisibleY * fVisible * MaskR);

    if (fAlpha < 0.2f)
        discard;

    float fRatio = g_LifeTime.x / g_LifeTime.y;
    
    if (1.f - fRatio <= 0.4f)
        fAlpha *= 1.f - fRatio;
    
    Out.vDiffuse = float4(vColor.rgb, fAlpha);

    float fWeight = Luminance(Out.vDiffuse.xyz);

    if (fWeight >= g_fEmissiveThreshold)
        Out.vEmissive = float4(Out.vDiffuse.xyz, 1.f);

    Out.vDiffuse.a *= g_Alpha;
    
    Out.vEmissive.xyz *= Out.vDiffuse.a;
    
    float z = In.vProjPos.z / In.vProjPos.w;
    
    float Weight = max(1e-5, exp(-z * g_WeightBlend));
    
    Out.vAccumColor = float4(Out.vDiffuse.rgb * Out.vDiffuse.a, 0.0f) * Weight;
    Out.vAccumAlpha.r = Out.vDiffuse.a * Weight;
    Out.vDiffuse = float4(0.f, 0.f, 0.f, 0.f);
    
    return Out;
}




// ==Test==
technique11 DefaultTechnique
{
    pass DefaultPass // 0
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_NoneCompare, 0);
        SetBlendState(BS_AccumBlend, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_TrailDefault();
    }

    pass TestPass // 1
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_NoneCompare, 0);
        SetBlendState(BS_AccumBlend, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_TraillRightTail();
    }

    pass TestPassA // 2
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_NoneCompare, 0);
        SetBlendState(BS_AccumBlend, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_TraillLeftTail();
    }

    pass PS_TraillTest //3
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_AccumBlend, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_TraillDesh();
    }

    pass PS_TraillTestB //4
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_NoneCompare, 0);
        SetBlendState(BS_AccumBlend, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_TraillDeshB();
    }

    pass TrailYOut // 5
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_NoneCompare, 0);
        SetBlendState(BS_AccumBlend, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_Y_OUT();
    }

    pass TrailYIn // 6
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_NoneCompare, 0);
        SetBlendState(BS_AccumBlend, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_Y_IN();
    }

    pass TestDissolve // 7
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_NoneCompare, 0);
        SetBlendState(BS_AccumBlend, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_TraillDissolve();
    }

    pass TestDistortionWave // 8
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Blend, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_DistortionWave();
    }

    pass TestDistortionPotal // 9
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Blend, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_DistortionPotal();
    }

    pass PS_Debug //10
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN();
    }

    pass TrailAlphaLeft // 11
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_NoneCompare, 0);
        SetBlendState(BS_AccumBlend, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_TrailAlphaLeft();
    }

    pass TrailAlphaRight // 12
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_NoneCompare, 0);
        SetBlendState(BS_AccumBlend, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_TrailAlphaRight();
    }

    pass DefaultMeshRender //13
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_DefaultMeshRender();
    }

    pass TrailSky // 14
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_NoneCompare, 0);
        SetBlendState(BS_AccumBlend, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_SkyTrail();
    }

    pass TrailSkyColor // 15
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_NoneCompare, 0);
        SetBlendState(BS_AccumBlend, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_SkyTrailColor();
    }

    pass SkyTrailMaskX // 16
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_NoneCompare, 0);
        SetBlendState(BS_AccumBlend, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_SkyTrailMask_X();
    }

    pass SkyTrailMaskY // 17
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_NoneCompare, 0);
        SetBlendState(BS_AccumBlend, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_SkyTrailMask_Y();
    }
}