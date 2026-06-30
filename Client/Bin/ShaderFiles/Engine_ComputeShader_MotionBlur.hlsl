#include "Engine_ComputeShader_Function.hlsli"

#pragma pack_matrix(row_major)

#define THREAD_X 16
#define THREAD_Y 16
#define THREAD_Z 1

#define MAX_RADIUS 8

Texture2D<float4> InputTexture : register(t0);
RWTexture2D<float4> OutputTexture : register(u0);

groupshared int2 vInSize;
groupshared int2 vOutSize;

/* ------------------------------------MOTION-BLUR------------------------------------ */

Texture2D<float4> DepthTexture : register(t1);
Texture2D<float4> VelocityMap : register(t2);

SamplerState ClampSampler : register(s0);

cbuffer MOTION_DATA : register(b1)
{
    float fLimitVelocity;
    float fLimitDepth;
    float fLengthScale;
    float fSampleDepthBias;
}

float4 Compute_Velocity(int2 vIndex, int2 vVelocitySize)
{
    float4 vVelocity = 0.f;
    
    //int iSampleX0 = min(vIndex.x, vVelocitySize.x - 1);
    //int iSampleX1 = min(vIndex.x + 1, vVelocitySize.x - 1);
    
    //int iSampleY0 = min(vIndex.y, vVelocitySize.y - 1);
    //int iSampleY1 = min(vIndex.y + 1, vVelocitySize.y - 1);
    
    //float4 Vector[4];
    
    //Vector[0] = VelocityMap.Load(int3(iSampleX0, iSampleY0, 0));
    //Vector[1] = VelocityMap.Load(int3(iSampleX1, iSampleY0, 0));
    //Vector[2] = VelocityMap.Load(int3(iSampleX0, iSampleY1, 0));
    //Vector[3] = VelocityMap.Load(int3(iSampleX1, iSampleY1, 0));
   
    //float fMaxLength = 0.f;

    //for (int i = 0; i < 4; ++i)
    //{
    //    float fLength = length(Vector[i].xy);

    //    if(fLength == 0.f)
    //        return 0.f;
            
    //    if(fLength > fMaxLength)
    //    {
    //        fMaxLength = fLength;
    //        vVelocity = Vector[i];
    //    }
    //}
   
    vVelocity = VelocityMap.Load(int3(vIndex, 0));
   
    return vVelocity;
}

float4 ComputeMotionBlur(uint3 DTID, int2 vInSize, int2 vOutSize)
{
    //int2 iIndex = DTID.xy * 2;
    int2 iIndex = DTID.xy;
    
    float4 vVelocity = Compute_Velocity(iIndex, vOutSize);
    
    float2 vDir = normalize(vVelocity.xy);
    
    float fVelocityLength = length(vVelocity.xy);

    float2 vTexcoord = (float2) DTID.xy / (float2) vInSize;
    
    if (fVelocityLength == 0.f || vVelocity.w != 0.f)
        return InputTexture.SampleLevel(ClampSampler, vTexcoord, 0);
    
    float2 vTexelSize = 1.f / (float2) vInSize;
    
    float2 vMotionScale = ((vVelocity.xy) * vTexelSize) * fLengthScale;
    
    int iSampleCount = MAX_RADIUS;
    
    float fMax = 1.f;
    
    if (vMotionScale.x > 0.f)
    {
        float fX = (1.f - vTexcoord.x) / (vMotionScale.x);
        fMax = min(fMax, fX);
    }
    else if (vMotionScale.x < 0.f)
    {
        float fX = (vTexcoord.x) / (vMotionScale.x * -1.f);
        fMax = min(fMax, fX);
    }

    if (vMotionScale.y > 0.f)
    {
        float fY = (1.f - vTexcoord.y) / (vMotionScale.y);
        fMax = min(fMax, fY);
    }
    else if (vMotionScale.y < 0.f)
    {
        float fY = (vTexcoord.y) / (vMotionScale.y * -1.f);
        fMax = min(fMax, fY);
    }
    
    vMotionScale *= fMax;
    
    float4 vColor = 0.f;
    float fTotalWeight = 0.f;
    
    for (int i = 0; i < iSampleCount; ++i)
    {
        float fRatio = ((int) i + 0.5f) / (float) iSampleCount;

        float2 vOffsetTex = vTexcoord + (vMotionScale * fRatio);

        float4 vSampleDepth = DepthTexture.SampleLevel(ClampSampler, vOffsetTex, 0);

        if ((vSampleDepth.y + fSampleDepthBias) < vVelocity.z || vSampleDepth.z != 0.f)
            continue;
       
        float4 vSampleColor = InputTexture.SampleLevel(ClampSampler, vOffsetTex, 0);
        
        float fWeight = exp2(fRatio * 3.f); //-(float) i / (float) iSampleCount * 3.f);
        
        vColor += vSampleColor * fWeight;
        fTotalWeight += fWeight;
    }
    
    float4 vFinalColor = 0.f;
    
    if(fTotalWeight > 0.f)
        vFinalColor = vColor / fTotalWeight; 
    else
        vFinalColor = InputTexture.SampleLevel(ClampSampler, vTexcoord, 0);
   
    vFinalColor.a = 1.f;
    
    return vFinalColor;
}

//groupshared float4 vSharedMotionColor[THREAD_Y + 1][THREAD_X + 1];

[numthreads(THREAD_X, THREAD_Y, THREAD_Z)]
void Motion_Blur(uint3 GroupID : SV_GroupID, uint3 DTID : SV_DispatchThreadID, uint3 GTID : SV_GroupThreadID, uint GroupIndex : SV_GroupIndex)
{
    if (GTID.x == 0 && GTID.y == 0)
    {
        InputTexture.GetDimensions(vInSize.x, vInSize.y);
        OutputTexture.GetDimensions(vOutSize.x, vOutSize.y);
    }
    
    GroupMemoryBarrierWithGroupSync();
    
    
    OutputTexture[DTID.xy] = ComputeMotionBlur(DTID, vInSize, vOutSize);
    
    //vSharedMotionColor[GTID.y][GTID.x] = ComputeMotionBlur(DTID, vInSize, vOutSize);

    //if (GTID.y >= THREAD_Y - 1)
    //{
    //    int3 OffsetID = int3(DTID.x, DTID.y + 1, 0);
    //    if (OffsetID.y >= (int) vInSize.y)
    //        OffsetID.y = (int) vInSize.y - 1;
            
    //    vSharedMotionColor[GTID.y + 1][GTID.x] = ComputeMotionBlur(OffsetID, vInSize, vOutSize);
    //}
    
    //if (GTID.x >= THREAD_X - 1)
    //{
    //    int3 OffsetID = int3(DTID.x + 1, DTID.y, 0);
    //    if (OffsetID.x >= (int) vInSize.x)
    //        OffsetID.x = (int) vInSize.x - 1;
            
    //    vSharedMotionColor[GTID.y][GTID.x + 1] = ComputeMotionBlur(OffsetID, vInSize, vOutSize);
    //}
    
    //if (GTID.x >= THREAD_X - 1 && GTID.y >= THREAD_Y -1 )
    //{
    //    int3 OffsetID = int3(DTID.x + 1, DTID.y + 1, 0);
        
    //    if (OffsetID.x >= (int) vInSize.x)
    //        OffsetID.x = (int) vInSize.x - 1;
            
    //    if (OffsetID.y >= (int) vInSize.y)
    //        OffsetID.y = (int) vInSize.y - 1;
            
    //    vSharedMotionColor[GTID.y + 1][GTID.x + 1] = ComputeMotionBlur(OffsetID, vInSize, vOutSize);
    //}
    
    //GroupMemoryBarrierWithGroupSync();
    
    //for (int i = 0; i < 4; i++)
    //{   
    //    int2 OutIndex = DTID.xy * 2;
 
    //    int2 Offset = int2(i % 2, clamp(i - 1, 0, 1));
        
    //    OutIndex += Offset;
       
    //    float2 vLowPos = (OutIndex + 0.5f) * ((float2) vInSize / (float2) vOutSize) - 0.5f;
        
    //    int2 iLowID = (int2) floor(vLowPos);
    //    float2 fFrac = vLowPos - (float2) iLowID;
        
    //    float4 vColor = 0.f;
        
    //    int2 vTexcoord = iLowID - (GroupID.xy * int2(THREAD_X, THREAD_Y));
        
    //    int iSampleX0 = clamp(vTexcoord.x, 0, THREAD_X + 1);
    //    int iSampleX1 = clamp(vTexcoord.x + 1, 0, THREAD_X + 1);
    
    //    int iSampleY0 = clamp(vTexcoord.y, 0, THREAD_Y + 1);
    //    int iSampleY1 = clamp(vTexcoord.y + 1, 0, THREAD_Y + 1);
       
    //    float4 vLT = vSharedMotionColor[iSampleY0][iSampleX0];
    //    float4 vRT = vSharedMotionColor[iSampleY0][iSampleX1];
    //    float4 vLB = vSharedMotionColor[iSampleY1][iSampleX0];
    //    float4 vRB = vSharedMotionColor[iSampleY1][iSampleX1];
    
    //    vColor = lerp(lerp(vLT, vRT, fFrac.x), lerp(vLB, vRB, fFrac.x), fFrac.y);
        
    //    OutputTexture[OutIndex] = vColor;
    //}
}