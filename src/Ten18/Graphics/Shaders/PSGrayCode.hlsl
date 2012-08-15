#include "Common.hlsli"

// uint BinaryToGray(uint n)
// {
//     return (n >> 1) ^ n;
// }
// 
// uint GrayToBinary(uint gray)
// {
//     uint t = gray;
//     t ^= (gray >> 16);
//     t ^= (t >> 8);
//     t ^= (t >> 4);
//     t ^= (t >> 2);
//     t ^= (t >> 1);
//     return t;
// }

float4 main(VS_Output input) : SV_Target
{
    // uint gray = BinaryToGray((uint)abs(input.Pos.x + Offset.x - 0.5f));
    // if (gray & (1 << BitStep))
    //     return float4(1, 1, 1, 1);
    // else
    //     return float4(0, 0, 0, 1);

    return txDiffuse.Sample(samLinear, input.Tex) * Diffuse;
}
