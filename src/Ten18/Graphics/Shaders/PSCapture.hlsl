#include "Common.hlsli"

float3 main(in VS_Output input) : SV_Target
{
    float3 color = txDiffuse.Sample(samLinear, input.Tex).rgb;
    color.r = input.Pos.x / RenderTargetSize.x;
    color.g = 0.0f; // input.Tex.y;
    color.b = 0.0f;
    return color;
}
