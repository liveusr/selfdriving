#ifndef ENCODER_HPP_
#define ENCODER_HPP_

#include <stdio.h>      // uart0_puts() / scanf()
#include "utilities.h"  // delay_ms()
#include "uart0_min.h"  //for uart0_puts()
#include "eint.h"
#include "scheduler_task.hpp"
#include "shared_handles.h"
#include "uart3.hpp"
#include "FreeRTOS.h"
#include "semphr.h"
#include "lpc_pwm.hpp"
#include "uart2.hpp"
#include "uart3.hpp"
#include "can.h"
#include <file_logger.h>
#include <lpc_pwm.hpp>
#include "inttypes.h"
#include <switches.hpp>
#include <io.hpp>
#include "can_common.hpp"

const uint32_t echo_pin = (1<<26);    //P0.26

#define start_timer    LPC_TIM3->TCR |= (1<<0)    //Start Timer
#define reset_timer    LPC_TIM3->TCR |= (1<<1); LPC_TIM3->TCR &= ~(1<<1)   // Set bit 1 to '0' again else TC and PC will always be '0'
#define disable_timer  LPC_TIM3->TCR &= ~(1<<0)              // Disable Timer

#define clr_interrupt  LPC_TIM3->IR |= (1<<0)  //clear interrrupt

#define use_hard_timer  //uncomment this if you wish to use hardware timer for distance calculation

extern float dcMotor;
extern float servoMotor;
extern volatile float actdcMotor;

void pwm_dcmotor(float dcmotordata);
void pwm_servomotor(float servodata);

void motorInit();
void reverseInit();
/*
 * Call back function for the external interrupt (needed for Echo pic)
 * @param : none
 */
void ext_callback();

/*
 * Function to init the P0.26 as external falling/rising edge interrupt
 * @param: ext interrupt type - eint_rising_edge or eint_falling_edge
 */
void initEchoInt(eint_intr_t eintType);

/*
 * Init the GPIO
 */
void init_io();

/*
 * Send trigger pulse task
 * Sends trigger pulse and also calculates the distance.
 */
class encoderTask : public scheduler_task
{
    public :
        encoderTask();
        bool init(void);
        bool run(void *p);
};

class pwmTask : public scheduler_task
{
    public :
        pwmTask();
        bool init(void);
        bool run(void *p);

};

class printTask : public scheduler_task
{
    public :
        printTask();
        bool init(void);
        bool run(void *p);

};

#endif /* ENCODER_HPP_ */
