#include "Engine_ComputeShader_Function.hlsli"

#pragma pack_matrix(row_major)

#define THREAD_X 16
#define THREAD_Y 16
#define THREAD_Z 1

#define EPSILON 1e-05

#define MAX_RADIUS 32

Texture2D<float4> InputTexture : register(t0);
RWTexture2D<float4> OutputTexture : register(u0);

groupshared int2 vInSize;
groupshared int2 vOutSize;

cbuffer RADIAL_DATA : register(b0)
{
    float fMinDistance;
    float fMaxDistance;
    float fLengthScale;
    float fPadding;
    float2 vPivot;
    float2 fPadding1;
}

float Hash21(float2 ID)
{
    float2 vInput = ID;
    vInput = frac(vInput * float2(123.32, 456.21));
    vInput += dot(vInput, vInput + 45.32);
    return frac(vInput.x * vInput.y);
}

SamplerState ClampSampler : register(s0);

float4 ComputeRadialBlur(uint3 DTID, int2 vInSize)
{
    float2 vTexcoord = (float2) DTID.xy / (float2) vInSize;
        
    float2 vDir = vTexcoord - vPivot;
    float fLength = length(vDir);
    
    float fScale = smoothstep(fMinDistance, fMaxDistance, fLength);
    
    float2 vRadialScale = vDir * fLengthScale * fScale;
    
    float fMax = 1.f;
    
    if (vRadialScale.x > 0.f)
    {
        float fX = (1.f - vTexcoord.x) / (vRadialScale.x);
        fMax = min(fMax, fX);
    }
    else if (vRadialScale.x < 0.f)
    {
        float fX = (vTexcoord.x) / (vRadialScale.x * -1.f);
        fMax = min(fMax, fX);
    }
    
    if (vRadialScale.y > 0.f)
    {
        float fY = (1.f - vTexcoord.y) / (vRadialScale.y);
        fMax = min(fMax, fY);
    }
    else if (vRadialScale.y < 0.f)
    {
        float fY = (vTexcoord.y) / (vRadialScale.y * -1.f);
        fMax = min(fMax, fY);
    }
    
    vRadialScale *= fMax;
    
    float4 vColor = 0.f;
    float4 vFinalColor = 0.f;
    float fTotalWeight = 0.f;
    
    int iSampleCount = clamp(MAX_RADIUS * fScale, 4, MAX_RADIUS);
    
    float Jitter = lerp(0.5f, 1.f, Hash21((float) DTID.xy));
    
    for (int i = 0; i < iSampleCount; ++i)
    {   
        float fRatio = ((float) i + Jitter) / (float) iSampleCount;

        float2 vOffset = vRadialScale * fRatio;
        
        float2 vOffsetTex = vTexcoord + vOffset;

        float4 vSampleColor = InputTexture.SampleLevel(ClampSampler, vOffsetTex, 0);

        float fWeight = exp2(-((float) i / (float) iSampleCount) * 3.f);
        
        vColor += vSampleColor * fWeight;
        fTotalWeight += fWeight;
    }
    
    if(fTotalWeight > 0.f)
        vFinalColor = vColor / fTotalWeight;
    else
        vFinalColor = InputTexture.Load(int3(DTID.xy, 0));

    return vFinalColor;
}

groupshared float4 vSharedRadialColor[THREAD_Y + 1][THREAD_X + 1];

[numthreads(THREAD_X, THREAD_Y, THREAD_Z)]
void RadialBlur(uint3 GroupID : SV_GroupID, uint3 DTID : SV_DispatchThreadID, uint3 GTID : SV_GroupThreadID, uint GroupIndex : SV_GroupIndex)
{
    if (all(GTID.xy == 0))
    {
        InputTexture.GetDimensions(vInSize.x, vInSize.y);
        OutputTexture.GetDimensions(vOutSize.x, vOutSize.y);
    }
    
    GroupMemoryBarrierWithGroupSync();
    
    vSharedRadialColor[GTID.y][GTID.x] = ComputeRadialBlur(DTID, vInSize);
    
    if (GTID.y == THREAD_Y - 1)
    {
        int3 OffsetID = int3(DTID.x, DTID.y + 1, 0);
        if (OffsetID.y >= (int) vInSize.y)
            OffsetID.y = (int) vInSize.y - 1;
        
        vSharedRadialColor[GTID.y + 1][GTID.x] = ComputeRadialBlur(OffsetID, vInSize);
    }
    
    if (GTID.x >= THREAD_X - 1)
    {
        int3 OffsetID = int3(DTID.x + 1, DTID.y, 0);
        if (OffsetID.x >= (int) vInSize.x)
            OffsetID.x = (int) vInSize.x - 1;
        
        vSharedRadialColor[GTID.y][GTID.x + 1] = ComputeRadialBlur(OffsetID, vInSize);
    }
    
    if (GTID.x >= THREAD_X - 1 && GTID.y >= THREAD_Y - 1)
    {
        int3 OffsetID = int3(DTID.x + 1, DTID.y + 1, 0);
    
        if (OffsetID.x >= (int) vInSize.x)
            OffsetID.x = (int) vInSize.x - 1;
        
        if (OffsetID.y >= (int) vInSize.y)
            OffsetID.y = (int) vInSize.y - 1;
        
        vSharedRadialColor[GTID.y + 1][GTID.x + 1] = ComputeRadialBlur(OffsetID, vInSize);
    }
    
    GroupMemoryBarrierWithGroupSync();
    
    for (int i = 0; i < 4; i++)
    {
        int2 OutIndex = DTID.xy * 2;
        
        int2 Offset = int2(i % 2, clamp(i - 1, 0, 1));
        
        OutIndex += Offset;
        
        float2 vLowPos = (OutIndex + 0.5f) * ((float2) vInSize / (float2) vOutSize) - 0.5f;
    
        int2 iLowID = (int2) floor(vLowPos);
        float2 fFrac = vLowPos - (float2) iLowID;
        
        float4 vColor = 0.f;
       
        int2 vTexcoord = iLowID - (GroupID.xy * int2(THREAD_X, THREAD_Y));
        
        int iSampleX0 = clamp(vTexcoord.x, 0, THREAD_X + 1);
        int iSampleX1 = clamp(vTexcoord.x + 1, 0, THREAD_X + 1);
    
        int iSampleY0 = clamp(vTexcoord.y, 0, THREAD_Y + 1);
        int iSampleY1 = clamp(vTexcoord.y + 1, 0, THREAD_Y + 1);
       
        float4 vLT = vSharedRadialColor[iSampleY0][iSampleX0];
        float4 vRT = vSharedRadialColor[iSampleY0][iSampleX1];
        float4 vLB = vSharedRadialColor[iSampleY1][iSampleX0];
        float4 vRB = vSharedRadialColor[iSampleY1][iSampleX1];
    
        vColor = lerp(lerp(vLT, vRT, fFrac.x), lerp(vLB, vRB, fFrac.x), fFrac.y);
        
        OutputTexture[OutIndex] = vColor;
    }
    
}
