#include <iostream>
#include <cstdio>
#include <cstdlib>

#include <sys/time.h>
#include <errno.h>
#include <sys/types.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#include "Timer.h"




/* ------------------------------定时器超时处理函数---------------------------------- */
void TSendHBTimer::TimeOutFunc(NSLink::T_Link *link) 
{
	time_t TCurrentTime = time(nullptr);
	
	if(TCurrentTime > expire)
	{
		//发送心跳
		std::cout << "TCurrentTime : "<< (int)TCurrentTime << " expire : " << (int)expire << std::endl;
		link->LTsend((char *)&Pid, sizeof(int), 0);

		//重置定时器
		expire = TCurrentTime + 9;
	}
}



void TrecvHBTimer::TimeOutFunc(NSLink::T_Link *link)
{
	//杀死子进程
	std::cout << "some process will be kill" << std::endl;
	int tmp = kill(Pid, SIGKILL);
	if(tmp)
	{
		std::cout << strerror(errno) << std::endl;
	}
	//关闭fd
	//close(link.accept_fd);
}



/* ------------------------------定时器堆操作函数---------------------------------- */
T_HeapCtl::T_HeapCtl(int heapsize)
    :HeapSize(heapsize), cur_HeapSize(-1)
{
    //创建堆数组
    TimerHeap = new T_Timer*[heapsize];
    if (!TimerHeap) 
	{
        std::cout << "init heap_timer failed" << std::endl;
        return;
    }
 
    for (int i = 0; i < heapsize; ++i)
        TimerHeap[i] = nullptr;
}




T_HeapCtl::T_HeapCtl(int curheapsize, int heapsize, T_Timer **ExcursiveHeap)
	:TimerHeap(nullptr),HeapSize(heapsize),cur_HeapSize(curheapsize)
{	
	if(curheapsize >= heapsize)
	{
		std::cout<< "error:cur_HeapSize >= HeapSize" <<std::endl;
		exit(0);
	}
		
	TimerHeap = new T_Timer *[heapsize];
	if(nullptr == TimerHeap)
	{
		std::cout << "error:faild new timerheap!" <<std::endl;
		exit(0);
	}
	//初始化定时器堆
	for(int i = 0; i < heapsize; i++)
		TimerHeap[i] = nullptr;

	//给定时器堆赋值
	if(cur_HeapSize >= 0)
	{
		//先将数组赋值，cur_HeapSize从0开始，和数组的访问下标一致
		for(int i = 0; i <= curheapsize; i++)
		{
			TimerHeap[i] = ExcursiveHeap[i];
		}

		if(curheapsize > 0)
		{
			//对每个节点采用“下滤法”，对第0个也要执行，减为负数才不执行
			for(int i = (curheapsize - 1)/2; i >= 0; i--)
			{
				DownFilter(i);
			}
		}

	}
	
}


	

T_HeapCtl::~T_HeapCtl()
{
	for(int i; i < HeapSize; i++)
		delete TimerHeap[i];

	delete[] TimerHeap;
}




void T_HeapCtl::AddTimer(T_Timer * timer)
{
	if(timer == nullptr)
	{
		std::cout << "can't add NULL timer" << std::endl;
	}
	if(cur_HeapSize >= HeapSize -1)
		ExpandHeap();

	//先假设添加定时器到heap尾，然后采用"上滤法"将定时器放到合适的位置
	int node = ++cur_HeapSize;
	int parent;

	for(; node > 0; node = parent)
	{
		parent = (node - 1)/2;
		if(TimerHeap[parent]->expire >= timer->expire)
			TimerHeap[node] = TimerHeap[parent];
		else
			break;
	}
	TimerHeap[node] = timer;
	std::cout << "node is:" << node << std::endl;
	std::cout << "addr is:" << TimerHeap[node] << std::endl;

}





//根据pid重置定时器
void T_HeapCtl::ServerResetTimer(int pid, int timeout)
{
	int i = 0;   //保存被重置的节点位置信息
	//遍历heap找到name对应的定时器，重设超时时间
	if(cur_HeapSize >= 0)
	{
		for(i; i <= cur_HeapSize; i++)
		{
			if(TimerHeap[i]->Pid == pid)
			{
				TimerHeap[i]->expire = time(nullptr) + timeout;
			}
		}
	}
	
	//重新给最小堆排序，从i节点进行下滤
	DownFilter(i);
}




void T_HeapCtl::DownFilter(int Node)       //从Node节点开始下滤
{
	T_Timer *Ttmp = TimerHeap[Node];
	int Child = 0;
	for(; LIFTChILD(Node) <= cur_HeapSize; Node = Child)    
	{
		Child = LIFTChILD(Node);
		if((Child <= (cur_HeapSize -1)) && 
					(TimerHeap[Child]->expire >= TimerHeap[Child+1]->expire))   //min(左、右孩子)
		{
			Child++;       //左节点 变为 右节点
		}

		if(Ttmp->expire >= TimerHeap[Child]->expire)
		{
			TimerHeap[Node] = TimerHeap[Child];
		}
	}
	
	TimerHeap[Node] = Ttmp;
}




void T_HeapCtl::ExpandHeap()
{
	HeapSize = 2 * HeapSize;
	T_Timer **tmp = new T_Timer*[HeapSize];
	if (tmp = nullptr) 
	{
        std::cout << "ExpandHeap() failed.\n" << std::endl;
		exit(0);
    }
	
		
    for (int i = 0; i < HeapSize; ++i)
        tmp[i] = nullptr;
 
 
    for (int i = 0; cur_HeapSize >= i; ++i)      //cur_HeapSize和数组下标相同
        tmp[i] = TimerHeap[i];
 
    delete[] TimerHeap;   //删除该指针
    TimerHeap = tmp;

}




void T_HeapCtl::DealTimeOut(NSLink::T_Link *link)    //判断有无定时器超时，并处理
{		
		//获得当前时间
		time_t cur_time = time(nullptr);
		T_Timer *PTimer = TimerHeap[0];

		//循环查看定时器
		while(cur_HeapSize >= 0)  //cur_HeapSize=0，有一个定时器
		{
			if(nullptr == PTimer)
			{
				std::cout<< "no tiemr in heap" <<std::endl;
				break;
			}

			//没定时器超时
			if(TimerHeap[0]->expire >= cur_time)
				break;
			
			//有定时器超时
			std::cout << "expire is: " << TimerHeap[0]->expire << "cur_time is: " << cur_time << std::endl;
			TimerHeap[0]->TimeOutFunc(link);
			//删除定时器
			delete TimerHeap[0];
			TimerHeap[0] = nullptr;
			if(0 < cur_HeapSize)
			{
				TimerHeap[0] = TimerHeap[cur_HeapSize--];
				DownFilter(0);
			}
		}
}



T_Timer** T_HeapCtl::ReturnHeap()
{
	return TimerHeap;
}


