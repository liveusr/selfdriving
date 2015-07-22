/*
 * controller_bt_android.cpp
 *
 *  Created on: Nov 7, 2014
 *      Author: MANish
 */

#include <stdio.h>
#include "file_logger.h"
#include "io.hpp"
#include <string.h>

#include "controller_bt_android.hpp"
#include "bt_android_communication.hpp"

#ifdef ENABLE_BT_ANDROID // enabled from can_common.hpp

static can_msg_id_t filter_list[] = {
        {CONTROLLER_MASTER,         CONTROLLER_ALL,             MSG_RESET},
        {CONTROLLER_MASTER,         CONTROLLER_ALL,             MSG_POWERUP_SYN},
        {CONTROLLER_MASTER,         CONTROLLER_ALL,             MSG_POWERUP_ACK},
        {CONTROLLER_MASTER,         CONTROLLER_ALL,             MSG_HEARTBEAT},

        {CONTROLLER_SENSOR,         CONTROLLER_ALL,             MSG_DIST_SENSOR_DATA},
        {CONTROLLER_SENSOR,         CONTROLLER_ALL,             MSG_OTHER_SENSOR_DATA},

        {CONTROLLER_GEO,            CONTROLLER_ALL,             MSG_GEO_HEADING_DATA},
        {CONTROLLER_GEO,            CONTROLLER_ALL,             MSG_GEO_LOCATION_DATA},
        {CONTROLLER_GEO,            CONTROLLER_BT_ANDROID,      MSG_CHECKPOINT_REQUEST},

        {CONTROLLER_MOTOR,          CONTROLLER_ALL,             MSG_SPEED_ENCODER_DATA},

        {CONTROLLER_IO,             CONTROLLER_ALL,             MSG_CAR_RESUME},
        {CONTROLLER_IO,             CONTROLLER_ALL,             MSG_CAR_PAUSE},
        {CONTROLLER_IO,             CONTROLLER_ALL,             MSG_DRIVE_MODE},

        {CONTROLLER_MOTOR,          CONTROLLER_MASTER,          MSG_HEARTBEAT_ACK},
        {CONTROLLER_SENSOR,         CONTROLLER_MASTER,          MSG_HEARTBEAT_ACK},
        {CONTROLLER_GEO,            CONTROLLER_MASTER,          MSG_HEARTBEAT_ACK},
        {CONTROLLER_IO,             CONTROLLER_MASTER,          MSG_HEARTBEAT_ACK},

        {CONTROLLER_BT_ANDROID,     CONTROLLER_BT_ANDROID,      MSG_DUMMY},
};

can_controller controller(CONTROLLER_BT_ANDROID, filter_list, sizeof(filter_list) / sizeof(can_msg_id_t));
static QueueHandle_t msg_queue = xQueueCreate(MSG_QUEUE_LEN, sizeof(msg_t));

static uint8_t heartbeat_count;

static uint16_t total_seconds;
static uint16_t total_tx;

BluetoothSendData *bt; // initialize this in controllerInit();
bt_checkpoint_data_t checkpoint_list[20];
uint16_t total_checkpoints;
static uint8_t checkpoint_num = 0;

dist_sensor_data_t dist_sensor;
other_sensor_data_t other_sensor;
geo_heading_data_t geo_heading;
geo_location_data_t geo_location;
checkpoint_request_data_t checkpoint_request;
speed_encoder_data_t speed_encoder;
drive_mode_data_t drive_mode;
uint8_t car_paused = 1;

/* TODO: populate this variable to send periodic data */
checkpoint_data_t periodic_checkpoint_data = {83.19, 37.23, 187, 6, 1, 1};
uint8_t is_first_checkpoint;

class periodicCheckpointTask : public scheduler_task
{
    public:
        periodicCheckpointTask(uint8_t priority) : scheduler_task("periodicCheckpointTask", 1024, priority)
        {
            setRunDuration(811); // in milliseconds
        }

        bool run(void *p)
        {
            if(!geo_location.is_valid) {
                return true;
            }

            LE.on(4);

            if(!is_first_checkpoint) {
                /* NOTE: this task runs only to send 1st checkpoint */
                return true;
            }

            checkpoint_num = 0;
            periodic_checkpoint_data.latitude = checkpoint_list[checkpoint_num].latitude;
            periodic_checkpoint_data.longitude = checkpoint_list[checkpoint_num].longitude;
            periodic_checkpoint_data.is_new_route = 1;
            periodic_checkpoint_data.is_final_checkpoint = (total_checkpoints > 1)? 0: 1;
            periodic_checkpoint_data.checkpoint_num = checkpoint_num;
            //periodic_checkpoint_data.total_distance = 0; // TBD

            controller.can_send(CONTROLLER_GEO, MSG_CHECKPOINT_DATA, (uint8_t *) &periodic_checkpoint_data, sizeof(periodic_checkpoint_data));

            printf("sending checkpoint %d\n", checkpoint_num);

            return true;
        }
};

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
            uint8_t str[80] = {0};

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

//                    sprintf((char *)str, "lat=%.6lf,long=%.6lf$", 37.336860, -121.882008);
//                    bt->bt_send((unsigned char *)str, strlen((const char*) str));

                    vTaskDelay(500);
                    break;

                case 0x02:
                    /* switch 2 pressed */
//                    {
//                    int can = 1;
//                    uint8_t str[80] = {0};
//                    sprintf((char *)str, "can_util=20$status=3,1$", can);
//                    bt->bt_send((unsigned char *)str, strlen((const char*) str));
//                    }
                    if(car_paused) {
                        car_paused = 0;
                        controller.can_send(CONTROLLER_ALL, MSG_CAR_RESUME, NULL, 0);
                    }
                    else {
                        car_paused = 1;
                        controller.can_send(CONTROLLER_ALL, MSG_CAR_PAUSE, NULL, 0);
                    }
                    vTaskDelay(500);
                    break;

                case 0x04:
                    /* switch 3 pressed */
//                    {
//                        uint8_t str[80] = {0};
//                        int tx_count = 10;
//                        int rx_count = 20;
//                        sprintf((char *)str, "txcount=1,%d$rxcount=3,%d$", tx_count, rx_count);
//                        bt->bt_send((unsigned char *)str, strlen((const char*) str));
//
//                    }
                    /* eng to boccardo */
                    checkpoint_list[0].latitude = 37.334717;
                    checkpoint_list[0].longitude = -121.880837;

                    checkpoint_list[1].latitude = 37.334595;
                    checkpoint_list[1].longitude = -121.880620;

                    checkpoint_list[2].latitude = 37.335501;
                    checkpoint_list[2].longitude = -121.878765;

                    total_checkpoints = 3;

//                    checkpoint_list[0].latitude = 37.334717;
//                    checkpoint_list[0].longitude = -121.880837;
//                    total_checkpoints = 1; //event center

                    is_first_checkpoint = 1;

                    vTaskDelay(500);
                    break;

                case 0x08:
                    /* switch 4 pressed */
//                    {
//                        uint8_t str[80] = {0};
//                        float lat = 37.33668892;
//                        float longi = -121.8794009;
//                        sprintf((char *)str, "lat=%f,long=%f$", lat, longi);
//                        bt->bt_send((unsigned char *)str, strlen((const char*) str));
//                    }
                    /* boccardo to eng */
//                    checkpoint_list[0].latitude = 37.334595;
//                    checkpoint_list[0].longitude = -121.880620;
//
//                    checkpoint_list[1].latitude = 37.334717;
//                    checkpoint_list[1].longitude = -121.880837;
//
//                    checkpoint_list[2].latitude = 37.335964;
//                    checkpoint_list[2].longitude = -121.881790;

//                    checkpoint_list[0].latitude = 37.336558;
//                    checkpoint_list[0].longitude = -121.881929;
//
//                    checkpoint_list[1].latitude = 37.337207;
//                    checkpoint_list[1].longitude = -121.882411;
//
//                    checkpoint_list[2].latitude = 37.337543;
//                    checkpoint_list[2].longitude = -121.881641;

                    checkpoint_list[0].latitude = 37.335964;
                    checkpoint_list[0].longitude = -121.881790;
                    total_checkpoints = 1; //eng bldg
//                    total_checkpoints = 3;
                    is_first_checkpoint = 1;

                    vTaskDelay(500);
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
    scheduler_add_task(new periodicCheckpointTask(PRIORITY_HIGH));
    scheduler_add_task(new BluetoothReceiveData(PRIORITY_HIGH));
    bt = new BluetoothSendData();

    /* setup debug variables */
    scheduler_task *dt = new debugTask(PRIORITY_HIGH);
    scheduler_add_task(dt);
    ((debugTask *)dt)->setInt(&geo_location.dist_to_next_checkpoint, 0);
    ((debugTask *)dt)->setInt(&geo_heading.destination_reached, 1);
    ((debugTask *)dt)->setLeftRight(&checkpoint_num, &total_checkpoints, 2);
    ((debugTask *)dt)->setInt(&heartbeat_count, 4);
    ((debugTask *)dt)->setHex(&error_flag, 5);
}

canRxBufferTask :: canRxBufferTask(uint8_t priority) : scheduler_task("CRB_canRxBufferTask", 1024, priority)
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

canRxProcessTask :: canRxProcessTask(uint8_t priority) : scheduler_task("CRP_canRxProcessTask", 2048, priority)
{
    /* Nothing to init */
}

bool canRxProcessTask :: run(void *p)
{
    msg_t msg;

    if(xQueueReceive(msg_queue, &msg, portMAX_DELAY) == pdFALSE) {
        return true;
    }

    if((error_flag & ERROR_MSG_QUEUE_FULL) == ERROR_MSG_QUEUE_FULL) {
        RESET_ERROR(ERROR_MSG_QUEUE_FULL);
    }

    switch(msg.msg_num) {
        case MSG_HEARTBEAT:
            handle_heartbeat(msg);
            break;

        case MSG_HEARTBEAT_ACK:
            handle_heartbeat_ack(msg);
            break;

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

        case MSG_CHECKPOINT_REQUEST:
            handle_checkpoint_request(msg);
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

        default:
            /* do nothing */
            break;
    }

    return true;
}

bool canRxProcessTask :: handle_heartbeat(msg_t msg)
{
    heartbeat_data_t data;
    int can_kbps, can_util;
    uint8_t str[80] = {0};

    data.rx_count = controller.get_rx_count();
    data.rx_bytes = controller.get_rx_bytes();
    data.tx_count = controller.get_tx_count();
    data.tx_bytes = controller.get_tx_bytes();

    vTaskDelay(CONTROLLER_BT_ANDROID * 10);
    controller.can_send(CONTROLLER_MASTER, MSG_HEARTBEAT_ACK, (uint8_t *) &data, sizeof(data));
    heartbeat_count = (heartbeat_count + 1) % 100;

    total_seconds++;
    total_tx += data.tx_bytes;
    total_tx += controller.get_tx_bytes();

    can_kbps = (total_tx * 8) / total_seconds;
    can_util = (((total_tx * 8) / total_seconds) / 1024);
    sprintf((char *)str, "can_util=%d,%d$", can_kbps, can_util);
    bt->bt_send((unsigned char *)str, strlen((const char*) str));

    sprintf((char *)str, "txcount=%d,%d$rxcount=%d,%d$status=%d,%d$", msg.src, data.tx_count, msg.src, data.rx_count, msg.src, 1);
    bt->bt_send((unsigned char *)str, strlen((const char*) str));

    sprintf((char *)str, "txcount=%d,%d$rxcount=%d,%d$status=%d,%d$", CONTROLLER_BT_ANDROID, controller.get_tx_count(), CONTROLLER_BT_ANDROID, controller.get_rx_count(), CONTROLLER_BT_ANDROID, 1);
    bt->bt_send((unsigned char *)str, strlen((const char*) str));

    return true;
}

bool canRxProcessTask :: handle_heartbeat_ack(msg_t msg)
{
    heartbeat_ack_data_t data;
    uint8_t str[80] = {0};

    data = msg.heartbeat_ack_data;
    total_tx += data.tx_bytes;

    sprintf((char *)str, "txcount=%d,%d$rxcount=%d,%d$status=%d,%d$", msg.src, data.tx_count, msg.src, data.rx_count, msg.src, 1);
    bt->bt_send((unsigned char *)str, strlen((const char*) str));

    return true;
}

bool canRxProcessTask :: handle_dist_sensor_data(msg_t msg)
{
    dist_sensor = msg.dist_sensor_data;
    uint8_t str[80] = {0};

    sprintf((char *)str, "s1=%d,%d,%d,%d$", dist_sensor.left, dist_sensor.middle, dist_sensor.right, dist_sensor.back);
    bt->bt_send((unsigned char *)str, strlen((const char*) str));

    return true;
}

bool canRxProcessTask :: handle_other_sensor_data(msg_t msg)
{
    other_sensor = msg.other_sensor_data;
    uint8_t str[80] = {0};

    sprintf((char *)str, "bat=%d$light=%d$", other_sensor.battery, other_sensor.light);
    bt->bt_send((unsigned char *)str, strlen((const char*) str));

    return true;
}

bool canRxProcessTask :: handle_geo_heading_data(msg_t msg)
{
    geo_heading = msg.geo_heading_data;
    uint8_t str[80] = {0};

    sprintf((char *)str, "heading=%d$", geo_heading.current_angle);
    bt->bt_send((unsigned char *)str, strlen((const char*) str));

    return true;
}

bool canRxProcessTask :: handle_geo_location_data(msg_t msg)
{
    geo_location = msg.geo_location_data;
    uint8_t str[80] = {0};

    sprintf((char *)str, "lat=%.6lf,long=%.6lf$", geo_location.latitude, geo_location.longitude);
    bt->bt_send((unsigned char *)str, strlen((const char*) str));
    printf("%s\n", str);

    sprintf((char *)str, "dist=%d,%d$", geo_location.dist_to_next_checkpoint, geo_location.dist_to_final_destination);
    bt->bt_send((unsigned char *)str, strlen((const char*) str));
    printf("%s\n", str);

    return true;
}

bool canRxProcessTask :: handle_checkpoint_request(msg_t msg)
{
    checkpoint_request = msg.checkpoint_request_data;

    is_first_checkpoint = 0;
    checkpoint_num = checkpoint_request.checkpoint_num;
    printf("received checkpoint request for checkpoint_num %d\n", checkpoint_num);

    periodic_checkpoint_data.latitude = checkpoint_list[checkpoint_num].latitude;
    periodic_checkpoint_data.longitude = checkpoint_list[checkpoint_num].longitude;
    periodic_checkpoint_data.is_new_route = 0;
    periodic_checkpoint_data.is_final_checkpoint = (checkpoint_num + 1 >= total_checkpoints)? 1: 0;
    periodic_checkpoint_data.checkpoint_num = checkpoint_num;
    //periodic_checkpoint_data.total_distance = 0; // TBD

    controller.can_send(CONTROLLER_GEO, MSG_CHECKPOINT_DATA, (uint8_t *) &periodic_checkpoint_data, sizeof(periodic_checkpoint_data));

    printf("sending new checkpoint %d\n", checkpoint_num);

    return true;
}

bool canRxProcessTask :: handle_speed_encoder_data(msg_t msg)
{
    speed_encoder = msg.speed_encoder_data;
    uint8_t str[80] = {0};

    sprintf((char *)str, "speed=%d$", speed_encoder.speed);
    bt->bt_send((unsigned char *)str, strlen((const char*) str));

    return true;
}

bool canRxProcessTask :: handle_car_resume(msg_t msg)
{
    uint8_t str[80] = "stop$";

    bt->bt_send((unsigned char *)str, strlen((const char*) str));

    return true;
}

bool canRxProcessTask :: handle_car_pause(msg_t msg)
{
    uint8_t str[80] = "start$";

    bt->bt_send((unsigned char *)str, strlen((const char*) str));

    return true;
}

bool canRxProcessTask :: handle_drive_mode(msg_t msg)
{
    drive_mode = msg.drive_mode_data;
    uint8_t str[80] = {0};

    sprintf((char *)str, "mode=%d$", drive_mode.mode);
    bt->bt_send((unsigned char *)str, strlen((const char*) str));

    return true;
}

#endif /* ENABLE_BT_ANDROID */
