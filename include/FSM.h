#ifndef FSM_H_
#define FSM_H_

#include <unordered_map>


enum StateEnum
{
	Test,
	Inquire,
	Modify
};


//״̬�Ļ���
class TState
{
public:
	TState(){};
	virtual void Run() = 0;

};


//�����״̬
class TState_Test : public TState
{
public:
	TState_Test(){};
	void Run();

};



//״̬������������״̬
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
