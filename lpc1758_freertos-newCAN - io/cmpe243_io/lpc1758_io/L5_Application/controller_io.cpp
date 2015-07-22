/*
 * controller_io.cpp
 *
 *  Created on: Nov 7, 2014
 *      Author: MANish
 */

#include <stdio.h>
#include "file_logger.h"
#include "io.hpp"
#include "lpc_sys.h"
#include "string.h"

#include "controller_io.hpp"
//
#include "iofunc.hpp"
//

static can_msg_id_t filter_list[] = {
        {CONTROLLER_MASTER,         CONTROLLER_ALL,     MSG_RESET},
        {CONTROLLER_MASTER,         CONTROLLER_ALL,     MSG_POWERUP_SYN},
        {CONTROLLER_MASTER,         CONTROLLER_ALL,     MSG_POWERUP_ACK},
        {CONTROLLER_MASTER,         CONTROLLER_ALL,     MSG_HEARTBEAT},
        {CONTROLLER_SENSOR,         CONTROLLER_ALL,     MSG_DIST_SENSOR_DATA},
        {CONTROLLER_SENSOR,         CONTROLLER_ALL,     MSG_OTHER_SENSOR_DATA},
        {CONTROLLER_GEO,            CONTROLLER_ALL,         MSG_GEO_HEADING_DATA},
        {CONTROLLER_GEO,            CONTROLLER_ALL,         MSG_GEO_LOCATION_DATA},
        
        {CONTROLLER_MOTOR,          CONTROLLER_ALL,     MSG_SPEED_ENCODER_DATA},
		{CONTROLLER_BT_ANDROID,     CONTROLLER_GEO,     MSG_CHECKPOINT_DATA},
        {CONTROLLER_BT_ANDROID,     CONTROLLER_ALL,     MSG_CAR_RESUME},
        {CONTROLLER_BT_ANDROID,     CONTROLLER_ALL,     MSG_CAR_PAUSE},
        {CONTROLLER_BT_ANDROID,     CONTROLLER_ALL,     MSG_DRIVE_MODE},
        //{CONTROLLER_MASTER,         CONTROLLER_MOTOR,   MSG_SPEED_DIR_COMMAND},

        {CONTROLLER_IO,             CONTROLLER_IO,      MSG_DUMMY}, //TO MAKE NUMBER OF MESSAGES EVEN

       /* //Hooking other's version number
       {CONTROLLER_MOTOR,          CONTROLLER_MASTER,     MSG_POWERUP_SYN_ACK},
        {CONTROLLER_SENSOR,         CONTROLLER_MASTER,     MSG_POWERUP_SYN_ACK},
        {CONTROLLER_GEO,            CONTROLLER_MASTER,     MSG_POWERUP_SYN_ACK},
        {CONTROLLER_BT_ANDROID,     CONTROLLER_MASTER,     MSG_POWERUP_SYN_ACK},*/

        //Hooking other's CAN Rx-Tx count
        {CONTROLLER_MOTOR,          CONTROLLER_MASTER,     MSG_HEARTBEAT_ACK},
        {CONTROLLER_SENSOR,         CONTROLLER_MASTER,     MSG_HEARTBEAT_ACK},
        {CONTROLLER_GEO,            CONTROLLER_MASTER,     MSG_HEARTBEAT_ACK},
        {CONTROLLER_BT_ANDROID,     CONTROLLER_MASTER,     MSG_HEARTBEAT_ACK},

};

can_controller controller(CONTROLLER_IO, filter_list, sizeof(filter_list) / sizeof(can_msg_id_t));
static QueueHandle_t msg_queue = xQueueCreate(MSG_QUEUE_LEN, sizeof(msg_t));
static uint8_t heartbeat_count;

/* TODO: populate this variable to send periodic data */
//dummy_data_t periodic_data;
//
//class periodicTask : public scheduler_task
//{
//    public:
//        periodicTask(uint8_t priority) : scheduler_task("periodicTask", 1024, priority)
//        {
//            periodic_data.c = 0; // ON/OFF
//            periodic_data.i = 1; // LED1
//
//            setRunDuration(500); // in milliseconds
//        }
//
//        bool run(void *p)
//        {
//            periodic_data.c = periodic_data.c + 1;
//            controller.can_send(CONTROLLER_MASTER, MSG_DUMMY, (uint8_t *) &periodic_data, sizeof(periodic_data));
//            printf("%d %d [%d]\n", periodic_data.i, periodic_data.c, sizeof(periodic_data));
//
//            return true;
//        }
//};

class debugTask : public scheduler_task
{
    private:
        static const uint8_t MAX_DEBUG_DATA = 6;
        uint8_t curr_index;

    public:
        struct {
                uint8_t *left_val;
                uint8_t *right_val;
                uint8_t *int_val;
                uint8_t *hex_val;
        } debug_data[MAX_DEBUG_DATA];

        debugTask(uint8_t priority) : scheduler_task("debugTask", 1024, priority)
        {
            int8_t i = 0;
            for(i = 0; i < MAX_DEBUG_DATA; i++) {
                debug_data[i].left_val = NULL;
                debug_data[i].right_val = NULL;
                debug_data[i].int_val = NULL;
                debug_data[i].hex_val = NULL;
            }

            curr_index = 0;

            setRunDuration(100); // in milliseconds
        }

        bool run(void *p)
        {
            if(curr_index == MAX_DEBUG_DATA) {
                LD.setLeftDigit('-');
                LD.setRightDigit('-');
            }
            else {
                if(debug_data[curr_index].hex_val) {
                    uint8_t c;
                    c = (*debug_data[curr_index].hex_val >> 4) & 0x0F;
                    LD.setLeftDigit((c < 10)? c: (c + 20));
                    c = *debug_data[curr_index].hex_val & 0x0F;
                    LD.setRightDigit((c < 10)? c: (c + 20));
                }
                else if(debug_data[curr_index].int_val) {
                    LD.setNumber(*debug_data[curr_index].int_val);
                }
                else if(debug_data[curr_index].left_val && debug_data[curr_index].right_val) {
                    LD.setLeftDigit(*debug_data[curr_index].left_val);
                    LD.setRightDigit(*debug_data[curr_index].right_val);
                }
                else {
                    curr_index = ((curr_index + 1) % (MAX_DEBUG_DATA + 1));
                }
            }

            switch(SW.getSwitchValues() & 0x0F) {
                case 0x01:
                    /* switch 1 pressed */
                    curr_index = ((curr_index + 1) % (MAX_DEBUG_DATA + 1));
                    vTaskDelay(500);
                    break;

                case 0x02:
                    /* switch 2 pressed */
                    break;

                case 0x04:
                    /* switch 3 pressed */
                    break;

                case 0x08:
                    /* switch 4 pressed */
                    break;
            }

            return true;
        }

        void setHex(void *val, uint8_t index)
        {
            debug_data[index].hex_val = (uint8_t *) val;
        }

        void setInt(void *val, uint8_t index)
        {
            debug_data[index].int_val = (uint8_t *) val;
        }

        void setLeftRight(void *valLeft, void *valRight, uint8_t index)
        {
            debug_data[index].left_val = (uint8_t *) valLeft;
            debug_data[index].right_val = (uint8_t *) valRight;
        }
};

void controllerInit(void)
{
    /* clear error flag */
    SET_ERROR(ERROR_NO_ERROR);

    controller.processBootSequence(CONTROLLER_VERSION);

    /* TODO: start any other tasks of module here, like periodic task */
    //scheduler_add_task(new periodicTask(PRIORITY_LOW));

	scheduler_add_task(new GLCD(PRIORITY_MEDIUM));
    scheduler_add_task(new LIGHT(PRIORITY_MEDIUM));
    scheduler_add_task(new SWITCH(PRIORITY_MEDIUM));

    /* setup debug variables */
    scheduler_task *dt = new debugTask(PRIORITY_MEDIUM);
    scheduler_add_task(dt);
    //((debugTask *)dt)->setLeftRight(&geo_heading.destination_reached, &geo_heading.is_valid, 0);
    ((debugTask *)dt)->setLeftRight(&io_geo_head.destination_reached, &io_geo_head.is_valid, 0);
    ((debugTask *)dt)->setInt(&heartbeat_count, 4);
    ((debugTask *)dt)->setHex(&error_flag, 5);
}

canRxBufferTask :: canRxBufferTask(uint8_t priority) : scheduler_task("canRxBufferTask", 1024, priority)
{
    /* Nothing to init */
}

bool canRxBufferTask :: run(void *p)
{
    msg_t msg;

    msg.len = sizeof(msg.data);
    controller.can_recv(&msg.src, &msg.msg_num, msg.data, &msg.len);

    if(msg.msg_num == MSG_RESET) {
        controller.processReset();
    }

    if(xQueueSend(msg_queue, &msg, 0) == errQUEUE_FULL) {
        SET_ERROR(ERROR_MSG_QUEUE_FULL);
        LOG_ERROR("msg_queue full\n");
        printf("msg_queue full\n");
    }

    return true;
}

canRxProcessTask :: canRxProcessTask(uint8_t priority) : scheduler_task("canRxProcessTask", 1024, priority)//2048
{
    /* Nothing to init */
}

bool canRxProcessTask :: run(void *p)
{
    msg_t msg;
    //printf("\nMessage no.:%s", message_id_str[msg.msg_num]);
    if(xQueueReceive(msg_queue, &msg, portMAX_DELAY) == pdFALSE) {
        return true;
    }

    if((error_flag & ERROR_MSG_QUEUE_FULL) == ERROR_MSG_QUEUE_FULL) {
        RESET_ERROR(ERROR_MSG_QUEUE_FULL);
    }
    //printf("\n%d",msg.msg_num);
    switch(msg.msg_num) {
        case MSG_HEARTBEAT:
            handle_heartbeat(msg);
			break;
			
		case MSG_HEARTBEAT_ACK:
            handle_heartbeat_ack(msg);
            break;
			
		/*case MSG_POWERUP_SYN_ACK:
            handle_powerup_syn_ack(msg);
            break;*/

        case MSG_DIST_SENSOR_DATA:
            handle_dist_sensor_data(msg);
            break;

        case MSG_OTHER_SENSOR_DATA:
            handle_other_sensor_data(msg);
            break;

        case MSG_GEO_HEADING_DATA:
            handle_geo_heading_data(msg);
            break;

        case MSG_GEO_LOCATION_DATA:
            handle_geo_location_data(msg);
            break;

        case MSG_SPEED_ENCODER_DATA:
            handle_speed_encoder_data(msg);
            break;

        case MSG_CAR_RESUME:
            handle_car_resume(msg);
            break;

        case MSG_CAR_PAUSE:
            handle_car_pause(msg);
            break;

        case MSG_DRIVE_MODE:
            handle_drive_mode(msg);
            break;

        case MSG_CHECKPOINT_DATA:
            handle_checkpoint_data(msg);
            break;

        default:
            /* do nothing */
            break;
    }

    return true;
}

bool canRxProcessTask :: handle_heartbeat(msg_t msg)
{
    heartbeat_data_t data;

    data.rx_count = controller.get_rx_count();
    data.rx_bytes = controller.get_rx_bytes();
    data.tx_count = controller.get_tx_count();
    data.tx_bytes = controller.get_tx_bytes();

    vTaskDelay(CONTROLLER_IO * 10);
    controller.can_send(CONTROLLER_MASTER, MSG_HEARTBEAT_ACK, (uint8_t *) &data, sizeof(data));
    heartbeat_count = (heartbeat_count + 1) % 100;

    //////
    memcpy(&mas_can, msg.data, sizeof(heartbeat_ack_data_t));
                    stat_time.master=0;
                    data_rcv.status==true;
    /////
    return true;
}


bool canRxProcessTask :: handle_dist_sensor_data(msg_t msg)
{
    //dist_sensor_data_t data;
    data_rcv.sensors=true;
    memcpy(&io_snr_data, msg.data, sizeof(dist_sensor_data_t));

    /* TODO: display this data on screen */

    return true;
}

bool canRxProcessTask :: handle_other_sensor_data(msg_t msg)
{
    //other_sensor_data_t data;
    data_rcv.other_sensors=true;
    memcpy(&io_other_snr_data, msg.data, sizeof(other_sensor_data_t));
    //printf("Battery = %d", io_other_snr_data.battery);
    //printf("Light = %d", io_other_snr_data.light);
    //io_other_snr_data.light=20;
    /* TODO: display this data on screen */

    return true;
}

bool canRxProcessTask :: handle_geo_heading_data(msg_t msg)
{
    data_rcv.geo_head_data=true;
    memcpy(&io_geo_head, msg.data, sizeof(geo_heading_data_t));

    /* TODO: display this data on screen */

    return true;
}

bool canRxProcessTask :: handle_geo_location_data(msg_t msg)
{
    data_rcv.geo_loc_data=true;
    memcpy(&io_geo_loc, msg.data, sizeof(geo_location_data_t));
    //geo_location = msg.geo_location_data;

    /* TODO: display this data on screen */
    //printf("V %2d CA %2d DA %2d LT %f LG %f FD %3d CD %3d\n", data.is_valid, data.current_angle, data.desired_angle, data.latitude, data.longitude, data.dist_to_next_checkpoint, data.dist_to_final_destination);

    return true;
}
bool canRxProcessTask :: handle_checkpoint_data(msg_t msg)
{
    data_rcv.chkpt_data=true;
    memcpy(&io_checkpoint_data, msg.data, sizeof(checkpoint_data_t));
    printf("\nGot checkpoint data");
    printf("\nNew route: %d",io_checkpoint_data.is_new_route);
    /* TODO: display this data on screen */

    return true;
}

bool canRxProcessTask :: handle_speed_encoder_data(msg_t msg)
{
    data_rcv.speed=true;
    data_rcv.speed_dir_data=true;
    memcpy(&io_speed, msg.data, sizeof(speed_encoder_data_t));
    //printf("\nSpeed = %d",io_speed.speed);
    //printf("\nDir = %d",io_speed.dir);
    //printf("\nBrake = %d",io_speed.brake);
    return true;
}

/*bool canRxProcessTask :: handle_speed_dir_cmd(msg_t msg)
{
    data_rcv.speed_dir_data=true;
    memcpy(&io_speed_dir_data, msg.data, sizeof(speed_dir_data_t));
    return true;
}*/

bool canRxProcessTask :: handle_car_resume(msg_t msg)
{
    ctrl_changed=true;
    resume_pause=true;
    return true;
}

bool canRxProcessTask :: handle_car_pause(msg_t msg)
{
    ctrl_changed=true;
    resume_pause=false;
    return true;
}

bool canRxProcessTask :: handle_drive_mode(msg_t msg)
{
    //drive_mode_data_t data;
    data_rcv.drive_mode=true;
    memcpy(&io_drive_mode, msg.data, sizeof(drive_mode_data_t));
    return true;
}

/*bool canRxProcessTask::handle_powerup_syn_ack(msg_t msg)
{
    data_rcv.versions=true;
    switch(msg.src)
    {
        case CONTROLLER_MOTOR:
            memcpy(&mot_ver, msg.data, sizeof(powerup_syn_ack_data_t));
            break;
        case CONTROLLER_SENSOR:
            memcpy(&sen_ver, msg.data, sizeof(powerup_syn_ack_data_t));
            break;
        case CONTROLLER_GEO:
            memcpy(&geo_ver, msg.data, sizeof(powerup_syn_ack_data_t));
            break;
        case CONTROLLER_BT_ANDROID:
            memcpy(&bt_ver, msg.data, sizeof(powerup_syn_ack_data_t));
            break;
        default:
            break;
    }

}*/

bool canRxProcessTask::handle_heartbeat_ack(msg_t msg)
{
    switch(msg.src)
    {
        case CONTROLLER_MOTOR:
            {
                memcpy(&mot_can, msg.data, sizeof(heartbeat_ack_data_t));
                stat_time.motor=0;
            }
            break;
        case CONTROLLER_SENSOR:
            {
                memcpy(&sen_can, msg.data, sizeof(heartbeat_ack_data_t));
                stat_time.sensor=0;
            }
            break;
        case CONTROLLER_GEO:
            {
                memcpy(&geo_can, msg.data, sizeof(heartbeat_ack_data_t));
                stat_time.geo=0;
            }
            break;
        case CONTROLLER_BT_ANDROID:
            {
                memcpy(&bt_can, msg.data, sizeof(heartbeat_ack_data_t));
                stat_time.bt=0;
            }
            break;
        default:
            {
                //ERROR
            }
            break;
    }
    stat_time.io=0;
    data_rcv.status=true;
}




