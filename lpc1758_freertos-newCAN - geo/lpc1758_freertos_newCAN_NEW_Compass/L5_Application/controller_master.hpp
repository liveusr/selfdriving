/*
 * controller_master.hpp
 *
 *  Created on: Nov 6, 2014
 *      Author: MANish
 */

#ifndef CONTROLLER_MASTER_HPP_
#define CONTROLLER_MASTER_HPP_

#include "can_common.hpp"
#include "scheduler_task.hpp"

#ifdef ENABLE_MASTER // enabled from can_common.hpp

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
        bool handle_heartbeat_ack(msg_t msg);
        bool handle_dist_sensor_data(msg_t msg);
        bool handle_geo_heading_data(msg_t msg);
        bool handle_speed_encoder_data(msg_t msg);
        bool handle_car_pause_data(msg_t msg);
        bool handle_car_resume_data(msg_t msg);
        bool handle_drive_mode_data(msg_t msg);
};

#endif /* ENABLE_MASTER */

#endif /* CONTROLLER_MASTER_HPP_ */
