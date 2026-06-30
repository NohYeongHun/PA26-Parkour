#include "Engine_ComputeShader_Function.hlsli"

#pragma pack_matrix(row_major)

#define THREAD_X 16
#define THREAD_Y 16
#define THREAD_Z 1

#define MAX_RADIUS 15
#define DOF_MAX_RADIUS 3
#define BLUR_RADIUS 6

Texture2D<float4> InputTexture : register(t0);
RWTexture2D<float4> OutputTexture : register(u0);

groupshared int2 vInSize;
groupshared int2 vOutSize;

groupshared float4 vSharedColorX[THREAD_Y][THREAD_X + (2 * MAX_RADIUS)];
groupshared float4 vSharedColorY[THREAD_Y + (2 * MAX_RADIUS)][THREAD_X];

/* ------------------------------------DOF------------------------------------ */

Texture2D<float4> DepthTexture : register(t1);

[numthreads(THREAD_X, THREAD_Y, THREAD_Z)]
void DOF_X(uint3 GroupID : SV_GroupID, uint3 DTID : SV_DispatchThreadID, uint3 GTID : SV_GroupThreadID, uint GroupIndex : SV_GroupIndex)
{
    if (GTID.x == 0 && GTID.y == 0)    
        OutputTexture.GetDimensions(vOutSize.x, vOutSize.y);
    
    GroupMemoryBarrierWithGroupSync();
    
    float4 vDofData = DepthTexture.Load(int3(DTID.xy, 0));
    
    vSharedColorX[GTID.y][GTID.x + MAX_RADIUS] = InputTexture.Load(int3(DTID.xy, 0));
    
    if (GTID.x < MAX_RADIUS)
    {
        int3 LeftID = int3(DTID.x - MAX_RADIUS, DTID.y, 0);
        int3 RightID = int3(DTID.x + THREAD_X, DTID.y, 0);
        
        if (LeftID.x < 0)
            LeftID.x = 0;
        
        if (RightID.x >= (int) vOutSize.x)
            RightID.x = (int) vOutSize.x - 1;
            
        vSharedColorX[GTID.y][GTID.x] = InputTexture.Load(LeftID);
        vSharedColorX[GTID.y][GTID.x + THREAD_X + MAX_RADIUS] = InputTexture.Load(RightID);
    }
    
    GroupMemoryBarrierWithGroupSync();

    float fCoc = vDofData.x;
    
    int iDofRadius = fCoc * (float) MAX_RADIUS;
    
    if (fCoc <= vDofData.z)
    {
        OutputTexture[DTID.xy] = vSharedColorX[GTID.y][GTID.x + MAX_RADIUS];
        return;
    }
    
    float fGaussianSigma = (float) iDofRadius / 3.f;
    
    float4 vColorX = 0.f;
    float fTotalWeight = 0.f;
    for (int i = -iDofRadius; i <= iDofRadius; ++i)
    {
        int iIndexX = GTID.x + MAX_RADIUS + i;
        
        float4 vSampleColor = vSharedColorX[GTID.y][iIndexX];
        
        float fGaussianWeight = exp(-(i * i) / (2.f * fGaussianSigma * fGaussianSigma));
        
        vColorX += vSampleColor * fGaussianWeight;
        fTotalWeight += fGaussianWeight;
    }
    
    OutputTexture[DTID.xy] = vColorX / fTotalWeight;
}

[numthreads(THREAD_X, THREAD_Y, THREAD_Z)]
void DOF_Y(uint3 GroupID : SV_GroupID, uint3 DTID : SV_DispatchThreadID, uint3 GTID : SV_GroupThreadID, uint GroupIndex : SV_GroupIndex)
{
    float4 vDofData = DepthTexture.Load(int3(DTID.xy, 0));
    
    if (GTID.x == 0 && GTID.y == 0)    
        OutputTexture.GetDimensions(vOutSize.x, vOutSize.y);
    
    GroupMemoryBarrierWithGroupSync();
    
    vSharedColorY[GTID.y + DOF_MAX_RADIUS][GTID.x] = InputTexture.Load(int3(DTID.xy, 0));

    if (GTID.y < DOF_MAX_RADIUS)
    {
        int3 LeftID = int3(DTID.x, DTID.y - DOF_MAX_RADIUS, 0);
        int3 RightID = int3(DTID.x, DTID.y + THREAD_Y, 0);
        
        if (LeftID.y < 0)
            LeftID.y = 0;
        
        if (RightID.y >= (int) vOutSize.y)
            RightID.y = (int) vOutSize.y - 1;
            
        vSharedColorY[GTID.y][GTID.x] = InputTexture.Load(LeftID);
        vSharedColorY[GTID.y + THREAD_Y + DOF_MAX_RADIUS][GTID.x] = InputTexture.Load(RightID);
    }
    
    GroupMemoryBarrierWithGroupSync();

    float fCoc = vDofData.x;
    
    int iDofRadius = fCoc * (float) DOF_MAX_RADIUS;
    
    if (fCoc <= vDofData.z)
    {
        OutputTexture[DTID.xy] = vSharedColorY[GTID.y + DOF_MAX_RADIUS][GTID.x];
        return;
    }
    
    float fGaussianSigma = (float) iDofRadius / 3.f;
    
    float4 vColorY = 0.f;
    float fTotalWeight = 0.f;
    for (int i = -iDofRadius; i <= iDofRadius; ++i)
    {
        int iIndexY = GTID.y + DOF_MAX_RADIUS + i;
        
        float4 vSampleColor = vSharedColorY[iIndexY][GTID.x];

        float fGaussianWeight = exp(-(i * i) / (2.f * fGaussianSigma * fGaussianSigma));

        vColorY += vSampleColor * fGaussianWeight;
        fTotalWeight += fGaussianWeight;
    }
    
    OutputTexture[DTID.xy] = vColorY / fTotalWeight;
}