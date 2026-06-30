typedef row_major matrix matrix_rm;
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

#define THREAD_X 64
#define THREAD_Y 1
#define THREAD_Z 1

// 입력 버퍼들
StructuredBuffer<GPUKeyFrame> g_AllKeyframes : register(t0);
StructuredBuffer<AnimInfo> g_AllAnimInfos : register(t1);
StructuredBuffer<GPUChannelInfo> g_ChannelInfos : register(t2);
RWStructuredBuffer<matrix_rm> g_OutLocalMatrices : register(u0);

// 매 프레임 C++에서 업데이트)
cbuffer AnimationInfoCB : register(b0)
{
    float g_TrackPosition;
    uint g_AnimIndex;
    uint g_RibAnimUsed;
    uint g_RibbonAnimIndex;
}

float4 MulQuaternion(float4 q1, float4 q2)
{
    float4 result;
    result.w = q1.w * q2.w - dot(q1.xyz, q2.xyz);
    result.xyz = q1.w * q2.xyz + q2.w * q1.xyz + cross(q1.xyz, q2.xyz);
    return normalize(result);
}

// 쿼터니언 slerp 직접 구현
float4 CustomSlerp(float4 q1, float4 q2, float t)
{
   // 1. 입력 쿼터니언을 정규화해서 안정성 확보
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

    // acos 입력값 보호
    cos_theta = clamp(cos_theta, -1.0f, 1.0f);
    
    float theta = acos(cos_theta);
    float sin_theta = sin(theta);
    
    float w1 = sin((1.0f - t) * theta) / sin_theta;
    float w2 = sin(t * theta) / sin_theta;

    // 3. 최종 결과도 정규화해서 오차 누적 방지
    return normalize(q1 * w1 + q2 * w2);
}

SRTKeyFrame MakeIdentitySRT()
{
    SRTKeyFrame srt;
    srt.scale = float4(1.f, 1.f, 1.f, 1.f);
    srt.rotation = float4(0.f, 0.f, 0.f, 1.f);
    srt.translation = float4(0.f, 0.f, 0.f, 1.f);
    return srt;
}

int FindChannelIndex(uint boneIndex, AnimInfo anim)
{
    for (uint i = 0; i < anim.iNumChannels; ++i)
    {
        uint idx = anim.iStartChannelIndexOffset + i;
        if (g_ChannelInfos[idx].iBoneIndex == boneIndex)
            return idx;
    }
    return -1;
}

uint FindKeyframeIndex(GPUChannelInfo channel, float trackPos)
{
    uint keyIndex = channel.iStartKeyframeOffset;

    for (uint k = 0; k < channel.iNumKeyframes - 1; ++k)
    {
        keyIndex = channel.iStartKeyframeOffset + k;
        if (g_AllKeyframes[keyIndex + 1].fTrackPosition > trackPos)
            break;
    }

    return keyIndex;
}


// 헬퍼 함수: SQT(Scale, Quaternion, Translation)로부터 변환 행렬을 생성합니다.
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





SRTKeyFrame CalculateSRT(uint boneIndex, uint animIndex, float fTrackPosition, bool isRibbon)
{
    SRTKeyFrame result = MakeIdentitySRT();
    AnimInfo anim = g_AllAnimInfos[animIndex];

    int channelIndex = FindChannelIndex(boneIndex, anim);
    if (channelIndex < 0)
        return result;

    GPUChannelInfo channel = g_ChannelInfos[channelIndex];

    if (channel.iNumKeyframes <= 1)
        return result;

    if (isRibbon && channel.iNumKeyframes == 2)
        return result;

    uint keyframeIndex = FindKeyframeIndex(channel, fTrackPosition);
    
    GPUKeyFrame key1 = g_AllKeyframes[keyframeIndex];
    GPUKeyFrame key2 = g_AllKeyframes[keyframeIndex + 1];

    float blendFactor = 0.f;
    float segmentDuration = key2.fTrackPosition - key1.fTrackPosition;
    
    if (segmentDuration > 0.0f)
        blendFactor = (g_TrackPosition - key1.fTrackPosition) / segmentDuration;

    float4 interpScale = lerp(key1.vScale, key2.vScale, blendFactor);
    float4 interpTranslation = lerp(key1.vTranslation, key2.vTranslation, blendFactor);
    float4 interpRotation = CustomSlerp(key1.vRotation, key2.vRotation, blendFactor);
   
    result.scale = interpScale;
    result.rotation = interpRotation;
    result.translation = interpTranslation;
    
    return result;
}



// 가산 블렌딩 방식
[numthreads(THREAD_X, THREAD_Y, THREAD_Z)]
void CSMain(uint3 dispatchThreadID : SV_DispatchThreadID) // SV_DispatchThreadID : 전체 작업에서의 스레드 ID
{
    // 현재 본 Index 가져오기.
    uint boneIndex = dispatchThreadID.x;
    
      // 1. Action Animation의 SRT 가져오기
    SRTKeyFrame actionSRT = CalculateSRT(boneIndex, g_AnimIndex, g_TrackPosition, false);
   
    matrix result_matrix;
    
    // Ribbon Animation을 사용한다면?
    if (1 == g_RibAnimUsed)
    {
        // 2. Ribbon Animation의 SRT 가져오기
        SRTKeyFrame ribbonSRT = CalculateSRT(boneIndex, g_RibbonAnimIndex, g_TrackPosition, true);
        
        // 변화량 가산 방식 (Ribbon이 BindPose로부터의 변화량인 경우)
        float4 finalScale = ribbonSRT.scale * actionSRT.scale;
        float4 finalRotation = MulQuaternion(ribbonSRT.rotation, actionSRT.rotation);
        float4 finalTranslation = ribbonSRT.translation +
                                 (actionSRT.translation - float4(0, 0, 0, 1)); // delta 적용
        result_matrix = ComposeMatrixFromSRT(finalScale, finalRotation, finalTranslation);
    }
    else
    {
        result_matrix = ComposeMatrixFromSRT(actionSRT.scale, actionSRT.rotation, actionSRT.translation);
    }
    
    // 최종 행렬이 아닌 '로컬' 행렬을 출력 버퍼에 쓴다.
    g_OutLocalMatrices[boneIndex] = result_matrix;
   
}