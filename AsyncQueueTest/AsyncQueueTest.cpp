#include <iostream>

#include "..\Slag\AsyncQueue.h"
#include <thread>
#include <windows.h>
#include <string>
#include <memory>

size_t n = 1 << 19;
size_t sleep_constant = 10000;
bool force_halt = false; //force halt

int run = 7;

typedef std::shared_ptr<size_t> ManagedMessage;
typedef AsyncQueue<ManagedMessage, true> HighWaterQueue;
//typedef AsyncQueue<std::shared_ptr<size_t>, true> ManagedQueue;

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

	HighWaterQueue* queue;
	size_t j;

	LARGE_INTEGER freq, timeStart, timePush, timeConsume;
	QueryPerformanceFrequency(&freq);
	ManagedMessage output1, output2;

	if (run & 1)
	{
		queue = new HighWaterQueue();
		size_t absTime1, absTime2;
		QueryPerformanceCounter(&timeStart);

		for ( size_t i = 0; i < n; ++i)
		{
			queue->EnQueue(ManagedMessage(new size_t(i)));
		}

		QueryPerformanceCounter(&timePush);
		std::cout << n << " element enqueued in " << (absTime1 = ((timePush.QuadPart-timeStart.QuadPart)*1000) / freq.QuadPart) << " ms" << std::endl;

		QueryPerformanceCounter(&timeStart);

		for (size_t i = 0; i < n; ++i)
		{
			queue->DeQueue(output1);
		}
	
		QueryPerformanceCounter(&timeConsume);
		std::cout << n << " element dequeued in " << (absTime2 = ((timeConsume.QuadPart-timeStart.QuadPart)*1000) / freq.QuadPart) << " ms" << std::endl;
		std::cout << "total time: " << absTime1 + absTime2 << " ms\n" << std::endl;
		delete queue;
	}
	if (run & 2)
	{
		queue = new HighWaterQueue();
		QueryPerformanceCounter(&timeStart);

		for (size_t i = 0; i < n; ++i)
		{
			queue->EnQueue(ManagedMessage(new size_t(i)));

			queue->DeQueue(output1);

		}
		QueryPerformanceCounter(&timePush);
		std::cout << n << " element processed in " << ((timePush.QuadPart-timeStart.QuadPart)*1000) / freq.QuadPart << " ms\n" << std::endl;

		delete queue;
	}
	if (run & 4)
	{
		queue = new HighWaterQueue();
		size_t consumed = 0;
		QueryPerformanceCounter(&timeStart);
		std::thread pusher([&]()
		{
			for (size_t i=0; i < n; ++i)
			{
				queue->EnQueue(ManagedMessage(new size_t(i)));
				if ((sleep_constant) != 0 && (i % sleep_constant == 0))
					Sleep(1);
			}
		});
		std::thread consumer([&]()
		{
			for (; true;)
			{
				if (!queue->DeQueue(output1))
					break;
				++consumed;
				//Sleep(1);//std::cout << "ok"<<std::endl;
			}
		});

		pusher.join();
		QueryPerformanceCounter(&timePush);

		size_t dropped = 0;
		if (force_halt)
			dropped = queue->WakeUp();
		else
			queue->WaitForEmpty();

		QueryPerformanceCounter(&timeConsume);

		std::cout << "pushing ended in " << ((timePush.QuadPart-timeStart.QuadPart)*1000) / freq.QuadPart << " ms" << std::endl;
		std::cout << "receiving ended in " << ((timeConsume.QuadPart-timeStart.QuadPart)*1000) / freq.QuadPart << " ms" << std::endl;

		queue->WakeUpIfEmpty();

		consumer.join();

		std::cout << n << " elements has been queued and " << consumed << " has been dequeued" << std::endl;
		std::cout << "highwater was " << (double)queue->GetHighWater()*100 / n << "%" << std::endl;
		std::cout << "dropped " << dropped << " elements\n" << std::endl;

		delete queue;
	}
	if (run & 8)
	{
		queue = new HighWaterQueue();
		size_t consumed1 = 0, consumed2 = 0, j2;

		QueryPerformanceCounter(&timeStart);
		std::thread pusher1([&]()
		{
			for (size_t i=0; i < n/2; ++i)
			{
				queue->EnQueue(ManagedMessage(new size_t(i)));
				if ((sleep_constant) != 0 && (i % sleep_constant == 0))
					Sleep(1);
			}
		});
		std::thread pusher2([&]()
		{
			for (size_t i=n/2; i < n; ++i)
			{
				queue->EnQueue(ManagedMessage(new size_t(i)));
				if ((sleep_constant) != 0 && (i % sleep_constant == 0))
					Sleep(1);
			}
		});

		std::thread consumer1([&]()
		{
			for (; true;)
			{
				if (!queue->DeQueue(output1))
					break;
				++consumed1;
					//Sleep(1);//std::cout << "ok"<<std::endl;
			}
		});

		std::thread consumer2([&]()
		{
			for (; true;)
			{
				if (!queue->DeQueue(output2))
					break;
				++consumed2;
				//Sleep(1);//std::cout << "ok"<<std::endl;
			}
		});

		pusher1.join();
		pusher2.join();

		QueryPerformanceCounter(&timePush);

		size_t dropped = 0;
		if (force_halt)
			dropped = queue->WakeUp();
		else
			queue->WaitForEmpty();

		QueryPerformanceCounter(&timeConsume);
		
		std::cout << "pushing ended in " << ((timePush.QuadPart-timeStart.QuadPart)*1000) / freq.QuadPart << " ms" << std::endl;
		std::cout << "receiving ended in " << ((timeConsume.QuadPart-timeStart.QuadPart)*1000) / freq.QuadPart << " ms" << std::endl;

		queue->WakeUpIfEmpty();

		consumer1.join();
		consumer2.join();

		std::cout << n << " elements has been queued and " << consumed1 << "+" << consumed2 << "=" << consumed1+consumed2 << " elmenents has been dequeued" << std::endl;
		std::cout << "highwater was " << (double)queue->GetHighWater()*100 / n << "%" << std::endl;
		std::cout << "dropped " << dropped << " elements\n" << std::endl;

		delete queue;
	}
	return 0;
	}