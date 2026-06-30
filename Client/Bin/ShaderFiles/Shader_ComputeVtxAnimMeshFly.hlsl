typedef row_major matrix matrix_rm;
//typedef row_major matrix_rm;
// 구조체 형식
// Depth 1 => Animation에서 소유하는 Channel 정보
struct AnimInfo
{
    uint iStartChannelIndexOffset;
    uint iNumChannels;
    float fDuration;
    uint iPadding;
};

// Depth2 Channel 에서 실행하는 StartKeyFrame
struct GPUChannelInfo
{
    uint iStartKeyframeOffset;
    uint iNumKeyframes;
    uint iBoneIndex;
    uint iPadding;
};

// Dpeth 3 채널이 소유하는 KeyFrame(매 TrackPosition마다 뼈의 이동 정보) 구조체.
struct GPUKeyFrame
{
    float4 vScale;
    float4 vRotation; // Quaternion
    float4 vTranslation;
    float fTrackPosition;
    float3 vPadding;
};

struct SRTKeyFrame
{
    float4 scale;
    float4 rotation;
    float4 translation;
};

// --- Resources (CModel::Ready_GPU_Buffers에서 생성한 버퍼들) ---
#define THREAD_X 64
#define THREAD_Y 1
#define THREAD_Z 1

// 입력(Input) 버퍼들
//StructuredBuffer<int> g_BoneHierarchy : register(t0);

StructuredBuffer<GPUKeyFrame> g_AllKeyframes : register(t0);
StructuredBuffer<AnimInfo> g_AllAnimInfos : register(t1);
StructuredBuffer<GPUChannelInfo> g_ChannelInfos : register(t2);

// 출력(Output) 버퍼 - 이제 '로컬' 행렬을 출력합니다.
RWStructuredBuffer<matrix_rm> g_OutLocalMatrices : register(u0);

// --- Per-Frame Data (매 프레임 C++에서 업데이트) ---
cbuffer AnimationInfoCB : register(b0)
{
    float g_TrackPosition;
    uint g_AnimIndex;
    uint g_RibAnimUsed;
    uint g_RibbonAnimIndex;
    
    bool g_IsBlendEnabled; // Blending Usable.
    float g_BlendParamLR; // 4 (LR -1(L) to 1(R))
    float g_fBlendParamDU; // 4 (DU -1(D) to 1(U))
    float g_Padding; // 4 (16바이트 정렬)

	// LR Blend Clip
    uint g_ClipIndexL;
    uint g_ClipIndexMidLR;
    uint g_ClipIndexR;
    uint g_WeightClipLR;

	// DU Blend Clip
    uint g_ClipIndexD;
    uint g_ClipIndexMidDU;
    uint g_ClipIndexU;
    uint g_WeightClipDU;
}

float4 InverseQuaternion(float4 q)
{
    float lenSq = dot(q, q);
    if (lenSq < 1e-6f)
        return float4(0.f, 0.f, 0.f, 1.f);

    return normalize(float4(-q.x, -q.y, -q.z, q.w));
}

float4 MulQuaternion(float4 q1, float4 q2)
{
    float4 result;
    result.w = q1.w * q2.w - dot(q1.xyz, q2.xyz);
    result.xyz = q1.w * q2.xyz + q2.w * q1.xyz + cross(q1.xyz, q2.xyz);
    return normalize(result);
}

float4 CustomSlerp(float4 q1, float4 q2, float t)
{
    q1 = normalize(q1);
    q2 = normalize(q2);

    float cos_theta = dot(q1, q2);

    if (cos_theta < 0.0f)
    {
        q2 = -q2;
        cos_theta = -cos_theta;
    }
    
    if (cos_theta > 0.9995f)
    {
        return normalize(lerp(q1, q2, t));
    }

    cos_theta = clamp(cos_theta, -1.0f, 1.0f);
    
    float theta = acos(cos_theta);
    float sin_theta = sin(theta);
    
    float w1 = sin((1.0f - t) * theta) / sin_theta;
    float w2 = sin(t * theta) / sin_theta;

    return normalize(q1 * w1 + q2 * w2);
}

// 이거 문젠가?
matrix_rm ComposeMatrixFromSRT(float4 s, float4 q, float4 t)
{
    matrix_rm m;
    float qx = q.x, qy = q.y, qz = q.z, qw = q.w;

    m._11 = s.x * (1 - 2 * qy * qy - 2 * qz * qz);
    m._12 = s.x * (2 * qx * qy + 2 * qw * qz);
    m._13 = s.x * (2 * qx * qz - 2 * qw * qy);
    m._14 = 0;

    m._21 = s.y * (2 * qx * qy - 2 * qw * qz);
    m._22 = s.y * (1 - 2 * qx * qx - 2 * qz * qz);
    m._23 = s.y * (2 * qy * qz + 2 * qw * qx);
    m._24 = 0;

    m._31 = s.z * (2 * qx * qz + 2 * qw * qy);
    m._32 = s.z * (2 * qy * qz - 2 * qw * qx);
    m._33 = s.z * (1 - 2 * qx * qx - 2 * qy * qy);
    m._34 = 0;

    m._41 = t.x;
    m._42 = t.y;
    m._43 = t.z;
    m._44 = 1;
	
    return m;
}

SRTKeyFrame MakeIdentitySRT()
{
    SRTKeyFrame srt;
    srt.scale = float4(1.f, 1.f, 1.f, 1.f);
    srt.rotation = float4(0.f, 0.f, 0.f, 1.f);
    srt.translation = float4(0.f, 0.f, 0.f, 1.f);
    return srt;
}

SRTKeyFrame CalculateSRT(uint boneIndex, uint animIndex, float fTrackPosition, bool isRibbon)
{
    SRTKeyFrame result = MakeIdentitySRT();
    AnimInfo anim = g_AllAnimInfos[animIndex];

    int channelIndex = -1;
    for (uint i = 0; i < anim.iNumChannels; ++i)
    {
        uint globalChannelIdx = anim.iStartChannelIndexOffset + i;
        if (g_ChannelInfos[globalChannelIdx].iBoneIndex == boneIndex)
        {
            channelIndex = globalChannelIdx;
            break;
        }
    }

    if (channelIndex == -1)
        return result;

    GPUChannelInfo channel = g_ChannelInfos[channelIndex];

    if (channel.iNumKeyframes <= 1)
    {
        GPUKeyFrame staticKey = g_AllKeyframes[channel.iStartKeyframeOffset];
        result.scale = staticKey.vScale;
        result.rotation = staticKey.vRotation;
        result.translation = staticKey.vTranslation;
        return result;
    }

    if (isRibbon && channel.iNumKeyframes == 2)
        return result;

    uint keyframeIndex = channel.iStartKeyframeOffset;
    for (uint k = 0; k < channel.iNumKeyframes - 1; ++k)
    {
        uint nextIndex = channel.iStartKeyframeOffset + k + 1;
        if (g_AllKeyframes[nextIndex].fTrackPosition > fTrackPosition)
        {
            keyframeIndex = channel.iStartKeyframeOffset + k;
            break;
        }
        keyframeIndex = channel.iStartKeyframeOffset + k;
    }

    GPUKeyFrame key1 = g_AllKeyframes[keyframeIndex];
    GPUKeyFrame key2 = g_AllKeyframes[keyframeIndex + 1];

    float blendFactor = 0.0f;
    float segmentDuration = key2.fTrackPosition - key1.fTrackPosition;
    if (segmentDuration > 0.0f)
    {
        blendFactor = (fTrackPosition - key1.fTrackPosition) / segmentDuration;
    }

    result.scale = lerp(key1.vScale, key2.vScale, blendFactor);
    result.rotation = CustomSlerp(key1.vRotation, key2.vRotation, blendFactor);
    result.translation = lerp(key1.vTranslation, key2.vTranslation, blendFactor);
    return result;
}

SRTKeyFrame Blend1D_SRT(uint clipA_idx, uint clipB_idx, float t, uint boneIndex, float trackPos)
{
    SRTKeyFrame srtA = CalculateSRT(boneIndex, clipA_idx, trackPos, false);
    SRTKeyFrame srtB = CalculateSRT(boneIndex, clipB_idx, trackPos, false);

    SRTKeyFrame result;
    result.scale = lerp(srtA.scale, srtB.scale, t);
    result.rotation = CustomSlerp(srtA.rotation, srtB.rotation, t);
    result.translation = lerp(srtA.translation, srtB.translation, t);
    return result;
}


float SafeDivide(float numerator, float denominator)
{
    if (abs(denominator) < 1e-6f)
        return 1.0f;
    return numerator / denominator;
}

SRTKeyFrame Calculate_Delta(SRTKeyFrame targetSRT, SRTKeyFrame weightSRT)
{
    SRTKeyFrame delta;
    
    delta.scale.x = SafeDivide(targetSRT.scale.x, weightSRT.scale.x);
    delta.scale.y = SafeDivide(targetSRT.scale.y, weightSRT.scale.y);
    delta.scale.z = SafeDivide(targetSRT.scale.z, weightSRT.scale.z);
    delta.scale.w = 1.0f;
    
    float4 invWeightRot = float4(-weightSRT.rotation.x, -weightSRT.rotation.y, -weightSRT.rotation.z, weightSRT.rotation.w);
    if (dot(invWeightRot, invWeightRot) < 1e-6f) 
        invWeightRot = float4(0, 0, 0, 1); 
    else
        invWeightRot = normalize(invWeightRot);
    
    delta.rotation = MulQuaternion(targetSRT.rotation, invWeightRot);
    
    delta.translation = targetSRT.translation - weightSRT.translation;
    return delta;
}

SRTKeyFrame Apply_Additive(SRTKeyFrame baseSRT, SRTKeyFrame deltaSRT)
{
    SRTKeyFrame result;
    result.scale = baseSRT.scale * deltaSRT.scale;
    result.rotation = MulQuaternion(deltaSRT.rotation, baseSRT.rotation);
    result.translation = baseSRT.translation + deltaSRT.translation;
    return result;
}

SRTKeyFrame GetDirectionalTargetSRT(
    uint boneIndex,
    float blendParam,
    uint midClip,
    uint negativeClip,
    uint positiveClip,
    float fTrackPosition)
{
    SRTKeyFrame target = CalculateSRT(boneIndex, midClip, fTrackPosition, false);

    if (blendParam > 0.0f)
    {
        target = Blend1D_SRT(midClip, positiveClip, saturate(blendParam), boneIndex, fTrackPosition);
    }
    else if (blendParam < 0.0f)
    {
        target = Blend1D_SRT(midClip, negativeClip, saturate(-blendParam), boneIndex, fTrackPosition);
    }

    return target;
}

SRTKeyFrame ApplyDirectionalBlendToPose(
    SRTKeyFrame baseSRT,
    uint boneIndex,
    float blendParam,
    uint weightClip,
    uint midClip,
    uint negativeClip,
    uint positiveClip,
    float trackPos)
{
    SRTKeyFrame weightSRT = CalculateSRT(boneIndex, weightClip, trackPos, false);
    SRTKeyFrame targetSRT = GetDirectionalTargetSRT(
        boneIndex,
        blendParam,
        midClip,
        negativeClip,
        positiveClip,
        trackPos
    );

    SRTKeyFrame deltaSRT = Calculate_Delta(targetSRT, weightSRT);
    return Apply_Additive(baseSRT, deltaSRT);
}

[numthreads(THREAD_X, THREAD_Y, THREAD_Z)]
void CSMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint boneIndex = dispatchThreadID.x;

    SRTKeyFrame basePoseSRT = CalculateSRT(boneIndex, g_AnimIndex, g_TrackPosition, false);

    SRTKeyFrame lrBlendedPoseSRT = basePoseSRT;
    SRTKeyFrame duBlendedPoseSRT = lrBlendedPoseSRT;

    if (g_IsBlendEnabled)
    {
        lrBlendedPoseSRT = ApplyDirectionalBlendToPose(
            basePoseSRT,
            boneIndex,
            g_BlendParamLR,
            g_WeightClipLR,
            g_ClipIndexMidLR,
            g_ClipIndexL,
            g_ClipIndexR,
            g_TrackPosition
        );

        duBlendedPoseSRT = ApplyDirectionalBlendToPose(
            lrBlendedPoseSRT,
            boneIndex,
            g_fBlendParamDU,
            g_WeightClipDU,
            g_ClipIndexMidDU,
            g_ClipIndexD,
            g_ClipIndexU,
            g_TrackPosition
        );

    }

    SRTKeyFrame outputPoseSRT = basePoseSRT;
    if (g_IsBlendEnabled)
    {
        outputPoseSRT = duBlendedPoseSRT;
    }

    matrix_rm outputLocalMatrix;

    if (g_RibAnimUsed == 1)
    {
        SRTKeyFrame ribbonPoseSRT =
            CalculateSRT(boneIndex, g_RibbonAnimIndex, g_TrackPosition, true);

        float4 combinedScale = ribbonPoseSRT.scale * outputPoseSRT.scale;
        float4 combinedRotation = MulQuaternion(ribbonPoseSRT.rotation, outputPoseSRT.rotation);
        float4 combinedTranslation = ribbonPoseSRT.translation +
                                     (outputPoseSRT.translation - float4(0.f, 0.f, 0.f, 1.f));

        outputLocalMatrix = ComposeMatrixFromSRT(
            combinedScale,
            combinedRotation,
            combinedTranslation
        );
    }
    else
    {
        outputLocalMatrix = ComposeMatrixFromSRT(
            outputPoseSRT.scale,
            outputPoseSRT.rotation,
            outputPoseSRT.translation
        );
    }

    g_OutLocalMatrices[boneIndex] = outputLocalMatrix;
}

