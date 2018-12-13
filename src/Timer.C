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




/* ------------------------------��ʱ����ʱ��������---------------------------------- */
void TSendHBTimer::TimeOutFunc(NSLink::T_Link *link) 
{
	time_t TCurrentTime = time(nullptr);
	
	if(TCurrentTime > expire)
	{
		//��������
		std::cout << "TCurrentTime : "<< (int)TCurrentTime << " expire : " << (int)expire << std::endl;
		link->LTsend((char *)&Pid, sizeof(int), 0);

		//���ö�ʱ��
		expire = TCurrentTime + 9;
	}
}



void TrecvHBTimer::TimeOutFunc(NSLink::T_Link *link)
{
	//ɱ���ӽ���
	std::cout << "some process will be kill" << std::endl;
	int tmp = kill(Pid, SIGKILL);
	if(tmp)
	{
		std::cout << strerror(errno) << std::endl;
	}
	//�ر�fd
	//close(link.accept_fd);
}



/* ------------------------------��ʱ���Ѳ�������---------------------------------- */
T_HeapCtl::T_HeapCtl(int heapsize)
    :HeapSize(heapsize), cur_HeapSize(-1)
{
    //����������
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
	//��ʼ����ʱ����
	for(int i = 0; i < heapsize; i++)
		TimerHeap[i] = nullptr;

	//����ʱ���Ѹ�ֵ
	if(cur_HeapSize >= 0)
	{
		//�Ƚ����鸳ֵ��cur_HeapSize��0��ʼ��������ķ����±�һ��
		for(int i = 0; i <= curheapsize; i++)
		{
			TimerHeap[i] = ExcursiveHeap[i];
		}

		if(curheapsize > 0)
		{
			//��ÿ���ڵ���á����˷������Ե�0��ҲҪִ�У���Ϊ�����Ų�ִ��
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

	//�ȼ������Ӷ�ʱ����heapβ��Ȼ�����"���˷�"����ʱ���ŵ����ʵ�λ��
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





//����pid���ö�ʱ��
void T_HeapCtl::ServerResetTimer(int pid, int timeout)
{
	int i = 0;   //���汻���õĽڵ�λ����Ϣ
	//����heap�ҵ�name��Ӧ�Ķ�ʱ�������賬ʱʱ��
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
	
	//���¸���С�����򣬴�i�ڵ��������
	DownFilter(i);
}




void T_HeapCtl::DownFilter(int Node)       //��Node�ڵ㿪ʼ����
{
	T_Timer *Ttmp = TimerHeap[Node];
	int Child = 0;
	for(; LIFTChILD(Node) <= cur_HeapSize; Node = Child)    
	{
		Child = LIFTChILD(Node);
		if((Child <= (cur_HeapSize -1)) && 
					(TimerHeap[Child]->expire >= TimerHeap[Child+1]->expire))   //min(���Һ���)
		{
			Child++;       //��ڵ� ��Ϊ �ҽڵ�
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
 
 
    for (int i = 0; cur_HeapSize >= i; ++i)      //cur_HeapSize�������±���ͬ
        tmp[i] = TimerHeap[i];
 
    delete[] TimerHeap;   //ɾ����ָ��
    TimerHeap = tmp;

}




void T_HeapCtl::DealTimeOut(NSLink::T_Link *link)    //�ж����޶�ʱ����ʱ��������
{		
		//��õ�ǰʱ��
		time_t cur_time = time(nullptr);
		T_Timer *PTimer = TimerHeap[0];

		//ѭ���鿴��ʱ��
		while(cur_HeapSize >= 0)  //cur_HeapSize=0����һ����ʱ��
		{
			if(nullptr == PTimer)
			{
				std::cout<< "no tiemr in heap" <<std::endl;
				break;
			}

			//û��ʱ����ʱ
			if(TimerHeap[0]->expire >= cur_time)
				break;
			
			//�ж�ʱ����ʱ
			std::cout << "expire is: " << TimerHeap[0]->expire << "cur_time is: " << cur_time << std::endl;
			TimerHeap[0]->TimeOutFunc(link);
			//ɾ����ʱ��
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

