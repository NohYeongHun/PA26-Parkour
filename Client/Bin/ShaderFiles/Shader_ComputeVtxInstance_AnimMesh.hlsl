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

struct AnimationInfoCB
{
    float fTrackPosition;
    uint iAnimIndex;
    bool bTemp1;
    uint iTemp2;
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
//StructuredBuffer<matrix_rm> g_InvBindBones : register(t4);

// 출력(Output) 버퍼 - 이제 '로컬' 행렬을 출력합니다.
RWStructuredBuffer<matrix_rm> g_OutLocalMatrices : register(u0);

// --- Per-Frame Data (매 프레임 C++에서 업데이트) ---
StructuredBuffer<AnimationInfoCB> g_AnimCellsInfo : register(t3);

cbuffer InstanceCB : register(b0)
{
    uint g_NumBones;
    uint g_NumInstance;
    uint g_Temp;
    float g_TempFloat;
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

    // acos 입력값 보호 (필수)
    cos_theta = clamp(cos_theta, -1.0f, 1.0f);
    
    float theta = acos(cos_theta);
    float sin_theta = sin(theta);
    
    float w1 = sin((1.0f - t) * theta) / sin_theta;
    float w2 = sin(t * theta) / sin_theta;

    // 3. 최종 결과도 정규화해서 오차 누적 방지
    return normalize(q1 * w1 + q2 * w2);
}

// 헬퍼 함수: SQT(Scale, Quaternion, Translation)로부터 변환 행렬을 생성합니다.
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

SRTKeyFrame CalculateSRT(uint boneIndex, uint animIndex, bool isRibbon, float fTrackPosition)
{
    SRTKeyFrame result;
    
    // 1. 현재 애니메이션 정보 가져오기
    AnimInfo anim = g_AllAnimInfos[animIndex];
    
    // 2. 단위 SRT 설정. 
    result.scale = float4(1.f, 1.f, 1.f, 1.f);
    result.rotation = float4(0.f, 0.f, 0.f, 1.f); // 단위 쿼터니언 (w = 1)
    result.translation = float4(0.f, 0.f, 0.f, 1.f);
    
    // 2. 현재 뼈에 해당하는 채널 찾기
    int channelIndex = -1;
    for (uint i = 0; i < anim.iNumChannels; i++)
    {
        // globalChannel Index인 이유 => g_ChannelInfos는 모든 애니메이션의 채널 정보를 담고 있기 때문.
        // 현재 채널인덱스를 이용해서 애니메이션 배열에서 
        // 본인덱스를 순회해서 스레드가 처리해야하는 본인덱스인지 찾는다.
        uint globalChannelIdx = anim.iStartChannelIndexOffset + i;
        if (g_ChannelInfos[globalChannelIdx].iBoneIndex == boneIndex)
        {
            channelIndex = globalChannelIdx;
            break;
        }
    }
    
    // 3. 애니메이션에서 이 뼈에 해당하는 채널이 없으면, 단위 행렬을 설정하고 종료
    if (channelIndex == -1)
    {
        return result;
    }
    
    // 4. 가지고 있는 채널 인덱스로 채널 정보 가져오기.
    GPUChannelInfo channel = g_ChannelInfos[channelIndex];

    // 5. 예외케이스 => 키프레임이 1개 이하면 보간할 필요가 없음 (정적인 뼈 이므로)
    // => 따로 보간 작업을 하지 않고 변환 행렬을 만들어서 boneIndex 위치에 바로 저장.
    if (channel.iNumKeyframes <= 1)
    {
        // 첫 번째 키프레임의 변환을 그대로 사용
        GPUKeyFrame staticKey = g_AllKeyframes[channel.iStartKeyframeOffset];
        result.scale = staticKey.vScale;
        result.rotation = staticKey.vRotation;
        result.translation = staticKey.vTranslation;
        return result;
    }
    
    // 6. Ribbon Animation의 경우 키프레임이 2개이면 항상 단위 SRT 반환
    if (isRibbon == true && channel.iNumKeyframes == 2)
    {
        return result;
    }
    
    // 7. 보간할 두 개의 키프레임을 찾을 인덱스를 채널의 시작 오프셋으로 초기화.
    uint keyframeIndex = channel.iStartKeyframeOffset;
    
    
    // 8. 채널의 모든 키프레임을 순회하면서 다음 키프레임의 시간이 현재 재생 기간 보다 크면 
    // 해당 키프레임과 그 다음 키프레임 사이를 보간하면됨.
    for (uint k = 0; k < channel.iNumKeyframes - 1; ++k)
    {
        // 다음 키프레임의 시간이 현재 재생 시간보다 크면, 현재 k와 k + 1 사이에서 보간하면 됨
        if (g_AllKeyframes[channel.iStartKeyframeOffset + k + 1].fTrackPosition > fTrackPosition)
        {
            keyframeIndex = channel.iStartKeyframeOffset + k;
            break; // 올바른 구간을 찾았으므로 반복 중단
        }
        // 끝까지 못찾았다면 마지막-1 인덱스를 사용하게 됨
        keyframeIndex = channel.iStartKeyframeOffset + k;
    }
    
    GPUKeyFrame key1 = g_AllKeyframes[keyframeIndex];
    GPUKeyFrame key2 = g_AllKeyframes[keyframeIndex + 1];

    // 9. 두 키프레임 사이의 보간 비율 계산
    float blendFactor = 0.f;
    float segmentDuration = key2.fTrackPosition - key1.fTrackPosition;
    
    if (segmentDuration > 0.0f)
    {
        // 선형 보간
        //blendFactor = (g_AnimCellsInfo[InstanceIndex].g_TrackPosition - key1.fTrackPosition) / segmentDuration;
        blendFactor = (fTrackPosition - key1.fTrackPosition) / segmentDuration;
    }

    // ... SRT 보간 코드 ... => 의심.
    float4 interpScale = lerp(key1.vScale, key2.vScale, blendFactor);
    float4 interpTranslation = lerp(key1.vTranslation, key2.vTranslation, blendFactor);
    
    // C++과 달리 HLSL에는 DirectXMath의 XMQuaternionSlerp가 없으므로 직접 구현한 customSlerp 사용
    float4 interpRotation = customSlerp(key1.vRotation, key2.vRotation, blendFactor);
   
    result.scale = interpScale;
    result.rotation = interpRotation;
    result.translation = interpTranslation;
    
    return result;
}


// 헬퍼 2: 델타(Delta) SRT 계산 (가산 블렌딩용)
// (targetSRT - weightSRT)
SRTKeyFrame Calculate_Delta(SRTKeyFrame targetSRT, SRTKeyFrame weightSRT)
{
    SRTKeyFrame delta;
    
    // 척도(Scale) 뺄셈 (나눗셈)
    delta.scale = targetSRT.scale / weightSRT.scale;
    
    // 회전(Rotation) 뺄셈: target * inverse(weight)
    // inverse(q) = (-q.xyz, q.w)
    float4 invWeightRot = float4(-weightSRT.rotation.x, -weightSRT.rotation.y, -weightSRT.rotation.z, weightSRT.rotation.w);
    delta.rotation = mulQuaternion(targetSRT.rotation, normalize(invWeightRot));
    
    // 이동(Translation) 뺄셈
    delta.translation = targetSRT.translation - weightSRT.translation;
    return delta;
}

// 헬퍼 3: 델타(Delta) SRT 적용 (가산)
// (baseSRT + deltaSRT)
SRTKeyFrame Apply_Additive(SRTKeyFrame baseSRT, SRTKeyFrame deltaSRT)
{
    SRTKeyFrame result;
    // 척도 덧셈 (곱셈)
    result.scale = baseSRT.scale * deltaSRT.scale;
    // 회전 덧셈: delta * base
    result.rotation = mulQuaternion(deltaSRT.rotation, baseSRT.rotation);
    // 이동 덧셈
    result.translation = baseSRT.translation + deltaSRT.translation;
    return result;
}

// 가산 블렌딩 방식
[numthreads(THREAD_X, THREAD_Y, THREAD_Z)]
void CSMain(uint3 dispatchThreadID : SV_DispatchThreadID) // SV_DispatchThreadID : 전체 작업에서의 스레드 ID
{
    // 현재 본 Index 가져오기.
    uint iGlobalIndex = dispatchThreadID.x;
    uint iInstanceIndex = iGlobalIndex / g_NumBones;
    uint boneIndex = iGlobalIndex % g_NumBones;
    if (iGlobalIndex >= g_NumBones * g_NumInstance)
        return;
    
    // 1. Action Animation의 SRT 가져오기
    SRTKeyFrame actionSRT = CalculateSRT(boneIndex, g_AnimCellsInfo[iInstanceIndex].iAnimIndex, false, g_AnimCellsInfo[iInstanceIndex].fTrackPosition);
    
    matrix result_matrix;
    
    // 2. Action Matrix를 바탕으로 Result Matrix 생성.
    result_matrix = ComposeMatrixFromSRT(actionSRT.scale, actionSRT.rotation, actionSRT.translation);
    
    // 3. 계층 구조 계산
    
    // 최종 행렬이 아닌 '로컬' 행렬을 출력 버퍼에 쓴다.
    g_OutLocalMatrices[iGlobalIndex] = result_matrix;
   
}