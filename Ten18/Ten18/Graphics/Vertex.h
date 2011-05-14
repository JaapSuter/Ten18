#pragma once

namespace Ten18 { namespace Graphics {

    struct Vertex
    {
        XMFLOAT3 Pos;
        XMFLOAT4 Color;
        XMFLOAT2 Tex;

        const static D3D11_INPUT_ELEMENT_DESC Layout[];
        const static UINT                     LayoutCount;
    };
}}
