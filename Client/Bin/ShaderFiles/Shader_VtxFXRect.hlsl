#include "Engine_Shader_Defines.hlsli"

matrix g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;

texture2D g_MaskTexture;
texture2D g_DiffuseTexture;

vector g_vCamPosition;

vector g_vColor;

float g_fXSize;
float g_fYSize;

float g_Sweep;      // 0 -> 1
float g_Soft;       //
int g_MaskFlag;
int g_ColorFlag;

int g_iCol;         //qt
int g_iRow;
float g_fPhase;
float2 g_vLifeTime;


struct VS_IN
{
    float3 vPosition : POSITION;
};

struct VS_OUT
{
    float4 vPosition : POSITION;
};

VS_OUT VS_MAIN(VS_IN In)
{
    VS_OUT Out = (VS_OUT) 0;
    
    Out.vPosition = mul(float4(In.vPosition, 1.f), g_WorldMatrix);
    
    return Out;
}
struct GS_IN
{
    float4 vPosition : POSITION;
};

struct GS_OUT
{
    float4 vPosition : SV_POSITION;
    float2 vTexcoord : TEXCOORD0;
    float4 vProjPos : TEXCOORD1;
};

[maxvertexcount(6)]
void GS_MAIN(point GS_IN In[1], inout TriangleStream<GS_OUT> Vertices)
{
    GS_OUT Out[4] = (GS_OUT[4])0;
    
    vector vRight, vUp, vLook;
    
    vLook = g_vCamPosition - In[0].vPosition;
    vRight = normalize(vector(cross(float3(0.f, 1.f, 0.f), vLook.xyz), 0.f)) * g_fXSize * 0.5f;
    vUp = normalize(vector(cross(vLook.xyz, vRight.xyz), 0.f)) * g_fYSize * 0.5f;
    
    matrix matVP = mul(g_ViewMatrix, g_ProjMatrix);
    
    Out[0].vPosition = mul(In[0].vPosition + vRight + vUp, matVP);
    Out[0].vTexcoord = float2(0.f, 0.f);
    Out[0].vProjPos = Out[0].vPosition;
    
    Out[1].vPosition = mul(In[0].vPosition - vRight + vUp, matVP);
    Out[1].vTexcoord = float2(1.f, 0.f);
    Out[1].vProjPos = Out[1].vPosition;
    
    Out[2].vPosition = mul(In[0].vPosition - vRight - vUp, matVP);
    Out[2].vTexcoord = float2(1.f, 1.f);
    Out[2].vProjPos = Out[2].vPosition;
    
    Out[3].vPosition = mul(In[0].vPosition + vRight - vUp, matVP);
    Out[3].vTexcoord = float2(0.f, 1.f);
    Out[3].vProjPos = Out[3].vPosition;
    
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
    float4 vProjPos : TEXCOORD1;
};

struct PS_OUT
{
    float4 vDiffuse : SV_TARGET0;
    float4 vEmissive : SV_TARGET1;
    float4 vAccumColor : SV_TARGET3; //TEST
    float4 vAccumAlpha : SV_TARGET4;
};

PS_OUT PS_MAIN(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;
    
    float4 vColor;
    
    vColor = g_DiffuseTexture.Sample(DefaultSampler, In.vTexcoord);
    
    if(g_MaskFlag == 0)
    {
        vColor.a = max(max(vColor.r, vColor.g), vColor.b);
    }

    if (vColor.a < 0.2f)
        discard;
    
    if (g_ColorFlag == 0)
        vColor *= g_vColor;
    else
        vColor.rgb = g_vColor.rgb;
    
    float2 LifeTime = g_vLifeTime;
    
    float Alpha = 1 - saturate(LifeTime.x / LifeTime.y);
    
    vColor.a *= Alpha;
    
    float fWeight = Luminance(vColor.xyz * g_EmssiveColorWeight);
 
    if (fWeight >= g_fEmissiveThreshold)
        Out.vEmissive = float4(vColor.xyz * g_EmssiveColorWeight, 1.f);
    
    Out.vEmissive.xyz *= vColor.a;

    float z = In.vProjPos.z / In.vProjPos.w;
    float Weight = max(1e-5, exp(-z * g_WeightBlend));
    Out.vAccumColor = float4(vColor.rgb * vColor.a, 0.0f) * Weight;
    Out.vAccumAlpha.r = vColor.a * Weight;
    
    return Out;
}

PS_OUT PS_TEST(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;
    
    float4 vColor;
    
    vColor = g_DiffuseTexture.Sample(ClampSampler, In.vTexcoord);
    
    if (g_MaskFlag == 0) // 알파컷 설정 0 이면 r,g,b 체크해서 -> a 에 저장해주는거. 1이면 걍 어차피 알파있는 텍스처니까 알파 그대로사용
    {
        vColor.a = max(max(vColor.r, vColor.g), vColor.b);
    }
    //else
    //{
    //    if (Out.vDiffuse.a < 0.3f)
    //        discard;
    //}
    
    if (vColor.a < 0.2f)
        discard;
    
    float fCenter = In.vTexcoord - (0.5, 0.5);
    
    float fCircle = length(fCenter) / 0.5f;
    
    float fVisible = smoothstep(g_Sweep - g_Soft, g_Sweep, fCircle);
    
    vColor.a *= fVisible;
    
    //if (Out.vDiffuse.a < 0.3f)
    //    discard;
    
    if (g_ColorFlag == 0)
        vColor *= g_vColor;
    else
        vColor.rgb = g_vColor.rgb;
   
    if (vColor.a < 0.1f)
        discard;
   
    float fWeight = Luminance(vColor.xyz * g_EmssiveColorWeight);
   
   if (fWeight >= g_fEmissiveThreshold)
        Out.vEmissive = float4(vColor.xyz * g_EmssiveColorWeight, 1.f);
    
    Out.vEmissive.xyz *= vColor.a;
    
    float z = In.vProjPos.z / In.vProjPos.w;
    float Weight = max(1e-5, exp(-z * g_WeightBlend));
    Out.vAccumColor = float4(vColor.rgb * vColor.a, 0.0f) * Weight;
    Out.vAccumAlpha.r = vColor.a * Weight;
 
    return Out;
}

PS_OUT PS_TESTA(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;
    
    float4 vColor;
    
    vColor = g_DiffuseTexture.Sample(ClampSampler, In.vTexcoord);
    
    if (g_MaskFlag == 0)
    {
        vColor.a = max(max(vColor.r, vColor.g), vColor.b);
     
    }
    
    if (vColor.a < 0.2f)
        discard;
    
    float fCenter = In.vTexcoord - (0.5, 0.5);
    
    float fCircle = length(fCenter) / 0.5f;
    
    float fVisible = 1 - smoothstep(g_Sweep - g_Soft, g_Sweep, fCircle);
    
    vColor.a *= fVisible;
    
    //테스트
    
    if (g_ColorFlag == 0)
        vColor *= g_vColor;
    else
        vColor.rgb = g_vColor.rgb;
    
    float2 LifeTime = g_vLifeTime;
    
    float Alpha = 1 - saturate(LifeTime.x / LifeTime.y);
    
    vColor.a *= Alpha;
    
    if (vColor.a < 0.1f)
        discard;
  
    float fWeight = Luminance(vColor.xyz * g_EmssiveColorWeight);
   
    if (fWeight >= g_fEmissiveThreshold)
        Out.vEmissive = float4(vColor.xyz * g_EmssiveColorWeight, 1.f);
    
    Out.vEmissive.xyz *= vColor.a;
 
    float z = In.vProjPos.z / In.vProjPos.w;
    float Weight = max(1e-5, exp(-z * g_WeightBlend));
    Out.vAccumColor = float4(vColor.rgb * vColor.a, 0.0f) * Weight;
    Out.vAccumAlpha.r = vColor.a * Weight;
    
    return Out;
}

PS_OUT PS_SPRITE(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;
    
    int CellCount = g_iRow * g_iCol;
    
    int fFrame = floor(frac(g_fPhase) * CellCount);
    
    int Col = fFrame % g_iCol;
    int Row = fFrame / g_iCol;
    
    float2 CellSize = float2(1.0 / (float) g_iCol, 1.0 / (float) g_iRow);
    
    float2 OffSet = float2(Col * CellSize.x, Row * CellSize.y);
    
    float2 Texcoord = In.vTexcoord * CellSize + OffSet;
    
    float4 vColor;
    
    vColor = g_DiffuseTexture.Sample(DefaultSampler, Texcoord);
    
    if (vColor.a < 0.1f)
        discard;
    
    //vColor.a = 1.f;
    
    if (g_ColorFlag == 0)
        vColor *= g_vColor;
    else
        vColor.rgb = g_vColor.rgb;
    
    float2 LifeTime = g_vLifeTime;
    
    float Alpha = 1 - saturate(LifeTime.x / LifeTime.y);
    
    vColor.a *= Alpha;
    
    if (vColor.a < 0.1f)
        discard;
    
    float fWeight = Luminance(vColor.xyz * g_EmssiveColorWeight);
   
    if (fWeight >= g_fEmissiveThreshold)
        Out.vEmissive = float4(vColor.xyz * g_EmssiveColorWeight, 1.f);
    
    Out.vEmissive.xyz *= vColor.a;
    
    float z = In.vProjPos.z / In.vProjPos.w;
    float Weight = max(1e-5, exp(-z * g_WeightBlend));
    Out.vAccumColor = float4(vColor.rgb * vColor.a, 0.0f) * Weight;
    Out.vAccumAlpha.r = vColor.a * Weight;
   
        return Out;
    }


technique11 DefaultTechnique
{
    pass DefaultPass // 0
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_NoneCompare, 0);
        SetBlendState(BS_AccumBlend, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = compile gs_5_0 GS_MAIN();
        PixelShader = compile ps_5_0 PS_MAIN();
    }
 
    pass OutPass //1
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_NoneCompare, 0);
        SetBlendState(BS_AccumBlend, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = compile gs_5_0 GS_MAIN();
        PixelShader = compile ps_5_0 PS_TEST();
    }

    pass InPass //2
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_NoneCompare, 0);
        SetBlendState(BS_AccumBlend, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = compile gs_5_0 GS_MAIN();
        PixelShader = compile ps_5_0 PS_TESTA();
    }
    
    pass BlendOut //3
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_NoneCompare, 0);
        SetBlendState(BS_AccumBlend, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = compile gs_5_0 GS_MAIN();
        PixelShader = compile ps_5_0 PS_TEST();
    }

    pass BlendIn //4
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_NoneCompare, 0);
        SetBlendState(BS_AccumBlend, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = compile gs_5_0 GS_MAIN();
        PixelShader = compile ps_5_0 PS_TESTA();
    }

    pass Sprite //5
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_NoneCompare, 0);
        SetBlendState(BS_AccumBlend, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = compile gs_5_0 GS_MAIN();
        PixelShader = compile ps_5_0 PS_SPRITE();
    }
}