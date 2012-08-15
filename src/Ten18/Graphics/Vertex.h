#pragma once

namespace Ten18 { namespace Graphics {

    struct Vertex
    {
        dx::XMFLOAT4 Pos;
        dx::XMFLOAT4 Color;
        dx::XMFLOAT2 Tex;

        const static D3D11_INPUT_ELEMENT_DESC Layout[];
        const static UINT                     LayoutCount;
    };
}}
