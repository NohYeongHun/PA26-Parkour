#include "Engine_ComputeShader_Function.hlsli"

//#pragma pack_matrix(row_major)

#define THREAD_X 8
#define THREAD_Y 8
#define LIGHT_THREAD_Z 8
#define BEER_THREAD_Z 1

#define MAX_MIPLEVEL 10
#define MIN_MIPLEVEL 3

#define EPSILON 1e-05

#define MAX_SECTOR 64

#define MAX_HALTON_SEQUENCE 16

static const float3 HALTON_SEQUENCE[MAX_HALTON_SEQUENCE] =
{
    float3(0.5f, 0.333333f, 0.2f),
    float3(0.25f, 0.666667f, 0.4f),
    float3(0.75f, 0.111111f, 0.6f),
	float3(0.125f, 0.444444f, 0.8f),
	float3(0.625f, 0.777778f, 0.04f),
	float3(0.375f, 0.222222f, 0.24f),
	float3(0.875f, 0.555556f, 0.44f),
	float3(0.0625f, 0.888889f, 0.64f),
	float3(0.5625f, 0.037037f, 0.84f),
	float3(0.3125f, 0.37037f, 0.08f),
	float3(0.8125f, 0.703704f, 0.28f),
	float3(0.1875f, 0.148148f, 0.48f),
	float3(0.6875f, 0.481482f, 0.68f),
	float3(0.4375f, 0.814815f, 0.88f),
	float3(0.9375f, 0.259259f, 0.12f),
	float3(0.03125f, 0.592593f, 0.32f)
};

float3 JITTER(uint2 XY, uint RandCount)
{
    float3 Jitter = HALTON_SEQUENCE[((RandCount + XY.x + XY.y) * 2) % MAX_HALTON_SEQUENCE];
    
    Jitter -= 0.5f;
    
    return Jitter;
}

struct LightData
{
    uint iType; // 0 = Directional, 1 = Point
    float fRange;
    float Padding[2];
    float4 vDiffuse;
    float4 vDirection;
    float4 vPosition;
    float4 vAmbient;
    float4 vSpecular;
};

StructuredBuffer<LightData> g_LightDatas : register(t0);

Texture2D<float> g_MipDepthTexture : register(t1);
Texture2DArray<float> g_ShadowMapTexture : register(t2);

Texture3D<float4> VFLightTexture : register(t3);
Texture3D<float4> PrevVFLightTexture : register(t4);

Texture3D<float> g_NoiseTexture : register(t5);

Texture2D<float4> g_TestNoiseTexture : register(t6);

RWTexture3D<float4> OutputTexture : register(u0);

SamplerState DefaultSampler : register(s0);
SamplerComparisonState ShadowSampler : register(s1);

cbuffer VF_Data : register(b0)
{
    float4x4 PrevViewMatrix;
    float4x4 PrevProjMatrix;
    float4x4 InvViewMatrix;
    float4x4 InvProjMatrix;
    float4 vCamPos; // --
    float3 vFogColor;
    float fPadding12; // --
    float3 vFroxelSize;
    float Padding; // --
    float fFogNear; 
    float fFogFar; 
    uint iSliceCount; 
    uint iLightCount;           // --
    float fCamNear; 
    float fCamFar; 
    float fWinSizeX; 
    float fWinSizeY;            // --
    float fLightIntensity; 
    float fDensity; 
    float fPhaseFunctionG; 
    float fDensityScale;        // --
    float fFogMaxHeight; 
    float fFogMaxDistance; 
    float fHegihtFallOff; 
    float fDistanceFallOff;     // --
    float fGroundFallOff; 
    float fNoiseScale; 
    float fNoiseTimeDelta; 
    bool IsTemporal;            // --
    uint iRandCount;
    float fRayPhaseFunctionG;
    float fRayIntensity;
    float fScatterWeight;       // --
    float fFogBaseIntensity;
    float fRayDensity;
    float fRayDensityScale;
    float vPadding;             // -- 
};

cbuffer ShadowMap_Data : register(b1)
{
    float4x4 g_SectorViewMatrix[MAX_SECTOR];
    float4x4 g_SectorProjMatrix[MAX_SECTOR];
    float4 g_vSectorUV[16];

    int iNumSector;
    int iNumSectorX;
    int iNumSectorToLayer;
    float Padding3;
    
    float2 vSectorWorldSize;
    float2 Padding4;
    
    float2 vMin;
    float2 Padding5;

    float2 vMax;
    float2 Padding6;
    
    float2 vShadowMapSize;
}

float ComputeSliceDepth(float fSlice, uint iSliceCount, float fFogNear, float fFogFar)
{
    return fFogNear * pow(fFogFar / fFogNear, fSlice / (float) iSliceCount);
}

float ComputeDepthToProjZ(float fViewZ, float fNear, float fFar)
{
    return ((fFar * fViewZ) / (fFar - fNear)) - (fFar * fNear) / (fFar - fNear); // (f * z / f-n) - (f*n / (f - n)  = 투영 보정 (w 나누기 하기 전 )
}

uint ComputeMipLevel(float4x4 ProjMatrix, float fViewZ, float fScreenSizeX, float fScreenSizeY)
{
    float fFovX = (1.f / ProjMatrix._11);
    float fFovY = (1.f / ProjMatrix._22);
    
    float fFroxelWidth = 2.f * fViewZ * fFovX / fScreenSizeX;
    float fFroxelHeight = 2.f * fViewZ * fFovY / fScreenSizeY;
    
    float fPixelSize = max(fFroxelWidth, fFroxelHeight);
    
    uint iMipLevel = (uint) ceil(log2(fPixelSize * fScreenSizeY));
    
    iMipLevel = max(min(iMipLevel, MAX_MIPLEVEL), MIN_MIPLEVEL);
    
    return iMipLevel;
}

float ReturnMinDepth(uint iMipLevel, uint2 DTID, float2 vScreenSize, float2 vFroxelSize)
{
    float fDepth = 0.f;
    
    uint FroxelScale = vScreenSize / vFroxelSize;
    
    uint iMipScale = 1 << iMipLevel;
    
    uint2 vIndex = (DTID.xy * FroxelScale) / iMipScale;
    
    float2 vTexcoord = (float2) vIndex + 0.5f * (iMipScale / vScreenSize);
    
    fDepth = g_MipDepthTexture.SampleLevel(DefaultSampler, vTexcoord, (iMipLevel - 1));
    
    return fDepth;
}

float HenyeyGreensteinPhasefunction(float3 LightDir, float3 LightOutDir, float G)
{
    float cosTheta = dot(LightDir, LightOutDir);
    float G2 = G * G;
    float Denom = pow(1.f + G2 - 2.f * G * cosTheta, 3.f / 2.f);
    
    return (1.f / (4.f * PI)) * ((1.f - G2) / max(Denom, EPSILON)); /**/
}

/*------------------------------------ SHADOW_MAP ------------------------------------*/

float ShadowPCF(float3 UVDepth, int iIndex, int iNumWeight, Texture2DArray<float> ShadowMap, float2 vTexelSize, float2 vMinUV, float2 vMaxUV)
{
    float fShadow = 0.f;
    
    int iRadius = iNumWeight * 2 + 1;
    
    [unroll]
    for (int x = -iNumWeight; x <= iNumWeight; ++x)
    {
        [unroll]
        for (int y = -iNumWeight; y <= iNumWeight; ++y)
        {
            float2 vOffset = float2(x, y) * vTexelSize;
            
            float2 vUV = UVDepth.xy + vOffset;
            
            vUV.x = clamp(vUV.x, vMinUV.x, vMaxUV.x);
            vUV.y = clamp(vUV.y, vMinUV.y, vMaxUV.y);
            
            fShadow += ShadowMap.SampleCmpLevelZero(ShadowSampler, float3(vUV, iIndex), UVDepth.z);
        }
    }
    
    fShadow /= pow(iRadius, 2);
    
    return fShadow;
}

int2 Find_Sector(float4 vWorldPos)
{
    float2 vSectorPos = vWorldPos.xz - vMin;
        
    int2 vSector = (int2) floor((vSectorPos + 0.1f) / vSectorWorldSize);
    
    int iIndex = vSector.x + (vSector.y * iNumSectorX);
    
    int iLayer = floor(iIndex / iNumSectorToLayer);
    
    int2 vIndex = int2(iIndex, iLayer);
    
    return vIndex;
}

struct NeighborData
{
    int iNumNeighbor;
    int2 vSectos[4];
};

NeighborData Check_Neighbor(float2 vTexcoord, int2 vSector, float fNeighborDistance)
{
    NeighborData Neighbor = (NeighborData) 0;
    
    bool4 Dir = false; // x = LEFT, y = RIGHT, z = UP, w = BOTTOM
    
    Dir.x = (vTexcoord.x - fNeighborDistance) <= 0.f;
    Dir.y = (vTexcoord.x + fNeighborDistance) >= 1.f;
    Dir.z = (vTexcoord.y - fNeighborDistance) <= 0.f;
    Dir.w = (vTexcoord.y + fNeighborDistance) >= 1.f; 
   
    int2 TempSectors[4];
    
    for (int i = 0; i < 4; ++i)
    {
        int2 vOffset = 0;
        vOffset.x = i >= 2 ? 0 : i == 1 ? 1 : 0;
        vOffset.y = i < 2 ? 0 : i == 2 ? -iNumSectorX : iNumSectorX;
        
        int iSectorOffset = vOffset.x + vOffset.y;
        int iNeighborSector = vSector.x + iSectorOffset;
        
        if (iNeighborSector >= iNumSector || iNeighborSector < 0)
            continue;
            
        int iLayer = floor(iNeighborSector / iNumSectorToLayer);
        
        if (Dir[i])
        {
            TempSectors[Neighbor.iNumNeighbor] = int2(iNeighborSector, iLayer);
            Neighbor.iNumNeighbor += 1;
        }
    }
    
    for (int j = 0; j < Neighbor.iNumNeighbor; ++j)
    {
        Neighbor.vSectos[j] = TempSectors[j];
    }
    
    return Neighbor;
}

float Compute_NeighborShadow(int2 vNeighborSector, float4 vWorldPos, Texture2DArray<float> ShadowMapTexture)
{
    int iIndex = vNeighborSector.x;

    float4x4 matVP = mul(g_SectorViewMatrix[iIndex], g_SectorProjMatrix[iIndex]);
    float4 vProjPos = mul(vWorldPos, matVP);
    vProjPos.xyz /= vProjPos.w;
    
    float2 vTexcoord = Compute_Texcoord(vProjPos.xy);
    
    int iUVIndex = iIndex % iNumSectorToLayer;
   
    float2 vStartTex = g_vSectorUV[iUVIndex].xy;
    float2 vEndTex = g_vSectorUV[iUVIndex].zw;
    
    float2 vTexRange = vEndTex - vStartTex;
    
    vTexcoord = (vTexcoord * vTexRange) + vStartTex;
    
    vTexcoord = clamp(vTexcoord, vStartTex, vEndTex);
    
    float2 vTexelSize = 1.f / vShadowMapSize;
    
//    float fBias = 0.01f;
    
    float fDepth = vProjPos.z;//    -fBias;
    
    float fShadow = 0.f;
    
    fShadow = ShadowPCF(float3(vTexcoord, fDepth), vNeighborSector.y, 1, ShadowMapTexture, vTexelSize, vStartTex, vEndTex);
    
    return fShadow;
}

float Compute_ShadowMap(float4 vWorldPos, Texture2DArray<float> ShadowMapTexture)
{
    float fShadow = 1.f;
    
    if (any(vWorldPos.xz < vMin) || any(vWorldPos.xz > vMax))
        return fShadow;
        
    int2 vSector = Find_Sector(vWorldPos);
    
    int iIndex = vSector.x;
    
    if (iIndex >= iNumSector || iIndex < 0)
        return fShadow;
        
    float4x4 matVP = mul(g_SectorViewMatrix[iIndex], g_SectorProjMatrix[iIndex]);
    float4 vProjPos = mul(vWorldPos, matVP);
    vProjPos.xyz /= vProjPos.w;
        
    if(vProjPos.z >= 1.f || vProjPos.z < 0.f)
        return fShadow;
        
    float2 vTexcoord = Compute_Texcoord(vProjPos.xy);                   // Sector Proj
    
    NeighborData Neighbor = Check_Neighbor(vTexcoord, vSector, 0.005f);  // 근처 Sector와 거리가 가깝다면 NeighborData 채우기 ( 상 하 좌 우 )
    
    if (any(Neighbor.iNumNeighbor))
    {
        for (int i = 0; i < Neighbor.iNumNeighbor; ++i)
        {
            fShadow = min(Compute_NeighborShadow(Neighbor.vSectos[i], vWorldPos, ShadowMapTexture), fShadow);
        }
    }
    
    int iUVIndex = iIndex % iNumSectorToLayer;
   
    float2 vStartTex = g_vSectorUV[iUVIndex].xy;
    float2 vEndTex = g_vSectorUV[iUVIndex].zw;
    
    float2 vTexRange = vEndTex - vStartTex;
    
    vTexcoord = (vTexcoord * vTexRange) + vStartTex;
    
    float2 vTexelSize = 1.f / vShadowMapSize;
    
    float fDepth = vProjPos.z;
   
    fShadow = min(ShadowPCF(float3(vTexcoord, fDepth), vSector.y, 2, ShadowMapTexture, vTexelSize, vStartTex, vEndTex), fShadow);
    //ShadowMapTexture.SampleCmpLevelZero(ShadowSampler, float3(vTexcoord, vSector.y), fDepth);
    
    return fShadow;
}
/*------------------------------------ SHADOW_MAP ------------------------------------*/

float4 ComputeWorldPosToDTidJitter(uint3 DTID)
{
    float3 vJitterID = (float3)DTID + JITTER(DTID.xy, iRandCount);
    
    float fViewZ = ComputeSliceDepth(vJitterID.z + 0.5f, iSliceCount, fFogNear, fFogFar);

    float2 vUV = (vJitterID.xy + 0.5f) / vFroxelSize.xy;
        
    float fNdcZ = ComputeDepthToProjZ(fViewZ, fCamNear, fCamFar) / fViewZ;
    
    float2 vNdcXY = float2(vUV.x * 2.f - 1.f, vUV.y * -2.f + 1.f);
   
    float4 vProjPos = float4(vNdcXY, fNdcZ, 1.f);
    
    vProjPos *= fViewZ;
    
    float4 vViewPos = mul(vProjPos, InvProjMatrix);
    
    float4 vWorldPos = mul(float4(vViewPos.xyz, 1.f), InvViewMatrix);

    return vWorldPos;
}

float4 ComputeWorldPosToDTid(uint3 DTID)
{
    float fViewZ = ComputeSliceDepth((float) (DTID.z) + 0.5f, iSliceCount, fFogNear, fFogFar);

    float2 vUV = ((float2) (DTID.xy) + 0.5f) / float2(vFroxelSize.xy);
        
    float fNdcZ = ComputeDepthToProjZ(fViewZ, fCamNear, fCamFar) / fViewZ;
    
    float2 vNdcXY = float2(vUV.x * 2.f - 1.f, vUV.y * -2.f + 1.f);
   
    float4 vProjPos = float4(vNdcXY, fNdcZ, 1.f);
    
    vProjPos *= fViewZ;
    
    float4 vViewPos = mul(vProjPos, InvProjMatrix);
    
    float4 vWorldPos = mul(float4(vViewPos.xyz, 1.f), InvViewMatrix);

    return vWorldPos;
}

[numthreads(THREAD_X, THREAD_Y, LIGHT_THREAD_Z)]
void ComputeLight(uint3 GroupID : SV_GroupID, uint3 DTID : SV_DispatchThreadID, uint3 GTID : SV_GroupThreadID, uint GroupIndex : SV_GroupIndex)
{   
    if (any(DTID >= vFroxelSize))
        return;
        
    float4 vWorldPosJitter = ComputeWorldPosToDTidJitter(DTID);
   
    float4 vWorldPos = ComputeWorldPosToDTid(DTID);
    
    float3 vLighting = 0.f;             
    float3 vRayLighting = 0.f;              
    float fRayAtt = 0.f;                
    
    float3 vOutDir = normalize(vCamPos.xyz - vWorldPosJitter.xyz);
        
    for (int i = 0; i < iLightCount; ++i)
    {
        LightData Light = g_LightDatas[i];
        
        float3 LightDirection = 0.f;
        float fAtt = 1.f;
        
        switch (Light.iType)
        {
            case 0: // DIRECTIONAL
                LightDirection = normalize(Light.vDirection * -1.f);
                float fVisible = Compute_ShadowMap(vWorldPosJitter, g_ShadowMapTexture);
                fAtt = fVisible;
                
                if(fVisible != 0.f)
                {
                    fRayAtt = pow(saturate(1.f - fVisible), 2.f); // Visible Inv
                    
                    float PhaseRay = HenyeyGreensteinPhasefunction(LightDirection, vOutDir, fRayPhaseFunctionG);
                    vRayLighting = (Light.vDiffuse.xyz * fRayAtt * PhaseRay);
                }
                break;
            case 1: // POINT
                LightDirection = Light.vPosition.xyz - vWorldPosJitter.xyz;
                float fLength = length(LightDirection);
                LightDirection = normalize(LightDirection);
                fAtt = saturate((Light.fRange - fLength) / Light.fRange);
                break;
        }
        
        float PhaseFunction = HenyeyGreensteinPhasefunction(LightDirection, vOutDir, fPhaseFunctionG);
        
        float fColorWeight = fAtt * PhaseFunction;
        
        float3 vFinalColor =  (Light.vDiffuse.xyz * fColorWeight);
        
        vLighting += vFinalColor;
    }

    /*=============== DENSITY ===============*/
    
    float fDistance = length(vCamPos.xyz - vWorldPosJitter.xyz);
    
//    float fDistanceWeight = saturate(1.f - exp(fDistance * fDistanceFallOff));
    float fDistanceWeight = saturate(exp(-fDistanceFallOff * (fFogMaxDistance - fDistance)));
    float fHeightWeight = vWorldPosJitter.y < fFogMaxHeight ?  1.f : saturate(exp(-fHegihtFallOff * (vWorldPosJitter.y - fFogMaxHeight)));;
    
    float fLightWeight = saturate(fDistanceWeight * fHeightWeight);// * fLightAtt);
    float fRayWeight = saturate(1.f - fLightWeight) * fRayAtt; // LightWieght 가 충분하다면 굳이 추가 X, LightWieght ( Fog가 보이지 않는곳 -> Ray는 살리기 )
    
    float fNoise = 1.f;
    
    if (fHeightWeight < 1.f)
    {
        float3 vNoiseUV = (vWorldPos.xyz) * fNoiseScale;
        vNoiseUV.x += fNoiseTimeDelta;
    
        fNoise = lerp(max(g_NoiseTexture.SampleLevel(DefaultSampler, vNoiseUV, 0), 0.1f), 1.f, fHeightWeight);
    }
    
    float fLightDensity = fDensity * fLightWeight * fNoise;
    float fCurRayDensity = fRayDensity * fRayWeight * fRayDensityScale;
    
    float fFinalDensity = fLightDensity + fCurRayDensity;
    
   // Temporal Reprojection
    float3 vFinalColor = (vLighting * fLightIntensity * fLightDensity) + (vRayLighting * fRayIntensity * fCurRayDensity);
   
    float4 vCurScatterning = float4(vFinalColor, fFinalDensity); // (vRayLighting * fRayIntensity * fCurRayDensity)
    
    float4 vFinalScatterning = 0.f;
   
    if (IsTemporal)
    {
        float4 vPrevViewPos = mul(vWorldPos, PrevViewMatrix);
        
        float4 vPrevProjPos = mul(vPrevViewPos, PrevProjMatrix);
        
        float fPrevViewZ = vPrevViewPos.z;
        
        vPrevProjPos.xyz /= vPrevProjPos.w;

        float3 vTexcoord = 0.f;
            
        float fNdcZ = saturate(log(fPrevViewZ / fFogNear) / log(fFogFar / fFogNear)); //ComputeDepthToProjZ(fPrevViewZ, fFogNear, fFogFar) / fPrevViewZ;
     
        vTexcoord.x = vPrevProjPos.x * 0.5f + 0.5f;
        vTexcoord.y = vPrevProjPos.y * -0.5f + 0.5f;
        vTexcoord.z = fNdcZ;
    
        if (all(vTexcoord.xy <= 0.995f) && all(vTexcoord.xy >= 0.005f) && fPrevViewZ > fFogNear && fPrevViewZ < fFogFar)
        {
            float4 vPrevScattering = PrevVFLightTexture.SampleLevel(DefaultSampler, vTexcoord, 0.f);
        
            vFinalScatterning = lerp(vPrevScattering, vCurScatterning, 0.15f);
        }
        else
            vFinalScatterning = vCurScatterning;
    }
    else
    {
       vFinalScatterning = vCurScatterning;
    }
   
    
    //vFinalScatterning *= fNoise;

    OutputTexture[DTID.xyz] = vFinalScatterning;
}

float4 ScatterStep(float3 AccumLight, float AccumTransmittance, float3 SliceLight, float SliceDensity, float Tickness)
{
    float Density = max(SliceDensity, 1e-5);
    
    Density *= fDensityScale;
    
    float SliceTransmittance = exp(-Density * Tickness);
    
    float3 SliceLightIntegral = SliceLight * (1.f - SliceTransmittance)/ Density;
    
    SliceLightIntegral *= fScatterWeight;
    
    float3 ResultLight = AccumLight + (SliceLightIntegral * AccumTransmittance);
    float ResultTransmittance = AccumTransmittance * SliceTransmittance;
    
    return float4(ResultLight, ResultTransmittance);
}

[numthreads(THREAD_X, THREAD_Y, BEER_THREAD_Z)]
void VolumetricFog(uint3 GroupID : SV_GroupID, uint3 DTID : SV_DispatchThreadID, uint3 GTID : SV_GroupThreadID, uint GroupIndex : SV_GroupIndex)
{
    if (any(DTID >= vFroxelSize))
        return;
        
    float4 Accum = float4(0.f, 0.f, 0.f, 1.f);
    uint3 vIndex = uint3(DTID.xy, 0);
    
    for (uint iSlice = 0; iSlice < vFroxelSize.z; ++iSlice)
    {
        vIndex.z = iSlice;
        
        float4 vLighting = VFLightTexture.Load(int4(vIndex, 0));
        
        uint iNextSlice = clamp(iSlice + 1, 0, vFroxelSize.z);
        
        float fTickness = ComputeSliceDepth(iNextSlice, iSliceCount, fFogNear, fFogFar) - ComputeSliceDepth(iSlice, iSliceCount, fFogNear, fFogFar);
        
        Accum = ScatterStep(Accum.xyz, Accum.a, vLighting.xyz, vLighting.a, fTickness);
       
        OutputTexture[vIndex] = Accum;
    }
}