#include "Engine_ComputeShader_Function.hlsli"

#pragma pack_matrix(row_major)

#define THREAD_X 16
#define THREAD_Y 16
#define THREAD_Z 1

#define BLUR_RADIUS 2

Texture2D<float4> g_DiffuseTexture : register(t0);
Texture2D<float4> g_StrengthTexture : register(t1);
Texture2D<float4> g_NormalTexture : register(t2);
Texture2D<float4> g_DepthTexture : register(t3);

RWTexture2D<float4> OutputTexture : register(u0);

const static int2 g_vSize = int2(1920, 1080);

const static float g_fBlurWieght[5] = { 0.15f, 0.2f, 0.3f, 0.2f, 0.15f };

groupshared int2 vInSize;

groupshared float4 vSharedDiffuseX[THREAD_Y][THREAD_X + (2 * BLUR_RADIUS)];
groupshared float4 vSharedNormalX[THREAD_Y][THREAD_X + (2 * BLUR_RADIUS)];
groupshared float  fSharedDepthX[THREAD_Y][THREAD_X + (2 * BLUR_RADIUS)];
groupshared float  fSharedStrengthX[THREAD_Y][THREAD_X + (2 * BLUR_RADIUS)];

[numthreads(THREAD_X, THREAD_Y, THREAD_Z)]
void SSSBlur_X(uint3 GroupID : SV_GroupID, uint3 DTID : SV_DispatchThreadID, uint3 GTID : SV_GroupThreadID, uint GroupIndex : SV_GroupIndex)
{
    if (all(GTID.xy == 0))
        g_DiffuseTexture.GetDimensions(vInSize.x, vInSize.y);

    GroupMemoryBarrierWithGroupSync();
        
    int iRadius = (int) BLUR_RADIUS;
        
    vSharedDiffuseX[GTID.y][GTID.x + iRadius] = g_DiffuseTexture.Load(int3(DTID.xy, 0));
    vSharedNormalX[GTID.y][GTID.x + iRadius] = g_NormalTexture.Load(int3(DTID.xy, 0));
    fSharedDepthX[GTID.y][GTID.x + iRadius] = g_DepthTexture.Load(int3(DTID.xy, 0)).y;
    
    float fStrength = all(g_StrengthTexture.Load(int3(DTID.xy, 0)).rg > 0.f) ? 1.f : 0.f;
    fSharedStrengthX[GTID.y][GTID.x + iRadius] = fStrength; 
    
    if (GTID.x < iRadius)
    {
        int3 LeftID = int3(DTID.x - iRadius, DTID.y, 0);
        int3 RightID = int3(DTID.x + THREAD_X, DTID.y, 0);
        
        if (LeftID.x < 0)
            LeftID.x = 0;

        if (RightID.x >= (int) g_vSize.x)
            RightID.x = (int) g_vSize.x - 1;
            
        vSharedDiffuseX[GTID.y][GTID.x] = g_DiffuseTexture.Load(int3(LeftID));
        vSharedNormalX[GTID.y][GTID.x] = g_NormalTexture.Load(int3(LeftID));
        fSharedDepthX[GTID.y][GTID.x] = g_DepthTexture.Load(int3(LeftID)).y;
        float fLeftStrength = all(g_StrengthTexture.Load(int3(LeftID)).rg > 0.f) ? 1.f : 0.f;
        fSharedStrengthX[GTID.y][GTID.x] = fLeftStrength;
        
        vSharedDiffuseX[GTID.y][GTID.x + THREAD_X + iRadius] = g_DiffuseTexture.Load(int3(RightID));
        vSharedNormalX[GTID.y][GTID.x + THREAD_X + iRadius] = g_NormalTexture.Load(int3(RightID));
        fSharedDepthX[GTID.y][GTID.x + THREAD_X + iRadius] = g_DepthTexture.Load(int3(RightID)).y;
    
        float fRightStrength = all(g_StrengthTexture.Load(int3(RightID)).rg > 0.f) ? 1.f : 0.f;
        fSharedStrengthX[GTID.y][GTID.x + THREAD_X + iRadius] = fRightStrength;
    }
    
    GroupMemoryBarrierWithGroupSync();
    
    float4 vOriginColor = vSharedDiffuseX[GTID.y][GTID.x + iRadius];
    
    float fCenterStrength = fSharedStrengthX[GTID.y][GTID.x + iRadius];
    
    if (fCenterStrength <= 0.1f)
    {
        OutputTexture[DTID.xy] = vOriginColor;
        return;
    }
    
    float4 vColor = 0.f;
    float fTotalWeight = 0.f;
    
    float fGaussianSigma = (float) iRadius / 3.f;
    
    float4 vOriginNormal = vSharedNormalX[GTID.y][GTID.x + iRadius];
    vOriginNormal = normalize(vOriginNormal * 2.f - 1.f);
    
    for (int i = -iRadius; i <= iRadius; ++i)
    {
        int iIndexX = GTID.x + iRadius + i;
        
        float4 vSampleColor = vSharedDiffuseX[GTID.y][iIndexX]; 
        float fSampleDepth = fSharedDepthX[GTID.y][iIndexX];
        float4 vSampleNormal = vSharedNormalX[GTID.y][iIndexX];
        float fSampleStrength = fSharedStrengthX[GTID.y][iIndexX];
        
        vSampleNormal = normalize(vSampleNormal * 2.f - 1.f);
        
        float fNormalWeight = (saturate(dot(vOriginNormal.xyz, vSampleNormal.xyz)));
        float fDepthWeight = 1.f;
        float fStrengthWeight = min(fCenterStrength, fSampleStrength);
        float fGaussianWeight = g_fBlurWieght[i + iRadius]; //exp(-(i * i) / (2.f * fGaussianSigma * fGaussianSigma));

        float fWeight = fNormalWeight * fDepthWeight * fStrengthWeight * fGaussianWeight;
        
        vColor += vSampleColor * fWeight;
        fTotalWeight += fWeight;
    }
    
    float4 vBlurColor = 0.f;
    
    vBlurColor = (fTotalWeight > 0.f) ? (vColor / fTotalWeight) : vOriginColor;
    
    OutputTexture[DTID.xy] = lerp(vOriginColor, vBlurColor, fCenterStrength);
}

groupshared float4 vSharedDiffuseY[THREAD_Y + (2 * BLUR_RADIUS)][THREAD_X];
groupshared float4 vSharedNormalY[THREAD_Y + (2 * BLUR_RADIUS)][THREAD_X];
groupshared float fSharedDepthY[THREAD_Y + (2 * BLUR_RADIUS)][THREAD_X];
groupshared float fSharedStrengthY[THREAD_Y + (2 * BLUR_RADIUS)][THREAD_X];

[numthreads(THREAD_X, THREAD_Y, THREAD_Z)]
void SSSBlur_Y(uint3 GroupID : SV_GroupID, uint3 DTID : SV_DispatchThreadID, uint3 GTID : SV_GroupThreadID, uint GroupIndex : SV_GroupIndex)
{
    if (all(GTID.xy == 0))
        g_DiffuseTexture.GetDimensions(vInSize.x, vInSize.y);

    GroupMemoryBarrierWithGroupSync();
        
    int iRadius = (int) BLUR_RADIUS;
        
    vSharedDiffuseY[GTID.y + iRadius][GTID.x] = g_DiffuseTexture.Load(int3(DTID.xy, 0));
    vSharedNormalY[GTID.y + iRadius][GTID.x] = g_NormalTexture.Load(int3(DTID.xy, 0));
    fSharedDepthY[GTID.y + iRadius][GTID.x] = g_DepthTexture.Load(int3(DTID.xy, 0)).y;
    
    float fStrength = all(g_StrengthTexture.Load(int3(DTID.xy, 0)).rg > 0.f) ? 1.f : 0.f;
    fSharedStrengthY[GTID.y + iRadius][GTID.x] = fStrength;
    
    if (GTID.y < iRadius)
    {
        int3 LeftID = int3(DTID.x, DTID.y - iRadius, 0);
        int3 RightID = int3(DTID.x, DTID.y + THREAD_Y, 0);
        
        if (LeftID.y < 0)
            LeftID.y = 0;

        if (RightID.y >= (int) g_vSize.y)
            RightID.y = (int) g_vSize.y - 1;
            
        vSharedDiffuseY[GTID.y][GTID.x] = g_DiffuseTexture.Load(int3(LeftID));
        vSharedNormalY[GTID.y][GTID.x] = g_NormalTexture.Load(int3(LeftID));
        fSharedDepthY[GTID.y][GTID.x] = g_DepthTexture.Load(int3(LeftID)).y;
        float fLeftStrength = all(g_StrengthTexture.Load(int3(LeftID)).rg > 0.f) ? 1.f : 0.f;
        fSharedStrengthY[GTID.y][GTID.x] = fLeftStrength;
        
        vSharedDiffuseY[GTID.y + THREAD_Y + iRadius][GTID.x] = g_DiffuseTexture.Load(int3(RightID));
        vSharedNormalY[GTID.y + THREAD_Y + iRadius][GTID.x] = g_NormalTexture.Load(int3(RightID));
        fSharedDepthY[GTID.y + THREAD_Y + iRadius][GTID.x] = g_DepthTexture.Load(int3(RightID)).y;
    
        float fRightStrength = all(g_StrengthTexture.Load(int3(RightID)).rg > 0.f) ? 1.f : 0.f;
        fSharedStrengthY[GTID.y + THREAD_Y + iRadius][GTID.x] = fRightStrength;
    }
    
    GroupMemoryBarrierWithGroupSync();
    
    float4 vOriginColor = vSharedDiffuseY[GTID.y + iRadius][GTID.x];
    
    float fCenterStrength = fSharedStrengthY[GTID.y + iRadius][GTID.x];
    
    if (fCenterStrength <= 0.1f)
    {
        OutputTexture[DTID.xy] = vOriginColor;
        return;
    }
    
    float4 vColor = 0.f;
    float fTotalWeight = 0.f;
    
    float fGaussianSigma = (float) iRadius / 3.f;
    
    float4 vOriginNormal = vSharedNormalY[GTID.y + iRadius][GTID.x];
    vOriginNormal = normalize(vOriginNormal * 2.f - 1.f);
    
    for (int i = -iRadius; i <= iRadius; ++i)
    {
        int iIndexY = GTID.y + iRadius + i;
        
        float4 vSampleColor = vSharedDiffuseY[iIndexY][GTID.x];
        float4 vSampleNormal = vSharedNormalY[iIndexY][GTID.x];

        float fSampleDepth = fSharedDepthY[iIndexY][GTID.x];
        float fSampleStrength = fSharedStrengthY[iIndexY][GTID.x];
        
        vSampleNormal = normalize(vSampleNormal * 2.f - 1.f);
        
        float fNormalWeight = (saturate(dot(vOriginNormal.xyz, vSampleNormal.xyz)));
        float fDepthWeight = 1.f;
        float fStrengthWeight = min(fCenterStrength, fSampleStrength);
        float fGaussianWeight = g_fBlurWieght[i + iRadius]; //exp(-(i * i) / (2.f * fGaussianSigma * fGaussianSigma));

        float fWeight = fNormalWeight * fDepthWeight * fStrengthWeight * fGaussianWeight;
        
        vColor += vSampleColor * fWeight;
        fTotalWeight += fWeight;
    }
    
    float4 vBlurColor = 0.f;
    
    vBlurColor = (fTotalWeight > 0.f) ? (vColor / fTotalWeight) : vOriginColor;
    
    OutputTexture[DTID.xy] = lerp(vOriginColor, vBlurColor, fCenterStrength);
}