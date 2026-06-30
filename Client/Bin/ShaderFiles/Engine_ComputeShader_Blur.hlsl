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

cbuffer BLUR_DATA : register(b0)
{
    float2 fOutSize;
    int iRadius;
    float Paddingblur;
}

StructuredBuffer<float> g_Weights : register(t1);

groupshared int2 vInSize;
groupshared int2 vOutSize;

groupshared float4 vSharedColorX[THREAD_Y][THREAD_X + (2 * MAX_RADIUS)];
groupshared float4 vSharedColorY[THREAD_Y + (2 * MAX_RADIUS)][THREAD_X];

[numthreads(THREAD_X, THREAD_Y, THREAD_Z)]
void GaussianBlur_X(uint3 GroupID : SV_GroupID, uint3 DTID : SV_DispatchThreadID, uint3 GTID : SV_GroupThreadID, uint GroupIndex : SV_GroupIndex)
{
    float fBlurRadius = iRadius;
    
    vSharedColorX[GTID.y][GTID.x + iRadius] = InputTexture.Load(int3(DTID.xy, 0));
    
    if (GTID.x < iRadius)
    {
        int3 LeftID = int3(DTID.x - iRadius, DTID.y, 0);
        int3 RightID = int3(DTID.x + THREAD_X, DTID.y, 0);
        
        if (LeftID.x < 0)
            LeftID.x = 0;
        
        if (RightID.x >= (int) fOutSize.x)
            RightID.x = (int) fOutSize.x - 1;
            
        vSharedColorX[GTID.y][GTID.x] = InputTexture.Load(LeftID);
        vSharedColorX[GTID.y][GTID.x + THREAD_X + iRadius] = InputTexture.Load(RightID);
    }
    
    GroupMemoryBarrierWithGroupSync();

    float4 vColorX = 0.f;
    
    for (int i = -iRadius; i <= iRadius; ++i)
    {
        int iIndexX = GTID.x + iRadius + i;
        
        float4 vSampleColor = vSharedColorX[GTID.y][iIndexX];
        
        vColorX += vSampleColor * g_Weights[i + iRadius];
    }
    
    OutputTexture[DTID.xy] = vColorX;
}

[numthreads(THREAD_X, THREAD_Y, THREAD_Z)]
void GaussianBlur_Y(uint3 GroupID : SV_GroupID, uint3 DTID : SV_DispatchThreadID, uint3 GTID : SV_GroupThreadID, uint GroupIndex : SV_GroupIndex)
{
    float fBlurRadius = (float) iRadius;
    
    vSharedColorY[GTID.y + iRadius][GTID.x] = InputTexture.Load(int3(DTID.xy, 0));
    
    if (GTID.y < iRadius)
    {
        int3 LeftID = int3(DTID.x, DTID.y - iRadius, 0);
        int3 RightID = int3(DTID.x, DTID.y + THREAD_Y, 0);
        
        if (LeftID.y < 0)
            LeftID.y = 0;
        
        if (RightID.y >= (int) fOutSize.y)
            RightID.y = (int) fOutSize.y - 1;
            
        vSharedColorY[GTID.y][GTID.x] = InputTexture.Load(LeftID);
        vSharedColorY[GTID.y + THREAD_Y + iRadius][GTID.x] = InputTexture.Load(RightID);
    }
    
    GroupMemoryBarrierWithGroupSync();
    
    float4 vColorY = 0.f;
    
    for (int j = -iRadius; j <= iRadius; ++j)
    {
        int iIndexY = GTID.y + iRadius + j;
        
        float4 vSampleColor = vSharedColorY[iIndexY][GTID.x];
        
        vColorY += vSampleColor * g_Weights[j + iRadius];
    }
    
    OutputTexture[DTID.xy] = vColorY;
}