#include <stdio.h>
#include "utilities.h"
#include "uart0_min.h"
#include "eint.h"
#include "encoder.hpp"
#include "soft_timer.hpp"
#include "tasks.hpp"
#include "io.hpp"
#include "uart2.hpp"
#include "uart3.hpp"
#include "can.h"
#include <file_logger.h>
#include <lpc_pwm.hpp>
#include "inttypes.h"
#include <switches.hpp>
#include "can_common.hpp"
#include "controller_motor.hpp"

#define conversion_factor (float)0.0223693629
#define wheel_circumfurance 35
#define pwmSettle 100

#define initSeq         (float)15.0
#define fwdExtreme      (float)20.05
#define revExtreme      (float)10.07
bool edge = 1;
int extIntCount = 0;
volatile float actdcMotor = 0;
float dcMotor = 0, servoMotor = 14.7;
float distance;
long yield = 0;
float prevdcMotor = 0.0, prevservoMotor = 0.0;
extern speed_encoder_data_t periodic_encoder_data;
extern float curr_pwm;

void init_io()
{
    unsigned int clock;
    clock = sys_get_cpu_clock();
    LPC_PINCON->PINSEL1 &= ~(3<<20); //P0.26 as GPIO (for Echo pin)

#ifdef use_hard_timer
    lpc_pconp(pconp_timer3, true);
    LPC_SC->PCLKSEL1 |= (1<<14);    //PCLK = CLK/1
    LPC_TIM3->CTCR = 0;             // Increment on PCLK
    LPC_TIM3->PR = 0x016D72B0; //500ms //0x000001E0; //10us   //0x000005A0; //30 us//0x00000780; //40 us  //0x0000069A; //35 us  //0x000005A0; //30 us  //0x000003C0; //20 us      // //0x00000032;  // 1us
    // Enable MR0 interrupt
    LPC_TIM3->MR0 = 1;          //Minimum value of 1. So interrupt will now be generated every 30+30 = 60us
    LPC_TIM3->MCR |= (1 << 0);
    LPC_TIM3->MCR |= (1 << 1);
    NVIC_EnableIRQ(TIMER3_IRQn);
#endif
    initEchoInt(eint_falling_edge);
}

void initEchoInt(eint_intr_t eintType)
{
    eint3_enable_port0(26,eintType,ext_callback);  //P0.26 - Echo pin
}

#ifdef use_hard_timer
extern "C"
{
    void TIMER3_IRQHandler()    //timer code just in case
    {
        if(LPC_TIM3->IR & (1<<0))       //if MR0 channel is 1
            clr_interrupt;
        portYIELD_FROM_ISR(yield);
        distance = extIntCount*wheel_circumfurance*conversion_factor;
        periodic_encoder_data.speed = distance;
//        max_speed = (max_speed < distance)? distance: max_speed;
        extIntCount = 0;
    }
}
#endif

void ext_callback()                     // Turning LED on and off for every rising edge on the encoder
{                                       // LED for testing purpose only
    static int led = 0;
    if(led == 0)
    {
        LE.on(4);
        led = 1;
    }
    else
    {
        LE.off(4);
        led = 0;
    }
    extIntCount++;
    portYIELD_FROM_ISR(yield);
}

encoderTask::encoderTask() : scheduler_task("encoder", 3 * 512, PRIORITY_LOW)
{
    //setRunDuration(1000);
}

bool encoderTask::init(void)
{
    init_io();
    LE.init();
    start_timer;
    return true;
}

bool encoderTask::run(void *p)
{
    return true;
}
#if 0
pwmTask::pwmTask() : scheduler_task("PWM", 3 * 512, PRIORITY_HIGH)
{
    //setRunDuration(1000);
}

bool pwmTask::init(void)
{
    motorInit();
    return true;
}

bool pwmTask::run(void *p)
{
    PWM pwm2(PWM::pwm3, 100);       // For Servo Motor
    PWM pwm1(PWM::pwm2, 100);       // For DC Motor
    dcMotor = 15.02747555 + (0.2213318273*actdcMotor);  // Formula used for converting Duty Cycle to mph
    pwm2.set(servoMotor);
    pwm1.set(dcMotor);
    delay_us(pwmSettle);
    while(speed < actdcMotor)
        dcMotor += 0.1;
    return true;
}
#endif

printTask::printTask() : scheduler_task("Print", 4 * 512, PRIORITY_LOW) // Print task for testing purpose only
{
    //setRunDuration(1000);
}

bool printTask::init(void)
{
    return true;
}

bool printTask::run(void *p)
{
    //printf("In print task\n");
   // if((int)speed != 0)
//    printf("Speed = %dmph\n", (int)periodic_encoder_data.speed);
    vTaskDelayMs(1000);
    return true;
}

void motorInit()
{
    PWM pwm1(PWM::pwm2, 100);       // For DC Motor
    static int Init = 15;           // For giving Initialization Sequence
    static int c = 0;
    while(c < 3)
    {
        pwm1.set(Init);
        delay_ms(300);
        c++;
    }
    actdcMotor = 1;
    delay_ms(500);
}

void pwm_dcmotor(float dcmotordata)
{
    curr_pwm = dcmotordata;
    //printf("in pwm_dcmotor\n");
   // PWM pwm2(PWM::pwm3, 100);       // For Servo Motor
    PWM pwm1(PWM::pwm2, 100);       // For DC Motor
    //dcMotor = 15.02747555 + (0.2213318273*motordata);  // Formula used for converting Duty Cycle to mph
    //pwm2.set(servoMotor);
    pwm1.set(dcmotordata);


}

void pwm_servomotor(float servodata)
{
    //printf("in pwm_servomotor\n");
    PWM pwm2(PWM::pwm3, 100);       // For Servo Motor
    pwm2.set(servodata);

}
void reverseInit()
{
    //printf("In reverse init\n");
    PWM pwm1(PWM::pwm2, 100);
    int i;
    float cnt = fwdExtreme;

    while(cnt>15.05)
    {
        pwm1.set(cnt);
        cnt = cnt - 0.3;
    }

    for(i=0; i<5000; i++)
        pwm1.set(initSeq);

    for(i=0; i<5000; i++)
        pwm1.set(revExtreme);

    for(i=0; i<5000; i++)
        pwm1.set(initSeq);

}
