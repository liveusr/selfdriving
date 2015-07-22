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

#define base    0
#define zone1   3
#define zone2   7
#define zone3   10
#define zone4   12

#define back_sensor_threshold   2

typedef struct {
        uint8_t  leftSensor;
        uint8_t  middleSensor;
        uint8_t  rightSensor;
        uint8_t  backSensor;
} sensor_zone_t;                //this struct stores the zones where the obstacles lie

typedef enum {
        STOP,
        REVERSE,
        FULL_LEFT,
        FULL_RIGHT,
        MID_LEFT,
        MID_RIGHT,
        SLIGHT_RIGHT,
        SLIGHT_LEFT,
        STRAIGHT
} obstacle_state_t;


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
        bool handle_geo_data(msg_t msg);
        bool handle_speed_encoder_data(msg_t msg);
        bool handle_car_pause_data(msg_t msg);
        bool handle_car_resume_data(msg_t msg);
        bool handle_drive_mode_data(msg_t msg);
};

#endif /* ENABLE_MASTER */

#endif /* CONTROLLER_MASTER_HPP_ */
