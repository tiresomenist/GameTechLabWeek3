cbuffer TransformBuffer : register(b0)
{
    row_major matrix MVP;
};

struct VS_INPUT
{
    float3 Pos : POSITION;
    float3 Normal : NORMAL;
    float2 UV : TEXCOORD;
};

struct PS_INPUT
{
    float4 Pos : SV_POSITION;
    float3 Normal : NORMAL;
    float2 UV : TEXCOORD;
};

PS_INPUT VS_Texture(VS_INPUT input)
{
    PS_INPUT output;
    output.Pos = mul(float4(input.Pos, 1.0f), MVP);
    output.Normal = input.Normal;
    output.UV = input.UV;
    return output;
}

Texture2D gTexture : register(t0);
SamplerState gSampler : register(s0);

float4 PS_Texture(PS_INPUT input) : SV_TARGET
{
    return gTexture.Sample(gSampler, input.UV);
}