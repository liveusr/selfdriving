/*
 * controller_bt_android.hpp
 *
 *  Created on: Nov 7, 2014
 *      Author: MANish
 */

#ifndef CONTROLLER_BT_ANDROID_HPP_
#define CONTROLLER_BT_ANDROID_HPP_

#include "can_common.hpp"
#include "scheduler_task.hpp"

#ifdef ENABLE_BT_ANDROID // enabled from can_common.hpp

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
        bool handle_heartbeat_ack(msg_t msg);
        bool handle_dist_sensor_data(msg_t msg);
        bool handle_other_sensor_data(msg_t msg);
        bool handle_geo_heading_data(msg_t msg);
        bool handle_geo_location_data(msg_t msg);
        bool handle_checkpoint_request(msg_t msg);
        bool handle_speed_encoder_data(msg_t msg);
        bool handle_car_resume(msg_t msg);
        bool handle_car_pause(msg_t msg);
        bool handle_drive_mode(msg_t msg);
};

#endif /* ENABLE_BT_ANDROID */

#endif /* CONTROLLER_BT_ANDROID_HPP_ */
