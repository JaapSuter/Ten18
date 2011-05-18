#pragma once
#include "Ten18/COM/ImplUnknown.h"
#include "Ten18/COM/COMPtr.h"
#include "Ten18/CLR/HostMalloc.h"
#include "Ten18/CLR/IAppDomainManagerEx.h"

namespace Ten18 {
    
    namespace CLR {

        class Host;

    class HostAssemblyStore : public IHostAssemblyStore
    {
    public:

        explicit HostAssemblyStore(Host& host);

        Ten18_QUERY_INTERFACE_BEGIN(IHostAssemblyStore)
            Ten18_QUERY_INTERFACE_IMPL(IHostAssemblyStore)
        Ten18_QUERY_INTERFACE_END()
                
        virtual HRESULT STDMETHODCALLTYPE ProvideAssembly(
            AssemblyBindInfo *pBindInfo,
            UINT64 *pAssemblyId,
            UINT64 *pContext,
            IStream **ppStmAssemblyImage,
            IStream **ppStmPDB);

        virtual HRESULT STDMETHODCALLTYPE ProvideModule( 
            ModuleBindInfo *pBindInfo,
            DWORD *pdwModuleId,
            IStream **ppStmModuleImage,
            IStream **ppStmPDB);

    private:

        IStream* ExtractResource(WORD resourceID);

        HostAssemblyStore(const HostAssemblyStore&);
        HostAssemblyStore& operator = (const HostAssemblyStore&);

        Host* mHost;        
        ULONG mUniqueAssemblyIds;        
    };
}}
