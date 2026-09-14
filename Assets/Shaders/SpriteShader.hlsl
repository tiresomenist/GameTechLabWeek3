cbuffer TransformBuffer : register(b0)
{
    row_major matrix MVP;
};

cbuffer SpriteBuffer : register(b1)
{
    float TotalTime;
    float AnimFPS;
    int Cols;
    int Rows;
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

PS_INPUT VS_Sprite(VS_INPUT input)
{
    PS_INPUT output;
    output.Pos = mul(float4(input.Pos, 1.0f), MVP);
    output.Normal = input.Normal;

    int totalFrames = Cols * Rows;
    int currentFrame = (int) (TotalTime * AnimFPS) % totalFrames;

    int col = currentFrame % Cols;
    int row = currentFrame / Cols;

    float2 uvScale = float2(1.0f / Cols, 1.0f / Rows);
    float2 uvOffset = float2(col * uvScale.x, row * uvScale.y);
    output.UV = input.UV * uvScale + uvOffset;

    return output;
}

Texture2D gTexture : register(t0);
SamplerState gSampler : register(s0);

float4 PS_Sprite(PS_INPUT input) : SV_TARGET
{
    return gTexture.Sample(gSampler, input.UV);
}