/*
 * ping_sensor.hpp
 *
 *  Created on: Oct 25, 2014
 *      Author: Manuj
 */

#ifndef SENSOR_ULTRASONIC_HPP_
#define SENSOR_ULTRASONIC_HPP_


#include <stdio.h>
#include "utilities.h"
#include "uart0_min.h"
#include "eint.h"
#include "scheduler_task.hpp"
#include "shared_handles.h"
#include "uart3.hpp"
#include "FreeRTOS.h"
#include "semphr.h"
#include "can_common.hpp"


#ifdef ENABLE_SENSOR // enabled from can_common.hpp

const uint32_t echo_pin_center = (1<<2);    //P2.2
//const uint32_t echo_pin_center = (1<<26);    //P0.26
const uint32_t echo_pin_right = (1<<0);    //P2.0
const uint32_t echo_pin_left = (1<<1);    //P2.1

extern uint8_t ultraDist[5];    // [0]:left, [1]:center, [2]:right

//ISR's for the sensor interrupts
void ext_callback_center();
void ext_callback_right();
void ext_callback_left();


void trigger_center();
void echo_center();
void trigger_right();
void echo_right();
void trigger_left();
void echo_left();


/*
 * Function to init the P0.26,p2.0.p2 as external falling edge interrupt
 * @param: ext interrupt type - eint_rising_edge or eint_falling_edge
 */
void initEchoInt_center(eint_intr_t eintType);
void initEchoInt_right_(eint_intr_t eintType);
void initEchoInt_left(eint_intr_t eintType);

/*
 * Init the GPIO
 */
void init_io_center();
void init_io_right();
void init_io_left();

/* Task to send triggers to Left and Right Sensors
 *
 */
class sendTrigTask : public scheduler_task
{
    public :
        sendTrigTask();
        bool init(void);
        bool run(void *p);
};

/*
 * Task to receive the sensor left data from queue
 */
class RxLeft : public scheduler_task
{
    public :
        RxLeft();
        //bool init(void);
        bool run(void *p);
};

/*
 * Task to receive the sensor Right data from queue
 */
class RxRight : public scheduler_task
{
    public :
        RxRight();
        //bool init(void);
        bool run(void *p);
};

class readAdcData : public scheduler_task
{
    public :
        readAdcData();
        bool init(void);
        bool run(void *p);
};

class otherSensorsTask : public scheduler_task
{
    public :
        otherSensorsTask();
        bool init(void);
        bool run(void *p);
};


#endif /* ENABLE_SENSOR */

#endif /* PING_SENSOR_HPP_ */
