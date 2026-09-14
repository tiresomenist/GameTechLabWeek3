cbuffer constants : register(b0)
{
    row_major float4x4 MVP;
}

Texture2D MaterialTexture : register(t0);
SamplerState MaterialSampler : register(s0);

struct VS_INPUT
{
    float3 position : POSITION;
    float2 uv : TEXCOORD;
};

struct PS_INPUT
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
};

PS_INPUT mainVS_Texture(VS_INPUT input)
{
    PS_INPUT output;
    output.position = mul(float4(input.position, 1.0f), MVP);
    output.uv = input.uv;
    return output;
}

float4 mainPS_Texture(PS_INPUT input) : SV_TARGET
{
    return MaterialTexture.Sample(MaterialSampler, input.uv);
}
