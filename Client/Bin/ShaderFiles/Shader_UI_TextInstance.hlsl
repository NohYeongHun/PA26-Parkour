// UI Text

// Header
#include "Engine_Shader_State.hlsli"



// Define (as const / for debug)
#define PI          3.14159265359f
#define _BOOL(x)    ((x) != 0.0f)
#define	IS_BETWEEN(condition, minValue, maxValue)		(((minValue) <= (condition)) && ((condition) < (maxValue)))
#define	IS_BETWEEN_FLOAT2(condition, space)		        (((space.x) <= (condition)) && ((condition) < (space.y)))


//#define KSTA_DEBUG_1_RETURN_AFTERGRAD
//#define KSTA_DEBUG_2_RETURN_AFTEROUTLINE

// ==============================
// * Custom Value List for Instances (mExtraN)
// ==============================
// 1. | 11. [Alpha per Inst] | Damage Uses..
// 2. | Damage Uses.         | | | |
// 3. |                      | | | |
// 4. |                      | | | |
// ==============================

// ==============================
// * Global Variables
// ==============================



SamplerState FontSampler = sampler_state
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Clamp;
    AddressV = Clamp;
};
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



Texture2D g_Texture : register(t0);

// Basic Variables
matrix g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;
float g_AlphaStrength;
	
	
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

//float2 g_ScreenSize;				// [8]
	
float4		g_FontColor;			// [16] RGB + A
	
uint		g_FontFlag = 0;			// [4]
	
float4      g_FontOutlineColor = { 1.0f, 0.0f, 1.0f, 1.0f }; // [16] RGBA Outline Color
float2		g_FontTexPerPixel;      // [8] 1.f / Texture Size 
float		g_FontOutlineWidth;     // [4] Outline Width Size
	
float4		g_FontGradColor;		// [16] RGBA Gradiant Color (->)
	
	
bool		g_isTargetExist;
float4		g_vTargetWorldPos;
float4		g_vCamPosition;
float2      g_LifeTime;

float       g_FontScale;






#define FL_NONE                 0
#define FL_OUTLINE              1 << 0
#define FL_GRAD                 1 << 1
#define FL_ALPHA_EDITABLE       1 << 2

#define FL_END                  1 << 3






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

    // 각 인스턴스 별 크기 및 위치 계산
    //float2 vInstSca = float2(length(In.vSInstRight.xyz), length(In.vSInstUp.xyz));
    float2 vInstSca = float2(In.vSInstRight.x, In.vSInstUp.y);
	
    float2 quadLocal = In.vPosition.xy + float2(0.5f, -0.5f);	// 피벗 보정. 폰트의 기준점에 따른 위치 보정 위함.
    float2 quadLocalPx = float2(quadLocal.x * vInstSca.x,		// 크기 비례 위치 이동 (원본 글자 크기의 사각형을 만들기 위함)
                                quadLocal.y * vInstSca.y);

    // 타겟이 없는 경우
    float2 pixelPos;
    pixelPos.x = In.vSInstTrans.x + quadLocalPx.x;
    pixelPos.y = In.vSInstTrans.y - quadLocalPx.y;

    float z_ndc = 0.1f; // 깊이 버퍼 대비?

    // 타겟이 있는 경우
    if (g_isTargetExist)
    {
        // 타겟 위치의 정점 변환 (월드 -> 뷰 -> 투영 -> NDC -> 스크린)
        float3 worldCenter = g_vTargetWorldPos.xyz;
        float4 targetView = mul(float4(worldCenter, 1.0f), g_ViewMatrix);
        float4 targetProj = mul(targetView, g_ProjMatrix);
        if (targetProj.w <= 0.0f)   { Out.vPosition = float4(-2, -2, 0, 1);  return Out; } // 카메라 뒤면 버림
        float3 targetNDC = targetProj.xyz / targetProj.w;
        float2 targetScreenPos;
        targetScreenPos.x = (targetNDC.x + 1.0f) * 0.5f * g_ScreenSize.x;
        targetScreenPos.y = (1.0f - targetNDC.y) * 0.5f * g_ScreenSize.y;

		// 변환 완료 이후 스크린 공간..
		// 이 차례에 스크린 기준으로 제공된 Transform 행렬 반영. 이상한데서 계산하면 안됨.
        float2 advancePx = In.vSInstTrans.xy;
        pixelPos = targetScreenPos + float2(advancePx.x + quadLocalPx.x,
                                            advancePx.y - quadLocalPx.y);

        // 깊이 버퍼용? 잘 모르겠음
        z_ndc = targetNDC.z;
    }

    // 스크린 -> NDC 변환.
	// 타겟이 있는 경우에는 왜 이렇게 하느냐?..
	// 
	// 스크린까지 올렸던 건 스크린 기준으로 제공된 Transform 의 반영을 위함이었던 것.
	// PS의 UV에서는 NDC를 기준으로 사용하므로 이를 전달함.
    float2 ndc = (pixelPos / g_ScreenSize) * float2(2, -2) + float2(-1, 1);
    Out.vPosition = float4(ndc.xy, z_ndc, 1.0f);

    Out.vTexcoord = In.vTexcoord;
    Out.vWorldPos = float4(pixelPos, 0, 1);
    Out.vProjPos = Out.vPosition;

    // for Pixel Shaders
    Out.vSInstCoordX = In.vSInstCoordX;
    Out.vSInstCoordY = In.vSInstCoordY;
    Out.vClipTexcoordX = In.vClipTexcoordX;
    Out.vClipTexcoordY = In.vClipTexcoordY;
    Out.vSInstPos = In.vSInstTrans.xy;
    Out.vSInstSca = vInstSca;
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




PS_OUT PS_MAIN(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;
    //Out.vColor = float4(1.f, 0.f, 1.f, 1.f);
    //return Out;
	
    // Apply InstCoord for atlas / sprite style
	//PS_OUT Out = (PS_OUT) 0;
	float2 fixedUV = float2(lerp(In.vSInstCoordX.x, In.vSInstCoordX.y, In.vTexcoord.x),			// 이걸로 In.vSInstCoord 범위에 따라.. 이용?
                            lerp(In.vSInstCoordY.x, In.vSInstCoordY.y, In.vTexcoord.y));
    
    // Apply ClipTexcoord for clipped ui. like as HP Bar
    // Calc Clip Space
	float2 clipX = float2(lerp(In.vSInstCoordX.x, In.vSInstCoordX.y, In.vClipTexcoordX.x),		// 예시로 텍스쳐를 0.2 ~ 0.8 범위만 쓰는데, 클립 범위는 0.5 ~ 1.0 이라면
                            lerp(In.vSInstCoordX.x, In.vSInstCoordX.y, In.vClipTexcoordX.y));	// 0.2 ~ 0.8 범위 내에서의 0.5 및 1.0을 클립 범위로 삼음. ( result : 0.5 ~ 0.8 )
	float2 clipY = float2(lerp(In.vSInstCoordY.x, In.vSInstCoordY.y, In.vClipTexcoordY.x),
                            lerp(In.vSInstCoordY.x, In.vSInstCoordY.y, In.vClipTexcoordY.y));
    // discard
	if (fixedUV.x < clipX.x || fixedUV.x > clipX.y ||
        fixedUV.y < clipY.x || fixedUV.y > clipY.y)
		discard;
	
    Out.vColor = g_Texture.Sample(FontSampler, fixedUV);
	Out.vColor.a = Out.vColor.a * (1.f - g_AlphaStrength);
	
	// ==============================
	// * declare here.
	// ==============================
	
    //float alphaCenter = g_Texture.Sample(FontSampler, fixedUV).r; // [0 ~ 1] 현재 바라보는 픽셀 색상에서 a값 추출
    //    
    //
    //
    //float fillMask = smoothstep(0.5f, 0.8f, alphaCenter);       // [0 ~ 1] 글자에 색상 채워진 정도를 저장. 경계 부드럽게
    //float4 fillColor = float4(g_FontColor.rgb, g_FontColor.a * fillMask);
    //
    //Out.vColor.rgb = g_FontColor.rgb;
    //Out.vColor.a = g_FontColor.a * fillMask;                    // 글자가 존재하는 영역만을 남김. (r채널에 의해 구분)
    
    //
    
    
    float fDistance = 1.f - g_Texture.Sample(FontSampler, fixedUV).r;
       
    const float fFontWidth = 0.6f;
    const float fDiscardWidth = 0.3f;
    const float fGradWidth              = 0.2f;
    const float fFontOutlineWidth       = saturate((g_FontOutlineWidth / 40.f));        // fontOutlineWidth = 2.f;
        
    const float fTotalWidth             = fFontWidth + fGradWidth * 2.f + fFontOutlineWidth + fDiscardWidth;
    
    float4 vFontColor                   = g_FontColor;
    float4 vOutlineColor                = g_FontColor;
    float fAdditionalAlpha              = 1.f;
    
    // ===== outline =====
    if (g_FontFlag & FL_OUTLINE)
    {
        vOutlineColor = g_FontOutlineColor;
    }
    // END== outline =====
    
    // ===== grad (wip) =====
    if (g_FontFlag & FL_GRAD)           
    {
        // Gradiant
        const float4 GradRColor = g_FontGradColor;
    }
    
    // END== grad (wip) =====
    
    // ===== additional alpha =====
    if (g_FontFlag & FL_ALPHA_EDITABLE)
    {
        // 11. Alpha per Inst
        fAdditionalAlpha = (1.f - In.mExtra0.x);
    }
    // END== additional alpha =====
    
    
    
     
    const float2 vFontInnerWidth = { 0.f, fFontWidth / fTotalWidth };
    const float2 vFontGrad2OuterWidth = { vFontInnerWidth.y, vFontInnerWidth.y + fGradWidth / fTotalWidth };
    const float2 vFontOuterWidth = { vFontGrad2OuterWidth.y, vFontGrad2OuterWidth.y + fFontOutlineWidth / fTotalWidth };
    const float2 vFontGrad2DiscardWidth = { vFontOuterWidth.y, vFontOuterWidth.y + fGradWidth / fTotalWidth };
    
    float4 vResultColor = (float4) 0;
    
    if (fDistance < vFontInnerWidth.y)
    {
        vResultColor.rgb = vFontColor.rgb;
        vResultColor.a = 1.f;
    }
    else if (fDistance < vFontGrad2OuterWidth.y)
    {
        float fRatio = smoothstep(vFontGrad2OuterWidth.x, vFontGrad2OuterWidth.y, fDistance);
        vResultColor.rgb = lerp(vFontColor.rgb, vOutlineColor.rgb, fRatio);
        vResultColor.a = 1.f;
    }
    else if (fDistance < vFontOuterWidth.y)
    {
        vResultColor.rgb = vOutlineColor.rgb;
        vResultColor.a = 1.f;
    }
    else if (fDistance < vFontGrad2DiscardWidth.y)
    {
        float fRatio = smoothstep(vFontGrad2DiscardWidth.x, vFontGrad2DiscardWidth.y, fDistance);
        vResultColor.rgb = vOutlineColor.rgb;
        vResultColor.a = 1.f - smoothstep(vFontGrad2DiscardWidth.x, vFontGrad2DiscardWidth.y, fRatio);
    }
    else
    {
        discard;
    }
    
    Out.vColor.rgb = vResultColor.rgb;
    Out.vColor.a = vResultColor.a * fAdditionalAlpha;
    
    
    return Out;
}




technique11 DefaultTechnique
{
    pass DefaultPass
    {
        SetRasterizerState(RS_CullOff);
        SetBlendState(BS_FontAlpha, float4(0, 0, 0, 0), 0xFFFFFFFF);
        SetDepthStencilState(DSS_Off, 0);
        VertexShader = compile vs_5_0 VS_INSTANCE();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN();
    }
}