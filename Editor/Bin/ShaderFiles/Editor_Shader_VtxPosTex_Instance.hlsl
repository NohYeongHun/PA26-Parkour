// ==============================
// * Sampler, State
// ==============================
sampler DefaultSampler = sampler_state
{
    filter = min_mag_mip_linear;
    AddressU = wrap;
    AddressV = wrap;
};

RasterizerState RS_Default
{
    FillMode = solid;
    CullMode = back;
    FrontCounterClockwise = false;
};

RasterizerState RS_Cull_None
{
    FillMode = solid;
    CullMode = none;
    FrontCounterClockwise = false;
};

DepthStencilState DSS_Default
{
    DepthEnable = true;
    DepthWriteMask = all;
    DepthFunc = LESS_EQUAL;
};

BlendState BS_Default
{
    BlendEnable[0] = false;
};

BlendState BS_AlphaBlend
{
    BlendEnable[0] = true;
    BlendEnable[1] = true;

    SrcBlend = SRC_ALPHA;
    DestBlend = INV_SRC_ALPHA;
    BlendOp = Add;
};


// ==============================
// * Global Variables
// ==============================


#define PI      3.14159265359f

// Basic Variables
matrix      g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;
Texture2D   g_Texture;
float       g_AlphaStrength;


// Gradient Variables
float2      g_ScreenLT = { 0.f, 0.f }, g_ScreenRB = { 1920.f, 1080.f };     // based on worldspace.         for discard by pos (esc menu, inventory, etc..)
bool        g_InverseScreenDiscard = false;                                 // 좌상단 끝이 0, 0 / 우하단 끝이 스크린X, 스크린Y 크기에 해당
float4      g_BlendToOuterWidth = { 0.f, 0.f, 0.f, 0.f };                   // (좌, 우, 상, 하) (left, right, top, bottom)
float2      g_ScreenSize = { 1920.f, 1080.f };


// Cutout Variables
float       g_CutoutAlphaDiscard = 0.3f;


// Nine-Sector Variables
float2      g_ImageSize = { 0.f, 0.f };
float2      g_SectorBorder = { 0.f, 0.f };                                  // based on local texcoord.     for 9sector
float       g_UIScale = 1.f;                                                // UI Scaler


// Variant UI Variables
#define UIFLAG_ERROR                0           // 플래그를 주지 않았을 때의 초기값
#define UIFLAG_COOLDOWN_CIRCLE      1           // 반시계방향으로 나타나는 쿨타임 구현용
#define UIFLAG_COOLDOWN_RECT        2           // 단순 사각형에서 내려오는 쿨타임 구현용
#define UIFLAG_END                  3
uint g_iVariantFlag = UIFLAG_ERROR;





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
    
    if (    ((startPos.x < originPos.x  && originPos.x < endPos.x   ) &&
            (startPos.y < originPos.y   && originPos.y < endPos.y   ))  )
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
    
    if      (originPos.x < border.x)                    // 왼쪽.
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
    
    
    if      (originPos.y < border.y)                    // 위쪽
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
struct VS_IN_INSTANCE
{
    float3 vPosition        : POSITION;
    float2 vTexcoord        : TEXCOORD0;
    
    float4 vSInstRight      : TEXCOORD1;
    float4 vSInstUp         : TEXCOORD2;
    float4 vSInstLook       : TEXCOORD3;
    float4 vSInstTrans      : TEXCOORD4;
    
    float2 vSInstCoordX     : TEXCOORD5;
    float2 vSInstCoordY     : TEXCOORD6;
    float2 vClipTexcoordX   : TEXCOORD7;
    float2 vClipTexcoordY   : TEXCOORD8;
    
    float4 mExtra0          : TEXCOORD9;
    float4 mExtra1          : TEXCOORD10;
    float4 mExtra2          : TEXCOORD11;
    float4 mExtra3          : TEXCOORD12;

};

struct VS_OUT
{
    float4 vPosition        : SV_POSITION;
    float2 vTexcoord        : TEXCOORD0;
    float4 vWorldPos        : TEXCOORD1;
    float4 vProjPos         : TEXCOORD2;
    
    float2 vSInstCoordX     : TEXCOORD3;
    float2 vSInstCoordY     : TEXCOORD4;
    float2 vClipTexcoordX   : TEXCOORD5;
    float2 vClipTexcoordY   : TEXCOORD6;
    
    float2 vSInstPos        : TEXCOORD7;
    float2 vSInstSca        : TEXCOORD8;
    
    float4 mExtra0          : TEXCOORD9;
    float4 mExtra1          : TEXCOORD10;
    float4 mExtra2          : TEXCOORD11;
    float4 mExtra3          : TEXCOORD12;
};


VS_OUT VS_INSTANCE(VS_IN_INSTANCE In)
{
    VS_OUT Out = (VS_OUT) 0;
    // [ 인스턴싱용 ] 각 인스턴스별 Vertex 의 Out 정의
    
    float4x4 matWV, matWVP;
    
    matWV = mul(g_WorldMatrix, g_ViewMatrix);
    matWVP = mul(matWV, g_ProjMatrix);
    
    
    float4x4 matAdditionalTransform = float4x4(
        In.vSInstRight,
        In.vSInstUp,
        In.vSInstLook,
        In.vSInstTrans
    );
    
    float4 vWorldPos = mul(float4(In.vPosition, 1.f), matAdditionalTransform);
    vWorldPos = mul(vWorldPos, matWVP);
    
    Out.vPosition = vWorldPos;
    Out.vTexcoord = In.vTexcoord;
    Out.vWorldPos = mul(float4(In.vPosition, 1.f), g_WorldMatrix);
    Out.vProjPos = Out.vPosition;
    
    Out.vSInstPos = In.vSInstTrans.xy;
    Out.vSInstSca = float2(length(In.vSInstRight.xyz), length(In.vSInstUp.xyz));
    // 이후 픽셀에서 사용
    
    // Pixel에서 사용 위해 바로 Output
    Out.vSInstCoordX = In.vSInstCoordX;
    Out.vSInstCoordY = In.vSInstCoordY;
    Out.vClipTexcoordX = In.vClipTexcoordX;
    Out.vClipTexcoordY = In.vClipTexcoordY;
    Out.mExtra0 = In.mExtra0;
    Out.mExtra1 = In.mExtra1;
    Out.mExtra2 = In.mExtra2;
    Out.mExtra3 = In.mExtra3;
    
    return Out;
}






// ==============================
// * Pixel Shader
// ==============================
struct PS_IN
{
    float4 vPosition        : SV_POSITION;
    float2 vTexcoord        : TEXCOORD0;
    float4 vWorldPos        : TEXCOORD1;
    float4 vProjPos         : TEXCOORD2;
    
    float2 vSInstCoordX     : TEXCOORD3;        // [각 인스턴스] 가 사용할 원본 텍스쳐 상의 Texcoord 정보 (아틀라스, 스프라이트 등 사용 목적)
    float2 vSInstCoordY     : TEXCOORD4;        // [각 인스턴스] 가 사용할 원본 텍스쳐 상의 Texcoord 정보 (아틀라스, 스프라이트 등 사용 목적)
    float2 vClipTexcoordX   : TEXCOORD5;        // [각 인스턴스] 가 사용할 본인이 차지하는 공간 상에서 Visible 하게 해 줄 범위. (체력 바 게이지 등에 사용 목적)
    float2 vClipTexcoordY   : TEXCOORD6;        // [각 인스턴스] 가 사용할 본인이 차지하는 공간 상에서 Visible 하게 해 줄 범위. (체력 바 게이지 등에 사용 목적)
    
    float2 vSInstPos        : TEXCOORD7;
    float2 vSInstSca        : TEXCOORD8;
    
    float4 mExtra0          : TEXCOORD9;
    float4 mExtra1          : TEXCOORD10;
    float4 mExtra2          : TEXCOORD11;
    float4 mExtra3          : TEXCOORD12;
};

struct PS_OUT
{
    float4 vColor           : SV_TARGET0;
};

// 여러 적들의 HP바를 한번에 그리는 등에 사용하기 위해, 인스턴스마다 제각각,
// 본인 좌표 및 크기를 기준으로 클리핑을 적용한다.
bool Calc_InstClip(float2 CoordPos, float2 InstPos, float2 InstScale, float2 ClipX /* 0~1 */, float2 ClipY /* 0~1 */) // true 일 시 Clip (discard)
{
}

/*

float2 vSInstCoordX     : TEXCOORD3;
float2 vSInstCoordY     : TEXCOORD4;
float2 vClipTexcoordX   : TEXCOORD5;
float2 vClipTexcoordY   : TEXCOORD6;

이 4가지 데이터를 이용하여 실제 적용되도록 만들기 필요.

*/


PS_OUT PS_MAIN(PS_IN In)
{
    // Apply InstCoord for atlas / sprite style
    PS_OUT Out = (PS_OUT) 0;
    float2 fixedUV = float2(lerp(In.vSInstCoordX.x, In.vSInstCoordX.y, In.vTexcoord.x), // 이걸로 In.vSInstCoord 범위에 따라.. 이용?
                            lerp(In.vSInstCoordY.x, In.vSInstCoordY.y, In.vTexcoord.y));
    
    // Apply ClipTexcoord for clipped ui. like as HP Bar
    // Calc Clip Space
    float2 clipX = float2 ( lerp(In.vSInstCoordX.x, In.vSInstCoordX.y, In.vClipTexcoordX.x),    // 예시로 텍스쳐를 0.2 ~ 0.8 범위만 쓰는데, 클립 범위는 0.5 ~ 1.0 이라면
                            lerp(In.vSInstCoordX.x, In.vSInstCoordX.y, In.vClipTexcoordX.y) );  // 0.2 ~ 0.8 범위 내에서의 0.5 및 1.0을 클립 범위로 삼음. ( result : 0.5 ~ 0.8 )
    float2 clipY = float2 ( lerp(In.vSInstCoordY.x, In.vSInstCoordY.y, In.vClipTexcoordY.x),
                            lerp(In.vSInstCoordY.x, In.vSInstCoordY.y, In.vClipTexcoordY.y) );
    // discard
    if (fixedUV.x < clipX.x || fixedUV.x > clipX.y ||
        fixedUV.y < clipY.x || fixedUV.y > clipY.y)
        discard;
    
    
        
    // In.vSInstCoordX.x 와 In.vSInstCoordX.y 사이의 값을 0~1로 생각하여
    // In.vClipTexcoordX.x, y 가 그 기준으로 밖에 있다면 discard.
    
    Out.vColor = g_Texture.Sample(DefaultSampler, fixedUV);
    
    return Out;
}

PS_OUT PS_CUTOUT_UI(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;
    float2 fixedUV = float2(lerp(In.vSInstCoordX.x, In.vSInstCoordX.y, In.vTexcoord.x),
                            lerp(In.vSInstCoordY.x, In.vSInstCoordY.y, In.vTexcoord.y));
    float2 clipX = float2 ( lerp(In.vSInstCoordX.x, In.vSInstCoordX.y, In.vClipTexcoordX.x),  
                            lerp(In.vSInstCoordX.x, In.vSInstCoordX.y, In.vClipTexcoordX.y) );
    float2 clipY = float2 ( lerp(In.vSInstCoordY.x, In.vSInstCoordY.y, In.vClipTexcoordY.x),
                            lerp(In.vSInstCoordY.x, In.vSInstCoordY.y, In.vClipTexcoordY.y) );
    if (fixedUV.x < clipX.x || fixedUV.x > clipX.y ||
        fixedUV.y < clipY.x || fixedUV.y > clipY.y)
        discard;
    
    Out.vColor = g_Texture.Sample(DefaultSampler, fixedUV);
        
    if (Out.vColor.a <= g_CutoutAlphaDiscard)
        discard;
    
    return Out;
}


PS_OUT PS_ALPHAENABLED_UI(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;
    float2 fixedUV = float2(lerp(In.vSInstCoordX.x, In.vSInstCoordX.y, In.vTexcoord.x),
                            lerp(In.vSInstCoordY.x, In.vSInstCoordY.y, In.vTexcoord.y));
    float2 clipX = float2(lerp(In.vSInstCoordX.x, In.vSInstCoordX.y, In.vClipTexcoordX.x),
                            lerp(In.vSInstCoordX.x, In.vSInstCoordX.y, In.vClipTexcoordX.y));
    float2 clipY = float2(lerp(In.vSInstCoordY.x, In.vSInstCoordY.y, In.vClipTexcoordY.x),
                            lerp(In.vSInstCoordY.x, In.vSInstCoordY.y, In.vClipTexcoordY.y));
    if (fixedUV.x < clipX.x || fixedUV.x > clipX.y ||
        fixedUV.y < clipY.x || fixedUV.y > clipY.y)
        discard;
    
    
    Out.vColor = g_Texture.Sample(DefaultSampler, fixedUV);
    Out.vColor.a = Out.vColor.a * (1.f - g_AlphaStrength);
    
    // 범위 내에 없으면 discard
    if (!Check_isInSpace(In.vPosition.xy, g_ScreenLT, g_ScreenRB, g_InverseScreenDiscard))
        discard;
        
    return Out;
}

PS_OUT PS_GRADIENT_UI(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;
    float2 fixedUV = float2(lerp(In.vSInstCoordX.x, In.vSInstCoordX.y, In.vTexcoord.x),
                            lerp(In.vSInstCoordY.x, In.vSInstCoordY.y, In.vTexcoord.y));
    float2 clipX = float2(lerp(In.vSInstCoordX.x, In.vSInstCoordX.y, In.vClipTexcoordX.x),
                            lerp(In.vSInstCoordX.x, In.vSInstCoordX.y, In.vClipTexcoordX.y));
    float2 clipY = float2(lerp(In.vSInstCoordY.x, In.vSInstCoordY.y, In.vClipTexcoordY.x),
                            lerp(In.vSInstCoordY.x, In.vSInstCoordY.y, In.vClipTexcoordY.y));
    if (fixedUV.x < clipX.x || fixedUV.x > clipX.y ||
        fixedUV.y < clipY.x || fixedUV.y > clipY.y)
        discard;

    
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
    
    
    float alphaRatioX = (g_InverseScreenDiscard)? 1.f : 0.f;
    float alphaRatioY = (g_InverseScreenDiscard)? 1.f : 0.f;
    
    //if (!g_InverseScreenDiscard)
    //{
        if      (g_BlendToOuterWidth.x > 0 || g_BlendToOuterWidth.y > 0) // Outer Gradient
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
    
        if      (g_BlendToOuterWidth.z > 0 || g_BlendToOuterWidth.w > 0) // Outer Gradient
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
    
   
    float alphaRatio        = max(alphaRatioX, alphaRatioY);
    float finalAlphaRatio   = (g_InverseScreenDiscard)? alphaRatio : (1.f - alphaRatio); // inverse 여부 반영    
    
     
    Out.vColor = g_Texture.Sample(DefaultSampler, fixedUV);
    Out.vColor.a = Out.vColor.a * (1.f - g_AlphaStrength);
    
    Out.vColor.a *= finalAlphaRatio;
    
    return Out;
}

PS_OUT PS_NINESECTOR_UI(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;
    float2 fixedUV = float2(lerp(In.vSInstCoordX.x, In.vSInstCoordX.y, In.vTexcoord.x),
                            lerp(In.vSInstCoordY.x, In.vSInstCoordY.y, In.vTexcoord.y));
    float2 clipX = float2(lerp(In.vSInstCoordX.x, In.vSInstCoordX.y, In.vClipTexcoordX.x),
                            lerp(In.vSInstCoordX.x, In.vSInstCoordX.y, In.vClipTexcoordX.y));
    float2 clipY = float2(lerp(In.vSInstCoordY.x, In.vSInstCoordY.y, In.vClipTexcoordY.x),
                            lerp(In.vSInstCoordY.x, In.vSInstCoordY.y, In.vClipTexcoordY.y));
    if (fixedUV.x < clipX.x || fixedUV.x > clipX.y ||
        fixedUV.y < clipY.x || fixedUV.y > clipY.y)
        discard;

    
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
    
    
    
    
    float2 finalUV;
    finalUV.x = lerp(In.vSInstCoordX.x, In.vSInstCoordX.y, resultUV.x);
    finalUV.y = lerp(In.vSInstCoordY.x, In.vSInstCoordY.y, resultUV.y);
    
    Out.vColor = g_Texture.Sample(DefaultSampler, finalUV);
    Out.vColor.a = Out.vColor.a * (1.f - g_AlphaStrength);
    
    Out.vColor.a *= finalAlphaRatio;
    
    return Out;
}


PS_OUT PS_VARIENT_UI(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;
    float2 fixedUV = float2(lerp(In.vSInstCoordX.x, In.vSInstCoordX.y, In.vTexcoord.x),
                            lerp(In.vSInstCoordY.x, In.vSInstCoordY.y, In.vTexcoord.y));
    float2 clipX = float2(lerp(In.vSInstCoordX.x, In.vSInstCoordX.y, In.vClipTexcoordX.x),
                            lerp(In.vSInstCoordX.x, In.vSInstCoordX.y, In.vClipTexcoordX.y));
    float2 clipY = float2(lerp(In.vSInstCoordY.x, In.vSInstCoordY.y, In.vClipTexcoordY.x),
                            lerp(In.vSInstCoordY.x, In.vSInstCoordY.y, In.vClipTexcoordY.y));
    if (fixedUV.x < clipX.x || fixedUV.x > clipX.y ||
        fixedUV.y < clipY.x || fixedUV.y > clipY.y)
        discard;
    
    Out.vColor = g_Texture.Sample(DefaultSampler, fixedUV);
    
    //Out.vColor = float4(1.f, 0.f, 1.f, 1.f);
    //return Out; // 플래그 지정 제대로 안했으면 마젠타 처리
    Out.vColor = lerp(Out.vColor, float4(1.f, 0.f, 1.f, 1.f), 0.5f);
    Out.vColor.a = Out.vColor.a * (1.f - g_AlphaStrength);

    return Out; // 플래그 지정 제대로 안했으면 마젠타 처리
    
    
    
    
    
    
    // bind variables
    float fCooldown = In.mExtra0.x; // 0 ~ 1. instance 0 은 e, 1 은 r 에 대응
    
    
    //switch (g_iVariantFlag)
    //{
    //    case UIFLAG_COOLDOWN_CIRCLE: // 1
    //    {
    //        // circle cd
    //        // g_fLeftCDRate 가 1 일때는 밝은 색으로
    //        // g_fLeftCDRate 가 0 일때는 경계가 반시계방향으로 돌며 점차 원래대로의 색으로 바뀌도록
    //        
    //        float2 center = float2(0.5f, 0.5f);
    //        float2 dir = normalize(fixedUV - center); // 중앙에서 목표 UV좌표로의 방향.
    //        float angle = atan2(dir.y, dir.x); // +x(3시) 방향 = 0, 반시계방향이 + 기준의 라디안 상대각도를 구함
    //        angle += PI / 2; // +90도를 줘서, 기존 3시 방향이었던 각도 기준을 12시로 전환
    //        if (angle < 0)
    //            angle += 2 * PI; // 정규화 ([-180 ~ 0], [0 ~ 180] to [180 ~ 360], [0 ~ 180])
    //
    //        float fCooldownAngle = 2 * PI * fCooldown; // 진행각도. cooldown 이 0~1 이므로 0도~360도로 치환됨.
    //
    //        if (angle <= fCooldownAngle)
    //        {
    //        // 이미 지난 부분은 원래의 색으로
    //            return Out;
    //        }
    //        else
    //        {
    //        // 지나지 않은 부분은 좀 더 하얀 색으로
    //            Out.vColor.rgb *= 1.2f;
    //            return Out;
    //        }
    //        
    //    }
    //    break;
    //    
    //    case UIFLAG_COOLDOWN_RECT: // 2
    //    {
    //    // rect cd. 
    //    // g_fLeftCDRate 가 1 일때는 어두운 색으로
    //    // g_fLeftCDRate 가 0 일때는 경계가 아래로 내려가며 밝아지도록
    //        if (fixedUV.y > fCooldown)
    //        {
    //        // 밝게 표시될 부분
    //            return Out;
    //        }
    //        else
    //        {
    //        // 어둡게 표시될 부분
    //            Out.vColor *= 0.8f;
    //            return Out;
    //        }
    //        
    //        
    //    }
    //    break;
    //    
    //    default:
    //    {
    //        //Out.vColor = float4(1.f, 0.f, 1.f, 1.f);
    //        //return Out; // 플래그 지정 제대로 안했으면 마젠타 처리
    //        Out.vColor = lerp(Out.vColor, float4(1.f, 0.f, 1.f, 1.f), 0.5f);
    //        Out.vColor.a = l
    //        return Out; // 플래그 지정 제대로 안했으면 마젠타 처리
    //    }
    //}
    
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
        VertexShader = compile vs_5_0 VS_INSTANCE();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN();
    }

    pass CutOutPass
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_INSTANCE();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_CUTOUT_UI();
    }

    pass AlphaPass
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_INSTANCE();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_ALPHAENABLED_UI();
    }

    pass AlphaGradientPass          // AlphaGradient
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_INSTANCE();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_GRADIENT_UI();
    }

    pass NineSectorPass             // AlphaGradient + Nine-Sector
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_INSTANCE();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_NINESECTOR_UI();
    }

    pass VariantUIPass // AlphaPass + a. for cooldown, etc. not designed for animation.
    { // 쿨타임 등의 용도로 사용할 특수한 경우용 짬통 pass.. flag로 내부에서 사용할 것 나눔      
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_INSTANCE();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_VARIENT_UI();
    }
}
