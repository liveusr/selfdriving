/*
 * controller_dummy.hpp
 *
 *  Created on: Nov 6, 2014
 *      Author: MANish
 */

#ifndef CONTROLLER_DUMMY_HPP_
#define CONTROLLER_DUMMY_HPP_

#include "can_common.hpp"
#include "scheduler_task.hpp"

#define CONTROLLER_VERSION  0x21
#define MSG_QUEUE_LEN       32

void controllerInit(void);

class canRxBufferTask : public scheduler_task
{
    public:
        canRxBufferTask(uint8_t priority);
        bool run(void *p);
};

class canRxProcessTask : public scheduler_task
{
    public:
        canRxProcessTask(uint8_t priority);
        bool run(void *p);
        bool handle_reset(msg_t msg);
        bool handle_dummy(msg_t msg);
};

#endif /* CONTROLLER_DUMMY_HPP_ */
