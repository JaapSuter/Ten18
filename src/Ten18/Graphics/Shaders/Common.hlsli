Texture2D txDiffuse : register(t0);
SamplerState samLinear : register(s0);

cbuffer ConstantBuffer
{
    matrix World;
    matrix View;
    matrix Projection;
    float4 Diffuse = { 0.3, 0.1, 0.6, 1.0 };
    float2 Offset;
    float2 RenderTargetSize;
    uint   BitStep;
}

struct VS_Output
{
    float4 Pos : SV_POSITION;
    float2 Tex : TEXCOORD0;
};
