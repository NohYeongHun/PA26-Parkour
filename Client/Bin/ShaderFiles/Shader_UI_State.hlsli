// UI¿ë Çì´õ

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