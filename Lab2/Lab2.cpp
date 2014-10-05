// Lab2.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <fstream>
#include "ThreadPool.h"
#include <synchapi.h>


int  Sum(int param)
{
	int sum = 0;
	for (int i = 0; i < param; ++i)
	{
		sum += i;
	}
	Sleep(3000);
	return sum;
}

int Error(int param)
{
	Sleep(2000);
	int zero = 0;
	//int a = 1/zero;
	throw new std::exception("Exception");
	std::cout << "Error done\n";
	return 0;
}

int Long(int param)
{
	Sleep(20000);
	return 0;
}

int _tmain(int argc, _TCHAR* argv[])
{
	std::cout << "Log type (0 - console, other - file):";
	int logCommand;
	std::ofstream ofs("log.txt");
	std::ostream* log = &std::cout;
	std::cin >> logCommand;
	if(logCommand == 0)
	{
		ofs.close();
	}
	else
	{
		log = &ofs;
	}

	std::cout << "Threads count:";
	int threadsCount;
	std::cin >> threadsCount;
	ThreadPool* pool = new ThreadPool(threadsCount, log);
	while (true)
	{
		char command;
		std::cin >> command;
		switch (command)
		{
		case 'a':
			pool->AddFunction(Sum,1);
			break;
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			pool->AddFunction(Sum,command - '0');
			break;
		case 'e':
			pool->AddFunction(Error,0);
			break;
		case 'l':
			pool->AddFunction(Long,0);
			break;
		case 'c':
			delete pool;
			if(logCommand != 0)
			{
				ofs.close();
			}
			return 0;
		}
	}

}



