#include "Engine_ComputeShader_Function.hlsli"

#define THREAD_X 8
#define THREAD_Y 8
#define THREAD_Z 4

Texture2DArray<float> InputTexture : register(t0);
RWTexture2DArray<float> OutputTexture : register(u0);

groupshared int3 InputDiemsnion;

[numthreads(THREAD_X, THREAD_Y, THREAD_Z)]
void SHADOWMAP_DS(uint3 GroupID : SV_GroupID, uint3 DTID : SV_DispatchThreadID, uint3 GTID : SV_GroupThreadID, uint GroupIndex : SV_GroupIndex)
{
    if (GTID.x == 0 && GTID.y == 0)
        InputTexture.GetDimensions(InputDiemsnion.x, InputDiemsnion.y, InputDiemsnion.z);
    
    GroupMemoryBarrierWithGroupSync();
    
    int iIndexX = DTID.x * 2;
    int iIndexY = DTID.y * 2;
    int iLayer = DTID.z;
    float fDepth = 1.f;
    
    int iSampleX0 = min(iIndexX, InputDiemsnion.x - 1);
    int iSampleX1 = min(iIndexX + 1, InputDiemsnion.x - 1);
    
    int iSampleY0 = min(iIndexY, InputDiemsnion.y - 1);
    int iSampleY1 = min(iIndexY + 1, InputDiemsnion.y - 1);
    
    fDepth = min(InputTexture.Load(int4(iSampleX0, iSampleY0, iLayer, 0)), fDepth);
    fDepth = min(InputTexture.Load(int4(iSampleX1, iSampleY0, iLayer, 0)), fDepth);
    fDepth = min(InputTexture.Load(int4(iSampleX0, iSampleY1, iLayer, 0)), fDepth);
    fDepth = min(InputTexture.Load(int4(iSampleX1, iSampleY1, iLayer, 0)), fDepth);
    
    OutputTexture[DTID.xyz] = fDepth;
}
