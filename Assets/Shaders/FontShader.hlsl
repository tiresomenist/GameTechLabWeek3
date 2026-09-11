cbuffer constants : register(b0)
{
    row_major float4x4 MVP;
};

Texture2D g_FontTexture : register(t0);
SamplerState g_FontSampler : register(s0);

struct VS_FONT_INPUT
{
    float3 position : POSITION;
    float2 uv : TEXCOORD;
};

struct PS_FONT_INPUT
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
};

PS_FONT_INPUT FontVS(VS_FONT_INPUT input)
{
    PS_FONT_INPUT output;
    output.position = mul(float4(input.position, 1.0f), MVP);
    output.uv = input.uv;
    return output;
}

float4 FontPS(PS_FONT_INPUT input) : SV_TARGET
{
    float4 texColor = g_FontTexture.Sample(g_FontSampler, input.uv);

    float alpha = texColor.r;

    clip(alpha - 0.05f);

    return float4(1.0f, 1.0f, 1.0f, alpha);
}