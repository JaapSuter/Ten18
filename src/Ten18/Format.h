#pragma once

namespace Ten18
{
    class Format
    {
    private:
        class Unused {};

    public:

        const std::wstring& str() const { return mResult; }
        const wchar_t* c_str() const { return mResult.c_str(); }

        Format& DebugOut();
        const Format& DebugOut() const;

        template <class Str, class A0, class A1, class A2, class A3, class A4, class A5, class A6>
        Format(Str&& fmt, A0&& a0, A1&& a1, A2&& a2, A3&& a3, A4&& a4, A5&& a5, A6&& a6)
        {
            DoAll(fmt, std::forward<A0>(a0), std::forward<A1>(a1), std::forward<A2>(a2), std::forward<A3>(a3), std::forward<A4>(a4), std::forward<A5>(a5), std::forward<A6>(a6));
        }
        
        template <class Str, class A0, class A1, class A2, class A3, class A4, class A5>
        Format(Str&& fmt, A0&& a0, A1&& a1, A2&& a2, A3&& a3, A4&& a4, A5&& a5)
        {
            DoAll(fmt, std::forward<A0>(a0), std::forward<A1>(a1), std::forward<A2>(a2), std::forward<A3>(a3), std::forward<A4>(a4), std::forward<A5>(a5), Unused());
        }

        template <class Str, class A0, class A1, class A2, class A3, class A4>
        Format(Str&& fmt, A0&& a0, A1&& a1, A2&& a2, A3&& a3, A4&& a4)
        {
            DoAll(fmt, std::forward<A0>(a0), std::forward<A1>(a1), std::forward<A2>(a2), std::forward<A3>(a3), std::forward<A4>(a4), Unused(), Unused());
        }

        template <class Str, class A0, class A1, class A2, class A3>
        Format(Str&& fmt, A0&& a0, A1&& a1, A2&& a2, A3&& a3)
        {
            DoAll(fmt, std::forward<A0>(a0), std::forward<A1>(a1), std::forward<A2>(a2), std::forward<A3>(a3),  Unused(), Unused(), Unused());
        }

        template <class Str, class A0, class A1, class A2>
        Format(Str&& fmt, A0&& a0, A1&& a1, A2&& a2)
        {
            DoAll(fmt, std::forward<A0>(a0), std::forward<A1>(a1), std::forward<A2>(a2),  Unused(), Unused(), Unused(), Unused());
        }

        template <class Str, class A0, class A1>
        Format(Str&& fmt, A0&& a0, A1&& a1)
        {
            DoAll(fmt, std::forward<A0>(a0), std::forward<A1>(a1),  Unused(), Unused(), Unused(), Unused(), Unused());
        }

        template <class Str, class A0>
        Format(Str&& fmt, A0&& a0)
        {
            DoAll(std::forward<Str>(fmt), std::forward<A0>(a0),  Unused(), Unused(), Unused(), Unused(), Unused(), Unused());
        }

        template <class Str>
        Format(Str&& fmt)
        {
            DoAll(std::forward<Str>(fmt), Unused(), Unused(), Unused(), Unused(), Unused(), Unused(), Unused());
        }

    private:

        Format(const Format&);
        Format& operator = (const Format&);

        template <class Str, class A0, class A1, class A2, class A3, class A4, class A5, class A6>
        void DoAll(Str&& fmt, A0&& a0, A1&& a1, A2&& a2, A3&& a3, A4&& a4, A5&& a5, A6&& a6)
        {
            boolalpha(mStream);
            fixed(mStream);
            const auto numDecimals = 4;
            mStream.precision(numDecimals);
            
            StreamOut(fmt);
            mResult = mStream.str();

            DoOne(0, std::forward<A0>(a0));
            DoOne(1, std::forward<A1>(a1));
            DoOne(2, std::forward<A2>(a2));
            DoOne(3, std::forward<A3>(a3));
            DoOne(4, std::forward<A4>(a4));
            DoOne(5, std::forward<A5>(a5));
            DoOne(6, std::forward<A6>(a6));
        }

        template <class Arg>
        void DoOne(int idx, Arg&& arg)
        {
            mTemp.clear();
            mStream.str(mTemp);
            StreamOut(std::forward<Arg>(arg));
            wchar_t regex[] = { L'%', static_cast<wchar_t>(L'0' + idx), 0 };
            std::regex_replace(std::back_inserter(mTemp), std::begin(mResult), std::end(mResult), std::wregex(regex), mStream.str());
            mResult = mTemp;
        }

        void DoOne(int, Unused) {}

        template<class Arg>
        void StreamOut(Arg&& arg) { mStream << std::forward<Arg>(arg); }

        void StreamOut(std::string&& str) { mStream << str.c_str(); }        

        std::wstring mResult;
        std::wstring mTemp;
        std::wostringstream mStream;
    };
}
