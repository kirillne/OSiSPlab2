#include "stdafx.h"
#include "ThreadPool.h"
#include <synchapi.h>

//Start function for each thread. Because i can't start thread from object function this->ThreadProc
static DWORD WINAPI StartProc(LPVOID lpParam)
{
	StartThreadArgument* threadArgument = (StartThreadArgument*)lpParam;
	threadArgument ->pool->ThreadProc(lpParam);
	return 0;
}

ThreadPool::ThreadPool(int threadsCount = 3, std::ostream* log = &std::cout)
{
	if(threadsCount > 0)
	{
		logStream = log;

		this->threadsCount = threadsCount;
		threads = (HANDLE *)calloc(threadsCount, sizeof(HANDLE));
		busyThreads = (bool *)calloc(threadsCount, sizeof(HANDLE)); //if busyThreads[i] == true => thread have some function
		busyThreadsCount = 0;

		hSemaphore = CreateSemaphore(NULL,0,100,NULL);
		functionAddTime = time(NULL); //time when function was added last time
		InitializeCriticalSection(&criticalSection);

		for(int i = 0; i < threadsCount; i++)
		{
			busyThreads[i] = false;
			StartThreadArgument* threadArgument = (StartThreadArgument*) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY,
				sizeof(StartThreadArgument));
			threadArgument->ThreadNumber = i;
			threadArgument->pool = this;
			threads[i] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)StartProc,(LPVOID)threadArgument, 0, NULL);
		}
		*logStream << "Created " << threadsCount << " threads.\n";
	}
}



DWORD WINAPI ThreadPool::ThreadProc(LPVOID lpParam)
{
	StartThreadArgument* threadArgument = (StartThreadArgument*)lpParam;
	while (true)
	{
		if( WaitForSingleObject(hSemaphore,2000) == WAIT_TIMEOUT)
		{
			DeleteUnusedThreads(threadArgument->ThreadNumber);
		}
		else
		{
			DoFunction(threadArgument->ThreadNumber);
		}
	}

	return 0;
}

void ThreadPool::DoFunction(int threadNumber )
{
	FuncForThread* newArgument;

	//Get new function
	EnterCriticalSection(&criticalSection);
	{
		newArgument = funcQueue.front();
		funcQueue.pop();
		busyThreadsCount++;  //Mark thread as busy
		busyThreads[threadNumber] = true;
		*logStream << "Thread "<<  threadNumber <<": start new function.\n";
	}
	LeaveCriticalSection(&criticalSection);

	//Safety work with function
	try
	{
		*logStream << "Thread "<<  threadNumber <<": result "
			<<newArgument->WorkFunc (newArgument->WorkFuncArgument) << "\n";
	}
	catch (const std::exception& ex)
	{

		*logStream << "Thread "<<  threadNumber <<": throwed " << ex.what() << ".\n";
	}
	catch(...)
	{
		*logStream << "Thread "<< threadNumber <<": Some error.\n";

	}

	//Mark thread as not busy
	EnterCriticalSection(&criticalSection);
	{
		busyThreadsCount--;
		busyThreads[threadNumber] = false;
	}
	LeaveCriticalSection(&criticalSection);
}

void ThreadPool::DeleteUnusedThreads(int threadNumber )
{
	EnterCriticalSection(&criticalSection);
	{
		if(difftime(time(NULL), functionAddTime) > 10)
		{

			busyThreads[threadNumber] = true; //Thread can't delete itsalfe 
			int threadsToKill = (threadsCount - busyThreadsCount)/2;
			for (int i = 0; i < threadsCount && threadsToKill > 0; ++i)
			{
				if(!busyThreads[i])
				{
					threadsToKill--;
					busyThreadsCount++;
					busyThreads[i] = true;
					TerminateThread(threads[i],0);
					threads[i] = NULL;
					*logStream << "Thread "<<  i <<": deleted.\n";
				}
			}
			busyThreads[threadNumber] = false;
			functionAddTime = time(NULL);

		}
	}
	LeaveCriticalSection(&criticalSection);
}


void ThreadPool::AddFunction(FUNC func, int argument)
{
	FuncForThread* threadArgument = (FuncForThread*) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY,
		sizeof(FuncForThread));
	threadArgument->WorkFunc = func;
	threadArgument->WorkFuncArgument = argument;
	funcQueue.push(threadArgument);
	functionAddTime = time(NULL);
	ReleaseSemaphore(hSemaphore,1,NULL);
	if(busyThreadsCount == threadsCount)
	{
		*logStream << "There are no free threards in the pool.\n";
	}

	*logStream << "New function added.\n";

}

ThreadPool::~ThreadPool(void)
{
	for (int i = 0; i < threadsCount; ++i)
	{
		if(threads[i] != NULL)
		{
			TerminateThread(threads[i],0);
		}
	}
	free(threads);
	free(busyThreads);
	DeleteCriticalSection(&criticalSection);
	CloseHandle(hSemaphore);
}

