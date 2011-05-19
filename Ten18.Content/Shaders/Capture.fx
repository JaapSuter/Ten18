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

float4 PS(VS_OUTPUT input) : SV_Target
{
    return txDiffuse.Sample(samLinear, input.Tex).rgba; // * input.Color * Diffuse;
}
