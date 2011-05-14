#pragma once

namespace Ten18 { namespace Graphics {

    struct Constants
    {   
        XMMATRIX mWorld;
        XMMATRIX mView;
        XMMATRIX mProjection;
        XMVECTOR mDiffuse;    
        XMFLOAT2 mOffset;
        unsigned int mBitStep;
    };
}}
