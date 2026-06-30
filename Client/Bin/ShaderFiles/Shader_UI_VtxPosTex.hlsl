// UI용
#include "Shader_UI_State.hlsli"



// ==============================
// * Global Variabless
// ==============================

// Basic Variables
matrix g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;
Texture2D g_Texture;
float g_AlphaStrength;

Texture2D g_TextureFX;
float g_FXStrength;

// Gradient Variables
float2 g_ScreenLT = { 0.f, 0.f }, g_ScreenRB = { 1920.f, 1080.f }; // based on worldspace.         for discard by pos (esc menu, inventory, etc..)
bool g_InverseScreenDiscard = false; // 좌상단 끝이 0, 0 / 우하단 끝이 스크린X, 스크린Y 크기에 해당
float4 g_BlendToOuterWidth = { 0.f, 0.f, 0.f, 0.f }; // (좌, 우, 상, 하) (left, right, top, bottom)
float2 g_ScreenSize = { 1920.f, 1080.f };


// Cutout Variables
float g_CutoutAlphaDiscard = 0.3f;


// Nine-Sector Variables
float2 g_ImageSize = { 0.f, 0.f };
float2 g_SectorBorder = { 0.f, 0.f }; // based on local texcoord.     for 9sector
float g_UIScale = 1.f; // UI Scaler






//float2      g_ScreenLT = { 200.f, 200.f }, g_ScreenRB = { 1720.f, 880.f };     // based on worldspace.         for discard by pos (esc menu, inventory, etc..)
//bool        g_InverseScreenDiscard = false;                                 // 좌상단 끝이 0, 0 / 우하단 끝이 스크린X, 스크린Y 크기에 해당
//float4      g_BlendToOuterWidth = { 0.f, 0.f, 0.f, 0.f };                   // (좌, 우, 상, 하) (left, right, top, bottom)



// ==============================
// * Function 
// ==============================

bool Check_isInSpace(float2 originPos, float2 vScreenLT, float2 vScreenRB, bool isInverse = false)
{
    // all float2 is based on Screen Space.
    // (LT : 0, 0 / RB : ScreenWidthX, ScreenWidthY)
    
    bool isInSpace = (
        originPos.x >= vScreenLT.x &&
        originPos.x <= vScreenRB.x &&
        originPos.y >= vScreenLT.y &&
        originPos.y <= vScreenRB.y
    );
    
    if (isInverse)
        isInSpace = !isInSpace;
    
    return isInSpace;
}

float Check_SpaceRatio(float2 originPos, float2 startPos, float2 endPos)
{
    // returns Ratio from startPos to endPos. (StartPos : 0, EndPos : 1)
    // Only works based on screen space.
    
    // (LT : 0, 0 / RB : ScreenWidthX, ScreenWidthY)
    
    float result = 0.f;
    
    if (((startPos.x < originPos.x && originPos.x < endPos.x) &&
            (startPos.y < originPos.y && originPos.y < endPos.y)))
    {
        float AtoB = length(originPos - startPos);
        float BtoC = length(endPos - originPos);
        
        result = AtoB / (AtoB + BtoC);
    }
    else
    {
        if (originPos.x < startPos.x)
            return 0.f;
        else if (originPos.x > endPos.x)
            return 1.f;
        else if (originPos.y < startPos.y)
            return 0.f;
        else if (originPos.y > endPos.y)
            return 1.f;
    }
    
    return result;
}

float Check_SpaceRatioP(float originPoint, float startPoint, float endPoint)
{
    return saturate((originPoint - startPoint) / (endPoint - startPoint));
}

float2 Calc_NineSectorUV(float2 originPos, float2 modSize, float2 border, float2 imageSize) // 1. texcoord 상 좌표?
{
    // 전체 대비 왼쪽/위 로부터 얼마나 오른쪽/아래에 있는지의 비율
    
    float2 resultUV;
    
    // x축 계산
    
    if (originPos.x < border.x)                    // 왼쪽.
        resultUV.x = originPos.x / imageSize.x;
    else if ((modSize.x - border.x) < originPos.x)      // 오른쪽. 
        resultUV.x = (originPos.x - (modSize.x - imageSize.x)) / imageSize.x;
    else
    {
        // [ originPos.x - border.x ] ~ [ originPos.x ] 의 사잇값인 originPosX_OnMod 를
        // [ border.x ] ~ [ modSize.x - border.x ] 사이로 비율을 맞춰야 함 
        
        float originPosX_OnMod = originPos.x - border.x;

        float startX = border.x;
        float endX = modSize.x - border.x;
        
        float resultRatio = originPosX_OnMod / (endX - startX);
        
        resultUV.x = (border.x + (resultRatio * (imageSize.x - border.x * 2))) / imageSize.x;
    }
    
    
    if (originPos.y < border.y)                    // 위쪽
        resultUV.y = originPos.y / imageSize.y;
    else if ((modSize.y - border.y) < originPos.y)      // 아래쪽.
        resultUV.y = (originPos.y - (modSize.y - imageSize.y)) / imageSize.y;
    else
    {
        float originPosX_OnMod = originPos.y - border.y;

        float startY = border.y;
        float endY = modSize.y - border.y;
        
        float resultRatio = originPosX_OnMod / (endY - startY);
        
        resultUV.y = (border.y + (resultRatio * (imageSize.y - border.y * 2))) / imageSize.y;
    }
    
    return resultUV;
}




// ==============================
// * Vertex Shader
// ==============================
struct VS_IN
{
    float3 vPosition : POSITION;
    float2 vTexcoord : TEXCOORD0;
};

struct VS_OUT
{
    float4 vPosition : SV_POSITION;
    float2 vTexcoord : TEXCOORD0;
    float4 vWorldPos : TEXCOORD1;
    float4 vProjPos : TEXCOORD2;
};



VS_OUT VS_MAIN(VS_IN In)
{
    VS_OUT Out = (VS_OUT) 0;
    
    /* 정점의 로컬위치 * 월드 * 뷰 * 투영 */ 
        
    float4 vFinalPos;
    
    
    
    float4x4 matWV, matWVP;
    
    matWV = mul(g_WorldMatrix, g_ViewMatrix);
    matWVP = mul(matWV, g_ProjMatrix);
        
    vFinalPos = mul(float4(In.vPosition, 1.f), matWVP);
    
    
    
    
    Out.vPosition = vFinalPos;
    Out.vTexcoord = In.vTexcoord;
    Out.vWorldPos = mul(float4(In.vPosition, 1.f), g_WorldMatrix);
    Out.vProjPos = Out.vPosition;
    
    return Out;
}
    
// ==============================
// * Pixel Shader
// ==============================
struct PS_IN
{
    float4 vPosition : SV_POSITION;
    float2 vTexcoord : TEXCOORD0;
    float4 vWorldPos : TEXCOORD1;
    float4 vProjPos : TEXCOORD2;
};

struct PS_OUT
{
    float4 vColor : SV_TARGET0;
    
};

PS_OUT PS_MAIN(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;
    
    Out.vColor = g_Texture.Sample(DefaultSampler, In.vTexcoord);
    
    return Out;
}

PS_OUT PS_MAIN_BLEND(PS_IN In) //?
{
    PS_OUT Out = (PS_OUT) 0;
    
    Out.vColor = g_Texture.Sample(DefaultSampler, In.vTexcoord);
    
    float2 vTexcoord;
    
    vTexcoord.x = (In.vProjPos.x / In.vProjPos.w) * 0.5f + 0.5f;
    vTexcoord.y = (In.vProjPos.y / In.vProjPos.w) * -0.5f + 0.5f;
    //vector vDepthDesc = g_DepthTexture.Sample(DefaultSampler, vTexcoord);
    
    Out.vColor.a = Out.vColor.a; // * saturate(vDepthDesc.y - In.vProjPos.w);
    
    return Out;
}

PS_OUT PS_CUTOUT_UI(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;
    
    Out.vColor = g_Texture.Sample(DefaultSampler, In.vTexcoord);
        
    if (Out.vColor.a <= g_CutoutAlphaDiscard)
        discard;
    
    return Out;
}


PS_OUT PS_ALPHAENABLED_UI(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;
    
    Out.vColor = g_Texture.Sample(DefaultSampler, In.vTexcoord);
    Out.vColor.a = Out.vColor.a * (1.f - g_AlphaStrength);
    
    // 범위 내에 없으면 discard
    if (!Check_isInSpace(In.vPosition.xy, g_ScreenLT, g_ScreenRB, g_InverseScreenDiscard))
        discard;
        
    return Out;
}

PS_OUT PS_GRADIENT_UI(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;
    

    
    // gradient 목적지 좌표 구함.
    float2 FixedScreenLT = { g_ScreenLT.x - g_BlendToOuterWidth.x, g_ScreenLT.y - g_BlendToOuterWidth.z };
    float2 FixedScreenRB = { g_ScreenRB.x + g_BlendToOuterWidth.y, g_ScreenRB.y + g_BlendToOuterWidth.w };
    
    // discard 클리핑용. 그려질 부분을 모두 감싸는 사각형 좌표 구함. 반전의 경우엔 모두 감싸지는 사각형.
    if (!g_InverseScreenDiscard)
    {
        float2 ClipScreenLT = float2(min(FixedScreenLT.x, g_ScreenLT.x), min(FixedScreenLT.y, g_ScreenLT.y));
        float2 ClipScreenRB = float2(max(FixedScreenRB.x, g_ScreenRB.x), max(FixedScreenRB.y, g_ScreenRB.y));
    
        if (!Check_isInSpace(In.vPosition.xy, ClipScreenLT, ClipScreenRB, g_InverseScreenDiscard))
            discard;
    }
    else
    {
        float2 ClipScreenLT = float2(max(FixedScreenLT.x, g_ScreenLT.x), max(FixedScreenLT.y, g_ScreenLT.y));
        float2 ClipScreenRB = float2(min(FixedScreenRB.x, g_ScreenRB.x), min(FixedScreenRB.y, g_ScreenRB.y));
    
        if (!Check_isInSpace(In.vPosition.xy, ClipScreenLT, ClipScreenRB, g_InverseScreenDiscard))
            discard;
    }
    
    
    float alphaRatioX = (g_InverseScreenDiscard) ? 1.f : 0.f;
    float alphaRatioY = (g_InverseScreenDiscard) ? 1.f : 0.f;
    
    //if (!g_InverseScreenDiscard)
    //{
    if (g_BlendToOuterWidth.x > 0 || g_BlendToOuterWidth.y > 0) // Outer Gradient
    {
        if (g_ScreenLT.x >= In.vPosition.x) // 왼쪽 밖에 있음
            alphaRatioX = Check_SpaceRatioP(In.vPosition.x, g_ScreenLT.x, FixedScreenLT.x);
        else if (g_ScreenRB.x <= In.vPosition.x) // 오른쪽 밖에 있음
            alphaRatioX = Check_SpaceRatioP(In.vPosition.x, g_ScreenRB.x, FixedScreenRB.x);
    }
    else if (g_BlendToOuterWidth.x < 0 || g_BlendToOuterWidth.y < 0) // Inner Gradient
    {
        if (g_ScreenLT.x <= In.vPosition.x && In.vPosition.x <= (g_ScreenLT.x + g_ScreenRB.x) / 2.f) // 왼쪽 안에 있음
            alphaRatioX = Check_SpaceRatioP(In.vPosition.x, FixedScreenLT.x, g_ScreenLT.x);
        else if ((g_ScreenLT.x + g_ScreenRB.x) / 2.f <= In.vPosition.x && In.vPosition.x <= g_ScreenRB.x)
            alphaRatioX = Check_SpaceRatioP(In.vPosition.x, FixedScreenRB.x, g_ScreenRB.x);
    }
    
    if (g_BlendToOuterWidth.z > 0 || g_BlendToOuterWidth.w > 0) // Outer Gradient
    {
        if (In.vPosition.y <= g_ScreenLT.y)        // 위쪽 밖
            alphaRatioY = Check_SpaceRatioP(In.vPosition.y, g_ScreenLT.y, FixedScreenLT.y);
        else if (In.vPosition.y >= g_ScreenRB.y)   // 아래쪽 밖
            alphaRatioY = Check_SpaceRatioP(In.vPosition.y, g_ScreenRB.y, FixedScreenRB.y);
    }
    else if (g_BlendToOuterWidth.z < 0 || g_BlendToOuterWidth.w < 0) // Inner Gradient
    {
        if (In.vPosition.y >= g_ScreenLT.y && In.vPosition.y <= (g_ScreenLT.y + g_ScreenRB.y) / 2.f)
            alphaRatioY = Check_SpaceRatioP(In.vPosition.y, FixedScreenLT.y, g_ScreenLT.y);
        else if (In.vPosition.y >= (g_ScreenLT.y + g_ScreenRB.y) / 2.f && In.vPosition.y <= g_ScreenRB.y)
            alphaRatioY = Check_SpaceRatioP(In.vPosition.y, FixedScreenRB.y, g_ScreenRB.y);
    }
    //}
    
   
    float alphaRatio = max(alphaRatioX, alphaRatioY);
    float finalAlphaRatio = (g_InverseScreenDiscard) ? alphaRatio : (1.f - alphaRatio); // inverse 여부 반영    
    
     
    Out.vColor = g_Texture.Sample(DefaultSampler, In.vTexcoord);
    Out.vColor.a = Out.vColor.a * (1.f - g_AlphaStrength);
    
    Out.vColor.a *= finalAlphaRatio;
    
    return Out;
}

PS_OUT PS_NINESECTOR_UI(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;
    

    
    // gradient 목적지 좌표 구함.
    float2 FixedScreenLT = { g_ScreenLT.x - g_BlendToOuterWidth.x, g_ScreenLT.y - g_BlendToOuterWidth.z };
    float2 FixedScreenRB = { g_ScreenRB.x + g_BlendToOuterWidth.y, g_ScreenRB.y + g_BlendToOuterWidth.w };
    
    // discard 클리핑용. 그려질 부분을 모두 감싸는 사각형 좌표 구함. 반전의 경우엔 모두 감싸지는 사각형.
    if (!g_InverseScreenDiscard)
    {
        float2 ClipScreenLT = float2(min(FixedScreenLT.x, g_ScreenLT.x), min(FixedScreenLT.y, g_ScreenLT.y));
        float2 ClipScreenRB = float2(max(FixedScreenRB.x, g_ScreenRB.x), max(FixedScreenRB.y, g_ScreenRB.y));
    
        if (!Check_isInSpace(In.vPosition.xy, ClipScreenLT, ClipScreenRB, g_InverseScreenDiscard))
            discard;
    }
    else
    {
        float2 ClipScreenLT = float2(max(FixedScreenLT.x, g_ScreenLT.x), max(FixedScreenLT.y, g_ScreenLT.y));
        float2 ClipScreenRB = float2(min(FixedScreenRB.x, g_ScreenRB.x), min(FixedScreenRB.y, g_ScreenRB.y));
    
        if (!Check_isInSpace(In.vPosition.xy, ClipScreenLT, ClipScreenRB, g_InverseScreenDiscard))
            discard;
    }
    
    
    float alphaRatioX = (g_InverseScreenDiscard) ? 1.f : 0.f;
    float alphaRatioY = (g_InverseScreenDiscard) ? 1.f : 0.f;
    
    //if (!g_InverseScreenDiscard)
    //{
    if (g_BlendToOuterWidth.x > 0 || g_BlendToOuterWidth.y > 0) // Outer Gradient
    {
        if (g_ScreenLT.x >= In.vPosition.x) // 왼쪽 밖에 있음
            alphaRatioX = Check_SpaceRatioP(In.vPosition.x, g_ScreenLT.x, FixedScreenLT.x);
        else if (g_ScreenRB.x <= In.vPosition.x) // 오른쪽 밖에 있음
            alphaRatioX = Check_SpaceRatioP(In.vPosition.x, g_ScreenRB.x, FixedScreenRB.x);
    }
    else if (g_BlendToOuterWidth.x < 0 || g_BlendToOuterWidth.y < 0) // Inner Gradient
    {
        if (g_ScreenLT.x <= In.vPosition.x && In.vPosition.x <= (g_ScreenLT.x + g_ScreenRB.x) / 2.f) // 왼쪽 안에 있음
            alphaRatioX = Check_SpaceRatioP(In.vPosition.x, FixedScreenLT.x, g_ScreenLT.x);
        else if ((g_ScreenLT.x + g_ScreenRB.x) / 2.f <= In.vPosition.x && In.vPosition.x <= g_ScreenRB.x)
            alphaRatioX = Check_SpaceRatioP(In.vPosition.x, FixedScreenRB.x, g_ScreenRB.x);
    }
    
    if (g_BlendToOuterWidth.z > 0 || g_BlendToOuterWidth.w > 0) // Outer Gradient
    {
        if (In.vPosition.y <= g_ScreenLT.y)        // 위쪽 밖
            alphaRatioY = Check_SpaceRatioP(In.vPosition.y, g_ScreenLT.y, FixedScreenLT.y);
        else if (In.vPosition.y >= g_ScreenRB.y)   // 아래쪽 밖
            alphaRatioY = Check_SpaceRatioP(In.vPosition.y, g_ScreenRB.y, FixedScreenRB.y);
    }
    else if (g_BlendToOuterWidth.z < 0 || g_BlendToOuterWidth.w < 0) // Inner Gradient
    {
        if (In.vPosition.y >= g_ScreenLT.y && In.vPosition.y <= (g_ScreenLT.y + g_ScreenRB.y) / 2.f)
            alphaRatioY = Check_SpaceRatioP(In.vPosition.y, FixedScreenLT.y, g_ScreenLT.y);
        else if (In.vPosition.y >= (g_ScreenLT.y + g_ScreenRB.y) / 2.f && In.vPosition.y <= g_ScreenRB.y)
            alphaRatioY = Check_SpaceRatioP(In.vPosition.y, FixedScreenRB.y, g_ScreenRB.y);
    }
    //}

    
   
    float alphaRatio = max(alphaRatioX, alphaRatioY);
    float finalAlphaRatio = (g_InverseScreenDiscard) ? alphaRatio : (1.f - alphaRatio); // inverse 여부 반영    
    
    
    
    
    float2 vSize =
    {
        length(g_WorldMatrix[0].xyz) * g_UIScale,
        length(g_WorldMatrix[1].xyz) * g_UIScale,
    };
    float2 border = g_SectorBorder * g_UIScale;
    
    float2 localPos = In.vTexcoord * vSize;

    // 9-slice 계산된 UV
    float2 resultUV = Calc_NineSectorUV(localPos, vSize, border, g_ImageSize);
    
    
    
    
    Out.vColor = g_Texture.Sample(DefaultSampler, resultUV);
    Out.vColor.a = Out.vColor.a * (1.f - g_AlphaStrength);
    
    Out.vColor.a *= finalAlphaRatio;
    
    return Out;
}



// ==============================
// * Technique (Pass)
// ==============================
technique11 DefaultTechnique
{
    pass DefaultPass
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN();
    }

    pass CutOutPass
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_CUTOUT_UI();
    }

    pass AlphaPass
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_ALPHAENABLED_UI();
    }

    pass AlphaGradientPass // AlphaGradient
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_GRADIENT_UI();
    }

    pass NineSectorPass // AlphaGradient + Nine-Sector
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_NINESECTOR_UI();
    }
}
