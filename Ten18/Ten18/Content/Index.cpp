#include "Ten18/Content/Index.h"
#include "Ten18/Assert.h"
#include "Ten18/Content/Index.Generated.h"

namespace Ten18 { namespace Content {

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

const Index::Entry& Index::Get(const wchar_t* name)
{
    auto ret = TryGet(name);
    
    if (ret == nullptr)
    {
        Ten18_ASSERT_FAIL("Unable to find embedded data with name: %S", name);    
        static const Ten18::Content::Index::Entry Null = { "Ten18.Content.Index.Null", 0, nullptr };
        return Null;
    }
    
    return *ret;
}

    
}}
