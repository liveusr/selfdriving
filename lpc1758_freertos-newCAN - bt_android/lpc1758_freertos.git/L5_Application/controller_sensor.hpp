/*
 * controller_sensor.hpp
 *
 *  Created on: Nov 7, 2014
 *      Author: MANish
 */

#ifndef CONTROLLER_SENSOR_HPP_
#define CONTROLLER_SENSOR_HPP_

#include "can_common.hpp"
#include "scheduler_task.hpp"

#ifdef ENABLE_SENSOR // enabled from can_common.hpp

#define CONTROLLER_VERSION  0x21
#define MSG_QUEUE_LEN       32

void controllerInit(void);

class canRxBufferTask : public scheduler_task
{
    public:
        canRxBufferTask(uint8_t priority);
        bool run(void *p);
        bool handle_heartbeat(msg_t msg);
};

class canRxProcessTask : public scheduler_task
{
    public:
        canRxProcessTask(uint8_t priority);
        bool run(void *p);
        bool handle_heartbeat(msg_t msg);
};

#endif /* ENABLE_SENSOR */

#endif /* CONTROLLER_SENSOR_HPP_ */
