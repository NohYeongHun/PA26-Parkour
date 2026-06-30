#include "Engine_ComputeShader_Function.hlsli"

#define THREAD_X 8
#define THREAD_Y 8
#define THREAD_Z 8

float3 Mod289(float3 vInput)
{
    return vInput - floor(vInput * (1.f / 289.f)) * 289.f;
}

float4 Mod289(float4 vInput)
{
    return vInput - floor(vInput * (1.f / 289.f)) * 289.f;
}

float4 Permute(float4 vInput)
{
    return Mod289(((vInput * 34.f) + 10.f) * vInput);
}
                        
float4 TaylorInvSqrt(float4 vInput)
{
    return 1.79284291400159f - 0.85373472095314f * vInput;
}
                      
float SNoise(float3 vInput)
{
    const float2 C = float2(1.f / 6.f, 1.f / 3.f);
    const float4 D = float4(0.f, 0.5f, 1.f, 2.f);
    
    float3 I = floor(vInput + dot(vInput, C.yyy));
    float3 X0 = vInput - I + dot(I, C.xxx);
    
    float3 G = step(X0.yzx, X0.xyz);
    float3 L = 1.f - G;
    float3 I1 = min(G.xyz, L.zxy);
    float3 I2 = max(G.xyz, L.zxy);
    
    float3 X1 = X0 - I1 + C.xxx;
    float3 X2 = X0 - I2 + C.yyy;
    float3 X3 = X0 - D.yyy;
    
    I = Mod289(I);
    
    float4 P = Permute(Permute(Permute(I.z + float4(0.f, I1.z, I2.z, 1.f)) + I.y + float4(0.f, I1.y, I2.y, 1.f)) + I.x + float4(0.f, I1.x, I2.x, 1.f));

    float N_ = 0.142857142857;
    float3 NS = N_ * D.wyz - D.xzx;
    
    float4 J = P - 49.f * floor(P * NS.z * NS.z);
    
    float4 X_ = floor(J * NS.z);
    float4 Y_ = floor(J - 7.f * X_);
    
    float4 X = X_ * NS.x + NS.yyyy;
    float4 Y = Y_ * NS.x + NS.yyyy;
    float4 H = 1.f - abs(X) - abs(Y);
    
    float4 B0 = float4(X.xy, Y.xy);
    float4 B1 = float4(X.zw, Y.zw);
    
    float4 S0 = floor(B0) * 2.f + 1.f;
    float4 S1 = floor(B1) * 2.f + 1.f;
    float4 SH = -step(H, 0.f);
    
    float4 A0 = B0.xzyw + S0.xzyw * SH.xxyy;
    float4 A1 = B1.xzyw + S1.xzyw * SH.zzww;
    
    float3 P0 = float3(A0.xy, H.x);
    float3 P1 = float3(A0.zw, H.y);
    float3 P2 = float3(A1.xy, H.z);
    float3 P3 = float3(A1.zw, H.w);
    
    float4 Normalize = TaylorInvSqrt(float4(dot(P0, P0), dot(P1, P1), dot(P2, P2), dot(P3, P3)));
    P0 *= Normalize.x;
    P1 *= Normalize.y;
    P2 *= Normalize.z;
    P3 *= Normalize.w;
    
    float4 Mix = max(0.5f - float4(dot(X0, X0), dot(X1, X1), dot(X2, X2), dot(X3, X3)), 0.f);
    Mix = Mix * Mix;
    
    return 105.f * dot(Mix * Mix, float4(dot(P0, X0), dot(P1, X1), dot(P2, X2), dot(P3, X3)));
}
                      
RWTexture3D<float> OutputTexture : register(u0);

groupshared int3 OutputDimension;

[numthreads(THREAD_X, THREAD_Y, THREAD_Z)]
void SimpleXNoise(uint3 GroupID : SV_GroupID, uint3 DTID : SV_DispatchThreadID, uint3 GTID : SV_GroupThreadID, uint GroupIndex : SV_GroupIndex)
{
    if (any(GTID == 0))
        OutputTexture.GetDimensions(OutputDimension.x, OutputDimension.y, OutputDimension.z);
    GroupMemoryBarrierWithGroupSync();
    
    float fNoiseScale = 0.4f;
    
    float3 vUV = (float3)DTID / (float3) OutputDimension;
    vUV *= fNoiseScale;
   
    float fNoise = 0.f;
    float fAmp = 1.f;
    float fFreq = 1.f;
    
    for (uint i = 0; i < 4; ++i)
    {
        fNoise += fAmp * SNoise(vUV * fFreq);
        fFreq *= 2;
        fAmp += 0.5f;
    }

    fNoise = saturate(fNoise * 0.5f + 0.5f);
    
    OutputTexture[DTID] = fNoise;
}
