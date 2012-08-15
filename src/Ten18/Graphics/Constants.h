#pragma once

namespace Ten18 { namespace Graphics {

    struct Constants
    {   
        dx::XMMATRIX mWorld;
        dx::XMMATRIX mView;
        dx::XMMATRIX mProjection;
        dx::XMVECTOR mDiffuse;    
        dx::XMFLOAT2 mOffset;
        dx::XMFLOAT2 mRenderTargetSize;
        unsigned int mBitStep;        
    };
}}
