typedef row_major matrix matrix_rm;
// 구조체 형식
// Depth 1 => Animation에서 소유하는 Channel 정보
struct AnimInfo
{
    uint iStartChannelIndexOffset;
    uint iNumChannels;
    float fDuration;
    uint iPadding;
};

// Depth 2: Channel 정보 (시작 KeyFrame 오프셋 등)
struct GPUChannelInfo
{
    uint iStartKeyframeOffset;
    uint iNumKeyframes;
    uint iBoneIndex;
    uint iPadding;
};

// Depth 3: 채널이 소유하는 KeyFrame (매 TrackPosition마다 뼈의 이동 정보)
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

#define THREAD_X 64
#define THREAD_Y 1
#define THREAD_Z 1


StructuredBuffer<GPUKeyFrame> g_AllKeyframes : register(t0);
StructuredBuffer<AnimInfo> g_AllAnimInfos : register(t1);
StructuredBuffer<GPUChannelInfo> g_ChannelInfos : register(t2);

RWStructuredBuffer<matrix_rm> g_OutLocalMatrices : register(u0);

// 매 프레임 C++에서 업데이트
cbuffer AnimationInfoCB : register(b0)
{
    float g_TrackPosition;
    uint g_AnimIndex;
    bool g_IsRibAnimUsed;
    uint g_RibbonAnimIndex;
}

float4 mulQuaternion(float4 q1, float4 q2)
{
    float4 result;
    result.w = q1.w * q2.w - dot(q1.xyz, q2.xyz);
    result.xyz = q1.w * q2.xyz + q2.w * q1.xyz + cross(q1.xyz, q2.xyz);
    return normalize(result);
}

// 쿼터니언 slerp 직접 구현
float4 customSlerp(float4 q1, float4 q2, float t)
{
    // 입력 쿼터니언 정규화
    q1 = normalize(q1);
    q2 = normalize(q2);

    float cos_theta = dot(q1, q2);

    // 짧은 경로 회전 보장
    if (cos_theta < 0.0f)
    {
        q2 = -q2;
        cos_theta = -cos_theta;
    }
    
    // 두 쿼터니언이 거의 같으면, lerp로 대체 (0으로 나누기 방지)
    if (cos_theta > 0.9995f)
    {
        return normalize(lerp(q1, q2, t));
    }

    // acos 입력값 보호 (필수)
    cos_theta = clamp(cos_theta, -1.0f, 1.0f);
    
    float theta = acos(cos_theta);
    float sin_theta = sin(theta);
    
    float w1 = sin((1.0f - t) * theta) / sin_theta;
    float w2 = sin(t * theta) / sin_theta;

    // 결과 정규화로 오차 누적 방지
    return normalize(q1 * w1 + q2 * w2);
}

// SQT(Scale, Quaternion, Translation) → 변환 행렬 생성 (CSE 최적화)
matrix_rm ComposeMatrixFromSRT(float4 s, float4 q, float4 t)
{
    float qx = q.x, qy = q.y, qz = q.z, qw = q.w;
    float qx2 = qx * qx, qy2 = qy * qy, qz2 = qz * qz;
    float qxqy = qx * qy, qxqz = qx * qz, qyqz = qy * qz;
    float qwqx = qw * qx, qwqy = qw * qy, qwqz = qw * qz;

    return matrix_rm(
        s.x * (1.f - 2.f * (qy2 + qz2)),  s.x * 2.f * (qxqy + qwqz),  s.x * 2.f * (qxqz - qwqy),  0.f,
        s.y * 2.f * (qxqy - qwqz),        s.y * (1.f - 2.f * (qx2 + qz2)), s.y * 2.f * (qyqz + qwqx),  0.f,
        s.z * 2.f * (qxqz + qwqy),        s.z * 2.f * (qyqz - qwqx),  s.z * (1.f - 2.f * (qx2 + qy2)), 0.f,
        t.x, t.y, t.z, 1.f
    );
}

// 애니메이션의 현재 진행도를 이용하여 두 키프레임 사이의 SRT를 반환
SRTKeyFrame CalculateSRT(uint boneIndex, uint animIndex, float fTrackPosition, bool isRibbon)
{
    SRTKeyFrame result;
    
    AnimInfo anim = g_AllAnimInfos[animIndex];
    
    result.scale = float4(1.f, 1.f, 1.f, 1.f);
    result.rotation = float4(0.f, 0.f, 0.f, 1.f);   // 단위 쿼터니언
    result.translation = float4(0.f, 0.f, 0.f, 1.f);

    // 현재 뼈에 해당하는 채널 검색
    int channelIndex = -1;
    for (uint i = 0; i < anim.iNumChannels; i++)
    {
        uint globalChannelIdx = anim.iStartChannelIndexOffset + i;
        if (g_ChannelInfos[globalChannelIdx].iBoneIndex == boneIndex)
        {
            channelIndex = globalChannelIdx;
            break;
        }
    }
    
    if (channelIndex == -1)
    {
        return result;
    }
    
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
    {
        return result;
    }
    
    // 7~8. 보간할 키프레임 구간 검색 (다음 키프레임 시간 > 현재 재생 시간이 되는 구간)
    uint keyStart = channel.iStartKeyframeOffset;
    uint keyframeIndex = keyStart;
    for (uint k = 0; k < channel.iNumKeyframes - 1; ++k)
    {
        keyframeIndex = keyStart + k;
        if (g_AllKeyframes[keyframeIndex + 1].fTrackPosition > fTrackPosition)
            break;
    }
    
    GPUKeyFrame key1 = g_AllKeyframes[keyframeIndex];
    GPUKeyFrame key2 = g_AllKeyframes[keyframeIndex + 1];

    float blendFactor = 0.f;
    float segmentDuration = key2.fTrackPosition - key1.fTrackPosition;
    
    if (segmentDuration > 0.0f)
        blendFactor = (fTrackPosition - key1.fTrackPosition) / segmentDuration;

    float4 interpScale = lerp(key1.vScale, key2.vScale, blendFactor);
    float4 interpTranslation = lerp(key1.vTranslation, key2.vTranslation, blendFactor);
    float4 interpRotation = customSlerp(key1.vRotation, key2.vRotation, blendFactor);
   
    result.scale = interpScale;
    result.rotation = interpRotation;
    result.translation = interpTranslation;
    
    return result;
}


[numthreads(THREAD_X, THREAD_Y, THREAD_Z)]
void CSMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint boneIndex = dispatchThreadID.x;
    SRTKeyFrame actionSRT = CalculateSRT(boneIndex, g_AnimIndex, g_TrackPosition, false);

    matrix result_matrix;
    if (g_IsRibAnimUsed)
    {
        SRTKeyFrame ribbonSRT = CalculateSRT(boneIndex, g_RibbonAnimIndex, g_TrackPosition, true);
        float4 finalScale = ribbonSRT.scale * actionSRT.scale;
        float4 finalRotation = mulQuaternion(ribbonSRT.rotation, actionSRT.rotation);
        float4 finalTranslation = ribbonSRT.translation + actionSRT.translation;
        finalTranslation.w = 1.f;
        result_matrix = ComposeMatrixFromSRT(finalScale, finalRotation, finalTranslation);
    }
    else
    {
        result_matrix = ComposeMatrixFromSRT(actionSRT.scale, actionSRT.rotation, actionSRT.translation);
    }

    g_OutLocalMatrices[boneIndex] = result_matrix;
}

