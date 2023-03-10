//*****************************************************************************
//
// Author		: Konstantin Chizhov
// Date			: 2010
// All rights reserved.

// Redistribution and use in source and binary forms, with or without modification, 
// are permitted provided that the following conditions are met:
// Redistributions of source code must retain the above copyright notice, 
// this list of conditions and the following disclaimer.

// Redistributions in binary form must reproduce the above copyright notice, 
// this list of conditions and the following disclaimer in the documentation and/or 
// other materials provided with the distribution.

// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND 
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
// IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY 
// OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, 
// EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//*****************************************************************************



#include "containers.h"
#include "util.h"
#include "atomic.h"

typedef void (*task_t)();

typedef struct
{
	task_t task;
	uint16_t period;
} timer_t;

template<uint8_t TasksLenght, uint8_t TimersLenght>
class Dispatcher
{
public:

	static void Init()
	{	
		_tasks.Clear();
		for(uint8_t i=0; i<_timers.Size(); i++)
		{
			_timers[i].task = 0;
			_timers[i].period = 0;
		}
	}

	static void SetTask(task_t task)
	{
		ATOMIC{	_tasks.Write(task);}
	}

	static void SetTimer(task_t task, uint16_t period) __attribute__ ((noinline))
	{
		uint8_t i_idle=0;
		ATOMIC
		{
			for(uint8_t i=0; i<_timers.Size(); i++)
			{
				if(_timers[i].task == 0)
				{
					i_idle = i;
				}
				if(_timers[i].task == task)
				{
					_timers[i].period = period;
					return;
				}
			}
			_timers[i_idle].task = task;
			_timers[i_idle].period = period;
		}
	}

	static void StopTimer(task_t task)
	{
		ATOMIC
		{
			for(uint8_t i=0; i<_timers.Size(); i++)
			{
				if(_timers[i].task == task)
				{
					_timers[i].task = 0;
					return;
				}
			}
		}
	}

	static void Poll()
	{
		task_t task;
		//NOTE: no beed to block task Queue here. This is the only place the Queue read.
		//cli();
		if(_tasks.Read(task))
		{
		//	sei();
			task();
		}
		//sei();
	}

	static void TimerHandler()
	{
		for(uint8_t i=0; i<_timers.Size(); i++)
		{
			if(_timers[i].task != 0 && --_timers[i].period == 0)
			{
				_tasks.Write(_timers[i].task); 
				_timers[i].task = 0;
			}
		}
	}

private:
	static Queue<TasksLenght, task_t> _tasks;
	static Array<TimersLenght, timer_t> _timers;
};

template<uint8_t TasksLenght, uint8_t TimersLenght>
Array<TimersLenght, timer_t> Dispatcher<TasksLenght, TimersLenght>::_timers;

template<uint8_t TasksLenght, uint8_t TimersLenght>
Queue<TasksLenght, task_t> Dispatcher<TasksLenght, TimersLenght>::_tasks;




