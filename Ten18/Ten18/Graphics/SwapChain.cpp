#include "Ten18/PCH.h"
#include "Ten18/Graphics/SwapChain.h"
#include "Ten18/Graphics/Device.h"
#include "Ten18/Windows/Window.h"
#include "Ten18/COM/EmbeddedResourceStream.h"
#include "Ten18/Resources/Resources.h"
#include "Ten18/Expect.h"

using namespace Ten18;
using namespace Ten18::COM;
using namespace Ten18::Graphics;
using namespace Ten18::Windows;

SwapChain::SwapChain(const std::function<void ()>& tick, Device& device, Window& window)
    :
mWindow(window),
mDevice(device),
mTick(tick),
mSwapChain(),
mRenderTargetView(),
mClientEmpty(),
mClientRect(),
mClientPos()
{
    DXGI_SWAP_CHAIN_DESC desc = {};
    desc.BufferCount = 1;
    desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    desc.OutputWindow = window.Handle();
    desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.Windowed = TRUE;

    mConstants.mWorld = XMMatrixIdentity();
    mConstants.mView = XMMatrixIdentity();

    Expect.HR = device.mDXGIFactory1->CreateSwapChain(device.mD3D11Device.Raw(), &desc, mSwapChain.AsTypedDoubleStar());
    Expect.HR = device.mDXGIFactory1->MakeWindowAssociation(window.Handle(), DXGI_MWA_NO_WINDOW_CHANGES);
    
    window.OnRenderDo([&] (const Window&) { Tick(); });

    device.mSwapChains.push_back(this);
    
}

void SwapChain::Repose()
{
    POINT clientPos = {};
    RECT clientRect = {};
    Expect.True = ClientToScreen(mWindow.Handle(), &clientPos);
    Expect.True = GetClientRect(mWindow.Handle(), &clientRect);

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

    Expect.HR = mSwapChain->ResizeBuffers(1, clientRect.right - clientRect.left, clientRect.bottom - clientRect.top, DXGI_FORMAT_R8G8B8A8_UNORM, 0);
    
    COM::COMPtr<ID3D11Texture2D> backBuffer;
    Expect.HR = mSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), backBuffer.AsVoidDoubleStar());

    Expect.HR = mDevice.mD3D11Device->CreateRenderTargetView(backBuffer.Raw(), nullptr, mRenderTargetView.AsTypedDoubleStar());
}

void SwapChain::Tick()
{
    if (nullptr == mWindow.Handle())
        return;

    Repose();

    if (mClientEmpty)
        return;

    mTick();

    static DWORD dwTimeStart = 0;
    DWORD dwTimeCur = GetTickCount();
    if( dwTimeStart == 0 )
        dwTimeStart = dwTimeCur;
    float t = (dwTimeCur - dwTimeStart) / 1000.0f;

    float cycling[] = { 0.5f + cosf(t * 3.3f) * 0.5f + 0.5f, 0.125f, 0.5f + sinf(t * 8.3f) * 0.5f + 0.5f, 1.0f };
    (void)cycling;
    float black[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    auto white = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    auto translucent = XMFLOAT4(1.0f, 1.0f, 1.0f, 0.8f);

    mDevice.mImmediateContext->ClearRenderTargetView(mRenderTargetView.Raw(), black);

    D3D11_VIEWPORT vp = {};
    vp.Width = static_cast<float>(mClientRect.right - mClientRect.left);
    vp.Height = static_cast<float>(mClientRect.bottom - mClientRect.top);
    vp.MaxDepth = 1.0f;

    mDevice.mImmediateContext->RSSetViewports(1, &vp);
    mDevice.mImmediateContext->OMSetRenderTargets(1, mRenderTargetView.AsUnsafeArrayOfOne(), nullptr);

    auto near = 0.0f;
    auto far = 1.0f;
    auto ortho = XMMatrixOrthographicOffCenterLH(float(mClientPos.x), float(mClientPos.x + mClientRect.right - mClientRect.left),
                                                 float(mClientPos.y + mClientRect.bottom - mClientRect.top), float(mClientPos.y),
                                                 near, far);

    ortho = XMMatrixOrthographicOffCenterLH(0, vp.Width, vp.Height, 0, near, far);

    mConstants.mProjection = XMMatrixTranspose(ortho);
    mConstants.mDiffuse = XMLoadFloat4(&white);
    mConstants.mDiffuse = XMLoadFloat4(&translucent);    
    mConstants.mOffset = XMFLOAT2(float(mClientPos.x), float(mClientPos.x + mClientRect.right - mClientRect.left));

    static unsigned int BitStepCounter = 0;
    const unsigned int MaxBitStep = 13;
    const unsigned int DivBitStep = 200;
    mConstants.mBitStep = (BitStepCounter / DivBitStep) % MaxBitStep;
    ++BitStepCounter;
    
    mDevice.mImmediateContext->UpdateSubresource(mDevice.mConstantBuffer.Raw(), 0, nullptr, &mConstants, 0, 0);

    mDevice.mImmediateContext->Draw(4, 0);

    if (mDevice.mFpsTex)
    {
        mDevice.mImmediateContext->PSSetShaderResources(0, 1, &mDevice.mFpsTex);
        mDevice.mImmediateContext->Draw(4, 0);
    }
        
    Expect.HR = mSwapChain->Present(0, 0);
}
