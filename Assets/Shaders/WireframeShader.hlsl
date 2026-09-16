struct PS_INPUT
{
    float4 Position : SV_POSITION;
    float4 Color : COLOR;
    float2 UV : TEXCOORD0;
};

float4 mainPS(PS_INPUT input) : SV_TARGET
{
    return float4(0.0f, 1.0f, 0.0f, 1.0f);
}