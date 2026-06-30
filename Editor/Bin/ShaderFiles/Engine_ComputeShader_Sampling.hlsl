#include "Engine_ComputeShader_Function.hlsli"

#define THREAD_X 16
#define THREAD_Y 16
#define THREAD_Z 1

Texture2D<float4> InputTexture : register(t0);
RWTexture2D<float4> OutputTexture : register(u0);

groupshared int2 vInSize;
groupshared int2 vOutSize;

[numthreads(THREAD_X, THREAD_Y, THREAD_Z)]
void DownSample(uint3 GroupID : SV_GroupID, uint3 DTID : SV_DispatchThreadID, uint3 GTID : SV_GroupThreadID, uint GroupIndex : SV_GroupIndex)
{
    if (GTID.x == 0 && GTID.y == 0)
        InputTexture.GetDimensions(vInSize.x, vInSize.y);
    
    GroupMemoryBarrierWithGroupSync();
    
    int iIndexX = DTID.x * 2;
    int iIndexY = DTID.y * 2;
    
    float4 vColor = 0.f;
    
    int iSampleX0 = min(iIndexX, vInSize.x - 1);
    int iSampleX1 = min(iIndexX + 1, vInSize.x - 1);
    
    int iSampleY0 = min(iIndexY, vInSize.y - 1);
    int iSampleY1 = min(iIndexY + 1, vInSize.y - 1);
    
    vColor += InputTexture.Load(int3(iSampleX0, iSampleY0, 0));
    vColor += InputTexture.Load(int3(iSampleX1, iSampleY0, 0));
    vColor += InputTexture.Load(int3(iSampleX0, iSampleY1, 0));
    vColor += InputTexture.Load(int3(iSampleX1, iSampleY1, 0));
    
    vColor *= 0.25f;
   
    OutputTexture[DTID.xy] = vColor;
}

[numthreads(THREAD_X, THREAD_Y, THREAD_Z)]
void UpSample(uint3 GroupID : SV_GroupID, uint3 DTID : SV_DispatchThreadID, uint3 GTID : SV_GroupThreadID, uint GroupIndex : SV_GroupIndex)
{
    if (GTID.x == 0 && GTID.y == 0)
    {
        InputTexture.GetDimensions(vInSize.x, vInSize.y);
        OutputTexture.GetDimensions(vOutSize.x, vOutSize.y);
    }
    
    GroupMemoryBarrierWithGroupSync();
    //int2 vInSize;
    //InputTexture.GetDimensions(vInSize.x, vInSize.y);
    
    //int2 vOutSize;
    //OutputTexture.GetDimensions(vOutSize.x, vOutSize.y);
    
    float2 vTexcoord = float2(DTID.xy);
    
    float2 vLowPos = (vTexcoord + 0.5f) * ((float2) vInSize / (float2) vOutSize) - 0.5f;
    
    int2 iLowID = (int2) floor(vLowPos);
    float2 fFrac = vLowPos - (float2) iLowID;
    float4 vColor = 0.f;
       
    int iSampleX0 = min(iLowID.x, vInSize.x - 1);
    int iSampleX1 = min(iLowID.x + 1, vInSize.x - 1);
    
    int iSampleY0 = min(iLowID.y, vInSize.y - 1);
    int iSampleY1 = min(iLowID.y + 1, vInSize.y - 1);
       
    float4 vLT = InputTexture.Load(int3(iSampleX0, iSampleY0, 0));
    float4 vRT = InputTexture.Load(int3(iSampleX1, iSampleY0, 0));
    float4 vLB = InputTexture.Load(int3(iSampleX0, iSampleY1, 0));
    float4 vRB = InputTexture.Load(int3(iSampleX1, iSampleY1, 0));
    
    vColor = lerp(lerp(vLT, vRT, fFrac.x), lerp(vLB, vRB, fFrac.x), fFrac.y);
    
    OutputTexture[DTID.xy] = vColor;
}

Texture2D<float4> BaseTexture : register(t1);

cbuffer BLOOM_DATA : register(b1)
{
    float2 vOutSizeBloom;
    float fBloomIntensity;
    float Padding;
}

[numthreads(THREAD_X, THREAD_Y, THREAD_Z)]
void UpSample_Bloom(uint3 GroupID : SV_GroupID, uint3 DTID : SV_DispatchThreadID, uint3 GTID : SV_GroupThreadID, uint GroupIndex : SV_GroupIndex)
{
    
    float2 fTexcoord = float2(DTID.xy) / vOutSizeBloom;
    float2 fFrac = fmod(fTexcoord * 0.5f * vOutSizeBloom, 1.f);
    
    int2 iLowID = int2(fTexcoord * 0.5f * vOutSizeBloom);
    
    float4 vColor = 0.f;
       
    float4 vLT = InputTexture.Load(int3(iLowID.x, iLowID.y, 0));
    float4 vRT = InputTexture.Load(int3(iLowID.x + 1, iLowID.y, 0));
    float4 vLB = InputTexture.Load(int3(iLowID.x, iLowID.y + 1, 0));
    float4 vRB = InputTexture.Load(int3(iLowID.x + 1, iLowID.y + 1, 0));
    
    vColor = lerp(lerp(vLT, vRT, fFrac.x), lerp(vLB, vRB, fFrac.x), fFrac.y);
    
    vColor *= (0.5f + fBloomIntensity);
    
    float4 vBaseColor = BaseTexture.Load(int3(DTID.xy, 0));
    
//    float4 vFinalColor = vBaseColor + (vColor);
    float4 vFinalColor = 1.f - exp(-(vBaseColor + vColor * 2.f));
    vFinalColor.a = vColor.a;
    
    OutputTexture[DTID.xy] = vFinalColor;
}

