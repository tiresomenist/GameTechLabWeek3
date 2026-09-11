Texture2D FontAtlas : register(t0);
SamplerState FontSampler : register(s0);

cbuffer constants : register(b0)
{
    row_major float4x4 VP;
}

struct VSInput
{
    float3 position : POSITION;
    float2 texCoord : TEXCOORD0;
};

struct VSOutput
{
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD0;
};

VSOutput VS_Font(VSInput input)
{
    VSOutput output;
    // FFont builds camera-facing world vertices; apply View * Projection once.
    output.position = mul(float4(input.position, 1.0f), VP);
    output.texCoord = input.texCoord;
    return output;
}

float4 PS_Font(VSOutput input) : SV_TARGET
{
    // This atlas has white glyphs on an opaque black background.
    float4 texel = FontAtlas.Sample(FontSampler, input.texCoord);
    float coverage = texel.r;
    clip(coverage - 0.001f);
    return float4(1.0f, 1.0f, 1.0f, coverage);
}
