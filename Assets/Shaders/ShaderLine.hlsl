cbuffer TransformBuffer : register(b0)
{
    row_major float4x4 MVP;
};

struct VS_INPUT
{
    float3 Position : POSITION;
    float4 Color : COLOR;
};

struct VS_OUTPUT
{
    float4 Position : SV_POSITION;
    float4 Color : COLOR;
};

VS_OUTPUT mainVS_Line(VS_INPUT Input)
{
    VS_OUTPUT Output;
    Output.Position = mul(float4(Input.Position, 1.0f), MVP);
    Output.Color = Input.Color;
    return Output;
}

float4 mainPS_Line(VS_OUTPUT Input) : SV_TARGET
{
    return Input.Color;
}
