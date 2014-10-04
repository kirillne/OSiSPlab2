#include "stdafx.h"
#include "ThreadPool.h"
#include <synchapi.h>


static DWORD WINAPI StartProc(LPVOID lpParam)
{
	ThreadArgument* threadArgument = (ThreadArgument*)lpParam;
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
		busyThreads = (bool *)calloc(threadsCount, sizeof(HANDLE));
		busyThreadsCount = 0;

		hSemaphore = CreateSemaphore(NULL,0,100,NULL);
		functionAddTime = time(NULL);
		InitializeCriticalSection(&criticalSection);
		for(int i = 0; i < threadsCount; i++)
		{
			busyThreads[i] = false;
			ThreadArgument* threadArgument = (ThreadArgument*) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY,
				sizeof(ThreadArgument));
			threadArgument->ThreadNumber = i;
			threadArgument->WorkFunc = StartProc;
			threadArgument->WorkFuncArgument = 0;
			threadArgument->pool = this;
			threads[i] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)StartProc,(LPVOID)threadArgument, 0, NULL);

		}
		*logStream << "Created " << threadsCount << " threads.\n";
	}
}



DWORD WINAPI ThreadPool::ThreadProc(LPVOID lpParam)
{
	ThreadArgument* threadArgument = (ThreadArgument*)lpParam;
	while (true)
	{
		if( WaitForSingleObject(hSemaphore,2000) == WAIT_TIMEOUT)
		{
			DeleteUnusedThreads(threadArgument->ThreadNumber);
		}
		else
		{
			FUNC newArgument;

			EnterCriticalSection(&criticalSection);
			{
				newArgument = funcQueue.front();
				busyThreadsCount++;
				busyThreads[threadArgument->ThreadNumber] = true;
				*logStream << "Thread "<<  threadArgument->ThreadNumber <<": start new function.\n";
			}
			LeaveCriticalSection(&criticalSection);

			try
			{
				newArgument(0);	
			}
			catch (const std::exception& ex)
			{
			
				*logStream << "Thread "<<  threadArgument->ThreadNumber <<": throwed " << ex.what() << ".\n";
			}
			catch(...)
			{
				*logStream << "Thread "<<  threadArgument->ThreadNumber <<": Some error.\n";
		
			}


			EnterCriticalSection(&criticalSection);
			{
				busyThreadsCount--;
				busyThreads[threadArgument->ThreadNumber] = false;
			}
			LeaveCriticalSection(&criticalSection);
		}
	}

	return 0;
}

void ThreadPool::DeleteUnusedThreads(int threadNumber )
{
	EnterCriticalSection(&criticalSection);
	{
		if(difftime(time(NULL), functionAddTime) > 10)
		{

			busyThreads[threadNumber] = true;
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
	ThreadArgument* threadArgument = (ThreadArgument*) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY,
		sizeof(ThreadArgument));
	threadArgument->WorkFunc = func;
	threadArgument->WorkFuncArgument = argument;
	threadArgument->pool = this;
	funcQueue.push(func);
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

