#pragma once

namespace Ten18 {

	public ref class Cli
	{
    public:
        static void Initialize();
		static System::Threading::Tasks::Task^ TickAsync();
        
        static void ProcessTickSignals();
        
    private:
        typedef System::Threading::Tasks::TaskCompletionSource<int> TaskSignal;
        typedef System::Collections::Concurrent::ConcurrentQueue<TaskSignal^> TaskQueue;
        
        delegate void DelegateTickSignals(); 
        static DelegateTickSignals^ sDelegateTickSignals;
        static TaskQueue^ sWaitingForSignal;        
	};
}
