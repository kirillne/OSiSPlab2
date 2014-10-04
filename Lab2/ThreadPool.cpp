#include "stdafx.h"
#include "ThreadPool.h"



ThreadPool::ThreadPool(int threadsCount = 3, std::ostream* log)
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
			threadArgument->WorkFunc = EmptyProc;
			threadArgument->WorkFuncArgument = 0;
			threads[i] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)EmptyProc,(LPVOID)threadArgument, 0, NULL);
		}
		*logStream << "Created " << threadsCount << " threads.";
	}
}

static DWORD WINAPI EmptyProc(LPVOID lpParam)
{
	return 1;
}

DWORD WINAPI ThreadPool::ThreadProc(LPVOID lpParam)
{
	ThreadArgument* threadArgument = (ThreadArgument*)lpParam;
	threadArgument->WorkFunc(&threadArgument->WorkFuncArgument);
	
	while (true)
	{
		if( WaitForSingleObject(hSemaphore,10000) == WAIT_TIMEOUT)
		{
			DeleteUnusedThreads(threadArgument);
		}
		else
		{
			ThreadArgument* newArgument;

			EnterCriticalSection(&criticalSection);
			{
				newArgument = funcQueue.front();
				busyThreadsCount++;
				busyThreads[threadArgument->ThreadNumber] = true;
				*logStream << "Thread "<<  threadArgument->ThreadNumber <<": start new function.";
			}
			LeaveCriticalSection(&criticalSection);

			newArgument->ThreadNumber = threadArgument->ThreadNumber;
			try
			{
				newArgument->WorkFunc(&newArgument->WorkFuncArgument);	
			}
			catch (const std::exception& ex)
			{
				*logStream << "Thread "<<  threadArgument->ThreadNumber <<": throwed " << (const char*)ex.what << ".";
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

void ThreadPool::DeleteUnusedThreads(ThreadArgument* threadArgument )
{
	if(difftime(time(NULL), functionAddTime) > 30)
	{
		EnterCriticalSection(&criticalSection);
		{
			busyThreads[threadArgument->ThreadNumber] = true;
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
					*logStream << "Thread "<<  i <<": deleted.";
				}
			}
			busyThreads[threadArgument->ThreadNumber] = false;
		}
		LeaveCriticalSection(&criticalSection);
	}
}


void ThreadPool::AddFunction(FUNC func, int argument)
{
	ThreadArgument* threadArgument = (ThreadArgument*) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY,
		sizeof(ThreadArgument));
	threadArgument->WorkFunc = func;
	threadArgument->WorkFuncArgument = argument;
	funcQueue.push(threadArgument);
	functionAddTime = time(NULL);
	ReleaseSemaphore(hSemaphore,1,NULL);
	*logStream << "New function added.";
			
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

