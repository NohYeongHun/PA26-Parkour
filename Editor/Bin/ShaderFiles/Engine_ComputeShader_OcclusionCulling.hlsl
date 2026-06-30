#include "Engine_ComputeShader_Function.hlsli"

#pragma pack_matrix(row_major)

#define THREAD_X 256
#define THREAD_Y 1
#define THREAD_Z 1

#define RADIUS_THRESHOLD 5
#define MAX_DEPTH 10

struct BoxPoint
{
    float4 vCorners[8];
    float3 vCenter;
    float fRadius;
};

static const float INF = 3.402823e38f;
static const float NINF = -3.402823e38f;

StructuredBuffer<BoxPoint> g_BoxPoints : register(t0);
Texture2D<float> InputTexture : register(t1);
RWStructuredBuffer<uint> OutputTexture : register(u0);

cbuffer OCDesc : register(b0)
{
    float4x4 g_ProjMatrix;
    uint g_iNumObjects;
    uint g_iHZBMipLevel;
    float g_fMip0SizeX;
    float g_fMip0SizeY;
};

float2 WorldToScreen_Center(float3 vCenter)
{
    float2 vTexcoord = 0.f;
    
    vector vProjPos = mul(float4(vCenter, 1.f), g_ProjMatrix);
    float2 vNDC = vProjPos.xy / vProjPos.w;
    
    vTexcoord.x = vNDC.x * 0.5f + 0.5f;
    vTexcoord.y = vNDC.y * -0.5f + 0.5f;
    
    return vTexcoord;
}

float2 WorldToScreen_Corner(float4 vCorner)
{
    float2 vTexcoord = 0.f;
    
    vector vProjPos = mul(vCorner, g_ProjMatrix);
    float2 vNDC = vProjPos.xy / vProjPos.w;
    
    vTexcoord.x = vNDC.x * 0.5f + 0.5f;
    vTexcoord.y = vNDC.y * -0.5f + 0.5f;
    
    return vTexcoord;
}

bool CheckOC(BoxPoint Box)
{
    // Small -> Center + Radius
    if(Box.fRadius < RADIUS_THRESHOLD)
    {
        int iMipLevel = clamp(g_iHZBMipLevel + 3, 0, MAX_DEPTH);
        int2 vSize = max(int2(1920, 1080) >> (iMipLevel + 1), int2(1, 1));

        float2 vTexcoord = WorldToScreen_Center(Box.vCenter);
        float fBoxDepth = Box.vCenter.z;
        
        int2 px = int2(saturate(vTexcoord) * (vSize - 1));
        float fHZBDepth = InputTexture.Load(int3(px, iMipLevel));
        if (0.f == fHZBDepth)
            return true;
        
        float eps = max(10.f, fBoxDepth * 0.01f);
        if (fBoxDepth < fHZBDepth + Box.fRadius + eps)
            return true;
        return false;
    }
    // Big >> Corner + Center
    else
    {
        int iMipLevel = clamp(g_iHZBMipLevel, 0, MAX_DEPTH);
        int2 vSize = max(int2(1920, 1080) >> (iMipLevel + 1), int2(1, 1));
        
        // Center
        float2 vTexcoord = WorldToScreen_Center(Box.vCenter);
        float fBoxDepth = Box.vCenter.z;
        
        int2 px = int2(saturate(vTexcoord) * (vSize - 1));
        float fHZBDepth = InputTexture.Load(int3(px, iMipLevel));
        if (0.f == fHZBDepth)
            return true;
        
        float eps = max(10.f, fBoxDepth * 0.01f);
        if (fBoxDepth < fHZBDepth + Box.fRadius + eps)
            return true;

        // Corner
        for (int i = 0; i < 8; ++i)
        {
            float fOffsetX = i & 1 ? 1.5f / vSize : -1.5f / vSize;
            float fOffsetY = i & 2 ? 1.5f / vSize : -1.5f / vSize;
            vTexcoord = WorldToScreen_Corner(Box.vCorners[i]);
            vTexcoord += float2(fOffsetX, fOffsetY);
            float fCornerDepth = Box.vCorners[i].z;
        
            px = int2(saturate(vTexcoord) * (vSize - 1));
            fHZBDepth = InputTexture.Load(int3(px, iMipLevel));
            if (0.f == fHZBDepth)
                return true;
        
            eps = max(10.f, fCornerDepth * 0.03f);
            if (fCornerDepth < fHZBDepth + eps)
                return true;
        }
        return false;
    }
}

bool CheckOC2(BoxPoint Box)
{
    float2 vUVMin = float2(1.f, 1.f);
    float2 vUVMax = float2(0.f, 0.f);
    float fMinDepth = INF;
    
    // Phase01
    
    // Corner
    for (int i = 0; i < 8; ++i)
    {
        float2 vTexcoord = WorldToScreen_Corner(Box.vCorners[i]);
        vUVMin = min(vUVMin, vTexcoord);
        vUVMax = max(vUVMax, vTexcoord);
        fMinDepth = min(fMinDepth, Box.vCorners[i].z);
    }
    
    float2 vExtent = (vUVMax - vUVMin) * float2(g_fMip0SizeX, g_fMip0SizeY);
    
    // Scale Skip
    float fExtent = max(vExtent.x, vExtent.y);
    if (fExtent < 8.f || fExtent > g_fMip0SizeX * 0.25f)
        return true;
    
    int iMipLevel = clamp(floor(log2(fExtent)) - 2, 0, MAX_DEPTH - 1);
    
    // OC
    int iSizeX = 0;
    int iSizeY = 0;
    int iNumOfLevel = 0;
    InputTexture.GetDimensions(iMipLevel, iSizeX, iSizeY, iNumOfLevel);
    
    float2 vUV[4];
    vUV[0] = vUVMin;
    vUV[1] = float2(vUVMax.x, vUVMin.y);
    vUV[2] = vUVMax;
    vUV[3] = float2(vUVMin.x, vUVMax.y);
    
    for (int i = 0; i < 4; ++i)
    {
        float fOffsetX = i & 1 ? 0.5f / iSizeX : -0.5f / iSizeX;
        float fOffsetY = i & 2 ? 0.5f / iSizeY : -0.5f / iSizeY;
        float2 vTex = vUV[i] + float2(fOffsetX, fOffsetY);
        
        int2 px = int2(saturate(vTex) * int2(iSizeX - 1, iSizeY - 1));
        float fHZBDepth = InputTexture.Load(int3(px, iMipLevel));
        
        float eps = max(10.f, fMinDepth * 0.03f);
        if (fMinDepth < fHZBDepth + eps)
            return true;
    }
    
    // Center
    float2 vCenterTexcoord = WorldToScreen_Center(Box.vCenter);
    int2 px = int2(saturate(vCenterTexcoord) * int2(iSizeX - 1, iSizeY - 1));
    float fHZBDepth = InputTexture.Load(int3(px, iMipLevel));
    float fCenterDepth = Box.vCenter.z;
    float eps = max(10.f, fCenterDepth * 0.03f);
    if (fCenterDepth < fHZBDepth + Box.fRadius + eps)
        return true;
    
    // Phase02 (MipLevel - 1)
    if(iMipLevel > 0)
    {
        iMipLevel -= 1;
        InputTexture.GetDimensions(iMipLevel, iSizeX, iSizeY, iNumOfLevel);
        for (int i = 0; i < 4; ++i)
        {
            float fOffsetX = i & 1 ? 0.5f / iSizeX : -0.5f / iSizeX;
            float fOffsetY = i & 2 ? 0.5f / iSizeY : -0.5f / iSizeY;
            float2 vTex = vUV[i] + float2(fOffsetX, fOffsetY);
        
            int2 px = int2(saturate(vTex) * int2(iSizeX - 1, iSizeY - 1));
            float fHZBDepth = InputTexture.Load(int3(px, iMipLevel));
        
            float eps = max(10.f, fMinDepth * 0.03f);
            if (fMinDepth < fHZBDepth + eps)
                return true;
        }
        px = int2(saturate(vCenterTexcoord) * int2(iSizeX - 1, iSizeY - 1));
        fHZBDepth = InputTexture.Load(int3(px, iMipLevel));
        float eps = max(10.f, fCenterDepth * 0.03f);
        if (fCenterDepth < fHZBDepth + Box.fRadius + eps)
            return true;
    }
    
    return false;
}

[numthreads(THREAD_X, THREAD_Y, THREAD_Z)]
void Occlusion_Culling(uint3 DTID : SV_DispatchThreadID)
{
    uint iIndex = DTID.x;

    if (iIndex >= g_iNumObjects)
        return;
    
   BoxPoint box = g_BoxPoints[iIndex];
   bool isVisible = CheckOC2(box);

   OutputTexture[iIndex] = isVisible ? 1 : 0;
}
