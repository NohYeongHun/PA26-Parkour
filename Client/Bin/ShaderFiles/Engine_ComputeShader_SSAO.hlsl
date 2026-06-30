#include "Engine_ComputeShader_Function.hlsli"

#pragma pack_matrix(row_major)

#define THREAD_X 16
#define THREAD_Y 16
#define THREAD_Z 1

Texture2D<float4> InputTexture : register(t0);
RWTexture2D<float4> OutputTexture : register(u0);

Texture2D<float4> g_NormalTexture : register(t1);
Texture2D<float4> g_DepthTexture : register(t2);
Texture2D<float4> g_NoiseTexture : register(t3);

SamplerState CS_DefaultSampler : register(s1);
SamplerState CS_PointClampSampler : register(s2);
SamplerState CS_NoiseSampler : register(s3);

cbuffer SSAO_BLUR_DATA : register(b1)
{
    float fSSAO_MinDepthDistance;
    float fWidth_Blur;
    float fHeight_Blur;
    float Paddingblur;         
}

#define SSAO_BLUR_RADIUS 4

groupshared float4 vSharedColorX[THREAD_Y][THREAD_X + (2 * SSAO_BLUR_RADIUS)];
groupshared float vSharedDepthX[THREAD_Y][THREAD_X + (2 * SSAO_BLUR_RADIUS)];
groupshared float4 vSharedNormalX[THREAD_Y][THREAD_X + (2 * SSAO_BLUR_RADIUS)];
    
[numthreads(THREAD_X, THREAD_Y, THREAD_Z)]
void SSAO_BLUR_X(uint3 GruopID : SV_GroupID, uint3 DTID : SV_DispatchThreadID, uint3 GTID : SV_GroupThreadID, uint GruopIndex : SV_GroupIndex)
{
    float fBlurRadius = (float) SSAO_BLUR_RADIUS;
    
    vSharedColorX[GTID.y][GTID.x + SSAO_BLUR_RADIUS] = InputTexture.Load(int3(DTID.xy, 0));
    vSharedDepthX[GTID.y][GTID.x + SSAO_BLUR_RADIUS] = g_DepthTexture.Load(int3(DTID.xy, 0)).y;
    vSharedNormalX[GTID.y][GTID.x + SSAO_BLUR_RADIUS] = Compute_Normal_DTID(g_NormalTexture, int3(DTID.xy, 0));
    
    if (GTID.x < SSAO_BLUR_RADIUS)
    {                   
        int3 LeftID = int3(DTID.x - SSAO_BLUR_RADIUS, DTID.y, 0);
        int3 RightID = int3(DTID.x + THREAD_X, DTID.y, 0);
        
        if(LeftID.x < 0)
            LeftID.x = 0;
        
        if (RightID.x >= fWidth_Blur)
            RightID.x = fWidth_Blur - 1.f;
        
        vSharedColorX[GTID.y][GTID.x] = InputTexture.Load(LeftID);
        vSharedDepthX[GTID.y][GTID.x] = g_DepthTexture.Load(LeftID).y;
        vSharedNormalX[GTID.y][GTID.x] = Compute_Normal_DTID(g_NormalTexture, LeftID);
        
        vSharedColorX[GTID.y][GTID.x + THREAD_X + SSAO_BLUR_RADIUS] = InputTexture.Load(RightID);
        vSharedDepthX[GTID.y][GTID.x + THREAD_X + SSAO_BLUR_RADIUS] =  g_DepthTexture.Load(RightID).y;
        vSharedNormalX[GTID.y][GTID.x + THREAD_X + SSAO_BLUR_RADIUS] = Compute_Normal_DTID(g_NormalTexture, RightID);
    }
    
    GroupMemoryBarrierWithGroupSync();
    
    float4 vColorX = 0.f;
    float fWeightX = 0.f;
    
    float4 vOriginColorX = vSharedColorX[GTID.y][GTID.x + SSAO_BLUR_RADIUS];
    float fOriginDepthX = vSharedDepthX[GTID.y][GTID.x + SSAO_BLUR_RADIUS];
    float4 vOriginNormalX = vSharedNormalX[GTID.y][GTID.x + SSAO_BLUR_RADIUS];
    
    for (int i = -SSAO_BLUR_RADIUS; i <= SSAO_BLUR_RADIUS; ++i)
    {
        int iIndexX = GTID.x + SSAO_BLUR_RADIUS + i;
        
        float4 vSampleColor = vSharedColorX[GTID.y][iIndexX];
        float fSampleDepth = vSharedDepthX[GTID.y][iIndexX];
        float4 vSampleNormal = vSharedNormalX[GTID.y][iIndexX];
        
        vColorX += Compute_SSAO_Blur(vOriginColorX, fOriginDepthX, vOriginNormalX, vSampleColor, fSampleDepth, vSampleNormal, fSSAO_MinDepthDistance, fWeightX);
    }
    
    vColorX /= (fBlurRadius * 2.f + 1.f);
    
    OutputTexture[DTID.xy] = vColorX;
  
}
groupshared float4 vSharedColorY[THREAD_Y + (2 * SSAO_BLUR_RADIUS)][THREAD_X];
groupshared float vSharedDepthY[THREAD_Y + (2 * SSAO_BLUR_RADIUS)][THREAD_X];
groupshared float4 vSharedNormalY[THREAD_Y + (2 * SSAO_BLUR_RADIUS)][THREAD_X];

[numthreads(THREAD_X, THREAD_Y, THREAD_Z)]
void SSAO_BLUR_Y(uint3 GruopID : SV_GroupID, uint3 DTID : SV_DispatchThreadID, uint3 GTID : SV_GroupThreadID, uint GruopIndex : SV_GroupIndex)
{
    float fBlurRadius = (float) SSAO_BLUR_RADIUS;
    
    vSharedColorY[GTID.y + SSAO_BLUR_RADIUS][GTID.x] = InputTexture.Load(int3(DTID.xy, 0));
    vSharedDepthY[GTID.y + SSAO_BLUR_RADIUS][GTID.x] = g_DepthTexture.Load(int3(DTID.xy, 0)).y;
    vSharedNormalY[GTID.y + SSAO_BLUR_RADIUS][GTID.x] = Compute_Normal_DTID(g_NormalTexture, int3(DTID.xy, 0));
    
    if (GTID.y < SSAO_BLUR_RADIUS)
    {
        int3 LeftID = int3(DTID.x, DTID.y - SSAO_BLUR_RADIUS, 0);
        int3 RightID = int3(DTID.x, DTID.y + THREAD_Y, 0);
        
        if (LeftID.y < 0)
            LeftID.y = 0;
        
        if (RightID.y >= fHeight_Blur)
            RightID.y = fHeight_Blur - 1.f;
        
        
        vSharedColorY[GTID.y][GTID.x] = InputTexture.Load(LeftID);
        vSharedDepthY[GTID.y][GTID.x] = g_DepthTexture.Load(LeftID).y;
        vSharedNormalY[GTID.y][GTID.x] = Compute_Normal_DTID(g_NormalTexture, LeftID);
        
        vSharedColorY[GTID.y + THREAD_Y + SSAO_BLUR_RADIUS][GTID.x] = InputTexture.Load(RightID);
        vSharedDepthY[GTID.y + THREAD_Y + SSAO_BLUR_RADIUS][GTID.x] = g_DepthTexture.Load(RightID).y;
        vSharedNormalY[GTID.y + THREAD_Y + SSAO_BLUR_RADIUS][GTID.x] = Compute_Normal_DTID(g_NormalTexture, RightID);
    }
    
    GroupMemoryBarrierWithGroupSync();
    
    float4 vOriginColorY = vSharedColorY[GTID.y + SSAO_BLUR_RADIUS][GTID.x];
    float fOriginDepthY = vSharedDepthY[GTID.y + SSAO_BLUR_RADIUS][GTID.x];
    float4 vOriginNormalY = vSharedNormalY[GTID.y + SSAO_BLUR_RADIUS][GTID.x];
    
    float4 vColorY = 0.f;
    float fWeightY = 0.f;
    
    for (int j = -SSAO_BLUR_RADIUS; j <= SSAO_BLUR_RADIUS; ++j)
    {
        int iIndexY = GTID.y + SSAO_BLUR_RADIUS + j;
        
        float4 vSampleColor = vSharedColorY[iIndexY][GTID.x];
        float fSampleDepth = vSharedDepthY[iIndexY][GTID.x];
        float4 vSampleNormal = vSharedNormalY[iIndexY][GTID.x];
        
        vColorY += Compute_SSAO_Blur(vOriginColorY, fOriginDepthY, vOriginNormalY, vSampleColor, fSampleDepth, vSampleNormal, fSSAO_MinDepthDistance, fWeightY);
    }
    vColorY /= (fBlurRadius * 2.f + 1.f);
    
    OutputTexture[DTID.xy] = vColorY;
}


