#pragma once

namespace Ten18
{
    namespace Util
    {
        int EnumerateNativeThreads(bool traceFullInfo = false, const char* msg = "");
        std::vector<HMODULE> EnumerateModules();

        template <class Elem, class N, N Max>
        std::wstring UsingFixedBuffer(const std::function<bool (Elem*, const N&, N&)>& body)
        {
            N size = N();
            Elem buffer[Max + 1] = {};
            if (!body(buffer, Max, size))
            {
                const N lastTryMultiplier = 4;
                const N lastTryMax = lastTryMultiplier * Max;
                
                Elem lastTry[lastTryMax + 1] = {};
                
                size = N();
                if (!body(lastTry, lastTryMax, size))
                    throw std::exception("UsingFixedBuffer Failure");

                Ten18_ASSERT(lastTry[lastTryMax] == Elem());
                return std::wstring(lastTry);
            }

            Ten18_ASSERT(buffer[Max] == Elem());
            return std::wstring(buffer);
        }

        template <class Unk>
        void Release(Unk*& unknown) { if (unknown) unknown->Release(); unknown = nullptr; }

        bool FileExists(const wchar_t *fileName);

        std::string Narrow(const std::wstring& wstr);
    }

    std::wstring to_string(const GUID& guid);
    
    template <class To, class From>
    To checked_reinterpret_cast(From f)
    {
        static_assert(sizeof(From) == sizeof(To), "checked_reinterpret_cast: error, sizeof(From) != sizeof(To)");
        static_assert(std::alignment_of<From>::value >= std::alignment_of<From>::value,
            "checked_reinterpret_cast: error, std::alignment_of(From) < std::alignment_of(To)");
        return reinterpret_cast<const To&>(f);
    }
}
