#include "Ten18/Content/Index.h"
#include "Ten18/Content/Index.Generated.h"
#include "Ten18/Memory.h"
#include "Ten18/Expect.h"
#include "Ten18/Scoped.h"
#include "Ten18/Util.h"
#include "Ten18/Assert.h"

namespace Ten18 { namespace Content {

const wchar_t* Index::Root()
{
    auto exeFileName = Util::UsingFixedBuffer<wchar_t, DWORD, MAX_PATH>([&] (wchar_t* buffer, const DWORD & max, DWORD & size) -> bool
        {
            size = GetModuleFileName(nullptr, buffer, max);
            return size < max;
        });

    if (exeFileName.find(L"Debug") != std::wstring::npos)
        return L"..\\..\\..\\obj\\x86\\Debug\\";
    else
        return L"..\\..\\..\\obj\\x86\\Release\\";
}

static char Normalize(char c)
{
    // Ridiculous? Yes, definitely. But case insensitive string compare 
    // across interop boundaries is a bit of a mess. I can try finding the
    // right library function to use (_wcsnicmp, CompareStringExW, etc.) or just
    // restrict it to an extreme and call it a day...
    if (c >= 'A' && c <= 'Z') return c;
    if (c >= 'a' && c <= 'z') return c - ('a' - 'A');
    if (c >= '0' && c <= '9') return c;
    return '_'; // This will never result in name collisions, and 640 kilobyte ought to be enough for everybody...
}

static bool CompareName(const Index::Entry& entry, const wchar_t* name)
{
    for (const char* en = entry.Name; Normalize(*en) == Normalize(static_cast<char>(*name)); ++en, ++name)
        if (*en == 0 || *name == 0)
            return *en == *name;
    return false;
}

const Index::Entry* Index::TryGet(const wchar_t* name)
{
    for (int i = 0; i < NumEntries; ++i)
        if (CompareName(Table[i], name))
            return &Table[i];
    
    return nullptr;
}

const Index::ScopedEntry Index::Get(const wchar_t* name)
{
    auto ret = TryGet(name);    
    if (ret != nullptr)
        return ScopedEntry(*ret);
    
    auto maybeFile = std::wstring(Root()) + name;

    auto fh = MakeScoped(CreateFile(maybeFile.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr),
        [] (HANDLE h)
        {
            if (h != INVALID_HANDLE_VALUE && h != nullptr)
                CloseHandle(h);
        });

    if (fh != INVALID_HANDLE_VALUE && fh != nullptr)
    {
        LARGE_INTEGER li = {};
        Expect.True = GetFileSizeEx(fh, &li);
        
        const size_t moreThanEnough = 10 * 1024 * 1024;
        Ten18_ASSERT(li.QuadPart < moreThanEnough);
        auto size = static_cast<size_t>(li.QuadPart);

        char* data = static_cast<char*>(Memory::Alloc(size, __FILE__, __LINE__));

        size_t remaining = size;
        for (DWORD read = 0; remaining > 0; remaining -= read)
            Expect.True = ReadFile(fh, data, remaining, &read, nullptr);
        
        return ScopedEntry(size, data);
    }

    Ten18_ASSERT_FAIL("Unable to find embedded data with name: %S", name);    
    static const Ten18::Content::Index::Entry Null = { "Ten18.Content.Index.Null", 0, nullptr };
    return ScopedEntry(Null);
}

    
}}
