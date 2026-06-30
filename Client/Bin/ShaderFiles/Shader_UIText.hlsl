// 폐기

Texture2D g_FontAtlas : register(t0);

SamplerState FontSampler = sampler_state
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Clamp;
    AddressV = Clamp;
};

// 화면 크기, 글자 색상 전달받는 상수버퍼
cbuffer FontCB : register(b0)
{
    float2 g_ScreenSize;        // [8]
    float2 padding;             // [8]
};
cbuffer FontColorCB : register(b1)
{
    float4 g_FontColor;         // [16] RGB + A
};

//float4 g_TargetWorldPos;
//float4x4 g_ViewMatrix;
//float4x4 g_ProjMatrix;
//float4 g_CamPos;



// Flag Variables..
cbuffer FontFlag : register(b2)
{
    uint g_FontFlag = 0;        // [4]
}

// Requires from Flag..
cbuffer FontOutLine : register(b3)
{
    float4 g_FontOutlineColor;      // [16] RGBA Outline Color
    float2 g_FontTexPerPixel;       // [8] 1.f / Texture Size 
    float g_FontOutlineWidth;       // [4] Outline Width Size
}
cbuffer FontGrad : register(b4)
{
    float4 g_FontGradColor;         // [16] RGBA Gradiant Color (->)
}
//cbuffer FontFixed : register(b5)
//{
//    float4 g_TargetWorldPos;                // [16] RGBA Gradiant Color (->)
//    float4 g_CamPos;                        // [16] Camera Position
//    float4x4 g_ViewMatrix, g_ProjMatrix;    // [256 * 2]  Camera Pipeline
//}




#define FL_NONE         0
#define FL_OUTLINE      1 << 0
#define FL_GRAD         1 << 1
#define FL_FIXED        1 << 2

#define FL_END          1 << 3


// 여기서 상태 정의
BlendState BS_FontAlpha
{
    BlendEnable[0] = TRUE;
    SrcBlend = SRC_ALPHA;
    DestBlend = INV_SRC_ALPHA;
    BlendOp = ADD;
};

RasterizerState RS_CullOff
{
    FillMode = Solid;
    CullMode = None;
};

DepthStencilState DSS_Off
{
    DepthEnable = FALSE;
};


// ==============================
// * Vertex Shader
// ==============================
struct VS_IN
{
    float2 vPosition : POSITION;
    float2 vTexcoord : TEXCOORD;
};
struct VS_OUT
{
    float4 vPosition : SV_Position;
    float2 vTexcoord : TEXCOORD;
};

// 정점 셰이더: 화면 좌표를 NDC(-1~1)로 변환
VS_OUT VS_Font(VS_IN In)
{
    VS_OUT Out = (VS_OUT) 0;

    
    if (g_FontFlag & FL_FIXED)
    {

    }

    
    
    if ((g_FontFlag & FL_NONE) ||
        (g_FontFlag & FL_OUTLINE) ||
        (g_FontFlag & FL_GRAD))
    {
        float2 ndc = (In.vPosition / g_ScreenSize) * float2(2, -2) + float2(-1, 1);
        Out.vPosition = float4(ndc, 0, 1);
        Out.vTexcoord = In.vTexcoord;
    }
    
    

    
    
    return Out;
}



// ==============================
// * Pixel Shader
// ==============================
// 픽셀 셰이더: 폰트 아틀라스 {R채널 = Alpha}로 사용!!!!
struct PS_IN
{
    float4 vPosition : SV_Position;
    float2 vTexcoord : TEXCOORD;
};
struct PS_OUT
{
    float4 vColor : SV_TARGET0;
};


PS_OUT PS_Font(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;
    
    float2 uv = In.vTexcoord;
    float alphaCenter = g_FontAtlas.Sample(FontSampler, uv).r;          // 현재 바라보는 픽셀 색상에서 a값 추출
        
    // test
    Out.vColor = float4(1.0f, 0.f, 1.0f, 1.0f);
    return Out;
    

    float fillMask = smoothstep(0.5f, 0.8f, alphaCenter);             // 글자에 색상 채워진 정도를 저장. 경계 부드럽게
    float4 fillColor = float4(g_FontColor.rgb, g_FontColor.a * fillMask);

    Out.vColor.rgb = g_FontColor.rgb;
    Out.vColor.a = g_FontColor.a * fillMask;
    
        
    
    if (g_FontFlag & FL_GRAD)           // ===== grad (wip) =====
    {
        // Gradiant
        const float4 GradRColor = g_FontGradColor;
        
        
        #ifdef KSTA_DEBUG_1_RETURN_AFTERGRAD
            return Out;
        #endif   
    }                                   // END== grad (wip) =====
    
    
    
    
    if (g_FontFlag & FL_OUTLINE)        // ===== outline =====
    {
        float outline = 0.0f;   // mask
        int width = (int) g_FontOutlineWidth;

        // 주변 탐색
        for (int x = -width; x <= width; x++)                                   // 픽셀 단위로 탐색
        {
            for (int y = -width; y <= width; y++)
            {
                float2 offsetUV = uv + float2(x * g_FontTexPerPixel.x,          // g_FontTexPerPixel 를 이용하여 x, y 를 실제 픽셀 단위로 변환한 UV좌표를 계산
                                              y * g_FontTexPerPixel.y);
                float alphaLevel = g_FontAtlas.SampleLevel(FontSampler, offsetUV, 0).r;     // 해당 픽셀의 알파 추출
                outline = max(outline, smoothstep(0.45f, 0.5f, alphaLevel));     // 루프를 도는 동안 가장 높은 outline 값을 갱신, 이를 통해 알파 값을 부드럽게 처리
            }
        }

        // 글자 내부가 아닌 픽셀에만 아웃라인 적용
        float outlineOnly = saturate(outline - fillMask);

        Out.vColor.rgb = lerp(Out.vColor.rgb, g_FontOutlineColor.rgb, outlineOnly);
        Out.vColor.a = max(Out.vColor.a, outlineOnly * g_FontOutlineColor.a);
        
        #ifdef KSTA_DEBUG_2_RETURN_AFTEROUTLINE
            return Out;
        #endif   
    }                                   // END== outline =====
    
    
    return Out;
}



// ==============================
// * Technique (Pass)
// ==============================
// 하나의 Pass에 모든 상태 + 셰이더 묶음
technique11 FontTech
{
    pass DefaultPass
    {
        SetRasterizerState(RS_CullOff);
        SetBlendState(BS_FontAlpha, float4(0, 0, 0, 0), 0xFFFFFFFF);
        SetDepthStencilState(DSS_Off, 0);

        VertexShader = compile vs_5_0 VS_Font();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_Font();
    }
}
