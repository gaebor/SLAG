#include <iostream>

#include "..\Slag\AsyncQueue.h"
#include <thread>
#include <windows.h>

size_t n = 1 << 19;
size_t sleep_constant = 10000;
bool force_halt = false; //force halt

int run = 7;

int main(int argc, char* argv[])
{
	for (;*argv != NULL;++argv)
	{
		if (std::string(*argv) == "-n")
			n = (1ull << atoi(*++argv));
		else if (std::string(*argv) == "-s")
			sleep_constant = _atoi64(*++argv);
		else if (std::string(*argv) == "-f")
			force_halt = true;
		else if (std::string(*argv) == "-r")
			run = atoi(*++argv);
	}

	AsyncQueue<size_t>* queue;
	size_t i, j;

	LARGE_INTEGER freq, time1, time2;
	QueryPerformanceFrequency(&freq);

	if (run & 1)
	{
		queue = new AsyncQueue<size_t>();
		size_t absTime1, absTime2;
		QueryPerformanceCounter(&time1);

		for ( i = 0; i < n; ++i)
		{
			queue->EnQueue(i);
		}

		QueryPerformanceCounter(&time2);
		std::cout << i<< " element enqueued in " << (absTime1 = ((time2.QuadPart-time1.QuadPart)*1000) / freq.QuadPart) << " ms" << std::endl;

		QueryPerformanceCounter(&time1);

		for (i = 0; i < n; ++i)
		{
			queue->DeQueue(i);
		}
	
		QueryPerformanceCounter(&time2);
		std::cout << i<< " element dequeued in " << (absTime2 = ((time2.QuadPart-time1.QuadPart)*1000) / freq.QuadPart) << " ms" << std::endl;
		std::cout << "total time: " << absTime1 + absTime2 << " ms\n" << std::endl;
		delete queue;
	}
	if (run & 2)
	{
		queue = new AsyncQueue<size_t>();
		QueryPerformanceCounter(&time1);

		for (i = 0; i < n; ++i)
		{
			queue->EnQueue(i);

			queue->DeQueue(j);

		}
		QueryPerformanceCounter(&time2);
		std::cout << i<< " element processed in " << ((time2.QuadPart-time1.QuadPart)*1000) / freq.QuadPart << " ms\n" << std::endl;

		delete queue;
	}
	if (run & 4)
	{
		queue = new AsyncQueue<size_t>();
		QueryPerformanceCounter(&time1);
		std::thread pusher([&]()
		{
			for (i=0; i < n; ++i)
			{
				queue->EnQueue(i);
				if ((sleep_constant) != 0 && (i % sleep_constant == 0))
					Sleep(1);
			}
		});

		std::thread consumer([&]()
		{
			for (; true;)
			{
				if (!queue->DeQueue(j))
					break;
				//Sleep(1);//std::cout << "ok"<<std::endl;
			}
		});

		pusher.join();
		QueryPerformanceCounter(&time2);

		std::cout << "pushing ended in " << ((time2.QuadPart-time1.QuadPart)*1000) / freq.QuadPart << " ms" << std::endl;

		size_t dropped = 0;
		if (force_halt)
			dropped = queue->WakeUp();
		else
			queue->WaitForEmpty();

		QueryPerformanceCounter(&time2);
		std::cout << "receiving ended in " << ((time2.QuadPart-time1.QuadPart)*1000) / freq.QuadPart << " ms" << std::endl;

		queue->WakeUpIfEmpty();

		consumer.join();

		std::cout << i << " elements has been queued and " << j << " has been dequeued" << std::endl;
		//\nhighwater was " << (double)queue->GetHighWater()*100 / i << "%" << std::endl;
		std::cout << "dropped " << dropped << " elements" << std::endl;

		delete queue;
	}
	return 0;
	}