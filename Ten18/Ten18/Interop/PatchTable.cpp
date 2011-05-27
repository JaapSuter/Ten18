#include "Ten18/Interop/PatchTable.h"
#include "Ten18/Interop/PatchTable.Generated.h"

void Ten18::Interop::PatchTable::Update(void* data, int size)
{
    auto bytes = static_cast<unsigned char*>(data);
    const unsigned char OpCodes_Nop = 0;
    const unsigned char OpCodes_Ldc_I4 = 0x20;
    const unsigned char OpCodes_Ldc_I8 = 0x21;
    const unsigned int CookiePrefix = 0xBA5E1018;

    // Todo, Jaap Suter, May 2011, Boyer Moore this search...
    for (int i = 8; i < size; )
    {
        if      (bytes[i    ] != (unsigned char)((CookiePrefix >> 24) & 0xFF)) i += 1;
        else if (bytes[i - 1] != (unsigned char)((CookiePrefix >> 16) & 0xFF)) i += 1;
        else if (bytes[i - 2] != (unsigned char)((CookiePrefix >>  8) & 0xFF)) i += 1;
        else if (bytes[i - 3] != (unsigned char)((CookiePrefix      ) & 0xFF)) i += 1;
        else if (bytes[i - 8] != OpCodes_Ldc_I8)                               i += 1;
        else
        {
            const unsigned int cookieIndex = (bytes[i - 4] << 24)
                                           | (bytes[i - 5] << 16)
                                           | (bytes[i - 6] <<  8)
                                           | (bytes[i - 7]      );

            Ten18_ASSERT(cookieIndex < ARRAYSIZE(sPatchTable));
            auto funPtr = sPatchTable[cookieIndex];

            bytes[i - 8] = OpCodes_Ldc_I4;
            bytes[i - 7] = (unsigned char)((funPtr      ) & 0xFF);
            bytes[i - 6] = (unsigned char)((funPtr >>  8) & 0xFF);
            bytes[i - 5] = (unsigned char)((funPtr >> 16) & 0xFF);
            bytes[i - 4] = (unsigned char)((funPtr >> 24) & 0xFF);
            bytes[i - 3] = bytes[i - 2] = bytes[i - 1] = bytes[i - 0] = OpCodes_Nop;            

            i += 9;
        }
    }
}

