#include "Engine_Shader_Defines.hlsli"

matrix g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;

float4 g_CamPos;

//float4 g_GrassColor = float4(0.7019f, 0.24705f, 0.24705f, 1.f);
float4 g_GrassColor = float4(0.7844f, 0.4567f, 0.1137f, 1.f);
float g_fGrassColorIntensity = 0.53f;

Texture2D   g_DiffuseTexture[4];
Texture2D   g_NormalTexture[4];
Texture2D   g_MaskDiffuseTexture;
Texture2D   g_MetallicTexture;
vector      g_vMatrlAmbient = vector(1.0f, 1.0f, 1.0f, 1.0f);
vector      g_vMatrlSpecular = vector(0.4f, 0.4f, 0.4f, 0.4f);

//Texture2D   g_MaskTexture[4] : register(t8);
Texture2D g_MaskTexture[4];

matrix g_ShadowViewMatrix[4];
matrix g_ShadowProjMatrix[4];



float g_fOutLineRadius = 0.0005;
float g_fOutLineRadiusZ = 0.0005f;

bool g_HasNormal = false;
bool g_HasMask = false;
bool g_HasMetallic = false;
bool g_IsDynamicObject = false;
int g_iIndex = 0;


float g_DissolveTime = 1.f;
float g_DistortionTime = 0.f;
float g_fAlpha = 0.f;
bool g_DissolveStart = false;
struct VS_IN
{
    float3 vPosition : POSITION;
    float3 vNormal : NORMAL;
    float3 vTangent : TANGENT;
    float3 vBinormal : BINORMAL;
    float2 vTexcoord : TEXCOORD0;
};

struct VS_OUT
{
    float4 vPosition : SV_POSITION;
    float4 vNormal : NORMAL;
    float4 vTangent : TANGENT;
    float4 vBinormal : BINORMAL;
    float2 vTexcoord : TEXCOORD0;
    float4 vProjPos : TEXCOORD1;
    float4 vWorldPos : TEXCOORD2;
};

VS_OUT VS_MAIN(VS_IN In)
{
    VS_OUT Out = (VS_OUT) 0;
    
    matrix matWV, matWVP;
    
    matWV = mul(g_WorldMatrix, g_ViewMatrix);
    matWVP = mul(matWV, g_ProjMatrix);
    Out.vPosition = mul(float4(In.vPosition, 1.f), matWVP);
    Out.vNormal = normalize(mul(float4(In.vNormal, 0.f), g_WorldMatrix));
    Out.vTangent = normalize(mul(float4(In.vTangent, 0.f), g_WorldMatrix));
    Out.vBinormal = normalize(mul(float4(In.vBinormal, 0.f), g_WorldMatrix));
    Out.vTexcoord = In.vTexcoord;
    Out.vProjPos = mul(float4(In.vPosition, 1.f), matWVP);
    Out.vWorldPos = mul(float4(In.vPosition, 1.f), g_WorldMatrix);
    
    return Out;
}

struct PS_IN
{
    float4 vPosition : SV_POSITION;
    float4 vNormal : NORMAL;
    float4 vTangent : TANGENT;
    float4 vBinormal : BINORMAL;
    float2 vTexcoord : TEXCOORD0;
    float4 vProjPos : TEXCOORD1;
    float4 vWorldPos : TEXCOORD2;
};

struct PS_OUT_LIGHT
{
    float4 vDiffuse : SV_TARGET0;
    float4 vNormal : SV_TARGET1;
    float4 vDepth : SV_TARGET2;
    float4 vEmissive : SV_TARGET3;
    float4 vDistortion : SV_TARGET4;
    float4 vPBR : SV_TARGET5;
    float4 vSSS : SV_TARGET6;
};


PS_OUT_LIGHT PS_MAIN_NORMAL(PS_IN In)
{
    PS_OUT_LIGHT Out = (PS_OUT_LIGHT) 0;
    
    vector vMask = g_MaskTexture[0].Sample(DefaultSampler, In.vTexcoord);
    
    vector vDiffuse = g_DiffuseTexture[0].Sample(DefaultSampler, In.vTexcoord);
    if(length(vDiffuse) == 0.f)
        vDiffuse = 1.f;
    
    vector vMaskDiffiuse = g_DiffuseTexture[1].Sample(DefaultSampler, In.vTexcoord);

    if (g_HasMask)
    {
        Out.vDiffuse = vDiffuse * vMask.r + vDiffuse * (1.f - vMask.r);
        Out.vDiffuse = Out.vDiffuse * vMask.g + vMaskDiffiuse * (1.f - vMask.g);
    }
    else
    {
        Out.vDiffuse = vDiffuse;
    }
    Out.vDiffuse.w = 1.f;
    
    Out.vPBR.y = g_fGlobalStaticRoughness;
    Out.vPBR.x = g_fGlobalStaticMetallic;
    
    float4 vNormal;
    
    if (g_HasNormal)
    {
        if (g_HasMask)
        {
            vector vDefaultNormal = g_NormalTexture[0].SampleLevel(DefaultSampler, In.vTexcoord, 0);
            
            float4 vNormal1 = normalize(vDefaultNormal * 2.f - 1.f);
            //if (vDefaultNormal.x > vDefaultNormal.z && vDefaultNormal.y > vDefaultNormal.z)
            vNormal1.z = sqrt(1.f - saturate(dot(vDefaultNormal.xy, vDefaultNormal.xy)));

            vector vMaskNormal = g_NormalTexture[1].Sample(DefaultSampler, In.vTexcoord);
        
            //float4 vNormal2 = normalize(vMaskNormal * 2.f - 1.f);
            //if (vMaskNormal.x > vMaskNormal.z && vMaskNormal.y > vMaskNormal.z)
            //    vNormal2.z = sqrt(1.f - saturate(dot(vMaskNormal.xy, vMaskNormal.xy)));

            //if (vMask.r == 0.f && vMask.g == 0.f)
            //    vNormal = vNormal1;
            //else
            //{
            //    vNormal = vNormal1 * (vMask.r) + vNormal1 * (1.f - vMask.r);
            //    vNormal = vNormal * vMask.g + vNormal2 * (1.f - vMask.g);
            //}
            
            vNormal = vNormal1 * (vMask.r) + vNormal1 * (1.f - vMask.r);
        }
        else
        {
            vector vDefaultNormal = g_NormalTexture[0].Sample(DefaultSampler, In.vTexcoord);
		
	        
            vNormal = normalize(vDefaultNormal * 2.f - 1.f);
            if (vDefaultNormal.x > vDefaultNormal.z && vDefaultNormal.y > vDefaultNormal.z)
                vNormal.z = sqrt(1.f - saturate(dot(vDefaultNormal.xy, vDefaultNormal.xy)));
        }
        //vector vNormalDesc = vDefaultNormal * (1.f - vMask.r) + vMaskNormal * vMask.g;
        
        //vNormal = normalize(vNormalDesc * 2.f - 1.f);
        //if (vNormalDesc.x > vNormalDesc.z && vNormalDesc.y > vNormalDesc.z)
        //    vNormal.z = sqrt(1.f - saturate(dot(vNormalDesc.xy, vNormalDesc.xy)));

        //vNormal = vNormal1;  

        float3 vTangent = In.vTangent.xyz;
        float3 vBinormal = In.vBinormal.xyz * -1.f;
        float3 vInNormal = In.vNormal.xyz;

        float3x3 WorldMatrix;
        WorldMatrix = float3x3(vTangent, vBinormal, vInNormal);
        
        vNormal.xyz = normalize(mul(vNormal.xyz, WorldMatrix));
        vNormal.xyz = vNormal * 0.5f + 0.5f;
    }
    else
    {
        vNormal = In.vNormal; 
        vNormal = vNormal * 0.5f + 0.5f;
    }
    
    Out.vNormal = float4(vNormal.xyz, 1.f);
    Out.vDepth.x = In.vProjPos.z / In.vProjPos.w;
    Out.vDepth.y = In.vProjPos.w;
    
    Out.vSSS.z = In.vProjPos.z / In.vProjPos.w;
    Out.vSSS.w = In.vProjPos.w;
    
    Out.vDepth.w = 1.f;
    
    return Out;
}

PS_OUT_LIGHT PS_MAIN_NORMAL_ALPHA(PS_IN In)
{
    PS_OUT_LIGHT Out = (PS_OUT_LIGHT) 0;
    
    vector vMask = g_MaskTexture[0].Sample(DefaultSampler, In.vTexcoord);
    
    vector vDiffuse = g_DiffuseTexture[0].Sample(DefaultSampler, In.vTexcoord);
    vector vMaskDiffiuse = g_DiffuseTexture[1].Sample(DefaultSampler, In.vTexcoord);

    if (g_HasMask)
    {
        Out.vDiffuse = vDiffuse * vMask.r + vDiffuse * (1.f - vMask.r);
        Out.vDiffuse = Out.vDiffuse * vMask.g + vMaskDiffiuse * (1.f - vMask.g);

    }
    else
    {
        Out.vDiffuse = vDiffuse;
    }
    
    if (Out.vDiffuse.a <= 0.1f)
        discard;
    
    Out.vDiffuse.w = 1.f;
    
    Out.vPBR.y = g_fGlobalStaticRoughness;
    Out.vPBR.x = g_fGlobalStaticMetallic;
    
    if (g_IsDynamicObject)
    {
        Out.vDepth.z = 1.f;
 //       Out.vPBR.z = 1.f;
    }
    
    float4 vNormal;
    
    if (g_HasNormal)
    {
        if (g_HasMask)
        {
            vector vDefaultNormal = g_NormalTexture[0].SampleLevel(DefaultSampler, In.vTexcoord, 0);
            
            float4 vNormal1 = normalize(vDefaultNormal * 2.f - 1.f);
            //if (vDefaultNormal.x > vDefaultNormal.z && vDefaultNormal.y > vDefaultNormal.z)
            vNormal1.z = sqrt(1.f - saturate(dot(vDefaultNormal.xy, vDefaultNormal.xy)));

            vector vMaskNormal = g_NormalTexture[1].Sample(DefaultSampler, In.vTexcoord);
        
            //float4 vNormal2 = normalize(vMaskNormal * 2.f - 1.f);
            //if (vMaskNormal.x > vMaskNormal.z && vMaskNormal.y > vMaskNormal.z)
            //    vNormal2.z = sqrt(1.f - saturate(dot(vMaskNormal.xy, vMaskNormal.xy)));

            //if (vMask.r == 0.f && vMask.g == 0.f)
            //    vNormal = vNormal1;
            //else
            //{
            //    vNormal = vNormal1 * (vMask.r) + vNormal1 * (1.f - vMask.r);
            //    vNormal = vNormal * vMask.g + vNormal2 * (1.f - vMask.g);
            //}
            
            vNormal = vNormal1 * (vMask.r) + vNormal1 * (1.f - vMask.r);
        }
        else
        {
            vector vDefaultNormal = g_NormalTexture[0].Sample(DefaultSampler, In.vTexcoord);
		
	        
            vNormal = normalize(vDefaultNormal * 2.f - 1.f);
            if (vDefaultNormal.x > vDefaultNormal.z && vDefaultNormal.y > vDefaultNormal.z)
                vNormal.z = sqrt(1.f - saturate(dot(vDefaultNormal.xy, vDefaultNormal.xy)));
        }
        //vector vNormalDesc = vDefaultNormal * (1.f - vMask.r) + vMaskNormal * vMask.g;
        
        //vNormal = normalize(vNormalDesc * 2.f - 1.f);
        //if (vNormalDesc.x > vNormalDesc.z && vNormalDesc.y > vNormalDesc.z)
        //    vNormal.z = sqrt(1.f - saturate(dot(vNormalDesc.xy, vNormalDesc.xy)));

        //vNormal = vNormal1;  

        float3 vTangent = In.vTangent.xyz;
        float3 vBinormal = In.vBinormal.xyz * -1.f;
        float3 vInNormal = In.vNormal.xyz;

        float3x3 WorldMatrix;
        WorldMatrix = float3x3(vTangent, vBinormal, vInNormal);
        
        vNormal.xyz = normalize(mul(vNormal.xyz, WorldMatrix));
        vNormal.xyz = vNormal * 0.5f + 0.5f;
    }
    else
    {
        vNormal = In.vNormal;
        vNormal = vNormal * 0.5f + 0.5f;
    }
    
    Out.vNormal = float4(vNormal.xyz, 1.f);
    Out.vDepth.x = In.vProjPos.z / In.vProjPos.w;
    Out.vDepth.y = In.vProjPos.w;
    
    Out.vDepth.w = 1.f;
    
    return Out;
}

PS_OUT_LIGHT PS_MAIN_NORMAL_FOCUS(PS_IN In)
{
    PS_OUT_LIGHT Out = (PS_OUT_LIGHT) 0;
    
    vector vMask = g_MaskTexture[0].Sample(DefaultSampler, In.vTexcoord);
    
    vector vDiffuse = g_DiffuseTexture[0].Sample(DefaultSampler, In.vTexcoord);
    vector vMaskDiffiuse = g_DiffuseTexture[1].Sample(DefaultSampler, In.vTexcoord);
    
    Out.vDiffuse = vDiffuse * (1.f - vMask.r) + vMaskDiffiuse * vMask.g;
    Out.vDiffuse *= float4(0.7f, 1.f, 0.7f, 1.f);
    
    float3 vNormal;
    
    if(g_HasNormal)
    {
        vector vDefaultNormal = g_NormalTexture[0].Sample(DefaultSampler, In.vTexcoord);
        vector vMaskNormal = g_NormalTexture[1].Sample(DefaultSampler, In.vTexcoord);
        
        vector vNormalDesc = vDefaultNormal * (1.f - vMask.r) + vMaskNormal * vMask.g;
        
        vNormal = normalize(vNormalDesc * 2.f - 1.f);
        if (vNormalDesc.x > vNormalDesc.z && vNormalDesc.y > vNormalDesc.z)
            vNormal.z = sqrt(1.f - saturate(dot(vNormalDesc.xy, vNormalDesc.xy)));

        float3 vTangent = In.vTangent.xyz;
        float3 vBinormal = In.vBinormal.xyz * -1.f;
        float3 vInNormal = In.vNormal.xyz;

        float3x3 WorldMatrix;
        WorldMatrix = float3x3(vTangent, vBinormal, vInNormal);

        vNormal.xyz = normalize(mul(vNormal.xyz, WorldMatrix));
        vNormal.xyz = vNormal * 0.5f + 0.5f;
    }
    else
    {
        vNormal = In.vNormal.xyz;
        vNormal = vNormal * 0.5f + 0.5f;
    }
    
    Out.vNormal = float4(vNormal, 1.f);
    Out.vDepth.x = In.vProjPos.z / In.vProjPos.w;
    Out.vDepth.y = In.vProjPos.w;
    
    Out.vDepth.w = 1.f;
    
    return Out;
}


PS_OUT_LIGHT PS_MAIN_EMISSIVE_LIGHT(PS_IN In)
{
    PS_OUT_LIGHT Out = (PS_OUT_LIGHT) 0;
    
    vector vMask = g_MaskTexture[0].Sample(DefaultSampler, In.vTexcoord);
    
    vector vDiffuse = g_DiffuseTexture[0].Sample(DefaultSampler, In.vTexcoord);
    if (length(vDiffuse) == 0.f)
        vDiffuse = 1.f;
    
    vector vMaskDiffiuse = g_DiffuseTexture[1].Sample(DefaultSampler, In.vTexcoord);

    if (g_HasMask)
    {
        Out.vDiffuse = vDiffuse * vMask.r + vDiffuse * (1.f - vMask.r);
        Out.vDiffuse = Out.vDiffuse * vMask.g + vMaskDiffiuse * (1.f - vMask.g);
    }
    else
    {
        Out.vDiffuse = vDiffuse;
    }

    if (Out.vDiffuse.a > 0.f)
        //Out.vEmissive = float4((Out.vDiffuse.xyz) * 0.7f, 1.f);
        Out.vEmissive = float4(1.0f, 0.9f, 0.6f, 1.f);
    
    Out.vDiffuse.w = 1.f;
    
    Out.vPBR.y = g_fGlobalStaticRoughness;
    Out.vPBR.x = g_fGlobalStaticMetallic;
    
    float4 vNormal;
    
    if (g_HasNormal)
    {
        if (g_HasMask)
        {
            vector vDefaultNormal = g_NormalTexture[0].SampleLevel(DefaultSampler, In.vTexcoord, 0);
            
            float4 vNormal1 = normalize(vDefaultNormal * 2.f - 1.f);
            vNormal1.z = sqrt(1.f - saturate(dot(vDefaultNormal.xy, vDefaultNormal.xy)));

            vector vMaskNormal = g_NormalTexture[1].Sample(DefaultSampler, In.vTexcoord);
        
            vNormal = vNormal1 * (vMask.r) + vNormal1 * (1.f - vMask.r);
        }
        else
        {
            vector vDefaultNormal = g_NormalTexture[0].Sample(DefaultSampler, In.vTexcoord);
		
	        
            vNormal = normalize(vDefaultNormal * 2.f - 1.f);
            if (vDefaultNormal.x > vDefaultNormal.z && vDefaultNormal.y > vDefaultNormal.z)
                vNormal.z = sqrt(1.f - saturate(dot(vDefaultNormal.xy, vDefaultNormal.xy)));
        }

        float3 vTangent = In.vTangent.xyz;
        float3 vBinormal = In.vBinormal.xyz * -1.f;
        float3 vInNormal = In.vNormal.xyz;

        float3x3 WorldMatrix;
        WorldMatrix = float3x3(vTangent, vBinormal, vInNormal);
        
        vNormal.xyz = normalize(mul(vNormal.xyz, WorldMatrix));
        vNormal.xyz = vNormal * 0.5f + 0.5f;
    }
    else
    {
        vNormal = In.vNormal;
        vNormal = vNormal * 0.5f + 0.5f;
    }
    
    Out.vNormal = float4(vNormal.xyz, 1.f);
    Out.vDepth.x = In.vProjPos.z / In.vProjPos.w;
    Out.vDepth.y = In.vProjPos.w;
    
    Out.vSSS.z = In.vProjPos.z / In.vProjPos.w;
    Out.vSSS.w = In.vProjPos.w;
    
    Out.vDepth.w = 1.f;

    return Out;
}

/*======================================================SHADOW_BEGIN======================================================*/

struct VS_OUT_SHADOW
{
    float4 vPosition : POSITION;
};

VS_OUT_SHADOW VS_SHADOW(VS_IN In)
{
    VS_OUT_SHADOW Out = (VS_OUT_SHADOW) 0;
    
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
    uint iIndex : SV_RenderTargetArrayIndex;
};

[maxvertexcount(12)]
void GS_SHADOW(triangle GS_IN In[3], inout TriangleStream<GS_OUT> Vertices)
{
    for (int Face = 0; Face < 4; Face++)
    {
        GS_OUT Out = (GS_OUT) 0;
        Out.iIndex = Face;
        
        matrix matVP;
        matVP = mul(g_ShadowViewMatrix[Face] , g_ShadowProjMatrix[Face]);

        for (int i = 0; i < 3; i++)
        {
            Out.vPosition = mul(In[i].vPosition, matVP);
            Vertices.Append(Out);
        }
        Vertices.RestartStrip();
    }
}

struct PS_IN_SHADOW
{
    float4 vPosition : SV_POSITION;
};

void PS_SHADOW(PS_IN_SHADOW In)
{        
    if (In.vPosition.z >= 1.f)
        discard;
}

/*======================================================SHADOW_END======================================================*/

/*======================================================OUTLINE_BEGIN======================================================*/

struct VS_OUT_OUTLINE
{
    float4 vPosition : SV_POSITION;
};

VS_OUT_OUTLINE VS_OUTLINE(VS_IN In)
{
    VS_OUT_OUTLINE Out = (VS_OUT_OUTLINE) 0;
    
    matrix matVP;
    
    matrix matWV = mul(g_WorldMatrix, g_ViewMatrix);
    
    matVP = mul(g_ViewMatrix, g_ProjMatrix);
    
    vector vWorldPos = mul(float4(In.vPosition, 1.f), matWV);
    vector vNormal = normalize(mul(float4(In.vNormal, 0.f), matWV));
   
    vNormal = float4(vNormal.x, vNormal.y, (vNormal.z * g_fOutLineRadiusZ), 0.f);
  
    vector vOutLinePos = vWorldPos + (vNormal * g_fOutLineRadius);
    
    Out.vPosition = mul(float4(vOutLinePos), g_ProjMatrix);
    
    return Out;
}

struct PS_IN_OUTLINE
{
    float4 vPosition : SV_POSITION;
};

struct PS_OUT_OUTLINE
{
    float4 vColor : SV_TARGET0;
};

PS_OUT_OUTLINE PS_OUTLINE(PS_IN_OUTLINE In)
{
    PS_OUT_OUTLINE Out = (PS_OUT_OUTLINE) 0;

    Out.vColor = float4(0.3f, 0.15f, 0.f, 1.f);
    
    return Out;
}

/*======================================================OUTLINE_END======================================================*/

/*======================================================SHADOW_MAP_BEGIN======================================================*/

matrix  g_ShadowMapViewMatrix;
matrix  g_ShadowMapProjMatrix;
int     g_iShadowMapLayer;

struct VS_OUT_SHADOW_MAP
{
    float4 vPosition : SV_POSITION;
    uint iIndex : SV_RenderTargetArrayIndex;
};

VS_OUT_SHADOW_MAP VS_SHADOW_MAP(VS_IN In)
{
    VS_OUT_SHADOW_MAP Out = (VS_OUT_SHADOW_MAP) 0;
    
    matrix matWV, matWVP;
    
    matWV = mul(g_WorldMatrix, g_ShadowMapViewMatrix);
    matWVP = mul(matWV, g_ShadowMapProjMatrix);

    Out.vPosition = mul(float4(In.vPosition, 1.f), matWVP);
    Out.iIndex = g_iShadowMapLayer;
    
    return Out;
}

/*======================================================SHADOW_MAP_END======================================================*/

struct PS_OUT_EMISSIVE
{
    float4 vDiffuse : SV_TARGET0;
    float4 vEmissive : SV_TARGET1;
};


PS_OUT_EMISSIVE PS_EMISSIVE(PS_IN In)
{
    PS_OUT_EMISSIVE Out = (PS_OUT_EMISSIVE) 0;

    float4 vColor = g_DiffuseTexture[0].Sample(DefaultSampler, In.vTexcoord);
    
    Out.vDiffuse = float4(vColor.xyz, 1.f);
    
    float fWeight = Luminance(vColor.xyz);
    
    if(fWeight >= g_fEmissiveThreshold)
        Out.vEmissive = float4(vColor.xyz, 1.f);
        
    return Out;
}

PS_OUT_LIGHT PS_LOGOMOUNTAIN(PS_IN In)
{
    PS_OUT_LIGHT Out = (PS_OUT_LIGHT) 0;
    
    float4 vRockColor = g_DiffuseTexture[0].Sample(DefaultSampler, In.vTexcoord);
    float4 vSnowColor = g_DiffuseTexture[1].Sample(DefaultSampler, In.vTexcoord);
    
    float4 vMainMask = g_MaskTexture[0].Sample(DefaultSampler, In.vTexcoord);
    float4 vSnowMask = g_MaskTexture[1].Sample(DefaultSampler, In.vTexcoord);
    
    float4 vRockNormal = g_NormalTexture[0].Sample(DefaultSampler, In.vTexcoord);
    float4 vMainNormal = g_NormalTexture[1].Sample(DefaultSampler, In.vTexcoord);
    
    float3 vNormalDesc = lerp(vRockNormal.xyz, vMainNormal.xyz, vMainMask.g);
    
    float4 vDiffuse = lerp(vRockColor, vSnowColor, vMainMask.r);
    

    float3 vNormal;
    
    vNormal = vNormalDesc * 2.f - 1.f;
    
    vNormal.z = sqrt(1.f - saturate(dot(vNormalDesc.xy, vNormalDesc.xy)));
    
    float3 vTangent = In.vTangent.xyz;
    float3 vBinormal = In.vBinormal.xyz * -1.f;
    float3 vInNormal = In.vNormal.xyz;

    float3x3 WorldMatrix;
    WorldMatrix = float3x3(vTangent, vBinormal, vInNormal);
        
    vNormal = normalize(mul(vNormal, WorldMatrix));
    
    vNormal = lerp(vNormal, In.vNormal.xyz, vMainMask.r);
    
    if (vMainMask.r >= 0.8f)
    {
        float3 vLook = normalize(g_CamPos.xyz - In.vWorldPos.xyz);
    
        float fRimPower = 0.f;
    
        float fNdoV = dot(vNormal, vLook);
    
        fRimPower = 1.f - abs(fNdoV);
    
        fRimPower = smoothstep(cos(radians(45.f)), 1.f, fRimPower);
    
        fRimPower *= 0.5f;
        
        vDiffuse += vDiffuse * fRimPower;
    }
    
    
    vNormal = vNormal * 0.5f + 0.5f;
    
    Out.vDiffuse.xyz = vDiffuse.xyz;
    Out.vDiffuse.w = 1.f;
    
    Out.vNormal = float4(vNormal, 1.f);
    
    Out.vDepth.x = In.vProjPos.z / In.vProjPos.w;
    Out.vDepth.y = In.vProjPos.w;
    
    Out.vSSS.z = In.vProjPos.z / In.vProjPos.w;
    Out.vSSS.w = In.vProjPos.w;
    
    Out.vPBR.y = g_fGlobalStaticRoughness;
    Out.vPBR.x = g_fGlobalStaticMetallic;
    
    return Out;
}

PS_OUT_LIGHT PS_GRASS_ROCK_MA(PS_IN In)
{
    PS_OUT_LIGHT Out = (PS_OUT_LIGHT) 0;
    
    float4 vGrassDiffuse = g_DiffuseTexture[0].SampleLevel(DefaultSampler, (In.vTexcoord * 10.f), 0.f);
    float4 vRockDiffuse = g_DiffuseTexture[1].SampleLevel(DefaultSampler, In.vTexcoord, 0.f);
    
    float4 vMainNormal = g_NormalTexture[0].SampleLevel(DefaultSampler, In.vTexcoord, 0.f);
    float4 vGrasNormal = g_NormalTexture[1].SampleLevel(DefaultSampler, In.vTexcoord * 10.f, 0.f);
    float4 vDetailNormal = g_NormalTexture[2].SampleLevel(DefaultSampler, In.vTexcoord, 0.f);
    
    float4 vMask = g_MaskTexture[0].SampleLevel(DefaultSampler, In.vTexcoord, 0.f);
    
    //float4 vDiffuse = lerp(vRockDiffuse, (vGrassDiffuse * g_GrassColor), vMask.r);
    float4 vDiffuse = lerp(vRockDiffuse, lerp(vGrassDiffuse, g_GrassColor, g_fGrassColorIntensity), vMask.r);
  
   // float3 vNormalDesc = lerp(vMainNormal.xyz, vGrasNormal.xyz, vMask.r);
  //  float3 vNormalDesc = lerp(lerp(vMainNormal.xyz, vDetailNormal.xyz, vMask.b), vGrasNormal.xyz, vMask.r);
    float3 vNormalDesc = lerp(lerp(vDetailNormal.xyz, vMainNormal.xyz, vMask.b), vGrasNormal.xyz, vMask.r);
    
    float3 vNormal;
	        
    //vNormal = normalize(vNormalDesc * 2.f - 1.f);
    vNormal = vNormalDesc * 2.f - 1.f;
    
    
    vNormal.z = sqrt(1.f - saturate(dot(vNormalDesc.xy, vNormalDesc.xy)));
    
    float3 vTangent = In.vTangent.xyz;
    float3 vBinormal = In.vBinormal.xyz * -1.f;
    float3 vInNormal = In.vNormal.xyz;

    float3x3 WorldMatrix;
    WorldMatrix = float3x3(vTangent, vBinormal, vInNormal);
        
    vNormal = normalize(mul(vNormal, WorldMatrix));
    
    vNormal = vNormal * 0.5f + 0.5f;
    
    Out.vDiffuse.xyz = vDiffuse.xyz;
    Out.vDiffuse.w = 1.f;
    
    Out.vNormal = float4(vNormal, 1.f);
    
    Out.vDepth.x = In.vProjPos.z / In.vProjPos.w;
    Out.vDepth.y = In.vProjPos.w;
    
    Out.vSSS.z = In.vProjPos.z / In.vProjPos.w;
    Out.vSSS.w = In.vProjPos.w;
    
    Out.vPBR.y = g_fGlobalStaticRoughness;
    Out.vPBR.x = g_fGlobalStaticMetallic;
    Out.vDepth.w = 1.f;

    return Out;
}

PS_OUT_LIGHT PS_GRASS_ROCK_M_GREEN(PS_IN In)
{
    PS_OUT_LIGHT Out = (PS_OUT_LIGHT) 0;
    
    float4 vGrassDiffuse = g_DiffuseTexture[0].SampleLevel(DefaultSampler, (In.vTexcoord * 10.f), 0.f);
    float4 vRockDiffuse = g_DiffuseTexture[1].SampleLevel(DefaultSampler, In.vTexcoord, 0.f);
    
    float4 vMainNormal = g_NormalTexture[0].SampleLevel(DefaultSampler, In.vTexcoord, 0.f);
    float4 vGrasNormal = g_NormalTexture[1].SampleLevel(DefaultSampler, In.vTexcoord * 10.f, 0.f);
    float4 vDetailNormal = g_NormalTexture[2].SampleLevel(DefaultSampler, In.vTexcoord, 0.f);
    
    float4 vMask = g_MaskTexture[0].SampleLevel(DefaultSampler, In.vTexcoord, 0.f);
    
    //float4 vDiffuse = lerp(vRockDiffuse, (vGrassDiffuse * g_GrassColor), vMask.r);
    float4 vDiffuse = lerp(vRockDiffuse, lerp(vGrassDiffuse, g_GrassColor, g_fGrassColorIntensity), vMask.r);
    
    float3 vNormalDesc = lerp(lerp(vDetailNormal.xyz, vMainNormal.xyz, vMask.g), vGrasNormal.xyz, vMask.r);

    float3 vNormal;
	        
        //vNormal = normalize(vNormalDesc * 2.f - 1.f);
    vNormal = vNormalDesc * 2.f - 1.f;
    
    vNormal.z = sqrt(1.f - saturate(dot(vNormalDesc.xy, vNormalDesc.xy)));
    
    float3 vTangent = In.vTangent.xyz;
    float3 vBinormal = In.vBinormal.xyz * -1.f;
    float3 vInNormal = In.vNormal.xyz;

    float3x3 WorldMatrix;
    WorldMatrix = float3x3(vTangent, vBinormal, vInNormal);
        
    vNormal = normalize(mul(vNormal, WorldMatrix));
    
    vNormal = vNormal * 0.5f + 0.5f;
    
    Out.vDiffuse.xyz = vDiffuse.xyz;
    Out.vDiffuse.w = 1.f;
    
    Out.vNormal = float4(vNormal, 1.f);
    
    Out.vDepth.x = In.vProjPos.z / In.vProjPos.w;
    Out.vDepth.y = In.vProjPos.w;
    
    Out.vSSS.z = In.vProjPos.z / In.vProjPos.w;
    Out.vSSS.w = In.vProjPos.w;
    
    Out.vPBR.y = g_fGlobalStaticRoughness;
    Out.vPBR.x = g_fGlobalStaticMetallic;
    Out.vDepth.w = 1.f;

    return Out;
}

PS_OUT_LIGHT PS_GRASS_ROCK_M_BLUE(PS_IN In)
{
    PS_OUT_LIGHT Out = (PS_OUT_LIGHT) 0;
    
    float4 vGrassDiffuse = g_DiffuseTexture[0].SampleLevel(DefaultSampler, (In.vTexcoord * 10.f), 0.f);
    float4 vRockDiffuse = g_DiffuseTexture[1].SampleLevel(DefaultSampler, In.vTexcoord, 0.f);
    
    float4 vMainNormal = g_NormalTexture[0].SampleLevel(DefaultSampler, In.vTexcoord, 0.f);
    float4 vGrasNormal = g_NormalTexture[1].SampleLevel(DefaultSampler, In.vTexcoord * 10.f, 0.f);
    float4 vDetailNormal = g_NormalTexture[2].SampleLevel(DefaultSampler, In.vTexcoord, 0.f);
    
    float4 vMask = g_MaskTexture[0].SampleLevel(DefaultSampler, In.vTexcoord, 0.f);
    
    //float4 vDiffuse = lerp(vRockDiffuse, (vGrassDiffuse * g_GrassColor), vMask.r);
    float4 vDiffuse = lerp(vRockDiffuse, lerp(vGrassDiffuse, g_GrassColor, g_fGrassColorIntensity), vMask.r);
    
    float3 vNormalDesc = lerp(lerp(vDetailNormal.xyz, vMainNormal.xyz, vMask.b), vGrasNormal.xyz, vMask.r);
    
    float3 vNormal;
	        
        //vNormal = normalize(vNormalDesc * 2.f - 1.f);
    vNormal = vNormalDesc * 2.f - 1.f;
    
    vNormal.z = sqrt(1.f - saturate(dot(vNormalDesc.xy, vNormalDesc.xy)));
    
    float3 vTangent = In.vTangent.xyz;
    float3 vBinormal = In.vBinormal.xyz * -1.f;
    float3 vInNormal = In.vNormal.xyz;

    float3x3 WorldMatrix;
    WorldMatrix = float3x3(vTangent, vBinormal, vInNormal);
        
    vNormal = normalize(mul(vNormal, WorldMatrix));
    
    vNormal = vNormal * 0.5f + 0.5f;
    
    Out.vDiffuse.xyz = vDiffuse.xyz;
    Out.vDiffuse.w = 1.f;
    
    Out.vNormal = float4(vNormal, 1.f);
    
    Out.vDepth.x = In.vProjPos.z / In.vProjPos.w;
    Out.vDepth.y = In.vProjPos.w;
    
    Out.vSSS.z = In.vProjPos.z / In.vProjPos.w;
    Out.vSSS.w = In.vProjPos.w;
    
    Out.vPBR.y = g_fGlobalStaticRoughness;
    Out.vPBR.x = g_fGlobalStaticMetallic;
    Out.vDepth.w = 1.f;

    return Out;
}

PS_OUT_LIGHT PS_NONGRASS_ROCK_MA(PS_IN In)
{
    PS_OUT_LIGHT Out = (PS_OUT_LIGHT) 0;
    
    float4 vRockDiffuse = g_DiffuseTexture[1].SampleLevel(DefaultSampler, In.vTexcoord * 0.1f, 0.f);
    
    float4 vMainNormal = g_NormalTexture[0].SampleLevel(DefaultSampler, In.vTexcoord, 0.f);
    float4 vGrasNormal = g_NormalTexture[1].SampleLevel(DefaultSampler, In.vTexcoord * 10.f, 0.f);
    float4 vDetailNormal = g_NormalTexture[2].SampleLevel(DefaultSampler, In.vTexcoord * 0.1f, 0.f);
    
    float4 vMask = g_MaskTexture[0].SampleLevel(DefaultSampler, In.vTexcoord, 0.f);
    
    float4 vDiffuse = vRockDiffuse;
    
    //float3 vNormalDesc = vMainNormal.xyz;//    lerp(vDetailNormal.xyz, vMainNormal.xyz, vAlphaMask.r);
    float3 vNormalDesc = lerp(vDetailNormal.xyz, vMainNormal.xyz, vMask.b);
    
    float3 vNormal;
	        
        //vNormal = normalize(vNormalDesc * 2.f - 1.f);
    vNormal = vNormalDesc * 2.f - 1.f;
    
    vNormal.z = sqrt(1.f - saturate(dot(vNormalDesc.xy, vNormalDesc.xy)));
    
    float3 vTangent = In.vTangent.xyz;
    float3 vBinormal = In.vBinormal.xyz * -1.f;
    float3 vInNormal = In.vNormal.xyz;

    float3x3 WorldMatrix;
    WorldMatrix = float3x3(vTangent, vBinormal, vInNormal);
        
    vNormal = normalize(mul(vNormal, WorldMatrix));
    
    vNormal = vNormal * 0.5f + 0.5f;
    
    Out.vDiffuse.xyz = vDiffuse.xyz;
    Out.vDiffuse.w = 1.f;
    
    Out.vNormal = float4(vNormal, 1.f);
    
    Out.vDepth.x = In.vProjPos.z / In.vProjPos.w;
    Out.vDepth.y = In.vProjPos.w;
    
    Out.vSSS.z = In.vProjPos.z / In.vProjPos.w;
    Out.vSSS.w = In.vProjPos.w;
    
    Out.vPBR.y = g_fGlobalStaticRoughness;
    Out.vPBR.x = g_fGlobalStaticMetallic;
    Out.vDepth.w = 1.f;

    return Out;
}

PS_OUT_LIGHT PS_GRASS_ROCK_M_NONDETAIL(PS_IN In)
{
    PS_OUT_LIGHT Out = (PS_OUT_LIGHT) 0;
    
    float4 vGrassDiffuse = g_DiffuseTexture[0].SampleLevel(DefaultSampler, (In.vTexcoord * 10.f), 0.f);
    float4 vRockDiffuse = g_DiffuseTexture[1].SampleLevel(DefaultSampler, In.vTexcoord, 0.f);
    
    float4 vMainNormal = g_NormalTexture[0].SampleLevel(DefaultSampler, In.vTexcoord, 0.f);
    float4 vGrasNormal = g_NormalTexture[1].SampleLevel(DefaultSampler, In.vTexcoord * 10.f, 0.f);
    
    float4 vMask = g_MaskTexture[0].SampleLevel(DefaultSampler, In.vTexcoord, 0.f);
    
    //float4 vDiffuse = lerp(vRockDiffuse, (vGrassDiffuse * g_GrassColor), vMask.r);
    float4 vDiffuse = lerp(vRockDiffuse, lerp(vGrassDiffuse, g_GrassColor, g_fGrassColorIntensity), vMask.r);
    
    float3 vNormalDesc = lerp(vMainNormal.xyz, vGrasNormal.xyz, vMask.r);
    
    float3 vNormal;
	        
        //vNormal = normalize(vNormalDesc * 2.f - 1.f);
    vNormal = vNormalDesc * 2.f - 1.f;
    
    vNormal.z = sqrt(1.f - saturate(dot(vNormalDesc.xy, vNormalDesc.xy)));
    
    float3 vTangent = In.vTangent.xyz;
    float3 vBinormal = In.vBinormal.xyz * -1.f;
    float3 vInNormal = In.vNormal.xyz;

    float3x3 WorldMatrix;
    WorldMatrix = float3x3(vTangent, vBinormal, vInNormal);
        
    vNormal = normalize(mul(vNormal, WorldMatrix));
    
    vNormal = vNormal * 0.5f + 0.5f;
    
    Out.vDiffuse.xyz = vDiffuse.xyz;
    Out.vDiffuse.w = 1.f;
    
    Out.vNormal = float4(vNormal, 1.f);
    
    Out.vDepth.x = In.vProjPos.z / In.vProjPos.w;
    Out.vDepth.y = In.vProjPos.w;
    
    Out.vSSS.z = In.vProjPos.z / In.vProjPos.w;
    Out.vSSS.w = In.vProjPos.w;
    
    Out.vPBR.y = g_fGlobalStaticRoughness;
    Out.vPBR.x = g_fGlobalStaticMetallic;
    Out.vDepth.w = 1.f;

    return Out;
}

PS_OUT_LIGHT PS_ROCK_SONORO(PS_IN In)
{
    PS_OUT_LIGHT Out = (PS_OUT_LIGHT) 0;
    
    float4 vRockDiffuse = g_DiffuseTexture[0].SampleLevel(DefaultSampler, In.vTexcoord * 5.f, 0.f);

    float4 vMainNormal = g_NormalTexture[0].SampleLevel(DefaultSampler, In.vTexcoord, 0.f);
    float4 vRockNormal = g_NormalTexture[1].SampleLevel(DefaultSampler, In.vTexcoord * 5.f, 0.f);
    
    float4 vMask = g_MaskTexture[0].SampleLevel(DefaultSampler, In.vTexcoord, 0.f);
    
    float fMask = max(max(vMask.r, vMask.g), vMask.b);
    
    float4 vDiffuse = vRockDiffuse;
    
    float3 vNormalDesc = lerp(vMainNormal, vRockNormal, fMask);
    
    float3 vNormal;
	        
        //vNormal = normalize(vNormalDesc * 2.f - 1.f);
    vNormal = vNormalDesc * 2.f - 1.f;
    
    vNormal.z = sqrt(1.f - saturate(dot(vNormalDesc.xy, vNormalDesc.xy)));
    
    float3 vTangent = In.vTangent.xyz;
    float3 vBinormal = In.vBinormal.xyz * -1.f;
    float3 vInNormal = In.vNormal.xyz;

    float3x3 WorldMatrix;
    WorldMatrix = float3x3(vTangent, vBinormal, vInNormal);
        
    vNormal = normalize(mul(vNormal, WorldMatrix));
    
    vNormal = vNormal * 0.5f + 0.5f;
    
    Out.vDiffuse.xyz = vDiffuse.xyz;
    Out.vDiffuse.w = 1.f;
    
    Out.vNormal = float4(vNormal, 1.f);
    
    Out.vDepth.x = In.vProjPos.z / In.vProjPos.w;
    Out.vDepth.y = In.vProjPos.w;
    
    Out.vSSS.z = In.vProjPos.z / In.vProjPos.w;
    Out.vSSS.w = In.vProjPos.w;
    
    Out.vPBR.y = g_fGlobalStaticRoughness;
    Out.vPBR.x = g_fGlobalStaticMetallic;
    Out.vDepth.w = 1.f;

    return Out;
}

PS_OUT_LIGHT PS_ROCK_SONORO_BIG(PS_IN In)
{
    PS_OUT_LIGHT Out = (PS_OUT_LIGHT) 0;
    
    float4 vRockDiffuse = g_DiffuseTexture[0].SampleLevel(DefaultSampler, In.vTexcoord * 2.f, 0.f);

    float4 vMainNormal = g_NormalTexture[0].SampleLevel(DefaultSampler, In.vTexcoord, 0.f);
    float4 vRockNormal = g_NormalTexture[1].SampleLevel(DefaultSampler, In.vTexcoord * 2.f, 0.f);
    
    float4 vMask = g_MaskTexture[0].SampleLevel(DefaultSampler, In.vTexcoord, 0.f);
    
    float fMask = step(0.5f, max(max(vMask.r, vMask.g), vMask.b));
    
    float4 vDiffuse = vRockDiffuse;
    
    float3 vNormalDesc = lerp(vMainNormal, vRockNormal, fMask);
    
    float3 vNormal;
	        
        //vNormal = normalize(vNormalDesc * 2.f - 1.f);
    vNormal = vNormalDesc * 2.f - 1.f;
    
    vNormal.z = sqrt(1.f - saturate(dot(vNormalDesc.xy, vNormalDesc.xy)));
    
    float3 vTangent = In.vTangent.xyz;
    float3 vBinormal = In.vBinormal.xyz * -1.f;
    float3 vInNormal = In.vNormal.xyz;

    float3x3 WorldMatrix;
    WorldMatrix = float3x3(vTangent, vBinormal, vInNormal);
        
    vNormal = normalize(mul(vNormal, WorldMatrix));
    
    vNormal = vNormal * 0.5f + 0.5f;
    
    Out.vDiffuse.xyz = vDiffuse.xyz;
    Out.vDiffuse.w = 1.f;
    
    Out.vNormal = float4(vNormal, 1.f);
    
    Out.vDepth.x = In.vProjPos.z / In.vProjPos.w;
    Out.vDepth.y = In.vProjPos.w;
    
    Out.vSSS.z = In.vProjPos.z / In.vProjPos.w;
    Out.vSSS.w = In.vProjPos.w;
    
    Out.vPBR.y = g_fGlobalStaticRoughness;
    Out.vPBR.x = g_fGlobalStaticMetallic;
    Out.vDepth.w = 1.f;

    return Out;
}

PS_OUT_LIGHT PS_MAIN_EMISSIVE_GLASS(PS_IN In)
{
    PS_OUT_LIGHT Out = (PS_OUT_LIGHT) 0;
    
    vector vMask = g_MaskTexture[0].Sample(DefaultSampler, In.vTexcoord);
    
    vector vDiffuse = g_DiffuseTexture[0].Sample(DefaultSampler, In.vTexcoord);
    if (length(vDiffuse) == 0.f)
        vDiffuse = 1.f;
    
    vector vMaskDiffiuse = g_DiffuseTexture[1].Sample(DefaultSampler, In.vTexcoord);

    if (g_HasMask)
    {
        Out.vDiffuse = vDiffuse * vMask.r + vDiffuse * (1.f - vMask.r);
        Out.vDiffuse = Out.vDiffuse * vMask.g + vMaskDiffiuse * (1.f - vMask.g);
    }
    else
    {
        Out.vDiffuse = vDiffuse;
    }

    //if (vMask.a == 1.f)
    //    Out.vEmissive = float4((Out.vDiffuse.xyz) * 0.4f, 1.f);

    //천국에 엄청 큰 애들은 좀 더 낮춰야할지도?
    if (vMask.a == 1.f)
        Out.vEmissive = float4((Out.vDiffuse.xyz) * 0.2f, 1.f);

    Out.vDiffuse.w = 1.f;
    
    Out.vPBR.y = g_fGlobalStaticRoughness;
    Out.vPBR.x = g_fGlobalStaticMetallic;
    
    float4 vNormal;
    
    if (g_HasNormal)
    {
        if (g_HasMask)
        {
            vector vDefaultNormal = g_NormalTexture[0].SampleLevel(DefaultSampler, In.vTexcoord, 0);
            
            float4 vNormal1 = normalize(vDefaultNormal * 2.f - 1.f);
            vNormal1.z = sqrt(1.f - saturate(dot(vDefaultNormal.xy, vDefaultNormal.xy)));

            vector vMaskNormal = g_NormalTexture[1].Sample(DefaultSampler, In.vTexcoord);
        
            vNormal = vNormal1 * (vMask.r) + vNormal1 * (1.f - vMask.r);
        }
        else
        {
            vector vDefaultNormal = g_NormalTexture[0].Sample(DefaultSampler, In.vTexcoord);
		
	        
            vNormal = normalize(vDefaultNormal * 2.f - 1.f);
            if (vDefaultNormal.x > vDefaultNormal.z && vDefaultNormal.y > vDefaultNormal.z)
                vNormal.z = sqrt(1.f - saturate(dot(vDefaultNormal.xy, vDefaultNormal.xy)));
        }

        float3 vTangent = In.vTangent.xyz;
        float3 vBinormal = In.vBinormal.xyz * -1.f;
        float3 vInNormal = In.vNormal.xyz;

        float3x3 WorldMatrix;
        WorldMatrix = float3x3(vTangent, vBinormal, vInNormal);
        
        vNormal.xyz = normalize(mul(vNormal.xyz, WorldMatrix));
        vNormal.xyz = vNormal * 0.5f + 0.5f;
    }
    else
    {
        vNormal = In.vNormal;
        vNormal = vNormal * 0.5f + 0.5f;
    }
    
    Out.vNormal = float4(vNormal.xyz, 1.f);
    Out.vDepth.x = In.vProjPos.z / In.vProjPos.w;
    Out.vDepth.y = In.vProjPos.w;
    
    Out.vSSS.z = In.vProjPos.z / In.vProjPos.w;
    Out.vSSS.w = In.vProjPos.w;
    
    Out.vDepth.w = 1.f;

    return Out;
}

PS_OUT_LIGHT PS_MAIN_EMISSIVE_GRASS(PS_IN In)
{
    PS_OUT_LIGHT Out = (PS_OUT_LIGHT) 0;
    
    vector vMask = g_MaskTexture[0].Sample(DefaultSampler, In.vTexcoord);
    
    vector vDiffuse = g_DiffuseTexture[0].Sample(DefaultSampler, In.vTexcoord);
    if (length(vDiffuse) == 0.f)
        vDiffuse = 1.f;
    
    vector vMaskDiffiuse = g_DiffuseTexture[1].Sample(DefaultSampler, In.vTexcoord);


    Out.vDiffuse = vDiffuse;

    if (vMask.r == 0.f)
        Out.vEmissive = float4((vMask.xyz), 1.f);

    Out.vDiffuse.w = 1.f;
    
    Out.vPBR.y = g_fGlobalStaticRoughness;
    Out.vPBR.x = g_fGlobalStaticMetallic;
    
    float4 vNormal;
    
    if (g_HasNormal)
    {

        vector vDefaultNormal = g_NormalTexture[0].Sample(DefaultSampler, In.vTexcoord);
		
	        
        vNormal = normalize(vDefaultNormal * 2.f - 1.f);
        if (vDefaultNormal.x > vDefaultNormal.z && vDefaultNormal.y > vDefaultNormal.z)
            vNormal.z = sqrt(1.f - saturate(dot(vDefaultNormal.xy, vDefaultNormal.xy)));

        float3 vTangent = In.vTangent.xyz;
        float3 vBinormal = In.vBinormal.xyz * -1.f;
        float3 vInNormal = In.vNormal.xyz;

        float3x3 WorldMatrix;
        WorldMatrix = float3x3(vTangent, vBinormal, vInNormal);
        
        vNormal.xyz = normalize(mul(vNormal.xyz, WorldMatrix));
        vNormal.xyz = vNormal * 0.5f + 0.5f;
    }
    else
    {
        vNormal = In.vNormal;
        vNormal = vNormal * 0.5f + 0.5f;
    }
    
    Out.vNormal = float4(vNormal.xyz, 1.f);
    Out.vDepth.x = In.vProjPos.z / In.vProjPos.w;
    Out.vDepth.y = In.vProjPos.w;
    
    Out.vSSS.z = In.vProjPos.z / In.vProjPos.w;
    Out.vSSS.w = In.vProjPos.w;
    
    Out.vDepth.w = 1.f;

    return Out;
}
PS_OUT_LIGHT PS_MAIN_TREE_GRASS(PS_IN In)
{
    PS_OUT_LIGHT Out = (PS_OUT_LIGHT) 0;
    
    vector vMask = g_MaskTexture[0].Sample(DefaultSampler, In.vTexcoord);
    
    vector vDiffuse = g_DiffuseTexture[0].Sample(DefaultSampler, In.vTexcoord);
    vector vMaskDiffiuse = g_DiffuseTexture[1].Sample(DefaultSampler, In.vTexcoord);

    if (g_HasMask)
    {
        Out.vDiffuse = vDiffuse * vMask.r + vDiffuse * (1.f - vMask.r);
        Out.vDiffuse = Out.vDiffuse * vMask.g + vMaskDiffiuse * (1.f - vMask.g);

    }
    else
    {
        Out.vDiffuse = vDiffuse;
    }
    
    if (Out.vDiffuse.g > 0.7f && Out.vDiffuse.a <0.1f)
        discard;
    
    Out.vDiffuse.w = 1.f;
    
    Out.vPBR.y = g_fGlobalStaticRoughness;
    Out.vPBR.x = g_fGlobalStaticMetallic;
    
    if (g_IsDynamicObject)
    {
        Out.vDepth.z = 1.f;
    }
    
    float4 vNormal;
    
    if (g_HasNormal)
    {
        if (g_HasMask)
        {
            vector vDefaultNormal = g_NormalTexture[0].SampleLevel(DefaultSampler, In.vTexcoord, 0);
            
            float4 vNormal1 = normalize(vDefaultNormal * 2.f - 1.f);
            vNormal1.z = sqrt(1.f - saturate(dot(vDefaultNormal.xy, vDefaultNormal.xy)));

            vector vMaskNormal = g_NormalTexture[1].Sample(DefaultSampler, In.vTexcoord);
            
            vNormal = vNormal1 * (vMask.r) + vNormal1 * (1.f - vMask.r);
        }
        else
        {
            vector vDefaultNormal = g_NormalTexture[0].Sample(DefaultSampler, In.vTexcoord);
		
	        
            vNormal = normalize(vDefaultNormal * 2.f - 1.f);
            if (vDefaultNormal.x > vDefaultNormal.z && vDefaultNormal.y > vDefaultNormal.z)
                vNormal.z = sqrt(1.f - saturate(dot(vDefaultNormal.xy, vDefaultNormal.xy)));
        }

        float3 vTangent = In.vTangent.xyz;
        float3 vBinormal = In.vBinormal.xyz * -1.f;
        float3 vInNormal = In.vNormal.xyz;

        float3x3 WorldMatrix;
        WorldMatrix = float3x3(vTangent, vBinormal, vInNormal);
        
        vNormal.xyz = normalize(mul(vNormal.xyz, WorldMatrix));
        vNormal.xyz = vNormal * 0.5f + 0.5f;
    }
    else
    {
        vNormal = In.vNormal;
        vNormal = vNormal * 0.5f + 0.5f;
    }
    
    Out.vNormal = float4(vNormal.xyz, 1.f);
    Out.vDepth.x = In.vProjPos.z / In.vProjPos.w;
    Out.vDepth.y = In.vProjPos.w;
    
    Out.vDepth.w = 1.f;
    
    return Out;
}

PS_OUT_LIGHT PS_MAIN_EMISSIVE_SONORO(PS_IN In)
{
    PS_OUT_LIGHT Out = (PS_OUT_LIGHT) 0;
    
    vector vMask = g_MaskTexture[0].Sample(DefaultSampler, In.vTexcoord);
    
    vector vDiffuse = g_DiffuseTexture[0].Sample(DefaultSampler, In.vTexcoord);
    if (length(vDiffuse) == 0.f)
        vDiffuse = 1.f;
    
    vector vMaskDiffiuse = g_DiffuseTexture[1].Sample(DefaultSampler, In.vTexcoord);

    Out.vDiffuse = vDiffuse;

    if (vMask.a != 0.f)
        Out.vEmissive = float4(0.1f, 0.2f, 0.6f, 1.f);

    Out.vDiffuse.w = 1.f;
    
    Out.vPBR.y = g_fGlobalStaticRoughness;
    Out.vPBR.x = g_fGlobalStaticMetallic;
    
    float4 vNormal;
    
    if (g_HasNormal)
    {
        if (g_HasMask)
        {
            vector vDefaultNormal = g_NormalTexture[0].SampleLevel(DefaultSampler, In.vTexcoord, 0);
            
            float4 vNormal1 = normalize(vDefaultNormal * 2.f - 1.f);
            vNormal1.z = sqrt(1.f - saturate(dot(vDefaultNormal.xy, vDefaultNormal.xy)));

            vector vMaskNormal = g_NormalTexture[1].Sample(DefaultSampler, In.vTexcoord);
        
            vNormal = vNormal1 * (vMask.r) + vNormal1 * (1.f - vMask.r);
        }
        else
        {
            vector vDefaultNormal = g_NormalTexture[0].Sample(DefaultSampler, In.vTexcoord);
		
	        
            vNormal = normalize(vDefaultNormal * 2.f - 1.f);
            if (vDefaultNormal.x > vDefaultNormal.z && vDefaultNormal.y > vDefaultNormal.z)
                vNormal.z = sqrt(1.f - saturate(dot(vDefaultNormal.xy, vDefaultNormal.xy)));
        }

        float3 vTangent = In.vTangent.xyz;
        float3 vBinormal = In.vBinormal.xyz * -1.f;
        float3 vInNormal = In.vNormal.xyz;

        float3x3 WorldMatrix;
        WorldMatrix = float3x3(vTangent, vBinormal, vInNormal);
        
        vNormal.xyz = normalize(mul(vNormal.xyz, WorldMatrix));
        vNormal.xyz = vNormal * 0.5f + 0.5f;
    }
    else
    {
        vNormal = In.vNormal;
        vNormal = vNormal * 0.5f + 0.5f;
    }
    
    Out.vNormal = float4(vNormal.xyz, 1.f);
    Out.vDepth.x = In.vProjPos.z / In.vProjPos.w;
    Out.vDepth.y = In.vProjPos.w;
    
    Out.vSSS.z = In.vProjPos.z / In.vProjPos.w;
    Out.vSSS.w = In.vProjPos.w;
    
    Out.vDepth.w = 1.f;

    return Out;
}

PS_OUT_LIGHT PS_MAIN_EMISSIVE_SONORO_LIGHT(PS_IN In)
{
    PS_OUT_LIGHT Out = (PS_OUT_LIGHT) 0;
    
    vector vMask = g_MaskTexture[0].Sample(DefaultSampler, In.vTexcoord);
    
    vector vDiffuse = g_DiffuseTexture[0].Sample(DefaultSampler, In.vTexcoord);
    if (length(vDiffuse) == 0.f)
        vDiffuse = 1.f;
    
    Out.vDiffuse = vDiffuse;

    if (Out.vDiffuse.a > 0.f)
        Out.vEmissive = float4((Out.vDiffuse.xyz) * 0.7f, 1.f);
    
    Out.vDiffuse.w = 1.f;
    
    Out.vPBR.y = g_fGlobalStaticRoughness;
    Out.vPBR.x = g_fGlobalStaticMetallic;
    
    float4 vNormal;
    
    if (g_HasNormal)
    {

        vector vDefaultNormal = g_NormalTexture[0].Sample(DefaultSampler, In.vTexcoord);
		
	        
        vNormal = normalize(vDefaultNormal * 2.f - 1.f);
        if (vDefaultNormal.x > vDefaultNormal.z && vDefaultNormal.y > vDefaultNormal.z)
            vNormal.z = sqrt(1.f - saturate(dot(vDefaultNormal.xy, vDefaultNormal.xy)));

        float3 vTangent = In.vTangent.xyz;
        float3 vBinormal = In.vBinormal.xyz * -1.f;
        float3 vInNormal = In.vNormal.xyz;

        float3x3 WorldMatrix;
        WorldMatrix = float3x3(vTangent, vBinormal, vInNormal);
        
        vNormal.xyz = normalize(mul(vNormal.xyz, WorldMatrix));
        vNormal.xyz = vNormal * 0.5f + 0.5f;
    }
    else
    {
        vNormal = In.vNormal;
        vNormal = vNormal * 0.5f + 0.5f;
    }
    
    Out.vNormal = float4(vNormal.xyz, 1.f);
    Out.vDepth.x = In.vProjPos.z / In.vProjPos.w;
    Out.vDepth.y = In.vProjPos.w;
    
    Out.vSSS.z = In.vProjPos.z / In.vProjPos.w;
    Out.vSSS.w = In.vProjPos.w;
    
    Out.vDepth.w = 1.f;

    return Out;
}

PS_OUT_LIGHT PS_MAIN_EMISSIVE_THROW_OBJECT(PS_IN In)
{
    PS_OUT_LIGHT Out = (PS_OUT_LIGHT) 0;
    
    vector vMask = g_MaskTexture[0].Sample(DefaultSampler, In.vTexcoord);
    
    vector vDiffuse = g_DiffuseTexture[0].Sample(DefaultSampler, In.vTexcoord);
    if (length(vDiffuse) == 0.f)
        vDiffuse = 1.f;
    
    Out.vDiffuse = vDiffuse;

    Out.vEmissive = Out.vDiffuse * vMask.r*0.5f;
    
    Out.vDiffuse.w = 1.f;
    
    Out.vPBR.y = g_fGlobalStaticRoughness;
    Out.vPBR.x = g_fGlobalStaticMetallic;
    
    float4 vNormal;
    
    if (g_HasNormal)
    {

        vector vDefaultNormal = g_NormalTexture[0].Sample(DefaultSampler, In.vTexcoord);
		
	        
        vNormal = normalize(vDefaultNormal * 2.f - 1.f);
        if (vDefaultNormal.x > vDefaultNormal.z && vDefaultNormal.y > vDefaultNormal.z)
            vNormal.z = sqrt(1.f - saturate(dot(vDefaultNormal.xy, vDefaultNormal.xy)));

        float3 vTangent = In.vTangent.xyz;
        float3 vBinormal = In.vBinormal.xyz * -1.f;
        float3 vInNormal = In.vNormal.xyz;

        float3x3 WorldMatrix;
        WorldMatrix = float3x3(vTangent, vBinormal, vInNormal);
        
        vNormal.xyz = normalize(mul(vNormal.xyz, WorldMatrix));
        vNormal.xyz = vNormal * 0.5f + 0.5f;
    }
    else
    {
        vNormal = In.vNormal;
        vNormal = vNormal * 0.5f + 0.5f;
    }
    
    Out.vNormal = float4(vNormal.xyz, 1.f);
    Out.vDepth.x = In.vProjPos.z / In.vProjPos.w;
    Out.vDepth.y = In.vProjPos.w;
    
    Out.vSSS.z = In.vProjPos.z / In.vProjPos.w;
    Out.vSSS.w = In.vProjPos.w;
    
    Out.vDepth.w = 1.f;

    return Out;
}

PS_OUT_LIGHT PS_MAIN_TREE_BURN_EMISSIVE(PS_IN In)
{
    PS_OUT_LIGHT Out = (PS_OUT_LIGHT) 0;
    
    vector vDissolve = g_MaskTexture[0].Sample(DefaultSampler, In.vTexcoord);
    
    vector vDiffuse = g_DiffuseTexture[0].Sample(DefaultSampler, In.vTexcoord);
    if (length(vDiffuse) == 0.f)
        vDiffuse = 1.f;
    
    Out.vDiffuse = vDiffuse;

    if (g_DissolveStart)
    {
        if (vDissolve.r - g_DissolveTime <= 0.2f)
            discard;
        else if (vDissolve.r - g_DissolveTime < 0.3f)
            Out.vEmissive = float4(float3(Out.vDiffuse.rgb) * float3(0.7f, 0.3f, 0.f), 1.f);
    }
    Out.vDiffuse.w = 1.f;
    
    Out.vPBR.y = g_fGlobalStaticRoughness;
    Out.vPBR.x = g_fGlobalStaticMetallic;
    
    float4 vNormal;
    
    if (g_HasNormal)
    {

        vector vDefaultNormal = g_NormalTexture[0].Sample(DefaultSampler, In.vTexcoord);
		
	        
        vNormal = normalize(vDefaultNormal * 2.f - 1.f);
        if (vDefaultNormal.x > vDefaultNormal.z && vDefaultNormal.y > vDefaultNormal.z)
            vNormal.z = sqrt(1.f - saturate(dot(vDefaultNormal.xy, vDefaultNormal.xy)));

        float3 vTangent = In.vTangent.xyz;
        float3 vBinormal = In.vBinormal.xyz * -1.f;
        float3 vInNormal = In.vNormal.xyz;

        float3x3 WorldMatrix;
        WorldMatrix = float3x3(vTangent, vBinormal, vInNormal);
        
        vNormal.xyz = normalize(mul(vNormal.xyz, WorldMatrix));
        vNormal.xyz = vNormal * 0.5f + 0.5f;
    }
    else
    {
        vNormal = In.vNormal;
        vNormal = vNormal * 0.5f + 0.5f;
    }
    
    Out.vNormal = float4(vNormal.xyz, 1.f);
    Out.vDepth.x = In.vProjPos.z / In.vProjPos.w;
    Out.vDepth.y = In.vProjPos.w;
    
    Out.vSSS.z = In.vProjPos.z / In.vProjPos.w;
    Out.vSSS.w = In.vProjPos.w;
    
    Out.vDepth.w = 1.f;

    return Out;
}

struct PS_OUT_NONLIGHT
{
    vector vBackBuffer : SV_TARGET0;
    vector vNormal : SV_TARGET1;
    vector vDepth : SV_TARGET2;
    vector vPBR : SV_TARGET3;
};


PS_OUT_NONLIGHT PS_MAIN_DOME_DISTORTION_EMISSIVE(PS_IN In)
{
    PS_OUT_NONLIGHT Out = (PS_OUT_NONLIGHT) 0;
    
    float2 vTempTexcoord = In.vTexcoord + float2(g_DistortionTime * 0.01f, g_DistortionTime * 0.05f);

    vector vDiffuse = g_DiffuseTexture[0].Sample(DefaultSampler, frac(vTempTexcoord));
    
    Out.vBackBuffer = vDiffuse;
    Out.vBackBuffer.a = g_fAlpha;
    
    if (g_DissolveStart)
    {
        float vDissolve = g_MaskTexture[0].Sample(DefaultSampler, In.vTexcoord).r;

        if (vDissolve < g_DissolveTime * 0.2f)
            discard;
    }

    if (length(vDiffuse) == 0.f)
        vDiffuse = 1.f;

    return Out;
}

PS_OUT_LIGHT PS_MAIN_OVF_PALETTE(PS_IN In)
{
    PS_OUT_LIGHT Out = (PS_OUT_LIGHT) 0;
    
    vector vMask = g_MaskTexture[0].Sample(DefaultSampler, In.vTexcoord);
    
    vector vDiffuse = g_DiffuseTexture[0].Sample(DefaultSampler, In.vTexcoord);
    
    Out.vDiffuse = vDiffuse;
    
    if (length(vDiffuse) == 0.f)
        vDiffuse = 1.f;
 
    if (Out.vDiffuse.a >= 0.4f)
        Out.vEmissive = float4(Out.vDiffuse.rgb * Out.vDiffuse.a, 1.f);
        
    Out.vDiffuse.w = 1.f;
    
    Out.vPBR.y = g_fGlobalStaticRoughness;
    Out.vPBR.x = g_fGlobalStaticMetallic;
    
    float4 vNormal;
    
    if (g_HasNormal)
    {

        vector vDefaultNormal = g_NormalTexture[0].Sample(DefaultSampler, In.vTexcoord);
		
	        
        vNormal = normalize(vDefaultNormal * 2.f - 1.f);
        if (vDefaultNormal.x > vDefaultNormal.z && vDefaultNormal.y > vDefaultNormal.z)
            vNormal.z = sqrt(1.f - saturate(dot(vDefaultNormal.xy, vDefaultNormal.xy)));

        float3 vTangent = In.vTangent.xyz;
        float3 vBinormal = In.vBinormal.xyz * -1.f;
        float3 vInNormal = In.vNormal.xyz;

        float3x3 WorldMatrix;
        WorldMatrix = float3x3(vTangent, vBinormal, vInNormal);
        
        vNormal.xyz = normalize(mul(vNormal.xyz, WorldMatrix));
        vNormal.xyz = vNormal * 0.5f + 0.5f;
    }
    else
    {
        vNormal = In.vNormal;
        vNormal = vNormal * 0.5f + 0.5f;
    }
    
    Out.vNormal = float4(vNormal.xyz, 1.f);
    Out.vDepth.x = In.vProjPos.z / In.vProjPos.w;
    Out.vDepth.y = In.vProjPos.w;
    
    Out.vSSS.z = In.vProjPos.z / In.vProjPos.w;
    Out.vSSS.w = In.vProjPos.w;
    
    Out.vDepth.w = 1.f;

    return Out;
}

PS_OUT_LIGHT PS_MAIN_FORCE_EMISSIVE(PS_IN In)
{
    PS_OUT_LIGHT Out = (PS_OUT_LIGHT) 0;
    
    vector vMask = g_MaskTexture[0].Sample(DefaultSampler, In.vTexcoord);
    
    vector vDiffuse = g_DiffuseTexture[0].Sample(DefaultSampler, In.vTexcoord);
    
    Out.vDiffuse = vDiffuse;
    
    if (length(vDiffuse) == 0.f)
        vDiffuse = 1.f;
 
    if (vMask.r > 0.5f)
    {
        float TestEmissive =  Out.vDiffuse.gb * Out.vDiffuse.a;
        Out.vEmissive = float4(TestEmissive, TestEmissive, TestEmissive, 1.f);
    }
        Out.vDiffuse.w = 1.f;
    
        Out.vPBR.y = g_fGlobalStaticRoughness;
        Out.vPBR.x = g_fGlobalStaticMetallic;
    
        float4 vNormal;
    
        if (g_HasNormal)
        {
            vector vDefaultNormal = g_NormalTexture[0].Sample(DefaultSampler, In.vTexcoord);
		
	        
            vNormal = normalize(vDefaultNormal * 2.f - 1.f);
            if (vDefaultNormal.x > vDefaultNormal.z && vDefaultNormal.y > vDefaultNormal.z)
                vNormal.z = sqrt(1.f - saturate(dot(vDefaultNormal.xy, vDefaultNormal.xy)));

            float3 vTangent = In.vTangent.xyz;
            float3 vBinormal = In.vBinormal.xyz * -1.f;
            float3 vInNormal = In.vNormal.xyz;

            float3x3 WorldMatrix;
            WorldMatrix = float3x3(vTangent, vBinormal, vInNormal);
        
            vNormal.xyz = normalize(mul(vNormal.xyz, WorldMatrix));
            vNormal.xyz = vNormal * 0.5f + 0.5f;
        }
        else
        {
            vNormal = In.vNormal;
            vNormal = vNormal * 0.5f + 0.5f;
        }
    
        Out.vNormal = float4(vNormal.xyz, 1.f);
        Out.vDepth.x = In.vProjPos.z / In.vProjPos.w;
        Out.vDepth.y = In.vProjPos.w;
    
        Out.vSSS.z = In.vProjPos.z / In.vProjPos.w;
        Out.vSSS.w = In.vProjPos.w;
    
        Out.vDepth.w = 1.f;

        return Out;
    }

PS_OUT_LIGHT PS_MAIN_DOME_PHAZE2(PS_IN In)
{
    PS_OUT_LIGHT Out = (PS_OUT_LIGHT) 0;
    
    vector vMask = g_MaskTexture[0].Sample(DefaultSampler, In.vTexcoord);
    
    float2 vTempTexcoord = In.vTexcoord + float2(g_DistortionTime * 0.01f, g_DistortionTime * 0.05f);
    vector vDiffuse = g_DiffuseTexture[1].Sample(DefaultSampler, frac(vTempTexcoord));
    
    Out.vDistortion = vMask.r;
    Out.vDiffuse = vDiffuse;
    
    if (length(vDiffuse) == 0.f)
        vDiffuse = 1.f;

    Out.vDiffuse.w = 1.f;
    
    Out.vPBR.y = g_fGlobalStaticRoughness;
    Out.vPBR.x = g_fGlobalStaticMetallic;

    Out.vDepth.x = In.vProjPos.z / In.vProjPos.w;
    Out.vDepth.y = In.vProjPos.w;
    
    Out.vSSS.z = In.vProjPos.z / In.vProjPos.w;
    Out.vSSS.w = In.vProjPos.w;
    
    Out.vDepth.w = 1.f;
    return Out;
}

PS_OUT_LIGHT PS_MAIN_METEOR(PS_IN In)
{
    PS_OUT_LIGHT Out = (PS_OUT_LIGHT) 0;
    
    vector vMask = g_MaskTexture[0].Sample(DefaultSampler, In.vTexcoord);
    
    if (vMask.a == 0)
        Out.vDiffuse = float4(0.3388785421848297f,
   0.44620344042778015f,
   0.6740331649780273f,
   1.0);
    else
    {
        Out.vDiffuse = float4(0.33625346422195435f, 0.5338311195373535f, 0.8950276374816895f, 1.f);

        Out.vEmissive = Out.vDiffuse;
    }
    
    Out.vDiffuse.w = 1.f;
    
    Out.vPBR.y = g_fGlobalStaticRoughness;
    Out.vPBR.x = g_fGlobalStaticMetallic;

    Out.vDepth.x = In.vProjPos.z / In.vProjPos.w;
    Out.vDepth.y = In.vProjPos.w;
    
    Out.vSSS.z = In.vProjPos.z / In.vProjPos.w;
    Out.vSSS.w = In.vProjPos.w;
    
    Out.vDepth.w = 1.f;
    return Out;
}

technique11 DefaultTechnique
{
    pass DefaultPass // 0
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_NORMAL();
    }

    pass AlphaNotDiscard // 1
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_NORMAL_ALPHA();
    }
    pass AlphaNotDiscardNonCull // 2
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_NORMAL_ALPHA();
    }
    pass SelectedObject // 3
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_NORMAL_FOCUS();
    }

    pass LightObject // 4
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_EMISSIVE_LIGHT();
    }
    pass ShadowPass //5
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_SHADOW();
        GeometryShader = compile gs_5_0 GS_SHADOW();
        PixelShader = compile ps_5_0 PS_SHADOW();
    }
    
    pass OutlinePass //6
    {
        SetRasterizerState(RS_Cull_Front);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_OUTLINE();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_OUTLINE();
    }
    
    pass Emissive //7
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_EMISSIVE();
    }
    
    pass ShadowMapPass // 8
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_SHADOW_MAP();
        GeometryShader = NULL;
        PixelShader = NULL;
    }
    
    pass LogoMountain // 9
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_LOGOMOUNTAIN();
    }
    
    pass GrassRock_MA // 10
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_GRASS_ROCK_MA();
    }
    

    pass GrassRock_M_Green // 11
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_GRASS_ROCK_M_GREEN();
    }
    
    pass GrassRock_M_Blue // 12
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_GRASS_ROCK_M_BLUE();
    }
    
    pass NonGrassRock_MA // 13
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_NONGRASS_ROCK_MA();
    }
    
    pass GrassRock_M_NonDetail // 14
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_GRASS_ROCK_M_NONDETAIL();
    }
    
    pass Rock_Sonoro // 15
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_ROCK_SONORO();
    }
    
    pass Rock_Sonoro_Big // 16
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_ROCK_SONORO_BIG();
    }

    pass Asphodel_Glass // 17
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_EMISSIVE_GLASS();
    }

    pass Emissive_Grass // 18
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_EMISSIVE_GRASS();
    }

    pass Tree_Grass // 19
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_TREE_GRASS();
    }
    pass Sonoro_Emissive // 20
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_EMISSIVE_SONORO();
    }
    pass Sonoro_Light_Emissive // 21
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_EMISSIVE_SONORO_LIGHT();
    }

    pass Throw_Object_Emissive// 22
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_EMISSIVE_THROW_OBJECT();
    }

    pass Tree_Burn_Emissive // 23
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_TREE_BURN_EMISSIVE();
    }

    pass Dome_Distortion_Emissive// 24
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_DOME_DISTORTION_EMISSIVE();
    }

    pass Heaven_Palette // 25
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_OVF_PALETTE();
    }

    pass Heaven_Emissive // 26
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_FORCE_EMISSIVE();
    }

    pass Heaven_After_LeviRevive // 27
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_DOME_DISTORTION_EMISSIVE();
    }

    pass Meteor_Color// 28
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_METEOR();
    }
}
