#include "Engine_Shader_Function.hlsli"

//OBJECTS
Texture2D g_DiffuseTexture; // Color
Texture2D g_NormalTexture;  // Normal
Texture2D g_DepthTexture;   // (Depth.x = Proj.z / Proj.w) , (Depth.y = Proj.w)
Texture2D g_PBRTexture;     // (PBR.x = Metallic), (PBR.y = Roughness ), (PBR.z = IsDynamic) 
//Light
vector  g_vLightDirection = 0.f;
vector  g_vLightDiffuse = 1.f;
vector  g_vLightAmbient = 1.f;
vector  g_vLightPosition;
float   g_fLightRange; 
vector  g_vLightSpecular = 1.f;
vector  g_vMtrlSpecular = 1.f;

vector  g_vDynamicMtrlAmbient = 0.5f;
vector  g_vStaticMtrlAmbient = 0.3f;

struct VS_IN
{
    float3 vPosition : POSITION;
    float2 vTexcoord : TEXCOORD0;
};

struct VS_OUT
{
    float4 vPosition : SV_POSITION;
    float2 vTexcoord : TEXCOORD0;
};

VS_OUT VS_MAIN(VS_IN In)
{
    VS_OUT Out = (VS_OUT)0;
    
    matrix matWV, matWVP;
    
    matWV = mul(g_WorldMatrix, g_ViewMatrix);
    matWVP = mul(matWV, g_ProjMatrix);
    Out.vPosition = mul(float4(In.vPosition, 1.f), matWVP);
    Out.vTexcoord = In.vTexcoord;

    return Out;
}

struct PS_IN
{
    float4 vPosition : SV_POSITION;
    float2 vTexcoord : TEXCOORD0;
};

struct PS_OUT_BACKBUFFER
{
    float4 vColor : SV_TARGET0;
};

struct PS_OUT_LIGHT
{
    float4 vColor : SV_TARGET0;
};

PS_OUT_LIGHT PS_LIGHT_DIRECTIONAL(PS_IN In)
{
    PS_OUT_LIGHT Out = (PS_OUT_LIGHT) 0;
    
    vector vDiffuse = g_DiffuseTexture.Sample(DefaultSampler, In.vTexcoord);
    
    if (vDiffuse.r == 1.f && vDiffuse.g == 0.f && vDiffuse.b == 1.f)
        discard;
        
    float4 vNormal = Compute_Normal(g_NormalTexture, DefaultSampler, In.vTexcoord);

    vector vWorldPos = Compute_WorldPos(In.vTexcoord, g_DepthTexture);
    
    vector vLook = normalize(g_vCamPosition - vWorldPos);
    
    float3 vLightDir = g_vLightDirection.xyz * -1.f;
    
    vector vPBRDesc = g_PBRTexture.Sample(DefaultSampler, In.vTexcoord);
   
    float NdotL = dot(normalize(vLightDir), vNormal.xyz);
    
    float fRimPower = Compute_RimPower(vNormal, vLook, NdotL);

    float3 vRimColor = g_vLightDiffuse.xyz;
    
    float3 vAmbient = 0.f;
    
    float3 vLightDiffuse = 0.f;
    float3 vLightSpecular = 0.f;
    
    float3 vResultDiffuse = 0.f;
    float3 vResultSpecular = 0.f;
    
    float4 vAmbientColor = 0.f;
    
    vector vViewPos = Compute_ViewPos(In.vTexcoord, g_DepthTexture);
    float fViewZ = vViewPos.z;

    {
        Compute_BRDF_PBR(vNormal.xyz, vLook.xyz, vLightDir, vDiffuse.xyz, vPBRDesc.x, vPBRDesc.y, vResultDiffuse, vResultSpecular); //g_fGlobalStaticMetallic, g_fGlobalStaticRoughness);
        
        vector vViewPos = Compute_ViewPos(In.vTexcoord, g_DepthTexture);
        float fViewZ = vViewPos.z;

        vector vWorldPos = mul(vViewPos, g_ViewMatrixInv);
    
        vLightDiffuse = g_vLightDiffuse.xyz * vResultDiffuse;
        vLightSpecular = g_vLightDiffuse.xyz * vResultSpecular;

        vAmbientColor = vDiffuse;
        vAmbient = g_vStaticMtrlAmbient;
    }
    
    
    float4 vLightAmbient = float4((vAmbientColor.xyz * vAmbient.xyz), 1.f);
    
    Out.vColor = float4((vLightDiffuse + vLightSpecular + vLightAmbient.xyz), 1.f);
    
    return Out;
}


PS_OUT_LIGHT PS_LIGHT_POINT(PS_IN In)
{
    PS_OUT_LIGHT Out = (PS_OUT_LIGHT) 0;
    
    vector vDiffuse = g_DiffuseTexture.Sample(DefaultSampler, In.vTexcoord);
    
    if (vDiffuse.r == 1.f && vDiffuse.g == 0.f && vDiffuse.b == 1.f)
        discard;
        
    float4 vNormal = Compute_Normal(g_NormalTexture, DefaultSampler, In.vTexcoord);
    //vector vNormal = g_NormalTexture.Sample(DefaultSampler, In.vTexcoord);
    //vNormal = normalize(vector(vNormal.xyz * 2.f - 1.f, 0.f));
    
    vector vWorldPos = Compute_WorldPos(In.vTexcoord, g_DepthTexture);
    
    vector vLook = normalize(g_vCamPosition - vWorldPos);
    
    float3 vLightDir = g_vLightPosition.xyz - vWorldPos.xyz;
    float fDistance = length(vLightDir);
    
    float fAtt = saturate((g_fLightRange - fDistance) / g_fLightRange);
    
    vector vPBRDesc = g_PBRTexture.Sample(DefaultSampler, In.vTexcoord);
   
    float NdotL = dot(normalize(vLightDir), vNormal.xyz);
    float fRimPower = Compute_RimPower(vNormal, vLook, NdotL);
    float fToonShade = smoothstep(-0.3f, -0.1f, NdotL);
 
    float3 vRimColor = g_vLightDiffuse.xyz;
 
    
    float3 vLightDiffuse = 0.f;
    float3 vLightSpecular = 0.f;
    
    float3 vResultDiffuse = 0.f;
    float3 vResultSpecular = 0.f;

    Compute_BRDF_PBR(vNormal.xyz, vLook.xyz, vLightDir, vDiffuse.xyz, vPBRDesc.x, vPBRDesc.y, vResultDiffuse, vResultSpecular);
        
    vLightDiffuse = (g_vLightDiffuse.xyz * ((vResultDiffuse))) * fAtt;
    vLightSpecular = (g_vLightDiffuse.xyz * ((vResultSpecular)) + (fRimPower * vRimColor)) * fAtt;

    float4 vAmbientColor = lerp(vDiffuse, g_vLightDiffuse, g_vLightAmbient);
   
//    Out.vLightAcc.a = 1.f;
    
    float4 vAmbient = float4((vAmbientColor * g_vLightAmbient).xyz * fAtt, 1.f);
    
    Out.vColor = float4((vLightDiffuse + vLightSpecular + vAmbient.xyz), 1.f);
   
    return Out;
}

technique11 DefaultTechnique
{
    pass DirectionalPass // 0
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_None, 0);
        SetBlendState(BS_Blend, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_LIGHT_DIRECTIONAL();
    }
    pass PointPass // 1
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_None, 0);
        SetBlendState(BS_Blend, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_LIGHT_POINT();
    }
}