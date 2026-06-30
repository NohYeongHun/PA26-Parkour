#define THREAD_X 256
#define THREAD_Y 1
#define THREAD_Z 1

struct MorphDelta
{
    float3 vPosDelta; // 위치 변화량
    float3 vNormalDelta; // 노멀 변화량
};

// 렌더링 파이프 라인으로 넘겨질 최종 정점 정보.
struct OutputVertex
{
    float3 vPosition;
    float3 vNormal;
};

// 변형되지 않은 원본 정점 데이터
struct BaseVertex
{
    float3 vPosition;
    float3 vNormal;
};


// 원본 정점 데이터 (Read - Only)
StructuredBuffer<BaseVertex> g_BaseVertices : register(t0);

// 모든 쉐이프 키의 Delta Data가 일렬로 담긴 버퍼
StructuredBuffer<MorphDelta> g_AllMorphDeltas : register(t1);

// 현재 프레임의 Shape Key 가중치 => CPU에서 계산해서 넘겨줍니다.
StructuredBuffer<float> g_MorphWeights : register(t2);

// 최종 결과물 RW
RWStructuredBuffer<OutputVertex> g_OutVertices : register(u0);

// Constant Buffer
cbuffer MorphInfoCB : register(b0)
{
    uint g_NumVertices; // 전체 정점 개수
    uint g_NumActiveMorphs; // 현재 활성화된 쉐이프 키 개수.
    float2 vPadding;
}


float g_MorphWeightEpsilon = 0.0001f; // 가중치가 이보다 작으면 무시할 임계값

[numthreads(THREAD_X, THREAD_Y, THREAD_Z)]
void CSMain(uint3 dispatchThreadID : SV_DispatchThreadID) // SV_DispatchThreadID : 전체 작업에서의 스레드 ID
{
    // 현재 스레드가 처리할 정점 인덱스.
    uint iVertexID = dispatchThreadID.x;
    
    // 남는 스레드는 처리할 정점이 없으므로 종료.
    if (iVertexID >= g_NumVertices)
        return;
    
    // 원본 정점 정보 가져오기.
    BaseVertex baseVert = g_BaseVertices[iVertexID];
    
    float3 finalPos = baseVert.vPosition;
    float3 finalNormal = baseVert.vNormal;
    
    // 활성화된 모든 Morph Target을 순회하고 누적합니다.
    for (uint i = 0; i < g_NumActiveMorphs; ++i)
    {
        float fWeight = g_MorphWeights[i];
        
        if (fWeight < g_MorphWeightEpsilon)
            continue;
        
        // 데이터 접근 인덱스 계산.
        uint deltaIndex = (i * g_NumVertices) + iVertexID;
        
        MorphDelta delta = g_AllMorphDeltas[deltaIndex]; // 전역인덱스로 접근한다는 관점.
        
        finalPos += delta.vPosDelta * fWeight;
        finalNormal += delta.vNormalDelta * fWeight;
    }
    
    // 결과 저장.
    OutputVertex result;
    result.vPosition = finalPos;
    result.vNormal = normalize(finalNormal);

    g_OutVertices[iVertexID] = result;
}
