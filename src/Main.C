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


//����socket
NSLink::T_Link THBServerLink(AF_INET, SOCK_STREAM, 0);

//����socket
NSLink::T_Link TServerLink(AF_INET, SOCK_STREAM, 0);



int main (int argc, char *argv[])
{
	int HB_epollfd = 0;
	int NET_epollfd = 0;
	
	int HBReadyFdNum = 0;
	int NETReadyFdNum = 0;
	int EVENTFdNum = 0;
	int tmpnum = 0;     //ѭ������epoll�¼�
	
	int pid;       //����ӽ���pid
	int fork_pid;  //���fork�����ֵ
	int state;
	int ResetTimerFlag = 0;  //Ϊ1��ʾ��ʱ��������
	
	char Precvbuf[HBRECVMAX];   //���������Ϣ
	memset(Precvbuf, 0, HBRECVMAX);
	
	char PNetbuf[NETRECVMAX];    //���������Ϣ


	/*������epoll��ؽṹ����*/	
	struct epoll_event ev;            //����epoll
	struct epoll_event events[2048];      //����epoll
	struct epoll_event netev;         //������Ϣepoll
	struct epoll_event netevents[2048];   //������Ϣepoll

	pthread_mutex_t mutex;            //��
	pthread_mutexattr_t mutexattr;    //����״̬

	/* ���child_pid�Ͷ�Ӧ����ָ�� */
	std::unordered_map<int, NSLink::T_Link *> THBClientLinkMap;
	//std::unordered_map<int, NSLink::T_Link *>::iterator mapit;
	
	NSLink::T_Link *THBClientLink1 = new NSLink::T_Link(AF_INET, SOCK_STREAM, 0);
	NSLink::T_Link *THBClientLink2 = new NSLink::T_Link(AF_INET, SOCK_STREAM, 0);
	NSLink::T_Link *THBClientLink3 = new NSLink::T_Link(AF_INET, SOCK_STREAM, 0);
	NSLink::T_Link *THBClientLink4 = new NSLink::T_Link(AF_INET, SOCK_STREAM, 0);

	std::vector<NSLink::T_Link *> THBClientLinkVec = { THBClientLink1, THBClientLink2, THBClientLink3, THBClientLink4 };
	auto vecit = THBClientLinkVec.begin();

	T_HeapCtl THBtimerHeap(10);


	/*�����̣���������*/
	THBServerLink.LTsetsockopt();
	//std::cout << "HBserver,socket_fd:" << THBServerLink.socket_fd <<std::endl;
	THBServerLink.LTbind(HB_PORT_SERVER);
	//THBServerLink.LTSetnonblock(THBServerLink.socket_fd);  //���÷�����
	
	HB_epollfd = epoll_create(30);
	memset(&ev, 0, sizeof(ev));
	ev.events = EPOLLIN;
	ev.data.fd = THBServerLink.socket_fd;
	/* ����������socket_fd */
	if(epoll_ctl(HB_epollfd, EPOLL_CTL_ADD, THBServerLink.socket_fd, &ev) < 0 )   //��������socket_fd
	{
		close(THBServerLink.socket_fd);
		close(HB_epollfd);
		std::cout << "epoll_ctl failed!" <<std::endl;
		exit(1);
	}
	THBServerLink.LTlisten();



	/* �����̣����粿�� */
	TServerLink.LTsetsockopt();
	//std::cout << "Server,socket_fd:" << TServerLink.socket_fd << std::endl;
	TServerLink.LTbind(SREVER_PORT);
	//TServerLink.LTSetnonblock(THBServerLink.socket_fd);  //���÷�����
	NET_epollfd = epoll_create(30);
	
	memset(&netev, 0, sizeof(netev));
	netev.events = EPOLLIN;
	netev.data.fd = TServerLink.socket_fd;
	/* ���������socket_fd */
	if(epoll_ctl(NET_epollfd, EPOLL_CTL_ADD, TServerLink.socket_fd, &netev) < 0 )   //��������socket_fd
	{
		close(TServerLink.socket_fd);
		close(NET_epollfd);
		std::cout << "epoll_ctl failed!" <<std::endl;
		exit(1);
	}
	TServerLink.LTlisten();


	/*�ӽ��̣����粿��*/
	int EVENT_epollfd = 0;
	struct epoll_event EVENTev; 	  //�¼�����epoll
	struct epoll_event EVENTevents[2048];//�¼�����epoll
	EVENT_epollfd = epoll_create(30);
	memset(&ev, 0, sizeof(EVENTev));

	
	
	/* ���������ӽ��̾��⸺�� */
	pthread_mutexattr_init(&mutexattr);
	pthread_mutexattr_setpshared(&mutexattr, PTHREAD_PROCESS_SHARED);
	pthread_mutex_init(&mutex, &mutexattr);



	//����4���ӽ���
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
		vecit++;     //�ǵ�ʹ��vecitǰ��ʼ��һ�¡�
		
		if(fork_pid == 0 || fork_pid < 0)    //�����ӽ��̾ͻ��˳�����ֹ�ӽ����ٴδ����ӽ���
			break;
	}
	if(fork_pid < 0)
	{
		std::cout << "fork error!" << std::endl;
		exit(1);
	}
	
	/************************** �ӽ���ѭ�����������������ͻ��˷�����Ϣ *************************/
	if(0 == fork_pid) 
	{	
		//��ý���id
		pid = (int)getpid();
		
		/* �����ӽ�������socket */		
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

		//������ʱ������
		TSendHBTimer TsendHBtimer(pid);	
		//״̬��
		TFSM Tfsm;
		TState_Test *Tstate_test = new TState_Test();
		Tfsm.Register(Test, Tstate_test);
		

		/******�ͻ�������Ϊ����ʱ���������漰��ʱ���Ѳ������ࣺT_HeapCtl
		TSendHBTimer *PTsendtimer = new TSendHBTimer(pid);
		TSendHBTimer **PPTsendtimer = new TSendHBTimer*[1];
		PPTsendtimer[0] = PTsendtimer;
		T_HeapCtl Theapctl(0, 1, PPTsendtimer);      //�ӽ��̶�ʱ�������ѽ�������ѭ���м�鳬ʱ����������
		*/

		THBClientLink->LTConnect("192.168.1.111", HB_PORT_SERVER);   //������������
		THBClientLink->LTsend((char *)&pid, sizeof(int), 0);   //������
		while(1)
		{
			/* ��ʱ������ */
			TsendHBtimer.TimeOutFunc(THBClientLink);
			
			/* ��������socket_fd epoll */
			NETReadyFdNum = epoll_wait(NET_epollfd, netevents, 20, 0);
			/* ������·accept_fd epoll */
			EVENTFdNum = epoll_wait(EVENT_epollfd, EVENTevents, 20, 0);
			//std::cout << "EVENTFdNum is : " << EVENTFdNum << std::endl;
			/* socket_fd�仯 */
			if(NETReadyFdNum < 0)
			{
				std::cout << "child fork epoll_wait error!" <<std::endl;
				break;
			}
			else
			{
				for(int i = 0; i < NETReadyFdNum; i++)
				{
					//���裺������accept��epoll�����¼�������
					//����
					std::cout << "now in NET_epollfd " <<std::endl;
					pthread_mutex_lock(&mutex);
					//accept
					TServerLink.LTaccept();
					//epoll
					EVENTev.events = EPOLLIN | EPOLLET;
					EVENTev.data.fd = TServerLink.accept_fd;
					std::cout << "accept_fd is : " << TServerLink.accept_fd <<std::endl;
					if(epoll_ctl(EVENT_epollfd, EPOLL_CTL_ADD, TServerLink.accept_fd, &EVENTev) < 0 )   //��������socket_fd
					{
						close(TServerLink.socket_fd);
						close(EVENT_epollfd);
						std::cout << "Deal event epoll_ctl failed!" <<std::endl;
						exit(1);
					}

				}
			}

			/* accept_fd�仯 */
			if(EVENTFdNum < 0)
			{
				std::cout << "father fork epoll_wait error!" <<std::endl;
				break;
			}
			else
			{
				for(int i = 0; i < EVENTFdNum; i++)
				{
					//��������
					std::cout << "i is: " << i <<std::endl;
					std::cout << "pid is: " << (int)getpid() <<std::endl;
					std::cout << "EVENTevents.data.fd is : " << EVENTevents[i].data.fd << std::endl;
					TServerLink.LTrecv(EVENTevents[i].data.fd, PNetbuf, NETRECVMAX, 0);
					memcpy(&state, PNetbuf, sizeof(int));
					pthread_mutex_unlock(&mutex);
					
					Tfsm.GetStateRun(state);
					//����
					
				}
			}
		}

		/* �����ͻ�����Ϣ�����ԡ���ѯip(�������ݿ�)����¼(�������ݿ�) */
	}
	
	else /********************** ������ѭ���������ӽ��̡����������տͻ�����Ϣ **********************/
	{
		
		while(1)
		{
			HBReadyFdNum = epoll_wait(HB_epollfd, events, 20, 0);  //����(socket_fd��accept_fd)�������(socket_fd��accept_fd)
			if(HBReadyFdNum < 0)
			{
				std::cout << "father fork epoll_wait error!" <<std::endl;
				std::cout << strerror(errno) << std::endl;
				break;
			}
			//std::cout << "HBReadyFdNum is :" << HBReadyFdNum <<std::endl;
			for(tmpnum = 0; tmpnum < HBReadyFdNum; tmpnum++)
			{
				/* THBServerLink.socket_fd�仯������accept */
				if(events[tmpnum].data.fd == THBServerLink.socket_fd) 
				{
					std::cout << "events.data.fd(socket_fd) is" << events[0].data.fd <<std::endl;
					THBServerLink.LTaccept();
					/* epoll���accept_fd */
					ev.events = EPOLLIN;
					ev.data.fd = THBServerLink.accept_fd;
					std::cout << "accept_fd is :" << THBServerLink.accept_fd <<std::endl;
					if(epoll_ctl(HB_epollfd, EPOLL_CTL_ADD, THBServerLink.accept_fd, &ev) < 0 )   //epoll����socket_fd
					{
						close(THBServerLink.accept_fd);
						close(THBServerLink.socket_fd);
						close(HB_epollfd);
						std::cout << "epoll_ctl failed!" <<std::endl;
						exit(1);
					}
				}
				else   //THBServerLink.accept_fd�仯��(1)������ʱ��(2)���ö�ʱ����
				{
					/* ���յ���Ϣ */
					std::cout << "events.data.fd(accept_fd) is :" << events[0].data.fd <<std::endl;
					THBServerLink.LTrecv(events[tmpnum].data.fd, Precvbuf, sizeof(int), 0);
					memcpy(&pid, Precvbuf, sizeof(int));
					//pid = (int)(*Precvbuf);
					std::cout << "pid is:" << pid << std::endl;
					
					if(-1 == THBtimerHeap.cur_HeapSize)  //��ʱ��heapΪ��
					{
						std::cout << "add timer in empty heap" << std::endl;
						TrecvHBTimer *TrecvHBtimer = new TrecvHBTimer(pid);
						THBtimerHeap.AddTimer(TrecvHBtimer);

					}  
					else   //��ʱ��heap��Ϊ��
					{
						for(int i = 0; i <= THBtimerHeap.cur_HeapSize; i++)
						{
							T_Timer **tmp = THBtimerHeap.ReturnHeap();
							if(pid == tmp[i]->Pid)  //��ʱ���Ѵ���
							{
								//���ö�ʱ��
								std::cout << "reset timer" << std::endl;
								THBtimerHeap.ServerResetTimer(pid, 10);
								ResetTimerFlag = 1;
								break;
							}

						}
						if(0 == ResetTimerFlag)
						{
							//���Ӷ�ʱ��
							std::cout << "add timer" << std::endl;
							TrecvHBTimer *TrecvHBtimer = new TrecvHBTimer(pid);
							THBtimerHeap.AddTimer(TrecvHBtimer);
							ResetTimerFlag = 0;
						}
					}
					
				}
			}
			//��鶨ʱ����ʱ�����THBClientLink1����д��ָ�룬ʵ���ϲ������õ��������
			THBtimerHeap.DealTimeOut(THBClientLink1); 
		}
		exit(1);  //����ѭ����������
	}


}


