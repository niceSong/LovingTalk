#ifndef TIMER_H_
#define TIMER_H_


#include "Connect.h"

#define LIFTChILD(i) (2*i+1)

//定时器基类，派生不同功能的定时器。若需要精确至微妙的定时器，需要使用gettimeofday函数。
class T_Timer        
{
public:
	T_Timer(int OverTime_s, int pid)
	{
		expire = time(nullptr) + OverTime_s;      //超时时间，单位s
		Pid = pid;
	}
	
	virtual void TimeOutFunc(NSLink::T_Link *link) = 0;

	
	/* 数据部分 */
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
		
	void TimeOutFunc(NSLink::T_Link *link);       //杀死子进程

//private:
	int Pid;

};  //TrecvHBTimer
/*********************************以上是各定时器********************************/



/*********************************以下是定时器操作函数********************************/
class T_HeapCtl
{
public:
	explicit T_HeapCtl(int heapsize);
	T_HeapCtl(int curheapsize, int heapsize, T_Timer **ExcursiveHeap);
	~T_HeapCtl();	             //释放heap中的内存
	void AddTimer(T_Timer *timer);    //上滤方式最小堆排序
	void ServerResetTimer(int pid, int timeout);
	void DownFilter(int Node);            //下滤方式最小堆排序
	void ExpandHeap();               //扩大timerheap
	void DealTimeOut(NSLink::T_Link *link);
	T_Timer** ReturnHeap();
	int cur_HeapSize;         //当前堆大小,从0开始计算
	
	
private:
	T_Timer **TimerHeap;
	int HeapSize;             //最大堆大小：从1开始计算

};  //T_TimerHeap

#endif  //TIMER_H_
