typedef row_major matrix matrix_rm;

// --- Resources (CModel::Ready_GPU_Buffers에서 생성한 버퍼들) ---
#define THREAD_X 128
#define THREAD_Y 1
#define THREAD_Z 1

struct BoneInfo
{
    int iParentIndex;
    int iDepth;
    uint iPadding1;
    uint iPadding2;
};
// 입력(Input) 버퍼들
StructuredBuffer<matrix_rm> g_LocalMatrices : register(t0);
StructuredBuffer<BoneInfo> g_BoneInfo : register(t1);
// 출력(Output) 버퍼 - 이제 '로컬' 행렬을 출력합니다.
RWStructuredBuffer<matrix_rm> g_OutModelMatrices : register(u0);
cbuffer InstanceCB : register(b0)
{
    uint g_NumBones;
    uint g_NumInstance;
    uint g_MaxDepth;
    float g_TempFloat;
}
cbuffer PreTrancform : register(b1)
{
    row_major float4x4 g_PreTransform;
}

groupshared row_major float4x4 sharedMatrix[THREAD_X];

void Skinning(matrix_rm localMatrix, int iGlobalBoneIndex, bool isVaild)
{
    int iBoneIndex = iGlobalBoneIndex % g_NumBones;
   
    //유효하지 않은 스레드도 동기화에 참여
    for (uint i = 0; i <= g_MaxDepth; ++i)
    {
        //현재 뼈의 깊이와 일치하는지 확인
        if (isVaild && g_BoneInfo[iBoneIndex].iDepth == i)
        {
            
            int iParentIndex = g_BoneInfo[iBoneIndex].iParentIndex;
            if (iParentIndex == -1)
            {
                sharedMatrix[iBoneIndex] = mul(g_PreTransform, localMatrix);
            }
            else
            {
                row_major float4x4 parentMatrix = sharedMatrix[iParentIndex];
                
                //if (iBoneIndex == 1)
                //{
                //    float3 vRight = parentMatrix._11_12_13;
                //    float3 vUp = parentMatrix._21_22_23;
                //    float3 vLook = parentMatrix._31_32_33;
                //    
                //    parentMatrix._11_12_13 = normalize(vRight);
                //    parentMatrix._21_22_23 = normalize(vUp);
                //    parentMatrix._31_32_33 = normalize(vLook);
                //}
                
                sharedMatrix[iBoneIndex] = mul(localMatrix, parentMatrix);
            }
        }
        //한 그룹에 모든 스레드가 도달할 때까지 대기
        GroupMemoryBarrierWithGroupSync();
    }
}

[numthreads(THREAD_X, THREAD_Y, THREAD_Z)]
void CSMain(uint3 dispatchThreadID : SV_DispatchThreadID, uint3 GroupID : SV_GroupID) //SV_DispatchThreadID: 총 스레드 인덱스, SV_GroupID: 현재 그룹 인덱스
{
    // 스레드 인덱스가 유효한지 판별하기 위한 인덱스 계산(뼈 개수와 미스매칭 잡기)
    int iBoneIndex = dispatchThreadID.x % THREAD_X;
    uint iInstanceIndex = GroupID.x;
    // 유효한 인덱스인지 확인
    bool isValid = iBoneIndex < g_NumBones;
    
    //실제 스레드 그룹에서 사용할 현재 인스턴스 객체의 본 인덱스로 재배치
    uint iRealGlobalIndex = iInstanceIndex * g_NumBones + iBoneIndex;
    
    Skinning(g_LocalMatrices[iRealGlobalIndex], iRealGlobalIndex, isValid);

    if (isValid)
        g_OutModelMatrices[iRealGlobalIndex] = sharedMatrix[iBoneIndex];
}