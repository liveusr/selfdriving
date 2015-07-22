/*
 * controller_io.hpp
 *
 *  Created on: Nov 7, 2014
 *      Author: MANish
 */

#ifndef CONTROLLER_IO_HPP_
#define CONTROLLER_IO_HPP_

#include "can_common.hpp"
#include "scheduler_task.hpp"

#ifdef ENABLE_IO // enabled from can_common.hpp
#define CONTROLLER_VERSION  0x14
#define MSG_QUEUE_LEN       32

//
extern can_controller controller;
//
void controllerInit(void);

class canRxBufferTask : public scheduler_task
{
    public:
        canRxBufferTask(uint8_t priority);
        bool run(void *p);
        //bool handle_heartbeat(msg_t msg);
};

class canRxProcessTask : public scheduler_task
{
    public:
        canRxProcessTask(uint8_t priority);
        bool run(void *p);
        //bool handle_reset(msg_t msg);
        bool handle_heartbeat(msg_t msg);
		bool handle_heartbeat_ack(msg_t msg);
        bool handle_dist_sensor_data(msg_t msg);
        bool handle_other_sensor_data(msg_t msg);
        bool handle_geo_heading_data(msg_t msg);
        bool handle_geo_location_data(msg_t msg);
        bool handle_checkpoint_data(msg_t msg);
        bool handle_speed_encoder_data(msg_t msg);
        bool handle_car_resume(msg_t msg);
        bool handle_car_pause(msg_t msg);
        bool handle_drive_mode(msg_t msg);
        //bool handle_powerup_syn_ack(msg_t msg);
        
};
///////////////////////////////
typedef enum {
    OFF                 = 0x01,         //
    ON                  = 0x02,          //
    AUTO                = 0x03          //
} hl_mode_t;

typedef struct{
   uint8_t motor=0;
   uint8_t master=0;
   uint8_t sensor=0;
   uint8_t geo=0;
   uint8_t bt=0;
   uint8_t io=0;

}module_stat_time_t;

typedef struct{
   //
   bool versions =false;
   bool status=false;
   bool sensors=false;
   bool other_sensors=false;
   bool geo_head_data=false;
   bool geo_loc_data=false;
   bool chkpt_data=false;
   bool speed=false;
   bool speed_dir_data=false;
   bool headlights=false;
   bool drive_mode=false;
   //bool io=0;

}data_rcv_t;
///////////////////////////////

#endif /* ENABLE_IO */
#endif /* CONTROLLER_IO_HPP_ */
