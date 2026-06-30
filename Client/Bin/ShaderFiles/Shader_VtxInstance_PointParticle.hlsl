#include "Engine_Shader_Defines.hlsli"

matrix g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;
Texture2D g_DiffuseTexture;

vector g_vCamPosition;

vector g_vColor;

int g_iRow;
int g_iCol;

int g_MaskFlag;

struct VS_IN
{
    float3 vPosition : POSITION;   
    row_major float4x4 TransformMatrix : WORLD;
    float2 vLifeTime : TEXCOORD0;
    float2 fDelay : TEXCOORD1;
    float4 vVelTail : TEXCOORD2;
    float fPhase : TEXCOORD3;
};

struct VS_OUT
{
    float4 vPosition : POSITION;
    float fSize : PSIZE;
    float2 vLifeTime : TEXCOORD0;
    float2 fDelay : TEXCOORD1;
    float4 vVelTail : TEXCOORD2;
    float fPhase : TEXCOORD3;
};

VS_OUT VS_MAIN(VS_IN In)
{
    VS_OUT Out = (VS_OUT)0;    
 
    //vector vPosition = mul(float4(In.vPosition, 1.f), In.TransformMatrix);
    //Out.vPosition = mul(vPosition, g_WorldMatrix);
    
    vector vPosition = vector(In.TransformMatrix._41_42_43, 1.f);
    Out.vPosition = mul(vPosition, g_WorldMatrix);
    
    Out.fSize = length(In.TransformMatrix._11_12_13);
    Out.vLifeTime = In.vLifeTime;
    Out.vVelTail = In.vVelTail;
    Out.fPhase = In.fPhase;
    Out.fDelay = In.fDelay;
   
    return Out;
}

struct GS_IN
{
    float4 vPosition : POSITION;
    float fSize : PSIZE;
    float2 vLifeTime : TEXCOORD0;
    float2 fDelay : TEXCOORD1;
    float4 vVelTail : TEXCOORD2;
    float fPhase : TEXCOORD3;
};

struct GS_OUT
{
    float4 vPosition : SV_POSITION;
    float2 vTexcoord : TEXCOORD0;
    float2 vLifeTime : TEXCOORD1;
    float fPhase : TEXCOORD2;
    float2 fDelay : TEXCOORD3;
    float fViewZ : TEXCOORD4;
};

[maxvertexcount(6)]
void GS_MAIN(point GS_IN In[1], inout TriangleStream<GS_OUT> Vertices)
{
    GS_OUT Out[4] = (GS_OUT[4]) 0;
    
    if(In[0].fDelay.y > 0.5f)
        return;
    
    vector vRight, vUp, vLook;
    
    vLook = g_vCamPosition - In[0].vPosition;
    vRight = normalize(vector(cross(float3(0.f, 1.f, 0.f), vLook.xyz), 0.f)) * In[0].fSize * 0.5f;
    vUp = normalize(vector(cross(vLook.xyz, vRight.xyz), 0.f)) * In[0].fSize * 0.5f;
    
    float4 vViewPosCenter = mul(In[0].vPosition, g_ViewMatrix);
    
    matrix matVP = mul(g_ViewMatrix, g_ProjMatrix);
    
    Out[0].vPosition = mul(In[0].vPosition + vRight + vUp, matVP);
    Out[0].vTexcoord = float2(0.f, 0.f);
    Out[0].vLifeTime = In[0].vLifeTime;    
    Out[0].fPhase = In[0].fPhase;
    Out[0].fDelay = In[0].fDelay;
    Out[0].fViewZ = vViewPosCenter.z;
    
    Out[1].vPosition = mul(In[0].vPosition - vRight + vUp, matVP);
    Out[1].vTexcoord = float2(1.f, 0.f);
    Out[1].vLifeTime = In[0].vLifeTime;
    Out[1].fPhase = In[0].fPhase;
    Out[1].fDelay = In[0].fDelay;
    Out[1].fViewZ = vViewPosCenter.z;
    
    Out[2].vPosition = mul(In[0].vPosition - vRight - vUp, matVP);
    Out[2].vTexcoord = float2(1.f, 1.f);
    Out[2].vLifeTime = In[0].vLifeTime;
    Out[2].fPhase = In[0].fPhase;
    Out[2].fDelay = In[0].fDelay;
    Out[2].fViewZ = vViewPosCenter.z;
    
    Out[3].vPosition = mul(In[0].vPosition + vRight - vUp, matVP);
    Out[3].vTexcoord = float2(0.f, 1.f);
    Out[3].vLifeTime = In[0].vLifeTime;    
    Out[3].fPhase = In[0].fPhase;
    Out[3].fDelay = In[0].fDelay;
    Out[3].fViewZ = vViewPosCenter.z;
    
    Vertices.Append(Out[0]);
    Vertices.Append(Out[1]);
    Vertices.Append(Out[2]);
    Vertices.RestartStrip();
    
    Vertices.Append(Out[0]);
    Vertices.Append(Out[2]);
    Vertices.Append(Out[3]);
    Vertices.RestartStrip();
}

[maxvertexcount(6)]
void GS_Stretch(point GS_IN In[1], inout TriangleStream<GS_OUT> Vertices)
{
    GS_OUT Out[4] = (GS_OUT[4]) 0;
    
    if (In[0].fDelay.y > 0.5f)
        return;
 
    vector vRight, vUp, vLook, vViewDir;
    
    vViewDir = g_vCamPosition - In[0].vPosition;
    
    float tailLen = In[0].vVelTail.w;      //컴셰에서  float tailLen = clamp(fSpeed * 가중치, 최소, 최대) 계산해서 저장해놓은 값이 w     
    
    float fSpeed = length(In[0].vVelTail.xyz);     //혹시라도 스피드 값이 거의 없는 얘들은 따로 처리해주고자 스피드 확인
    
    if (fSpeed > 0.1f)
        vLook = vector(normalize(In[0].vVelTail.xyz), 0.f);
    else
        vLook = vViewDir;
    
    vRight = normalize(vector(cross(vViewDir.xyz, vLook.xyz), 0.f)) * In[0].fSize * 0.5f;
    vUp = normalize(vector(cross(vLook.xyz, vRight.xyz), 0.f)) * In[0].fSize * 0.5f;
    
    float3 Look = vLook * tailLen; 
    
    float4 vViewPosCenter = mul(In[0].vPosition, g_ViewMatrix);
    
    matrix matVP = mul(g_ViewMatrix, g_ProjMatrix);
    
    Out[0].vPosition = mul(In[0].vPosition + vRight + vUp, matVP);
    Out[0].vTexcoord = float2(0.f, 0.f);
    Out[0].vLifeTime = In[0].vLifeTime;
    Out[0].fPhase = In[0].fPhase;
    Out[0].fDelay = In[0].fDelay;
    Out[0].fViewZ = vViewPosCenter.z;
    
    Out[1].vPosition = mul(In[0].vPosition - vRight + vUp, matVP);
    Out[1].vTexcoord = float2(1.f, 0.f);
    Out[1].vLifeTime = In[0].vLifeTime;
    Out[1].fPhase = In[0].fPhase;
    Out[1].fDelay = In[0].fDelay;
    Out[1].fViewZ = vViewPosCenter.z;
    
    Out[2].vPosition = mul(In[0].vPosition - vRight - vUp + float4(Look, 0.f), matVP);
    Out[2].vTexcoord = float2(1.f, 1.f);
    Out[2].vLifeTime = In[0].vLifeTime;
    Out[2].fPhase = In[0].fPhase;
    Out[2].fDelay = In[0].fDelay;
    Out[2].fViewZ = vViewPosCenter.z;
    
    Out[3].vPosition = mul(In[0].vPosition + vRight - vUp + float4(Look, 0.f), matVP);
    Out[3].vTexcoord = float2(0.f, 1.f);
    Out[3].vLifeTime = In[0].vLifeTime;
    Out[3].fPhase = In[0].fPhase;
    Out[3].fDelay = In[0].fDelay;
    Out[3].fViewZ = vViewPosCenter.z;
    
    Vertices.Append(Out[0]);
    Vertices.Append(Out[1]);
    Vertices.Append(Out[2]);
    Vertices.RestartStrip();
    
    Vertices.Append(Out[0]);
    Vertices.Append(Out[2]);
    Vertices.Append(Out[3]);
    Vertices.RestartStrip();
}


struct PS_IN
{
    float4 vPosition : SV_POSITION;
    float2 vTexcoord : TEXCOORD0;
    float2 vLifeTime : TEXCOORD1;
    float fPhase : TEXCOORD2;
    float2 fDelay : TEXCOORD3;
    float fViewZ : TEXCOORD4;
};

struct PS_OUT
{
    float4 vEmissive : SV_TARGET1;
    float4 vAccumColor : SV_TARGET3; //TEST
    float4 vAccumAlpha : SV_TARGET4;
};

PS_OUT PS_MAIN(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;    
    
    float4 vColor;
    
    vColor = g_DiffuseTexture.Sample(DefaultSampler, In.vTexcoord);
    
    if(g_MaskFlag == 1)
    {
        vColor.a = max(max(vColor.r, vColor.g), vColor.b);
    }

    if (vColor.a <= 0.1f)
        discard;
    
    float2 LifeTime = In.vLifeTime;
    
    float Alpha = 1 - saturate(LifeTime.x / LifeTime.y);
    
    vColor *= g_vColor;
    vColor.a *= Alpha;
    
    float fWeight = Luminance(vColor.xyz * g_EmssiveColorWeight);
    
    if (fWeight >= g_fEmissiveThreshold)
        Out.vEmissive = float4(vColor.xyz * g_EmssiveColorWeight, 1.f);
    
    Out.vEmissive.xyz *= vColor.a;
    
    float z = abs(In.fViewZ);
    
    float Weight = max(1e-5, exp(-z * g_WeightBlend));
    
    Out.vAccumColor = float4(vColor.rgb * vColor.a, 0.0f) * Weight;
    Out.vAccumAlpha.r = vColor.a * Weight;
    
    return Out;
}

PS_OUT PS_SPRITE(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;
    
    if (In.fDelay.y > 0.5f)                                 //안그려도 됨
        discard;
    
    float fPhase = In.fPhase;
    int CellCount = g_iRow * g_iCol;                      // 2x2 면 4개
    
    int fFrame = floor(frac(fPhase) * CellCount);         //카운트 개수넘지 않게 fPhase 소수점(컴셰에서 +=로 계산해서 넘겨주는 값) 곱해서 정수로 뽑은 Frame 0부터 시작해서 올라감.
    
    int Col = fFrame % g_iCol;                            //UV 사이즈 계산, 이 식이 통할려면 아틀라스 이미지가 각각 서로 크기가 균등해야함.
    int Row = fFrame / g_iCol;
    
    float2 CellSize = float2(1.0 / (float) g_iCol, 1.0 / (float)g_iRow); // 0 ~ 1의 UV Offset 계산 Col이 증가할수록 UV의 x축 -> 오른쪽 이동 Row가 증가할수록 UV의 y축 -> 아래로 이동             
    
    float2 OffSet = float2(Col * CellSize.x, Row * CellSize.y);
    
    float2 Texcoord = In.vTexcoord * CellSize + OffSet;
    
    float4 vColor;
    
    vColor = g_DiffuseTexture.Sample(DefaultSampler, Texcoord);
    
    if (g_MaskFlag == 0)
    {
        vColor.a = max(max(vColor.r, vColor.g), vColor.b);
    }
    
    vColor *= g_vColor;
    
    float2 LifeTime = In.vLifeTime;
    
    float Alpha = 1 - saturate(LifeTime.x / LifeTime.y);
    
    vColor.a *= Alpha;
    
    if (vColor.a < 0.1f)
        discard;
    
    float fWeight = Luminance(vColor.xyz * g_EmssiveColorWeight);
    
    if (fWeight >= g_fEmissiveThreshold)
        Out.vEmissive = float4(vColor.xyz * g_EmssiveColorWeight, 1.f);
    
    Out.vEmissive.xyz *= vColor.a;

    float z = abs(In.fViewZ);
    float Weight = max(1e-5, exp(-z * g_WeightBlend));
    
    Out.vAccumColor = float4(vColor.rgb * vColor.a, 0.0f) * Weight;
    Out.vAccumAlpha.r = vColor.a * Weight;
    
    return Out;
}


technique11 DefaultTechnique
{
    //0
    pass DefaultPass
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_NoneCompare, 0);
        SetBlendState(BS_AccumBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();   
        GeometryShader = compile gs_5_0 GS_MAIN();
        PixelShader = compile ps_5_0 PS_MAIN();
    }

    //1
    pass StretchPass
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_NoneCompare, 0);
        SetBlendState(BS_AccumBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = compile gs_5_0 GS_Stretch();
        PixelShader = compile ps_5_0 PS_MAIN();
    }
   
    //2
    pass DefaultSpritePass
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_NoneCompare, 0);
        SetBlendState(BS_AccumBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = compile gs_5_0 GS_MAIN();
        PixelShader = compile ps_5_0 PS_SPRITE();
    }

    //3
    pass StretchSpritePass
    {
        SetRasterizerState(RS_Default); 
        SetDepthStencilState(DSS_NoneCompare, 0);
        SetBlendState(BS_AccumBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = compile gs_5_0 GS_Stretch();
        PixelShader = compile ps_5_0 PS_SPRITE();
    }
}
