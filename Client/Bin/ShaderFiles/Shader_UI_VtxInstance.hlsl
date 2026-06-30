// UI용
#include "Engine_Shader_State.hlsli"

#define PI          3.14159265359f
#define _BOOL(x)    ((x) != 0.0f)
// ==============================
// * Global Variables
// ==============================

// Basic Variables
matrix g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;
Texture2D g_Texture;
float g_AlphaStrength;

Texture2D g_TextureExtra0;
Texture2D g_TextureExtra1;
Texture2D g_TextureExtra2;
Texture2D g_TextureExtra3;
float g_FXStrength;

// Gradient Variables
float2 g_ScreenLT = { 0.f, 0.f }, g_ScreenRB = { 1920.f, 1080.f };  // based on worldspace.         for discard by pos (esc menu, inventory, etc..)
bool g_InverseScreenDiscard = false;                                // 좌상단 끝이 0, 0 / 우하단 끝이 스크린X, 스크린Y 크기에 해당
float4 g_BlendToOuterWidth = { 0.f, 0.f, 0.f, 0.f };                // (좌, 우, 상, 하) (left, right, top, bottom)
float2 g_ScreenSize = { 1920.f, 1080.f };


// Cutout Variables
float g_CutoutAlphaDiscard = 0.3f;


// Nine-Sector Variables
float2 g_ImageSize = { 0.f, 0.f };
float2 g_SectorBorder = { 0.f, 0.f }; // based on local texcoord.     for 9sector
float g_UIScale = 1.f; // UI Scaler




// Variant UI Variables
#define UIFLAG_ERROR                0           // 플래그를 주지 않았을 때의 초기값
#define UIFLAG_COOLDOWN_CIRCLE      1           // 반시계방향으로 나타나는 쿨타임 구현용
#define UIFLAG_COOLDOWN_CIRCLE_ADV  2
#define UIFLAG_COOLDOWN_RECT        3           // 단순 사각형에서 내려오는 쿨타임 구현용
#define UIFLAG_PLAYER_HP            4           // 플레이어 HP용
#define UIFLAG_PLAYER_TRANSMIT      5  
#define UIFLAG_SIMPLEMASK           6
#define UIFLAG_ACTIVEFEEDBACK       7
#define UIFLAG_ENEMY_HP             8

#define UIFLAG_OVFL_PALETTE         9
#define UIFLAG_SIMPLE_COLORIZE      10
#define UIFLAG_WAVECIRCLE           11
//#define UIFLAG_.. distort? dessolve?

#define UIFLAG_END                  12

uint g_iVariantFlag = UIFLAG_ERROR;

//#define SCROLL_HP


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

float2 RotateUV(float2 uv, float angle, float2 center = float2(0.5f, 0.5f))
{
    float s = sin(angle);
    float c = cos(angle);

    // 중심 기준으로 이동
    float2 p = uv - center;

    // 2D 회전
    float2 r;
    r.x = p.x * c - p.y * s;
    r.y = p.x * s + p.y * c;

    // 다시 원래 좌표계로
    return r + center;
}

float linearstep(float a, float b, float x)
{
    return saturate((x - a) / (b - a));
}


// ==============================
// * Vertex Shader
// ==============================
struct VS_IN_INSTANCE
{
    float3 vPosition : POSITION;
    float2 vTexcoord : TEXCOORD0;
    
    float4 vSInstRight : TEXCOORD1;
    float4 vSInstUp : TEXCOORD2;
    float4 vSInstLook : TEXCOORD3;
    float4 vSInstTrans : TEXCOORD4;
    
    float2 vSInstCoordX : TEXCOORD5;
    float2 vSInstCoordY : TEXCOORD6;
    float2 vClipTexcoordX : TEXCOORD7;
    float2 vClipTexcoordY : TEXCOORD8;
    
    float4 mExtra0 : TEXCOORD9;
    float4 mExtra1 : TEXCOORD10;
    float4 mExtra2 : TEXCOORD11;
    float4 mExtra3 : TEXCOORD12;
};

struct VS_OUT
{
    float4 vPosition : SV_POSITION;
    float2 vTexcoord : TEXCOORD0;
    float4 vWorldPos : TEXCOORD1;
    float4 vProjPos : TEXCOORD2;
    
    float2 vSInstCoordX : TEXCOORD3;
    float2 vSInstCoordY : TEXCOORD4;
    float2 vClipTexcoordX : TEXCOORD5;
    float2 vClipTexcoordY : TEXCOORD6;
    
    float2 vSInstPos : TEXCOORD7;
    float2 vSInstSca : TEXCOORD8;
    
    float4 mExtra0 : TEXCOORD9;
    float4 mExtra1 : TEXCOORD10;
    float4 mExtra2 : TEXCOORD11;
    float4 mExtra3 : TEXCOORD12;
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


VS_OUT VS_INSTANCE_VARIANT(VS_IN_INSTANCE In)
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
    float2 vFinalTexcoord = In.vTexcoord;
    float4 vFinalExtra0 = In.mExtra0;
    float4 vFinalExtra1 = In.mExtra1;
    float4 vFinalExtra2 = In.mExtra2;
    float4 vFinalExtra3 = In.mExtra3;
    
    
    // Variant : 계산 전에 계산용 행렬에 값 반영하여 원하는 transform 을 적용
    switch (g_iVariantFlag)
    {
        case UIFLAG_PLAYER_TRANSMIT:
        {
            // ==============================
            // * [4] PlayerEnergy
            // ==============================
            
            // fHeight 만큼 scale 늘리고 fHeight / 2 만큼 y 올려서 보정?
            float fHeight = -In.mExtra2.y;
            
            matAdditionalTransform[1].xyz *= fHeight;                   // Y축
            matAdditionalTransform[3].y += (fHeight - 1) * 0.5f;        // 이따만큼 올림
            
        } break;
        case UIFLAG_ACTIVEFEEDBACK:
        {
            // ==============================
            // * [6] Active Feedback (button touch feedback)
            // ==============================
            float2 vDestScale = In.mExtra0.xy;      // 목표 배율 (최초 1배)
            float fStartAlpha = In.mExtra0.z;
            float fTimeRatio = In.mExtra0.w;
            
            float fDeltaX = fTimeRatio;         // 보간 방법 바꾸고싶다면 이 fDeltaX를 수정하는 식으로?
            
            float2 vCurScale; // 현재 스케일이 0이 아님에 주의. 인스턴스별 크기가 이미 적용된 값이 들어옴.
            vCurScale.x = length(matAdditionalTransform[0].xyz);
            vCurScale.y = length(matAdditionalTransform[1].xyz);
            
            float2 vTargetScale = lerp(vCurScale, vCurScale * vDestScale, fDeltaX);

            matAdditionalTransform[0].xyz *= (vTargetScale.x / vCurScale.x);
            matAdditionalTransform[1].xyz *= (vTargetScale.y / vCurScale.y);

        } break;
        case UIFLAG_ENEMY_HP:
        {
            // ==============================
            // * [7] Dynamic Enemy HP
            // ==============================
            float fYScale = In.mExtra2.y;
            float fElapsedTime = In.mExtra2.z;
            float fCoordSpeed = In.mExtra2.w;
            
            float2 vCurScale; // 현재 스케일이 0이 아님에 주의. 인스턴스별 크기가 이미 적용된 값이 들어옴.
            vCurScale.y = length(matAdditionalTransform[1].xyz);
            matAdditionalTransform[1].xyz *= fYScale;
            
            // texcoord pushing
            #ifdef SCROLL_HP
            vFinalTexcoord.x -= fElapsedTime * fCoordSpeed * 1.f;
            #endif
            vFinalExtra3.xy = In.vTexcoord;
            
        } break;
        case UIFLAG_OVFL_PALETTE:
        {
            // ==============================
            // * [8] Overflow Palette (UI MiniGame Gimmick)
            // ==============================
            float2  vChangeStartPos     = In.mExtra2.xy;        // 퍼지기가 시작될 지점
            bool    IsChanging          = _BOOL(In.mExtra2.z);
            float   fChangedRadius      = In.mExtra2.w;
            
            const float     fScaler = 1.2f;
            const float     fToRatio = 0.9f;
            
            if (!IsChanging) break;
            
            float2 vInstPos = In.vSInstTrans.xy;
            float2 vInstSca = float2(length(In.vSInstRight.xyz), length(In.vSInstUp.xyz));
            
            float fChangeRatio; //
                
            float2 vVerticesPos[4] = {
                { vInstPos.x - vInstSca.x / 2.f, vInstPos.y + vInstSca.y / 2.f },   // LT 
                { vInstPos.x + vInstSca.x / 2.f, vInstPos.y - vInstSca.y / 2.f },   // RT 
                { vInstPos.x + vInstSca.x / 2.f, vInstPos.y - vInstSca.y / 2.f },   // RB 
                { vInstPos.x - vInstSca.x / 2.f, vInstPos.y + vInstSca.y / 2.f }    // LB 
            };
            float fDistPerVertices[4] = {
                length(vVerticesPos[0] - vChangeStartPos),
                length(vVerticesPos[1] - vChangeStartPos),    
                length(vVerticesPos[2] - vChangeStartPos),
                length(vVerticesPos[3] - vChangeStartPos)
            };
                
            uint iVertexIndex_Nearest = 0;
            uint iVertexIndex_MostFar = 0;
            for (uint i = 0; i < 4; i++)
            {
                if (i == 0) continue;
                    
                if (fDistPerVertices[iVertexIndex_Nearest] > fDistPerVertices[i])   iVertexIndex_Nearest = i;
                if (fDistPerVertices[iVertexIndex_MostFar] < fDistPerVertices[i])   iVertexIndex_MostFar = i;
            }
                
            float2 vVertex_Nearest = vVerticesPos[iVertexIndex_Nearest];
            float2 vVertex_MostFar = vVerticesPos[iVertexIndex_MostFar];
                
            float fRadTo_PosNearest = fDistPerVertices[iVertexIndex_Nearest];
            float fRadTo_PosMostFar = fDistPerVertices[iVertexIndex_MostFar];
                
            fChangeRatio = saturate(smoothstep(         // <<<<<<<<<<<<<<<<<<<<<<
                fRadTo_PosNearest,
                fRadTo_PosMostFar,
                fChangedRadius                
            ));
            if (fChangeRatio > fToRatio) break;
            
            
            float fMaxTimingRatio = fToRatio / 2.f;     // 0->0.1 : 0->1, 0.1->0.2 : 1->0
            float2 resultScale;
            resultScale.x = (fChangeRatio <= fMaxTimingRatio) ?
                smoothstep(0.f, fMaxTimingRatio, fChangeRatio)  :           // 0 0.1 0~0.2
                smoothstep(fToRatio, fMaxTimingRatio, fChangeRatio);
            resultScale.y = (fChangeRatio <= fMaxTimingRatio) ? 
                smoothstep(0.f, fMaxTimingRatio, fChangeRatio)  :
                smoothstep(fToRatio, fMaxTimingRatio, fChangeRatio);
            
            matAdditionalTransform[0][0] *= lerp(1.0f, fScaler, resultScale.x);;
            matAdditionalTransform[1][1] *= lerp(1.0f, fScaler, resultScale.y);;
            
        } break;
    }
    
    
    
    
    float4 vWorldPos = mul(float4(In.vPosition, 1.f), matAdditionalTransform);
    vWorldPos = mul(vWorldPos, matWVP);
    
    
    
    
    
    
    
    
    
    Out.vPosition = vWorldPos;
    Out.vTexcoord = vFinalTexcoord;
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
    Out.mExtra0 = vFinalExtra0;
    Out.mExtra1 = vFinalExtra1;
    Out.mExtra2 = vFinalExtra2;
    Out.mExtra3 = vFinalExtra3;
    
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
    
    float2 vSInstCoordX : TEXCOORD3; // [각 인스턴스] 가 사용할 원본 텍스쳐 상의 Texcoord 정보 (아틀라스, 스프라이트 등 사용 목적)
    float2 vSInstCoordY : TEXCOORD4; // [각 인스턴스] 가 사용할 원본 텍스쳐 상의 Texcoord 정보 (아틀라스, 스프라이트 등 사용 목적)
    float2 vClipTexcoordX : TEXCOORD5; // [각 인스턴스] 가 사용할 본인이 차지하는 공간 상에서 Visible 하게 해 줄 범위. (체력 바 게이지 등에 사용 목적)
    float2 vClipTexcoordY : TEXCOORD6; // [각 인스턴스] 가 사용할 본인이 차지하는 공간 상에서 Visible 하게 해 줄 범위. (체력 바 게이지 등에 사용 목적)
    
    float2 vSInstPos : TEXCOORD7;
    float2 vSInstSca : TEXCOORD8;
    
    float4 mExtra0 : TEXCOORD9;
    float4 mExtra1 : TEXCOORD10;
    float4 mExtra2 : TEXCOORD11;
    float4 mExtra3 : TEXCOORD12;
};

struct PS_OUT
{
    float4 vColor : SV_TARGET0;
};

// 여러 적들의 HP바를 한번에 그리는 등에 사용하기 위해, 인스턴스마다 제각각,
// 본인 좌표 및 크기를 기준으로 클리핑을 적용한다.
//bool Calc_InstClip(float2 CoordPos, float2 InstPos, float2 InstScale, float2 ClipX /* 0~1 */, float2 ClipY /* 0~1 */) // true 일 시 Clip (discard)
//{
//}

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
    float2 clipX = float2(lerp(In.vSInstCoordX.x, In.vSInstCoordX.y, In.vClipTexcoordX.x), // 예시로 텍스쳐를 0.2 ~ 0.8 범위만 쓰는데, 클립 범위는 0.5 ~ 1.0 이라면
                            lerp(In.vSInstCoordX.x, In.vSInstCoordX.y, In.vClipTexcoordX.y)); // 0.2 ~ 0.8 범위 내에서의 0.5 및 1.0을 클립 범위로 삼음. ( result : 0.5 ~ 0.8 )
    float2 clipY = float2(lerp(In.vSInstCoordY.x, In.vSInstCoordY.y, In.vClipTexcoordY.x),
                            lerp(In.vSInstCoordY.x, In.vSInstCoordY.y, In.vClipTexcoordY.y));
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
    float2 clipX = float2(lerp(In.vSInstCoordX.x, In.vSInstCoordX.y, In.vClipTexcoordX.x),
                            lerp(In.vSInstCoordX.x, In.vSInstCoordX.y, In.vClipTexcoordX.y));
    float2 clipY = float2(lerp(In.vSInstCoordY.x, In.vSInstCoordY.y, In.vClipTexcoordY.x),
                            lerp(In.vSInstCoordY.x, In.vSInstCoordY.y, In.vClipTexcoordY.y));
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
    float2 clipX= float2(lerp(In.vSInstCoordX.x, In.vSInstCoordX.y, In.vClipTexcoordX.x),
                            lerp(In.vSInstCoordX.x, In.vSInstCoordX.y, In.vClipTexcoordX.y));
    float2 clipY = float2(lerp(In.vSInstCoordY.x, In.vSInstCoordY.y, In.vClipTexcoordY.x),
                            lerp(In.vSInstCoordY.x, In.vSInstCoordY.y, In.vClipTexcoordY.y));
    
    
    #ifdef SCROLL_HP
    switch (g_iVariantFlag)
    {
        case UIFLAG_ENEMY_HP: // 7
        {
             
            
        } break;
        default :
         {
    #endif
            if (fixedUV.x < clipX.x || fixedUV.x > clipX.y ||
                fixedUV.y < clipY.x || fixedUV.y > clipY.y)
                discard;
            
            
    #ifdef SCROLL_HP       
        } break;
    }
    #endif
    
    Out.vColor = g_Texture.Sample(DefaultSampler, fixedUV);
    Out.vColor.a = Out.vColor.a * (1.f - g_AlphaStrength);
    

    
    
    switch (g_iVariantFlag)
    {
        case UIFLAG_COOLDOWN_CIRCLE : // 1
        {
            // ==============================
            // * [1] Circle Cooldown
            // ==============================
            // [CDRATE] [COLORMUL_1] [COLORMUL_2] [IS_USECUSTOMCOLOR]
            // [COLOR.x] [COLOR.y] [COLOR.z] [COLOR.w]
            // [STARTRATIO(DEG)] [ISUSENOISE] [ELAPSEDTIME] [UVSCROLLSPEED]
            // [MASKCOLOR.x] [MASKCOLOR.y] [MASKCOLOR.z] [MASKCOLOR.w]
            // ==============================
            float   fCooldown           = In.mExtra0.x; // 0 ~ 1.
            float   fColorMul1          = In.mExtra0.y;
            float   fColorMul2          = In.mExtra0.z;
            bool    isUseCustomColor    = _BOOL(In.mExtra0.w);
            float4  vCustomColor        = In.mExtra1.rgba;
            float   fStartRatio         = In.mExtra2.x;                         // 각도(degree) 및 시계방향 기준. 0 기준 12시부터 시작.
            
            bool    isUseNoise          = _BOOL(In.mExtra2.y);
            float   fElapsedTime        = In.mExtra2.z;
            float   fUVScrollSpeed      = In.mExtra2.w;
            float4  vMaskColor          = In.mExtra3;
            
            // g_fLeftCDRate 가 1 일때는 밝은 색으로
            // g_fLeftCDRate 가 0 일때는 경계가 반시계방향으로 돌며 점차 원래대로의 색으로 바뀌도록
            
            float2 localUV;
            localUV.x = saturate((fixedUV.x - In.vSInstCoordX.x) / (In.vSInstCoordX.y - In.vSInstCoordX.x));
            localUV.y = saturate((fixedUV.y - In.vSInstCoordY.x) / (In.vSInstCoordY.y - In.vSInstCoordY.x));
            
            float2 center = float2(0.5f, 0.5f);
            float2 dir = normalize(localUV - center);   // 중앙에서 목표 UV좌표로의 방향.
            float angle = atan2(dir.y, dir.x);          // +x(3시) 방향 = 0, 반시계방향이 + 기준의 라디안 상대각도를 구함
            angle += ((PI / 2.f) * (1 - fStartRatio / 90.f)); // +90도를 줘서, 기존 3시 방향이었던 각도 기준을 12시로 전환
            if (angle < 0) angle += 2.f * PI;             // 정규화 ([-180 ~ 0], [0 ~ 180] to [180 ~ 360], [0 ~ 180])
    
            float fCooldownAngle = 2.f * PI * fCooldown;  // 진행각도. cooldown 이 0~1 이므로 0도~360도로 치환됨.
            
            Out.vColor = g_Texture.Sample(DefaultSampler, fixedUV);
            
            
            if (angle <= fCooldownAngle)
            {
                // 이미 지난 부분은 원래의 색으로
                Out.vColor.rgba *= fColorMul1;
                if (isUseCustomColor)
                    Out.vColor *= vCustomColor;
                
                Out.vColor.a *= (1 - g_AlphaStrength);
            }
            else
            {
                // 지나지 않은 부분은 좀 더 하얀 색으로
                //if (fCooldown != 0.f)
                    Out.vColor.rgba *= fColorMul2;
                if (isUseCustomColor)
                    Out.vColor *= vCustomColor;
                
                Out.vColor.a *= (1 - g_AlphaStrength);
            }
            
            if (isUseNoise)
            {
                float2 originUV = In.vTexcoord;
                float2 offset = fElapsedTime * fUVScrollSpeed;
                
                float maskWeight = g_TextureExtra0.Sample(DefaultSampler, frac(originUV + offset)).x;
                
                Out.vColor.rgb = (1.f - maskWeight) * Out.vColor.rgb + (maskWeight) * vMaskColor.rgb;
                Out.vColor.a = (1.f - maskWeight) * Out.vColor.a + (maskWeight) * vMaskColor.a * Out.vColor.a;
            }
            
            
            return Out;
        } break;
        case UIFLAG_COOLDOWN_CIRCLE_ADV: // 2
        {
            // ==============================
            // * [2.] Circle Cooldown
            // ==============================
            // * matrix info [size : 2] (skillbtn_e, skillbtn_r)
            // [CDRATE] [COLORMUL_1] [COLORMUL_2] [IS_USECUSTOMCOLOR]
            // [COLOR.x] [COLOR.y] [COLOR.z] [COLOR.w]
            // [STARTRATIO(DEG)] [ISUSENOISE] [ELAPSEDTIME] [UVSCROLLSPEED]
            // >> MASK : [COLOR.x] [COLOR.y] [COLOR.z] [COLOR.w]
            // ==============================
            // (MASK) : Extra0
            // ==============================
            float fCooldown = In.mExtra0.x; // 0 ~ 1.
            float fColorMul1 = In.mExtra0.y;
            float fColorMul2 = In.mExtra0.z;
            bool isUseCustomColor = _BOOL(In.mExtra0.w);
            float4 vCustomColor = In.mExtra1.rgba;
            float fStartRatio = In.mExtra2.x;       // 각도(degree) 및 시계방향 기준. 0 기준 12시부터 시작.
            
            bool isUseNoise = _BOOL(In.mExtra2.y);
            float fElapsedTime = In.mExtra2.z;
            float fUVScrollSpeed = In.mExtra2.w;
            float4 vMaskColor = In.mExtra3;
            
            
            
            
            // g_fLeftCDRate 가 1 일때는 밝은 색으로
            // g_fLeftCDRate 가 0 일때는 경계가 반시계방향으로 돌며 점차 원래대로의 색으로 바뀌도록
            
            float2 localUV;
            localUV.x = saturate((fixedUV.x - In.vSInstCoordX.x) / (In.vSInstCoordX.y - In.vSInstCoordX.x));
            localUV.y = saturate((fixedUV.y - In.vSInstCoordY.x) / (In.vSInstCoordY.y - In.vSInstCoordY.x));
            
            float2 center = float2(0.5f, 0.5f);
            float2 dir = normalize(localUV - center);   // 중앙에서 목표 UV좌표로의 방향.
            float angle = atan2(dir.y, dir.x);          // +x(3시) 방향 = 0, 반시계방향이 + 기준의 라디안 상대각도를 구함
            angle += ((PI / 2.f) * (1 - fStartRatio / 90.f)); // +90도를 줘서, 기존 3시 방향이었던 각도 기준을 12시로 전환
            if (angle < 0) angle += 2.f * PI;             // 정규화 ([-180 ~ 0], [0 ~ 180] to [180 ~ 360], [0 ~ 180])
    
            float fCooldownAngle = 2.f * PI * fCooldown;  // 진행각도. cooldown 이 0~1 이므로 0도~360도로 치환됨.
            
            //Out.vColor = g_Texture.Sample(DefaultSampler, fixedUV);
            Out.vColor = In.mExtra1;
            
            if (angle <= fCooldownAngle)
            {
                // 이미 지난 부분은 원래의 색으로
                Out.vColor.rgba *= fColorMul1;
                if (isUseCustomColor)
                    Out.vColor *= vCustomColor * fColorMul1;
                
                Out.vColor.a *= (1 - g_AlphaStrength);
            }
            else
            {
                // 지나지 않은 부분은 좀 더 하얀 색으로
                if (fCooldown != 0.f)
                    Out.vColor.rgba *= fColorMul2;
                if (isUseCustomColor)
                    Out.vColor *= vCustomColor *  fColorMul2;
                
                Out.vColor.a *= (1 - g_AlphaStrength);
            }
            
            if (isUseNoise)
            {
                float2 originUV = In.vTexcoord;
                float2 offset = fElapsedTime * fUVScrollSpeed;
                
                float maskWeight = g_TextureExtra0.Sample(DefaultSampler, frac(originUV + offset)).x;
                
                Out.vColor.rgb = (1.f - maskWeight) * Out.vColor.rgb + (maskWeight) * vMaskColor.rgb;
                Out.vColor.a = (1.f - maskWeight) * Out.vColor.a + (maskWeight) * vMaskColor.a * Out.vColor.a;
            }
            
            
                float fDistUV = length(localUV - float2(0.5f, 0.5f));
            
                static const float fMinDistUV = 0.29f;
                static const float fMaxDistUV = 0.37f;
                static const float fClipDistUV = 0.43f;

                float fAlphaMultiplier_byDist = smoothstep(fMinDistUV, fMaxDistUV, fDistUV);
                Out.vColor.a *= fAlphaMultiplier_byDist;

                if (fDistUV > fClipDistUV)
                    Out.vColor.a *= 1.f - clamp((fDistUV - fClipDistUV) / 0.01f, 0.f, 1.f);
            
            
            
            return Out;
        } break;
        
        case UIFLAG_COOLDOWN_RECT   : // 2
        {
            // ==============================
            // * [2] Rect Cooldown (for PartyFrame)
            // ==============================
            // * matrix info [size : 3] (frame_rover, frame_augusta, frame_galbrena)
            // [CDRATE] [COLORMUL_1] [COLORMUL_2] -
            // ==============================
            float fCooldown = In.mExtra0.x; // 0 ~ 1.
            float fColorMul1 = In.mExtra0.y;
            float fColorMul2 = In.mExtra0.z;
            
            
            // g_fLeftCDRate 가 1 일때는 어두운 색으로
            // g_fLeftCDRate 가 0 일때는 경계가 아래로 내려가며 밝아지도록
            if (fixedUV.y < fCooldown)
            // 밝게 표시될 부분
                return Out;
            else
            // 어둡게 표시될 부분
                Out.vColor *= fColorMul2;
            
            Out.vColor.a *= (1 - g_AlphaStrength);
            return Out;
        }
        case UIFLAG_PLAYER_HP:           // 3
        {
            // ==============================
            // * [3] PlayerHP
            // ==============================
            // * matrix info [size : 2] (hp_background, hp_normal)
            // [COLORGRAD1.x] [COLORGRAD1.y] [COLORGRAD1.z] [COLORGRAD1.w]
            // [COLORGRAD2.x] [COLORGRAD2.y] [COLORGRAD2.z] [COLORGRAD2.w]
            // [HPRATE] [ISUSENOISE] [ELAPSEDTIME] [UVSCROLLSPEED]
            // >> MASK : [COLOR.x] [COLOR.y] [COLOR.z] [COLOR.w]
            // ==============================
            // (MASK) : Extra0
            // ==============================
            vector vColor1 = In.mExtra0.xyzw;
            vector vColor2 = In.mExtra1.xyzw;
            float fHPRatio = saturate(In.mExtra2.x);
            
            bool isUseNoise = _BOOL(In.mExtra2.y);
            float fElapsedTime = In.mExtra2.z;
            float fUVScrollSpeed = In.mExtra2.w;
            float4 vMaskColor = In.mExtra3;
            
            
            
            // 9sector.. 
            float2 vSize = {
                length(g_WorldMatrix[0].xyz) * g_UIScale,
                length(g_WorldMatrix[1].xyz) * g_UIScale,
            };
            
            float2 border = g_SectorBorder * g_UIScale;
            float2 localPos = In.vTexcoord * vSize;
                
            float2 resultUV = Calc_NineSectorUV(localPos, vSize, border, g_ImageSize); // calced
            float2 finalUV;
            finalUV.x = lerp(In.vSInstCoordX.x, In.vSInstCoordX.y, resultUV.x);
            finalUV.y = lerp(In.vSInstCoordY.x, In.vSInstCoordY.y, resultUV.y);
            Out.vColor = g_Texture.Sample(DefaultSampler, finalUV);
            // =====
            
            float2 HPclipX = float2(lerp(In.vSInstCoordX.x, In.vSInstCoordX.y, 0.f),
                                    lerp(In.vSInstCoordX.x, In.vSInstCoordX.y, fHPRatio));
            
            if (fixedUV.x < HPclipX.x || fixedUV.x > HPclipX.y)
                discard;
            
            //Out.vColor.rgb = vColor.rgb;
            Out.vColor.rgb = lerp(vColor1, vColor2, fixedUV.x).rgb;
            Out.vColor.a = Out.vColor.a * lerp(vColor1, vColor2, fixedUV.x).a * (1 - g_AlphaStrength);
            
            
            // 최종에 노이즈 반영. 다만 픽셀 위치를 반영해야.
            if (isUseNoise)
            {
                float2 originUV = In.vTexcoord;
                float2 offset = fElapsedTime * fUVScrollSpeed;
                //float2 worldPosAppliedUV = originUV / In.vWorldPos.xy;
                
                float2 textureRatio = float2(fwidth(originUV.x), fwidth(originUV.y));
                float ratioX = textureRatio.y / (textureRatio.x + 0.00001f);
                
                float2 calcedUV = originUV;
                calcedUV.x *= ratioX;

                float noiseScale = 0.15f;
                float maskWeight = g_TextureExtra0.Sample(DefaultSampler, frac(noiseScale * calcedUV + offset)).x;
                
                Out.vColor.rgb = (1.f - maskWeight) * Out.vColor.rgb + (maskWeight) * vMaskColor.rgb;
                Out.vColor.a = (1.f - maskWeight) * Out.vColor.a + (maskWeight) * vMaskColor.a * Out.vColor.a;
            }
            
            
            
            
            
            
            return Out;
        } break;
        case UIFLAG_PLAYER_TRANSMIT:      // 4
        {
            // ==============================
            // * [4] PlayerEnergy
            // ==============================
            // * matrix info [size : 41 * 3] (energy * 41, background * 41, static * 41)
            // [COLORGRAD1.x] [COLORGRAD1.y] [COLORGRAD1.z] [COLORGRAD1.w]
            // [COLORGRAD2.x] [COLORGRAD2.y] [COLORGRAD2.z] [COLORGRAD2.w]
            // [VISIBLE] [HEIGHT] -
            // ==============================
            vector vColor1 = In.mExtra0.rgba;
            vector vColor2 = In.mExtra1.rgba;
            bool isVisible = _BOOL(In.mExtra2.x);
            float fHeight = In.mExtra2.y;
            // border는 다 같은 이미지 여러 개 쓸 테니 여기 말고 전역으로 받는게 좋을 듯
            
            // 픽셀 자체의 크기는 픽셀 셰이더에서 제어해야 할 듯
            // 여기서는 height 값에 맞춰 9섹터만 지원하는 식으로
            
            if (!isVisible)
                discard;
            
            // 9sector.. 123
            float2 vSize = {
                length(g_WorldMatrix[0].xyz) * g_UIScale,
                length(g_WorldMatrix[1].xyz) * g_UIScale,
            };
            
            float2 border = g_SectorBorder * g_UIScale;
            float2 localPos = In.vTexcoord * vSize;
                
            float2 resultUV = Calc_NineSectorUV(localPos, vSize, border, g_ImageSize); // calced
            float2 finalUV;
            finalUV.x = lerp(In.vSInstCoordX.x, In.vSInstCoordX.y, resultUV.x);
            finalUV.y = lerp(In.vSInstCoordY.x, In.vSInstCoordY.y, resultUV.y);
            Out.vColor = g_Texture.Sample(DefaultSampler, finalUV);
            // =====
            
            
            //Out.vColor = float4(1.f, 0.f, 1.f, 1.f);
            Out.vColor.rgb  = Out.vColor.rgb * lerp(vColor1, vColor2, fixedUV.y).rgb; // 색상 추가
            Out.vColor.a    = saturate(Out.vColor.a * 1.5f);
            Out.vColor.a    = Out.vColor.a * lerp(vColor1, vColor2, fixedUV.x).a * (1 - g_AlphaStrength) * 2.f;
            
            return Out;
        } break;
        case UIFLAG_SIMPLEMASK :        // 5. T_MaskCircle.png
        {
            // ==============================
            // * [5] SimpleMask (for SkillIcon BG)
            // ==============================
            // * matrix info [size : ~5]
            // [COLOR.x] [COLOR.y] [COLOR.z] [COLOR.w]
            // [IS_ACTIVE]                                  // 필요 시 조건 추가
            // ==============================
            
            // rgb 의 평균값만큼 색을 준다.
            // rgb 의 평균값이 255에 가까우면 alpha가 1에 가까워진다.
            float4 vColor = In.mExtra0.rgba;
            bool isActive = _BOOL(In.mExtra1.x);
            
            if (!isActive)
                discard;
            
            float fAverageColor = (Out.vColor.x + Out.vColor.y + Out.vColor.z) / 3.f;
            float4 vAppliedColor = vColor * fAverageColor;
            vAppliedColor.a = fAverageColor * vColor.a * Out.vColor.a;
            
            Out.vColor = vAppliedColor;
            return Out;
        } break;
        case UIFLAG_ACTIVEFEEDBACK :    // 6
        {
            // ==============================
            // * [6] Active Feedback (button touch feedback)
            // ==============================
            // * matrix info [size : ~5? controls on hud]
            // [DESTSCALE.x] [DESTSCALE.y] [STARTALPHA] [TIMERATIO]
            // [COLOR.x] [COLOR.y] [COLOR.z] [COLOR.w]
            // ==============================
            float2 vDestScale = In.mExtra0.xy;
            float fStartAlpha = In.mExtra0.z;
            float fTimeRatio = In.mExtra0.w;
            float4 vColor = In.mExtra1.rgba;
            
            float fDeltaX = fTimeRatio;         // 보간 방법 바꾸고싶다면 이 fDeltaX를 수정하는 식으로?
            
            float fAverageColor = (Out.vColor.x + Out.vColor.y + Out.vColor.z) / 3.f;
            float4 vAppliedColor = vColor * fAverageColor;
            vAppliedColor.a = fAverageColor * vColor.a * Out.vColor.a;
            
            Out.vColor.a = vAppliedColor.a * lerp(1.f - fStartAlpha, 0.f, fDeltaX);
            
            return Out;
        } break;
        case UIFLAG_ENEMY_HP :          // 7
        {
            // ==============================
            // * [7] Dynamic Enemy HP
            // ==============================
            // * matrix info [size : dynamic] (per mobs)
            // [COLORGRAD1.x] [COLORGRAD1.y] [COLORGRAD1.z] [COLORGRAD1.w]
            // [COLORGRAD2.x] [COLORGRAD2.y] [COLORGRAD2.z] [COLORGRAD2.w]
            // [ALPHA] [YSCALE(VS)] [ELAPSEDTIME(VS)] [COORDSPEED(VS)]
            // [ORIGINCOORD(fromVS).xy]
            // ==============================
            vector vColor1 = In.mExtra0.xyzw;
            vector vColor2 = In.mExtra1.xyzw;
            float fAlpha = saturate(In.mExtra2.x);
            float2 vOriginCoord = In.mExtra3.xy;
            
            // 9sector.. 
            float2 vSize = {
                length(g_WorldMatrix[0].xyz) * g_UIScale,
                length(g_WorldMatrix[1].xyz) * g_UIScale,
            };
            
            float2 border = g_SectorBorder * g_UIScale;
            float2 localPos = In.vTexcoord * vSize;
                
            Out.vColor = g_Texture.Sample(DefaultSampler, In.vTexcoord);
            
            // =====
            
            // 원본 coord (vOriginCoord) 기준으로 discard. (밀린 coord 가 아닌 원본 coord가 필요해서 따로 정의)
            float2 fixedUV  = float2(lerp(In.vSInstCoordX.x, In.vSInstCoordX.y, vOriginCoord.x),
                                    lerp(In.vSInstCoordY.x, In.vSInstCoordY.y, vOriginCoord.y));
            float2 clipX    = float2(lerp(In.vSInstCoordX.x, In.vSInstCoordX.y, In.vClipTexcoordX.x),
                                    lerp(In.vSInstCoordX.x, In.vSInstCoordX.y, In.vClipTexcoordX.y));
            float2 clipY    = float2(lerp(In.vSInstCoordY.x, In.vSInstCoordY.y, In.vClipTexcoordY.x),
                                    lerp(In.vSInstCoordY.x, In.vSInstCoordY.y, In.vClipTexcoordY.y));
            
            float2 fixedUV_Flip     = float2(1.0f - fixedUV.x, fixedUV.y);
            
            if (fixedUV.x < clipX.x || fixedUV.x > clipX.y ||
                fixedUV.y < clipY.x || fixedUV.y > clipY.y)
                Out.vColor.a = 0.f;         // 추가적으로 그릴 수 있으니 discard 가 아닌, 알파로
            
            // =====

            float fEdgeAlphaWidth = 0.2f;
            
            //Out.vColor.rgb = vColor.rgb;
            Out.vColor.rgb = lerp(vColor1, vColor2, fixedUV.x).rgb;
            Out.vColor.a = Out.vColor.a * lerp(vColor1, vColor2, fixedUV.x).a * (1 - g_AlphaStrength) * fAlpha;
            
            //#ifdef SCROLL_HP
            float fEdgeAlpha = saturate(min(vOriginCoord.x /fEdgeAlphaWidth, (1.0f - vOriginCoord.x) / fEdgeAlphaWidth));
            Out.vColor.a *= fEdgeAlpha;
            //#endif
            
            //Out.vColor.rgba = g_Texture.Sample(DefaultSampler, In.vTexcoord);
            return Out;
        } break;
        case UIFLAG_OVFL_PALETTE :      // 8
        {
            // ==============================
            // * [8] Overflow Palette (UI MiniGame Gimmick)
            // ==============================
            // * matrix info [size : 80] (10 * 8. per blocks)
            // [COLORCURR.x] [COLORCURR.y] [COLORCURR.z] [COLORCURR.w]
            // [COLORDEST.x] [COLORDEST.y] [COLORDEST.z] [COLORDEST.w]
            // [CHGFRMPOS.x] [CHGFRMPOS.y] [IS_CHANGING] [CHNG_RADIUS] << 이거 timedelta 대신 쓸 수 있을 것 같음
            // [EXIMGSIZE.x] [EXIMGSIZE.y]
            // 텍스쳐를 하나 더 받아와서
            // 현재 winsize 및 inst transform (pos, sca) 기준으로 uv를 적절히 슬라이싱하여 적용하고
            // 색상을 흑백화 및 컬러링해서 out. 하면 될 것 같기도? 아닌가
            // ==============================
            // (MASK) : Extra0
            // ==============================
            
            float4  vCurrColor          = In.mExtra0.xyzw;
            float4  vDestColor          = In.mExtra1.xyzw;
            float2  vChangeStartPos     = In.mExtra2.xy;        // 퍼지기가 시작될 지점
            bool    IsChanging          = _BOOL(In.mExtra2.z);
            float   fChangedRadius      = In.mExtra2.w;
            int2    vEXImageSize        = In.mExtra3.xy;
            
            // 마스크 이미지 알파 적용
            float4 vMaskColor = g_Texture.Sample(DefaultSampler, In.vTexcoord);
            Out.vColor.a = vMaskColor.r;
            
            // 스크린 크기를 기준으로 uv를 반영한 좌표를 만듦.
        //#ifdef CHANGE_BYCIRCLE
            float2 vScreenX = { In.vSInstPos.x - In.vSInstSca.x / 2.f, In.vSInstPos.x + In.vSInstSca.x / 2.f };
            float2 vScreenY = { In.vSInstPos.y - In.vSInstSca.y / 2.f, In.vSInstPos.y + In.vSInstSca.y / 2.f };
                
            In.vTexcoord;   // 이게 인스턴스 기준 현재 포커싱중인 좌표
            float2 vFixedScreenPos = { 
                lerp(vScreenX.x, vScreenX.y, In.vTexcoord.x),
                lerp(vScreenY.x, vScreenY.y, 1.f - In.vTexcoord.y)
            };
        //#endif
            
            if (IsChanging) // 변화중
            {   
                // 스크린좌표로 변환하고, 현재 포커싱중인 점이 이를 지나면 색이 바뀌게끔..
                
                #ifndef CHANGE_BYCIRCLE
                float2 vSingleInstancePos = In.vSInstPos; ;           // 지금 이거 단순 인스턴스의 한 점을 기준삼는거라, 점 닿자마자 확 바뀌는 듯
                #endif
                
                #ifdef CHANGE_BYCIRCLE
                float2 vSingleInstancePos = vFixedScreenPos; //In.vSInstPos;           // 지금 이거 단순 인스턴스의 한 점을 기준삼는거라, 점 닿자마자 확 바뀌는 듯
                #endif
                
                float fLengthFromStartPos = length(vSingleInstancePos - vChangeStartPos);
                
                float4 vTargetColor;
                
                if (fLengthFromStartPos <= fChangedRadius)      // 가깝다! -> 변해야 됨
                    vTargetColor = vDestColor;
                else                                            // 멀다! -> 아직 변하면 안됨
                    vTargetColor = vCurrColor;
                    
                Out.vColor.rgb = vTargetColor.rgb;
                Out.vColor.a = vTargetColor.a * Out.vColor.a;
            }
            else            // 평시
            {
                Out.vColor.rgb = vCurrColor.rgb;
                Out.vColor.a = vCurrColor.a * Out.vColor.a;
            }   
            
                        
            // 큰 이미지의 특정 부분으로 texcoord를 조절하면 되긴 할텐데
            // 일단 이미지 사이즈 기준으로 좌표 구하고, 이미지 크기로 나누면 되는 것 아닌지?
            float2 vImgPos = vFixedScreenPos.xy + vEXImageSize.xy * 0.5f;      // 이미지 사이즈 기준 좌표
            float2 vImgPos_toUV = vImgPos.xy / vEXImageSize.xy;
            
            float2 vFixedTexUV;
            float4 vColorTex = smoothstep(0.f, 1.f, (g_TextureExtra0.Sample(DefaultSampler, vImgPos_toUV) * 1.35f));
            
            float4 vFinalColorTex;
            
            //float4 vColorTex = g_TextureExtra0.Sample(DefaultSampler, In.vTexcoord);      //backup
            
            
            float fColorAverage = (vColorTex.r + vColorTex.g + vColorTex.b) / 3.f;
            vFinalColorTex.rgb = fColorAverage.xxx;
            vFinalColorTex.a = vColorTex.a;
            
            
            // - 효과 넣으려면?
            // 인스턴스별로 얼마나 변화가 진행되었는지,
            // 그리고 이전 타겟이 상하좌우 중 어디로부터 왔는지,
            // 그리고 디졸브같은 노이즈 텍스쳐.. 등 필요할 듯
            
            // 1. 얼마나 변화가 진행되었는가. 유사 timedelta. with clamping
            // 
            // 꼭짓점 좌표를 구한 뒤 순회하여 변화 시작점과의 비교.
            // 1) [시작점->"Nearest" 꼭짓점 거리] == [CHNG_RADIUS] 인 시점에서 변화도 0.
            // 2) [시작점->"MostFar" 꼭짓점 거리] == [CHNG_RADIUS] 인 시점에서 변화도 1.
            // 3) 이 두 개 사이에서 lerp 때리면 됨
            
            if (IsChanging)
            {
                float fChangeRatio; //
                const float fSpeedMultiply = 1.f;
                
                float2 vVerticesPos[4] = {
                    { In.vSInstPos.x - In.vSInstSca.x / 2.f, In.vSInstPos.y + In.vSInstSca.y / 2.f },   // LT 
                    { In.vSInstPos.x + In.vSInstSca.x / 2.f, In.vSInstPos.y - In.vSInstSca.y / 2.f },   // RT 
                    { In.vSInstPos.x + In.vSInstSca.x / 2.f, In.vSInstPos.y - In.vSInstSca.y / 2.f },   // RB 
                    { In.vSInstPos.x - In.vSInstSca.x / 2.f, In.vSInstPos.y + In.vSInstSca.y / 2.f }    // LB 
                };
                float fDistPerVertices[4] = {
                    length(vVerticesPos[0] - vChangeStartPos),
                    length(vVerticesPos[1] - vChangeStartPos),    
                    length(vVerticesPos[2] - vChangeStartPos),
                    length(vVerticesPos[3] - vChangeStartPos)
                };
                
                uint iVertexIndex_Nearest = 0;
                uint iVertexIndex_MostFar = 0;
                for (uint i = 0; i < 4; i++)
                {
                    if (i == 0) continue;
                    
                    if (fDistPerVertices[iVertexIndex_Nearest] > fDistPerVertices[i])   iVertexIndex_Nearest = i;
                    if (fDistPerVertices[iVertexIndex_MostFar] < fDistPerVertices[i])   iVertexIndex_MostFar = i;
                }
                
                float2 vVertex_Nearest = vVerticesPos[iVertexIndex_Nearest];    // 가장 가까운 or 먼 점의 위치. 거리 비교용 재료.
                float2 vVertex_MostFar = vVerticesPos[iVertexIndex_MostFar];    // 가장 가까운 or 먼 점의 위치. 거리 비교용 재료.
                
                // 그냥 두 점의 경우 겹치는 시점의 radius를 구해서 그걸 기반으로 보간?
                
                float fRadTo_PosNearest = fDistPerVertices[iVertexIndex_Nearest];
                float fRadTo_PosMostFar = fDistPerVertices[iVertexIndex_MostFar];
                
                float fRawChangeRatio = smoothstep(fRadTo_PosNearest, fRadTo_PosMostFar, fChangedRadius) * (fSpeedMultiply);
                fChangeRatio = saturate(fRawChangeRatio);
                
                // =====
                
                //vFinalColorTex.rgb = lerp(vCurrColor, vDestColor, smoothstep(0, 1, fChangeRatio));
                
                const float g_fNoiseTileSize = 128.0f; // 상수로 제어

                float2 vNoiseUV = vFixedScreenPos / g_fNoiseTileSize;
                vNoiseUV = frac(vNoiseUV);

                float vNoiseTex1 = g_TextureExtra1.Sample(DefaultSampler, vNoiseUV).r;
                float vNoiseTex2 = g_TextureExtra2.Sample(DefaultSampler, vNoiseUV).r;
                float vNoiseTex3 = g_TextureExtra3.Sample(DefaultSampler, vNoiseUV).r;
                
                // 컷아웃 할 때 처럼, 알파를 fchangeratio 에 따라 비교.
                
                bool isChangedPixel = vNoiseTex3 <= fChangeRatio;
                    
                if (!isChangedPixel)
                    Out.vColor.rgb = vCurrColor.rgb;
                else
                    Out.vColor.rgb = vDestColor.rgb;
                
            }
            
            
            
            
            
            
            
            Out.vColor.rgb *= vFinalColorTex.rgb;
            Out.vColor.a *= vFinalColorTex.a * (1.f - g_AlphaStrength);
            
            return Out;
        } break;
        case UIFLAG_SIMPLE_COLORIZE:
        {
            // ==============================
            // * [9] Simple Colorize
            // ==============================
            // * matrix info
            // [COLORCURR.x] [COLORCURR.y] [COLORCURR.z] [COLORCURR.w]
            // ==============================
            
            float4  vColor          = In.mExtra0.rgba;
            float4  vColorTex       = smoothstep(0.f, 1.f, (g_Texture.Sample(DefaultSampler, fixedUV /*In.vTexcoord*/)));
            
            float   fColorAverage   = (vColorTex.r + vColorTex.g + vColorTex.b) / 3.f;
            Out.vColor.rgb = vColor.rgb * fColorAverage.xxx;
            Out.vColor.a = vColorTex.a * (1.f - g_AlphaStrength);
            
            return Out;
        }
        case UIFLAG_WAVECIRCLE:
        {
            //Out.vColor = float4(1.f, 0.f, 1.f, 1.f);
            //return Out;
            
            // ==============================
            // * [10] Wave Circle
            // ==============================
            // * matrix info
            // [COLORCURR.x] [COLORCURR.y] [COLORCURR.z] [COLORCURR.w]
            // [ISDISTORT] [TIMEELAPSED] [DISTORTSTRENGTH] [ROTATESPEED(DRG)]
            // [DISABLENORMALIZE] [fAlphaMultiplier]
            // ==============================
            // (NOISE) : Extra0
            // (MASK) : Extra1
            // ==============================
            float4 fCurColor        = In.mExtra0.rgba;
            bool isDistort          = _BOOL(In.mExtra1.x);
            float fTimeElapsed      = In.mExtra1.y;
            float fDistortStrength  = In.mExtra1.z;
            float fRotateSpeed      = In.mExtra1.w;
            float fAlphaMultiplier  = In.mExtra2.x;
            bool isDisableNormalize = _BOOL(In.mExtra2.y);
            
            float2 localUV = In.vTexcoord;
            //const float fAlphaMultiplier = 1.5f;        // 너무 알파 날아가서 보정
            
            
            if (isDistort)
            {
                const float fAdditionalStrength = 0.2f;
                const float fUVSlideMultiplier = 0.2f;
                const float fUVMultiplier = 0.75f;
                
                float2 noiseUV   = localUV * fUVMultiplier;
                //noiseUV.x += fTimeElapsed * fUVSlideMultiplier;
                float2 rotatedUV = RotateUV(noiseUV, fTimeElapsed * fRotateSpeed);
                float  noiseSample  = g_TextureExtra0.Sample(DefaultSampler, rotatedUV).r; // 0~1
                
                // 1. 로컬 UV 기준으로 중심에서의 방향/거리 계산ㅡ
                float2 center     = float2(0.5f, 0.5f);     // 로컬 UV의 중앙
                float2 fromCenter = localUV - center;       // 현재 포커싱중인 점의 중점기ㅡ준 상대위치를 구함
                float  dist       = length(fromCenter);     // dist화
            
                // 거리 벡터의 normalize. 단 중점은 0나누기하면 안되니까 예외처리.   
                // 이는 최종적으로 가중치에 의해 변화할 uv 변환량임. 정규화를 하였으므로, 정중앙이 아니라면 가중치를 주었을 때에 일정한 크기만큼 밀릴 것.
                float2 dirNormal = (dist > 0.0001f) ? fromCenter / dist : float2(0.0f, 0.0f);           // <<< 
                float fFinalStrength = fAdditionalStrength * fDistortStrength * noiseSample;
                
                
                // 같은 각도 상의 다른 픽셀들도 다른 값들을 가지므로 이로 인해 한 경로 상에 하나의 줄만 생기지 않는 등의 문제 발생?
                float2 dirScaled = fromCenter / 0.5f;
                
            
                // 4. 최종 오프셋 크기
                //float offsetAmount = wave * radialFalloff * fDistortStrength * fMaxOffset * noiseSample;
                float offsetAmount = fDistortStrength * fFinalStrength;
            
                // 5. UV를 "중심 방향"으로만 밀어줌 (방사형)
                float2 dirScale = (isDisableNormalize)? dirScaled : dirNormal;     // 정규화 안하는 선택지 적용
                fixedUV += dirScale * -offsetAmount;
            
            }
            
            
            
            float4 vColorTex = g_Texture.Sample(DefaultSampler, fixedUV);
            
            Out.vColor.rgb = fCurColor.rgb;
            Out.vColor.a = vColorTex.a * (1.f - g_AlphaStrength) * fCurColor.a * fAlphaMultiplier;
            return Out;
        }
        default:
        {
                Out.vColor = float4(1.f, 0.f, 1.f, 1.f);
                return Out; // 플래그 지정 제대로 안했으면 마젠타 처리
        }
    }
    
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

    pass AlphaGradientPass // AlphaGradient
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_INSTANCE();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_GRADIENT_UI();
    }

    pass NineSectorPass // AlphaGradient + Nine-Sector
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_INSTANCE();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_NINESECTOR_UI();
    }

    pass VariantUIPass  // AlphaPass + a. for cooldown, etc. not designed for animation.
    {                   // 쿨타임 등의 용도로 사용할 특수한 경우용 짬통 pass.. flag로 내부에서 사용할 것 나눔      
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_INSTANCE_VARIANT();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_VARIENT_UI();
    }

}
