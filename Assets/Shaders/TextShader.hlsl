cbuffer constants : register(b0)
{
    row_major float4x4 MVP;
}

Texture2D FontAtlas : register(t0);
SamplerState FontSampler : register(s0);

struct VS_INPUT
{
    float3 position : POSITION;
    float2 uv       : TEXCOORD;
    float4 color    : COLOR;
};

struct PS_INPUT
{
    float4 position : SV_POSITION;
    float2 uv       : TEXCOORD;
    float4 color    : COLOR;
};

PS_INPUT mainVS_Text(VS_INPUT input)
{
    PS_INPUT output;
    output.position = mul(float4(input.position, 1.0f), MVP);
    output.uv = input.uv;
    output.color = input.color;
    return output;
}

float4 mainPS_Text(PS_INPUT input) : SV_TARGET
{
    // 아틀라스는 1채널(R8) 텍스처: .r 값이 이 픽셀의 글자 커버리지 = 알파로 쓴다.
    float alpha = FontAtlas.Sample(FontSampler, input.uv).r;
    float4 result = input.color;
    result.a *= alpha;
    return result;
}
