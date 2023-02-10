#include <avr/io.h>
#include <timers.h>
#include <timer_utils.h>
#include <dispatcher.h>
#include <iopins.h>
#include <avr/interrupt.h>
#include <adc.h>

using namespace Timers;

const unsigned tickFreak = 1000;

typedef Timer0 Timer;
typedef TimerFreqSetup<Timer, tickFreak, 16000000> FSetup;
typedef Dispatcher<4, 4> Disp;
typedef IO::Pd6 PulsePin;
typedef IO::Pd7 DirPin;
typedef IO::Pb5 EndSensor2;
typedef IO::Pb4 EndSensor1;
ISR(TIMER0_OVF_vect)
{
	Timer::Set(FSetup::ReloadValue);
	Disp::TimerHandler();
}

void Toggle()
{
        PulsePin::Toggle();
	Disp::SetTimer(Toggle, 10);
}

void ChangeDirection()
{
	DirPin::Toggle();
	Disp::SetTimer(ChangeDirection, 1000);
}

bool isStopped = false;
ISR(PCINT0_vect) {
        if (isStopped) {
                Toggle();
                isStopped = false;
        } else {
                Disp::StopTimer(Toggle);
                isStopped = true;
        }
}

int main()
{
	Disp::Init();
	Timer::Start(FSetup::Divider);
	Timer::EnableInterrupt();
	PulsePin::SetDirWrite();
	DirPin::SetDirWrite();
        EndSensor1::SetDirRead();
        EndSensor2::SetDirRead();
	Adc::Init();
        PCICR |= 1 << PCIE0;
        PCMSK0 |= (1 << PCINT4) | (1 << PCINT5);
	sei();


        Toggle();
        ChangeDirection();
	while(1)
	{
		Disp::Poll();
	}
}
