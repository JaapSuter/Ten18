#ifndef PENTACORN_NATIVE_CAPTURE_MEDIA_FOUNDATION_MEDIA_BUFFER_LOCK_HPP
#define PENTACORN_NATIVE_CAPTURE_MEDIA_FOUNDATION_MEDIA_BUFFER_LOCK_HPP

#include "Pentacorn/Error.hpp"

namespace Pentacorn { namespace Capture { namespace MediaFoundation {

class MediaBufferLock
{   
public:
    MediaBufferLock(IMFMediaBuffer *pBuffer) : m_p2DBuffer(NULL), m_bLocked(FALSE)
    {
        m_pBuffer = pBuffer;
        m_pBuffer->AddRef();

        // Query for the 2-D buffer interface. OK if this fails.
        (void)m_pBuffer->QueryInterface(IID_PPV_ARGS(&m_p2DBuffer));
    }

    ~MediaBufferLock()
    {
        UnlockBuffer();
        
        if (m_pBuffer != nullptr)
           m_pBuffer->Release();

        if (m_p2DBuffer != nullptr)
            m_p2DBuffer->Release();
        
        m_p2DBuffer = nullptr;
        m_pBuffer = nullptr;
    }

    void LockBuffer(LONG  lDefaultStride, DWORD dwHeightInPixels, BYTE  **ppbScanLine0, LONG  *plStride)
    {
        if (m_p2DBuffer)
        {
            Error = m_p2DBuffer->Lock2D(ppbScanLine0, plStride);            
        }
        else
        {
            // Use non-2D version.
            BYTE *pData = NULL;

            Error = m_pBuffer->Lock(&pData, NULL, NULL);
            
            *plStride = lDefaultStride;
            if (lDefaultStride < 0)
            {
                // Bottom-up orientation. Return a pointer to the start of the
                // last row *in memory* which is the top row of the image.
                *ppbScanLine0 = pData + abs(lDefaultStride) * (dwHeightInPixels - 1);
            }
            else
            {
                // Top-down orientation. Return a pointer to the start of the
                // buffer.
                *ppbScanLine0 = pData;
            }
        }
    }

    void UnlockBuffer()
    {
        if (m_bLocked)
        {
            if (m_p2DBuffer)
                (void)m_p2DBuffer->Unlock2D();
            else
                (void)m_pBuffer->Unlock();
            m_bLocked = FALSE;
        }
    }

private:
    IMFMediaBuffer* m_pBuffer;
    IMF2DBuffer*    m_p2DBuffer;
    BOOL            m_bLocked;
};

}}}

#endif
