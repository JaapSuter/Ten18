Texture2D txDiffuse : register(t0);
SamplerState samLinear : register(s0);

cbuffer ConstantBuffer
{
    matrix World;
    matrix View;
    matrix Projection;
    float4 Diffuse = { 0.3, 0.1, 0.6, 1.0 };
    float2 Offset;
    uint   BitStep;
}

struct VS_OUTPUT
{
    float4 Pos : SV_POSITION;
    float4 Color : COLOR0;
    float2 Tex : TEXCOORD;
};

VS_OUTPUT VS(float4 Pos : POSITION, float4 Color : COLOR, float2 Tex : TEXCOORD)
{
    VS_OUTPUT output = (VS_OUTPUT)0;
    output.Pos = mul(       Pos, World);
    output.Pos = mul(output.Pos, View);
    output.Pos = mul(output.Pos, Projection);
    output.Color = Color;
    output.Tex = Tex;
    return output;
}
 
uint BinaryToGray(uint n)
{
    return (n >> 1) ^ n;
}

uint GrayToBinary(uint gray)
{
    uint t = gray;
    t ^= (gray >> 16);
    t ^= (t >> 8);
    t ^= (t >> 4);
    t ^= (t >> 2);
    t ^= (t >> 1);
    return t;
}

float4 PS(VS_OUTPUT input) : SV_Target
{
    uint gray = BinaryToGray((uint)(input.Pos.x + Offset.x - 0.5f));
    if (gray & (1 << BitStep))
        return float4(1, 1, 1, 1);
    else
        return float4(0, 0, 0, 1);

    // return txDiffuse.Sample(samLinear, input.Tex) * input.Color * Diffuse;
}
