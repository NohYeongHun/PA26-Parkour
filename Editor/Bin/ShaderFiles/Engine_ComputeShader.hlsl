#include "Engine_ComputeShader_Function.hlsli"

#pragma pack_matrix(row_major)

#define THREAD_X 8
#define THREAD_Y 8
#define THREAD_Z 1

Texture2D<float4> InputTexture : register(t0);
RWTexture2D<float4> OutputTexture : register(u0);

Texture2D<float4> g_NormalTexture : register(t1);
Texture2D<float4> g_DepthTexture : register(t2);
Texture2D<float4> g_NoiseTexture : register(t3);

SamplerState DefaultSampler : register(s1);
SamplerState PointClampSampler : register(s2);
SamplerState NoiseSampler : register(s3);

cbuffer SSAO_DATA : register(b0)
{
    vector vSampleVector[16];
    float4x4 CamViewMatrix;
    float4x4 CamProjMatrix;
    float4x4 ProjMatrixInv;
    float fWidth;
    float fHeight;
    int iSampleSize;
    float fSSAO_Radius;
    float fSSAO_MaxDistance;
    float3 Padding;
}

[numthreads(THREAD_X, THREAD_Y, THREAD_Z)]
void SSAO(uint3 GruopID : SV_GroupID, uint3 DTID : SV_DispatchThreadID, uint3 GTID : SV_GroupThreadID, uint GruopIndex : SV_GroupIndex)
{
    float2 vTexcoord = Compute_Texcoord(DTID, fWidth, fHeight);
    
    float4 vViewPos = Compute_ViewPos(vTexcoord, g_DepthTexture, DTID, ProjMatrixInv);

    float fViewPosZ = vViewPos.z;
    
    if (fViewPosZ == 0.f || fViewPosZ >= 2000.f)
    {
        OutputTexture[DTID.xy] = 1.f;
        return;
    }
    
    vector vNormal = Compute_Normal(g_NormalTexture, DefaultSampler, vTexcoord);
    vNormal = normalize(mul(vNormal, CamViewMatrix));
    
    float fNoiseTexelSize = 1.f / 512.f;
    
    float2 vNoiseTexcoord = 0.f;
    vNoiseTexcoord.x = (float) DTID.x * fNoiseTexelSize * 16.f;
    vNoiseTexcoord.y = (float) DTID.y * fNoiseTexelSize * 16.f;

    //float4 vNoiseNormal = g_NoiseTexture.SampleLevel(NoiseSampler, vNoiseTexcoord, 0);
    //float2 vNoiseXY = vNoiseNormal.xy * 2.f - 1.f;
    //vNoiseNormal = normalize(float4(vNoiseXY, 0.f, 0.f));
    vector vNoiseNormal = Compute_Normal(g_NoiseTexture, NoiseSampler, vNoiseTexcoord);
    vNoiseNormal = normalize(mul(vNoiseNormal, CamViewMatrix));
    
    float TotalOcclusion = 0.f;
    
    for (int i = 0; i < iSampleSize; ++i)
    {
        float Occlusion = 0.f;
    
        float3 vTangent = normalize(vNoiseNormal.xyz - (vNormal.xyz * dot(vNoiseNormal, vNormal)));
        float3 vBinormal = normalize(cross(vNormal.xyz, vTangent));
        float3x3 TBN = float3x3(vTangent.xyz, vBinormal.xyz, vNormal.xyz);
        
        float3 vRandomVector = mul(vSampleVector[i].xyz, TBN);
        
        float4 vSampeDir = float4(vRandomVector, 0.f); // 샘플 벡터를 vNormal 기준 반구 형태로 변형
       
        float4 vSamplePos = vViewPos + (vSampeDir * fSSAO_Radius);      // Radius 만큼 이동
        vSamplePos.w = 1.f;                                                         // 이상한 값이 드가는거 같음,,
        float fRandomZ = vSamplePos.z;
        
        float4 vProjPos = mul(vSamplePos, CamProjMatrix);
    
        float2 vProjPosXY = vProjPos.xy / vProjPos.w;
        
        float2 vSampleTexcoord = Compute_Texcoord_Proj(vProjPosXY);
            
        float SampleDepth = g_DepthTexture.SampleLevel(PointClampSampler, vSampleTexcoord, 0).y;
        
        if (SampleDepth == 0.f || SampleDepth >= fRandomZ) // 안그려져있거나, 랜덤 위치보다 뒤에 있다면
        {
            Occlusion = 1.f;
        }
        else
        {
            float fDistance = abs(SampleDepth - fViewPosZ);
            
            float4 vSampleViewPos = Compute_ViewPosTexcoord(vSampleTexcoord, g_DepthTexture, DefaultSampler, ProjMatrixInv);

            float fNormalWeight = saturate(dot(vNormal.xyz, normalize(vSampleViewPos.xyz - vViewPos.xyz))); // 현재 노말과 Sample 위치까지의 방향 벡터
            
            float fDistWeight = smoothstep(fSSAO_MaxDistance, 0.f, fDistance); // 깊이 차이가 클수록 강하게, 낮을수록 약하게    

            //if(fNormalWeight <= 0.1f)                                    // 만약 노말방향 뒤에 있다면
            //    Occlusion = 1.f;
            //else
            {
                Occlusion = (fDistWeight * (1.f - fNormalWeight));
            }
        }
        
        TotalOcclusion += Occlusion;
    }
    
    float AO = (TotalOcclusion / (float) iSampleSize);
//    AO = pow(AO, 2.f);
    
    OutputTexture[DTID.xy] = float4(AO, AO, AO, 1.f);
}


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
    
groupshared float4 vSharedColorY[THREAD_Y + (2 * SSAO_BLUR_RADIUS)][THREAD_X];
groupshared float vSharedDepthY[THREAD_Y + (2 * SSAO_BLUR_RADIUS)][THREAD_X];
groupshared float4 vSharedNormalY[THREAD_Y + (2 * SSAO_BLUR_RADIUS)][THREAD_X];

[numthreads(THREAD_X, THREAD_Y, THREAD_Z)]
void SSAO_BLUR(uint3 GruopID : SV_GroupID, uint3 DTID : SV_DispatchThreadID, uint3 GTID : SV_GroupThreadID, uint GruopIndex : SV_GroupIndex)
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
        vSharedDepthX[GTID.y][GTID.x + THREAD_X + SSAO_BLUR_RADIUS] = g_DepthTexture.Load(RightID).y;
        vSharedNormalX[GTID.y][GTID.x + THREAD_X + SSAO_BLUR_RADIUS] = Compute_Normal_DTID(g_NormalTexture, RightID);
    }
    
    GroupMemoryBarrierWithGroupSync();
    
    float4 vColorX = 0.f;
    
    float4 vOriginColorX = vSharedColorX[GTID.y][GTID.x + SSAO_BLUR_RADIUS];
    float fOriginDepthX = vSharedDepthX[GTID.y][GTID.x + SSAO_BLUR_RADIUS];
    float4 vOriginNormalX = vSharedNormalX[GTID.y][GTID.x + SSAO_BLUR_RADIUS];
    
    for (int i = -SSAO_BLUR_RADIUS; i <= SSAO_BLUR_RADIUS; ++i)
    {
        int iIndexX = GTID.x + SSAO_BLUR_RADIUS + i;
        
        float4 vSampleColor = vSharedColorX[GTID.y][iIndexX];
        float fSampleDepth = vSharedDepthX[GTID.y][iIndexX];
        float4 vSampleNormal = vSharedNormalX[GTID.y][iIndexX];
        
        vColorX += Compute_SSAO_Blur(vOriginColorX, fOriginDepthX, vOriginNormalX, vSampleColor, fSampleDepth, vSampleNormal, fSSAO_MinDepthDistance);
    }
    
    vColorX /= (fBlurRadius * 2.f + 1.f);
    
    vSharedColorY[GTID.y + SSAO_BLUR_RADIUS][GTID.x] = vColorX;
    vSharedDepthY[GTID.y + SSAO_BLUR_RADIUS][GTID.x] = fOriginDepthX;
    vSharedNormalY[GTID.y + SSAO_BLUR_RADIUS][GTID.x] = vOriginNormalX;
    
    if (GTID.y < SSAO_BLUR_RADIUS)
    {
        int3 LeftID = int3(DTID.x, DTID.y - SSAO_BLUR_RADIUS, 0);
        int3 RightID = int3(DTID.x, DTID.y + THREAD_Y, 0);
        
        if (LeftID.y < 0)
            LeftID.y = 0;
        
        if (RightID.y >= fHeight_Blur)
            RightID.y = fHeight_Blur -1.f;
        
        
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
    
    for (int j = -SSAO_BLUR_RADIUS; j <= SSAO_BLUR_RADIUS; ++j)
    {
        int iIndexY = GTID.y + SSAO_BLUR_RADIUS + j;
        
        float4 vSampleColor = vSharedColorY[iIndexY][GTID.x];
        float fSampleDepth = vSharedDepthY[iIndexY][GTID.x];
        float4 vSampleNormal = vSharedNormalY[iIndexY][GTID.x];
        
        vColorY += Compute_SSAO_Blur(vOriginColorY, fOriginDepthY, vOriginNormalY, vSampleColor, fSampleDepth, vSampleNormal, fSSAO_MinDepthDistance);
    }
    
    vColorY /= (fBlurRadius * 2.f + 1.f);
    
    OutputTexture[DTID.xy] = vColorY;
}