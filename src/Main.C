#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <vector>

#include <pthread.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <errno.h>
#include <string.h>
#include <memory.h>
#include <sys/epoll.h>  


#include "FSM.h"
#include "Connect.h"
#include "Timer.h"

#define HBRECVMAX 4
#define NETRECVMAX 10


//心跳socket
NSLink::T_Link THBServerLink(AF_INET, SOCK_STREAM, 0);

//网络socket
NSLink::T_Link TServerLink(AF_INET, SOCK_STREAM, 0);



int main (int argc, char *argv[])
{
	int HB_epollfd = 0;
	int NET_epollfd = 0;
	
	int HBReadyFdNum = 0;
	int NETReadyFdNum = 0;
	int EVENTFdNum = 0;
	int tmpnum = 0;     //循环访问epoll事件
	
	int pid;       //存放子进程pid
	int fork_pid;  //存放fork后产生值
	int state;
	int ResetTimerFlag = 0;  //为1表示定时器已重置
	
	char Precvbuf[HBRECVMAX];   //存放心跳消息
	memset(Precvbuf, 0, HBRECVMAX);
	
	char PNetbuf[NETRECVMAX];    //存放网络信息


	/*父进程epoll相关结构定义*/	
	struct epoll_event ev;            //心跳epoll
	struct epoll_event events[2048];      //心跳epoll
	struct epoll_event netev;         //网络消息epoll
	struct epoll_event netevents[2048];   //网络消息epoll

	pthread_mutex_t mutex;            //锁
	pthread_mutexattr_t mutexattr;    //锁的状态

	/* 存放child_pid和对应的类指针 */
	std::unordered_map<int, NSLink::T_Link *> THBClientLinkMap;
	//std::unordered_map<int, NSLink::T_Link *>::iterator mapit;
	
	NSLink::T_Link *THBClientLink1 = new NSLink::T_Link(AF_INET, SOCK_STREAM, 0);
	NSLink::T_Link *THBClientLink2 = new NSLink::T_Link(AF_INET, SOCK_STREAM, 0);
	NSLink::T_Link *THBClientLink3 = new NSLink::T_Link(AF_INET, SOCK_STREAM, 0);
	NSLink::T_Link *THBClientLink4 = new NSLink::T_Link(AF_INET, SOCK_STREAM, 0);

	std::vector<NSLink::T_Link *> THBClientLinkVec = { THBClientLink1, THBClientLink2, THBClientLink3, THBClientLink4 };
	auto vecit = THBClientLinkVec.begin();

	T_HeapCtl THBtimerHeap(10);


	/*父进程：心跳部分*/
	THBServerLink.LTsetsockopt();
	//std::cout << "HBserver,socket_fd:" << THBServerLink.socket_fd <<std::endl;
	THBServerLink.LTbind(HB_PORT_SERVER);
	//THBServerLink.LTSetnonblock(THBServerLink.socket_fd);  //设置非阻塞
	
	HB_epollfd = epoll_create(30);
	memset(&ev, 0, sizeof(ev));
	ev.events = EPOLLIN;
	ev.data.fd = THBServerLink.socket_fd;
	/* 添加心跳的socket_fd */
	if(epoll_ctl(HB_epollfd, EPOLL_CTL_ADD, THBServerLink.socket_fd, &ev) < 0 )   //加入心跳socket_fd
	{
		close(THBServerLink.socket_fd);
		close(HB_epollfd);
		std::cout << "epoll_ctl failed!" <<std::endl;
		exit(1);
	}
	THBServerLink.LTlisten();



	/* 父进程：网络部分 */
	TServerLink.LTsetsockopt();
	//std::cout << "Server,socket_fd:" << TServerLink.socket_fd << std::endl;
	TServerLink.LTbind(SREVER_PORT);
	//TServerLink.LTSetnonblock(THBServerLink.socket_fd);  //设置非阻塞
	NET_epollfd = epoll_create(30);
	
	memset(&netev, 0, sizeof(netev));
	netev.events = EPOLLIN;
	netev.data.fd = TServerLink.socket_fd;
	/* 添加网络的socket_fd */
	if(epoll_ctl(NET_epollfd, EPOLL_CTL_ADD, TServerLink.socket_fd, &netev) < 0 )   //加入网络socket_fd
	{
		close(TServerLink.socket_fd);
		close(NET_epollfd);
		std::cout << "epoll_ctl failed!" <<std::endl;
		exit(1);
	}
	TServerLink.LTlisten();


	/*子进程：网络部分*/
	int EVENT_epollfd = 0;
	struct epoll_event EVENTev; 	  //事件处理epoll
	struct epoll_event EVENTevents[2048];//事件处理epoll
	EVENT_epollfd = epoll_create(30);
	memset(&ev, 0, sizeof(EVENTev));

	
	
	/* 进程锁，子进程均衡负载 */
	pthread_mutexattr_init(&mutexattr);
	pthread_mutexattr_setpshared(&mutexattr, PTHREAD_PROCESS_SHARED);
	pthread_mutex_init(&mutex, &mutexattr);



	//创建4个子进程
	for(int i = 0; i < 4; i++)   
	{		
		fork_pid = fork();
		if(0 == fork_pid)
		{
			int tmppid = (int)getpid();
			THBClientLinkMap[tmppid] = *vecit;
			auto mapit = THBClientLinkMap.find(tmppid);
			std::cout << "mapit found" << mapit->first << "ls" << mapit->second << std::endl;
		}
		vecit++;     //记得使用vecit前初始化一下。
		
		if(fork_pid == 0 || fork_pid < 0)    //若是子进程就会退出，防止子进程再次创建子进程
			break;
	}
	if(fork_pid < 0)
	{
		std::cout << "fork error!" << std::endl;
		exit(1);
	}
	
	/************************** 子进程循环：发心跳、处理客户端发来消息 *************************/
	if(0 == fork_pid) 
	{	
		//获得进程id
		pid = (int)getpid();
		
		/* 创建子进程心跳socket */		
		NSLink::T_Link *THBClientLink = nullptr;
		auto mapit = THBClientLinkMap.find(pid);
		//std::cout << "in mapit is: " << *mapit << std::endl;
		//std::cout << "in mapit see pid is: " << mapit->first << std::endl;
		if(mapit != THBClientLinkMap.end())
		{
			THBClientLink = mapit->second;
		}
		
		//std::cout << "THBClientLink->socket_fd" << THBClientLink->socket_fd << std::endl;
		THBClientLink->LTsetsockopt();
		//THBClientLink->LTbind(HB_PORT_CLIENT);

		//创建定时器对象
		TSendHBTimer TsendHBtimer(pid);	
		//状态机
		TFSM Tfsm;
		TState_Test *Tstate_test = new TState_Test();
		Tfsm.Register(Test, Tstate_test);
		

		/******客户端心跳为单定时器，无需涉及定时器堆操作的类：T_HeapCtl
		TSendHBTimer *PTsendtimer = new TSendHBTimer(pid);
		TSendHBTimer **PPTsendtimer = new TSendHBTimer*[1];
		PPTsendtimer[0] = PTsendtimer;
		T_HeapCtl Theapctl(0, 1, PPTsendtimer);      //子进程定时器队列已建立，在循环中检查超时发心跳即可
		*/

		THBClientLink->LTConnect("192.168.1.111", HB_PORT_SERVER);   //发起三次握手
		THBClientLink->LTsend((char *)&pid, sizeof(int), 0);   //发心跳
		while(1)
		{
			/* 定时发心跳 */
			TsendHBtimer.TimeOutFunc(THBClientLink);
			
			/* 监听网络socket_fd epoll */
			NETReadyFdNum = epoll_wait(NET_epollfd, netevents, 20, 0);
			/* 监听网路accept_fd epoll */
			EVENTFdNum = epoll_wait(EVENT_epollfd, EVENTevents, 20, 0);
			//std::cout << "EVENTFdNum is : " << EVENTFdNum << std::endl;
			/* socket_fd变化 */
			if(NETReadyFdNum < 0)
			{
				std::cout << "child fork epoll_wait error!" <<std::endl;
				break;
			}
			else
			{
				for(int i = 0; i < NETReadyFdNum; i++)
				{
					//步骤：上锁、accept、epoll添加事件、解锁
					//上锁
					std::cout << "now in NET_epollfd " <<std::endl;
					pthread_mutex_lock(&mutex);
					//accept
					TServerLink.LTaccept();
					//epoll
					EVENTev.events = EPOLLIN | EPOLLET;
					EVENTev.data.fd = TServerLink.accept_fd;
					std::cout << "accept_fd is : " << TServerLink.accept_fd <<std::endl;
					if(epoll_ctl(EVENT_epollfd, EPOLL_CTL_ADD, TServerLink.accept_fd, &EVENTev) < 0 )   //加入心跳socket_fd
					{
						close(TServerLink.socket_fd);
						close(EVENT_epollfd);
						std::cout << "Deal event epoll_ctl failed!" <<std::endl;
						exit(1);
					}

				}
			}

			/* accept_fd变化 */
			if(EVENTFdNum < 0)
			{
				std::cout << "father fork epoll_wait error!" <<std::endl;
				break;
			}
			else
			{
				for(int i = 0; i < EVENTFdNum; i++)
				{
					//接受数据
					std::cout << "i is: " << i <<std::endl;
					std::cout << "pid is: " << (int)getpid() <<std::endl;
					std::cout << "EVENTevents.data.fd is : " << EVENTevents[i].data.fd << std::endl;
					TServerLink.LTrecv(EVENTevents[i].data.fd, PNetbuf, NETRECVMAX, 0);
					memcpy(&state, PNetbuf, sizeof(int));
					pthread_mutex_unlock(&mutex);
					
					Tfsm.GetStateRun(state);
					//解锁
					
				}
			}
		}

		/* 处理客户端消息：测试、查询ip(操作数据库)、登录(操作数据库) */
	}
	
	else /********************** 父进程循环：创建子进程、收心跳、收客户端消息 **********************/
	{
		
		while(1)
		{
			HBReadyFdNum = epoll_wait(HB_epollfd, events, 20, 0);  //心跳(socket_fd、accept_fd)，服务端(socket_fd、accept_fd)
			if(HBReadyFdNum < 0)
			{
				std::cout << "father fork epoll_wait error!" <<std::endl;
				std::cout << strerror(errno) << std::endl;
				break;
			}
			//std::cout << "HBReadyFdNum is :" << HBReadyFdNum <<std::endl;
			for(tmpnum = 0; tmpnum < HBReadyFdNum; tmpnum++)
			{
				/* THBServerLink.socket_fd变化，调用accept */
				if(events[tmpnum].data.fd == THBServerLink.socket_fd) 
				{
					std::cout << "events.data.fd(socket_fd) is" << events[0].data.fd <<std::endl;
					THBServerLink.LTaccept();
					/* epoll检测accept_fd */
					ev.events = EPOLLIN;
					ev.data.fd = THBServerLink.accept_fd;
					std::cout << "accept_fd is :" << THBServerLink.accept_fd <<std::endl;
					if(epoll_ctl(HB_epollfd, EPOLL_CTL_ADD, THBServerLink.accept_fd, &ev) < 0 )   //epoll监听socket_fd
					{
						close(THBServerLink.accept_fd);
						close(THBServerLink.socket_fd);
						close(HB_epollfd);
						std::cout << "epoll_ctl failed!" <<std::endl;
						exit(1);
					}
				}
				else   //THBServerLink.accept_fd变化，(1)创建定时器(2)重置定时器。
				{
					/* 接收到消息 */
					std::cout << "events.data.fd(accept_fd) is :" << events[0].data.fd <<std::endl;
					THBServerLink.LTrecv(events[tmpnum].data.fd, Precvbuf, sizeof(int), 0);
					memcpy(&pid, Precvbuf, sizeof(int));
					//pid = (int)(*Precvbuf);
					std::cout << "pid is:" << pid << std::endl;
					
					if(-1 == THBtimerHeap.cur_HeapSize)  //定时器heap为空
					{
						std::cout << "add timer in empty heap" << std::endl;
						TrecvHBTimer *TrecvHBtimer = new TrecvHBTimer(pid);
						THBtimerHeap.AddTimer(TrecvHBtimer);

					}  
					else   //定时器heap不为空
					{
						for(int i = 0; i <= THBtimerHeap.cur_HeapSize; i++)
						{
							T_Timer **tmp = THBtimerHeap.ReturnHeap();
							if(pid == tmp[i]->Pid)  //定时器已存在
							{
								//重置定时器
								std::cout << "reset timer" << std::endl;
								THBtimerHeap.ServerResetTimer(pid, 10);
								ResetTimerFlag = 1;
								break;
							}

						}
						if(0 == ResetTimerFlag)
						{
							//添加定时器
							std::cout << "add timer" << std::endl;
							TrecvHBTimer *TrecvHBtimer = new TrecvHBTimer(pid);
							THBtimerHeap.AddTimer(TrecvHBtimer);
							ResetTimerFlag = 0;
						}
					}
					
				}
			}
			//检查定时器超时。这个THBClientLink1是乱写的指针，实际上并不会用到这个参数
			THBtimerHeap.DealTimeOut(THBClientLink1); 
		}
		exit(1);  //跳出循环即不正常
	}


}



