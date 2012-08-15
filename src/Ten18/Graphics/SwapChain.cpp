#include "Ten18/PCH.h"
#include "Ten18/Graphics/SwapChain.h"
#include "Ten18/Window.h"
#include "Ten18/Graphics/GraphicsDevice.h"
#include "Ten18/COM/EmbeddedResourceStream.h"
#include "Ten18/Expect.h"

using namespace Ten18;
using namespace Ten18::COM;
using namespace Ten18::Graphics;

SwapChain::SwapChain(HWND hwnd)
    :
mHwnd(hwnd),
mDevice(GraphicsDevice::Instance()),
mSwapChain1(),
mRenderTargetView(),
mClientEmpty(),
mClientRect(),
mClientPos()
{
    DXGI_SWAP_CHAIN_DESC1 desc1 = {};
    desc1.BufferCount = 2;
    desc1.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    desc1.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    desc1.Scaling = DXGI_SCALING_NONE;
    desc1.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
    desc1.SampleDesc.Count = 1;
    desc1.SampleDesc.Quality = 0;
    
    Ten18_EXPECT.HR = mDevice.mDXGIFactory2->CreateSwapChainForHwnd(mDevice.mD3D11Device1.Raw(), mHwnd, &desc1, nullptr, nullptr, mSwapChain1.AsTypedDoubleStar());
    Ten18_EXPECT.HR = mSwapChain1->GetDesc1(&desc1);

    mConstants.mWorld = dx::XMMatrixIdentity();
    mConstants.mView = dx::XMMatrixIdentity();
    mConstants.mRenderTargetSize.x = static_cast<float>(desc1.Width);
    mConstants.mRenderTargetSize.y = static_cast<float>(desc1.Height);
    
    mDevice.mSwapChains.push_back(this);    
}

void SwapChain::Repose()
{
    POINT clientPos = {};
    RECT clientRect = {};
    Ten18_EXPECT.True = ClientToScreen(mHwnd, &clientPos);
    Ten18_EXPECT.True = GetClientRect(mHwnd, &clientRect);

    auto moved = (clientPos.x != mClientPos.x || clientPos.y != mClientPos.y);
    auto sized = !EqualRect(&clientRect, &mClientRect);
    
    if (!moved && !sized)
        return;

    mClientPos = clientPos;
    mClientRect = clientRect;
    
    if (IsRectEmpty(&mClientRect))
        return;

    mDevice.mReposeInducedDwmFlushReq = true;
    
    if (!sized)
        return;
    
    mRenderTargetView.Reset();

    Ten18_EXPECT.HR = mSwapChain1->ResizeBuffers(0, clientRect.right - clientRect.left, clientRect.bottom - clientRect.top, DXGI_FORMAT_R8G8B8A8_UNORM, 0);
    
    COM::COMPtr<ID3D11Texture2D> backBuffer;
    Ten18_EXPECT.HR = mSwapChain1->GetBuffer(0, __uuidof(ID3D11Texture2D), backBuffer.AsVoidDoubleStar());

    Ten18_EXPECT.HR = mDevice.mD3D11Device1->CreateRenderTargetView(backBuffer.Raw(), nullptr, mRenderTargetView.AsTypedDoubleStar());
}

void SwapChain::Tick()
{
    if (nullptr == mHwnd)
        return;

    Repose();

    if (mClientEmpty)
        return;

    static ULONGLONG dwTimeStart = 0;
    ULONGLONG dwTimeCur = GetTickCount64();
    if( dwTimeStart == 0 )
        dwTimeStart = dwTimeCur;
    float t = (dwTimeCur - dwTimeStart) / 1000.0f;
    Ten18_UNUSED(t);

    mDevice.mImmediateContext->ClearRenderTargetView(mRenderTargetView.Raw(), DirectX::Colors::Black.f);

    D3D11_VIEWPORT vp = {};
    vp.Width = static_cast<float>(mClientRect.right - mClientRect.left);
    vp.Height = static_cast<float>(mClientRect.bottom - mClientRect.top);
    vp.MaxDepth = 1.0f;

    mDevice.mImmediateContext->RSSetViewports(1, &vp);
    mDevice.mImmediateContext->OMSetRenderTargets(1, mRenderTargetView.AsUnsafeArrayOfOne(), nullptr);

    auto near = 0.0f;
    auto far = 1.0f;
    auto ortho = dx::XMMatrixOrthographicOffCenterLH(float(mClientPos.x), float(mClientPos.x + mClientRect.right - mClientRect.left),
                                                 float(mClientPos.y + mClientRect.bottom - mClientRect.top), float(mClientPos.y),
                                                 near, far);

    ortho = dx::XMMatrixOrthographicOffCenterLH(0, vp.Width, vp.Height, 0, near, far);

    auto translucent = dx::XMFLOAT4(1.0f, 1.0f, 1.0f, 0.8f);
    mConstants.mProjection = dx::XMMatrixTranspose(ortho);
    mConstants.mDiffuse = DirectX::Colors::White;
    mConstants.mDiffuse = dx::XMLoadFloat4(&translucent);    
    mConstants.mOffset = dx::XMFLOAT2(float(mClientPos.x), float(mClientPos.x + mClientRect.right - mClientRect.left));

    static unsigned int BitStepCounter = 0;
    const unsigned int MaxBitStep = 13;
    const unsigned int DivBitStep = 200;
    mConstants.mBitStep = (BitStepCounter / DivBitStep) % MaxBitStep;
    ++BitStepCounter;
    
    mDevice.mImmediateContext->UpdateSubresource(mDevice.mConstantBuffer.Raw(), 0, nullptr, &mConstants, 0, 0);
    mDevice.mImmediateContext->Draw(3, 0);

    if (mDevice.mFpsTex)
    {
        mDevice.mImmediateContext->PSSetShaderResources(0, 1, &mDevice.mFpsTex);
        mDevice.mImmediateContext->Draw(4, 0);
    }

    DXGI_PRESENT_PARAMETERS pp = {};
    Ten18_EXPECT.HR = mSwapChain1->Present1(0, 0, &pp);
}
