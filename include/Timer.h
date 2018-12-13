#ifndef TIMER_H_
#define TIMER_H_


#include "Connect.h"

#define LIFTChILD(i) (2*i+1)

//��ʱ�����࣬������ͬ���ܵĶ�ʱ��������Ҫ��ȷ��΢��Ķ�ʱ������Ҫʹ��gettimeofday������
class T_Timer        
{
public:
	T_Timer(int OverTime_s, int pid)
	{
		expire = time(nullptr) + OverTime_s;      //��ʱʱ�䣬��λs
		Pid = pid;
	}
	
	virtual void TimeOutFunc(NSLink::T_Link *link) = 0;

	
	/* ���ݲ��� */
	time_t expire;   //time_t ls type of "long" 
	int Pid;
	
};  //T_timer



class TSendHBTimer:public T_Timer
{
public:
	TSendHBTimer(int pid):T_Timer(9, pid)
	{}
		
	void TimeOutFunc(NSLink::T_Link *link);   


};  //THeartBeatTimer



class TrecvHBTimer:public T_Timer
{
public:
	TrecvHBTimer(int pid):T_Timer(10, pid)
	{}
		
	void TimeOutFunc(NSLink::T_Link *link);       //ɱ���ӽ���

//private:
	int Pid;

};  //TrecvHBTimer
/*********************************�����Ǹ���ʱ��********************************/



/*********************************�����Ƕ�ʱ����������********************************/
class T_HeapCtl
{
public:
	explicit T_HeapCtl(int heapsize);
	T_HeapCtl(int curheapsize, int heapsize, T_Timer **ExcursiveHeap);
	~T_HeapCtl();	             //�ͷ�heap�е��ڴ�
	void AddTimer(T_Timer *timer);    //���˷�ʽ��С������
	void ServerResetTimer(int pid, int timeout);
	void DownFilter(int Node);            //���˷�ʽ��С������
	void ExpandHeap();               //����timerheap
	void DealTimeOut(NSLink::T_Link *link);
	T_Timer** ReturnHeap();
	int cur_HeapSize;         //��ǰ�Ѵ�С,��0��ʼ����
	
	
private:
	T_Timer **TimerHeap;
	int HeapSize;             //���Ѵ�С����1��ʼ����

};  //T_TimerHeap

#endif  //TIMER_H_
