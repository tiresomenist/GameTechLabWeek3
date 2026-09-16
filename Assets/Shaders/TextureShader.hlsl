cbuffer TransformConstants : register(b0)
{
    row_major float4x4 MVP;
};

Texture2D ObjectTexture : register(t0);
SamplerState TextureSampler : register(s0);

cbuffer TextureDrawConstants : register(b1)
{
    float2 UVScale;
    float2 UVOffset;

    float4 Tint;

    float AlphaCutoff;
    float3 Padding;
};

struct VS_INPUT
{
    float3 Position : POSITION;
    float4 Color : COLOR;
    float2 UV : TEXCOORD0;
};

struct PS_INPUT
{
    float4 Position : SV_POSITION;
    float4 Color : COLOR;
    float2 UV : TEXCOORD0;
};

PS_INPUT mainVS(VS_INPUT Input)
{
    PS_INPUT Output;

    // 로컬 좌표를 월드·뷰·투영 변환함
    Output.Position = mul(float4(Input.Position, 1.0f), MVP);
    Output.Color = Input.Color;
    // 현재 프레임에 해당하는 아틀라스 영역으로 UV를 변환함
    Output.UV = Input.UV * UVScale + UVOffset;

    return Output;
}

float4 mainPS(PS_INPUT Input) : SV_TARGET
{
    // 텍스처에 정점 색상과 오브젝트별 색상을 곱함
    float4 Color = ObjectTexture.Sample(TextureSampler, Input.UV) * Input.Color * Tint;

    // 투명한 배경의 색상과 깊이 기록을 차단함
    if (AlphaCutoff > 0.0f)
    {
        clip(Color.a - AlphaCutoff);
    }

    return Color;
}
