// MainShader.hlsl
cbuffer constants : register(b0)
{
    row_major float4x4 MVP;
}


struct VS_INPUT
{
    float4 position : POSITION;
    float4 color : COLOR;
};

struct VS_OUTPUT
{
    float4 Pos : SV_POSITION;
};

struct PS_INPUT
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
};

PS_INPUT mainVS(VS_INPUT input)
{
    PS_INPUT output;
    
    //MVP행렬 곱으로 위치 변환
    output.position = mul(float4(input.position.xyz, 1.0f), MVP);
    
    output.color = input.color;
    
    return output;
}

float4 mainPS(PS_INPUT input) : SV_TARGET
{
    return input.color;
}

VS_OUTPUT VS_Highlight(VS_INPUT input)
{
    VS_OUTPUT output;
    
    float3 expandedPos = input.position.xyz * 1.05f;
    
    output.Pos = mul(float4(expandedPos, 1.0f), MVP);
    
    return output;
}

float4 PS_Highlight(VS_OUTPUT input) : SV_Target
{
    return float4(1.0f, 1.0f, 0.0f, 1.0f);
}
