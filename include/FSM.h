#ifndef FSM_H_
#define FSM_H_

#include <unordered_map>


enum StateEnum
{
	Test,
	Inquire,
	Modify
};


//状态的基类
class TState
{
public:
	TState(){};
	virtual void Run() = 0;

};


//具体的状态
class TState_Test : public TState
{
public:
	TState_Test(){};
	void Run();

};



//状态机，管理所有状态
class TFSM    
{
public:
	TFSM()
		{
			all_states = {{Test,nullptr}, {Inquire,nullptr}, {Modify,nullptr}};
		}
	~TFSM();
	void Register(int state, TState *Tstate);
	void GetStateRun(int state);
	
private:
	std::unordered_map<int, TState *> all_states;
	
};



#endif
