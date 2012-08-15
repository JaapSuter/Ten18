#pragma once

namespace Ten18 { namespace COM {

    template<class N, class T>
    class COMArray
    {
    public:

        T* Begin() { return mElem; }
        T* End() { return mElem + mCount; }

		static void Release(T&);
		static void NopRelease(T&) {};

		N* FillCount() { return &mCount; }
		T** FillArray() { return &mElem; }

		N Count() { return mCount; }

		T operator [] (N i) const
		{ 
			if (i < 0 || i >= mCount)
				throw std::exception("Out of bounds");
			return mElem[i];
		}

		COMArray(decltype(Release) release = COMArray::NopRelease) : mCount(), mElem(), rel(release) {}

        void Reset()
        {
            if (mElem)
            	for (N i = 0; i < mCount; ++i)
					rel(mElem[i]);

			CoTaskMemFree(mElem);
			mElem = nullptr;
            mCount = 0;
        }

        ~COMArray()
        {
            Reset();
        }

	private:

		N mCount;
		T* mElem;
		decltype(Release)* rel;

        COMArray(const COMArray&);
        COMArray& operator = (const COMArray&);		
    };
}}