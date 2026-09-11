cbuffer constants : register(b0)
{
    row_major float4x4 MVP;
}

Texture2D Atlas : register(t0);
SamplerState Sampler : register(s0);

struct VS_INPUT
{
    float4 position : POSITION;
    float2 uv : TEXCOORD0;
    float4 color : COLOR;
};

struct PS_INPUT
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
    float4 color : COLOR;
};

PS_INPUT mainVS(VS_INPUT input)
{
    PS_INPUT output;    
    output.position = mul(float4(input.position.xyz, 1.0f), MVP);
    output.uv = input.uv;
    output.color = input.color;    
    return output;
}

float4 mainPS(PS_INPUT input) : SV_TARGET
{
    float a = Atlas.Sample(Sampler, input.uv).r;
    return float4(input.color.rgb, input.color.a * a);
}
