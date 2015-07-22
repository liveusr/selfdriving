/*
 * can_common.cpp
 *
 *  Created on: Oct 23, 2014
 *      Author: Harita
 */

#include <string.h>
#include <stdio.h>
#include "io.hpp"
#include "file_logger.h"
#include "lpc_sys.h"
#include "utilities.h"

#include "can_common.hpp"

uint8_t error_flag;

/* convert enums to strings, this can be used to print debug logs */
char controller_id_str[8][32];
char message_id_str[32][32];

void can_controller :: enum_to_string(void)
{
    strcpy(controller_id_str[CONTROLLER_ALL],           "CONTROLLER_ALL");
    strcpy(controller_id_str[CONTROLLER_MOTOR],         "CONTROLLER_MOTOR");
    strcpy(controller_id_str[CONTROLLER_MASTER],        "CONTROLLER_MASTER");
    strcpy(controller_id_str[CONTROLLER_SENSOR],        "CONTROLLER_SENSOR");
    strcpy(controller_id_str[CONTROLLER_GEO],           "CONTROLLER_GEO");
    strcpy(controller_id_str[CONTROLLER_BT_ANDROID],    "CONTROLLER_BT_ANDROID");
    strcpy(controller_id_str[CONTROLLER_IO],            "CONTROLLER_IO");
    strcpy(controller_id_str[CONTROLLER_DUMMY],         "CONTROLLER_DUMMY");

    strcpy(message_id_str[MSG_RESET],                   "MSG_RESET");
    strcpy(message_id_str[MSG_POWERUP_SYN],             "MSG_POWERUP_SYN");
    strcpy(message_id_str[MSG_POWERUP_SYN_ACK],         "MSG_POWERUP_SYN_ACK");
    strcpy(message_id_str[MSG_POWERUP_ACK],             "MSG_POWERUP_ACK");
    strcpy(message_id_str[MSG_HEARTBEAT],               "MSG_HEARTBEAT");
    strcpy(message_id_str[MSG_HEARTBEAT_ACK],           "MSG_HEARTBEAT_ACK");
    strcpy(message_id_str[MSG_DIST_SENSOR_DATA],        "MSG_DIST_SENSOR_DATA");
    strcpy(message_id_str[MSG_OTHER_SENSOR_DATA],       "MSG_OTHER_SENSOR_DATA");
    strcpy(message_id_str[MSG_GEO_DATA],                "MSG_GEO_DATA");
    strcpy(message_id_str[MSG_SPEED_ENCODER_DATA],      "MSG_SPEED_ENCODER_DATA");
    strcpy(message_id_str[MSG_CAR_PAUSE],               "MSG_CAR_PAUSE");
    strcpy(message_id_str[MSG_CAR_RESUME],              "MSG_CAR_RESUME");
    strcpy(message_id_str[MSG_CHECKPOINT_REQUEST],      "MSG_CHECKPOINT_REQUEST");
    strcpy(message_id_str[MSG_CHECKPOINT_DATA],         "MSG_CHECKPOINT_DATA");
    strcpy(message_id_str[MSG_DRIVE_MODE],              "MSG_DRIVE_MODE");
    strcpy(message_id_str[MSG_SPEED_DIR_COMMAND],       "MSG_SPEED_DIR_COMMAND");
    strcpy(message_id_str[MSG_FREE_RUN_DIR],            "MSG_FREE_RUN_DIR");
    strcpy(message_id_str[MSG_ERROR],                   "MSG_ERROR");
    strcpy(message_id_str[MSG_DUMMY],                   "MSG_DUMMY");
}

bool can_controller :: can_setup(can_msg_id_t *filter_list, size_t len)
{
    can_std_id_t flist[len];

    //CAN_init(can1, 100, 100, 100, can_busoff_cb, can_bufovr_cb);
    CAN_init(can1, 100, 100, 100, 0, 0);

    if(filter_list) {
        /* sort the filter list */
        for(uint16_t j = 0; j < len; j++) {
            for(uint16_t i = 0; i < (len - 1); i++) {
                if(filter_list[i + 1].raw < filter_list[i].raw) {
                    int temp;
                    temp                    = filter_list[i].raw;
                    filter_list[i].raw      = filter_list[i + 1].raw;
                    filter_list[i + 1].raw  = temp;
                }
            }
        }

        /* create setup filter list */
        for(uint16_t i = 0; i < len; i++) {
            flist[i] = CAN_gen_sid(can1, filter_list[i].raw);
        }

        CAN_setup_filter(flist, len, NULL, 0, NULL, 0, NULL, 0);
    }
    else {
        CAN_bypass_filter_accept_all_msgs();
    }

    CAN_reset_bus(can1);

    return true;
}

can_controller :: can_controller(controller_id_t val, can_msg_id_t *filter_list, size_t filter_len)
{
    controller_id = val;
    rx_count = 0;
    tx_count = 0;
    enum_to_string();
    can_setup(filter_list, filter_len);
}

bool can_controller :: can_send(controller_id_t dst, msg_id_t msg_num, uint8_t * data, uint16_t len)
{
    can_msg_t msg = {0};
    can_msg_id_t msg_id = {0};

    LE.set(CAN_TX_LED, !(LE.getValues() & (1 << (CAN_TX_LED - 1))));

    msg_id.src = controller_id;
    msg_id.dst = dst;
    msg_id.msg_num = msg_num;

    msg.msg_id = msg_id.raw;
    msg.frame_fields.is_29bit = 0;

    if(len) {
        size_t actual_len = 0;

        msg.data.bytes[0] = len; // 0th byte of data store actual len of the data

        while(actual_len < len) {
            if(len - actual_len > 7) {
                /* split into packets of 7 bytes */
                msg.frame_fields.data_len = 7 + 1;

                /* copy 7 bytes of data to packet */
                for(uint16_t i = 0; i < 7; i++) {
                    msg.data.bytes[i + 1] = data[i + actual_len];
                }
                actual_len += 7;
            }
            else {
                /* last packet of data */
                msg.frame_fields.data_len = (len - actual_len) + 1;

                /* copy 7 bytes of data to packet */
                for(uint16_t i = 0; i < (len - actual_len); i++) {
                    msg.data.bytes[i + 1] = data[i + actual_len];
                }
                actual_len = len;
            }

            if(false == CAN_tx(can1, &msg, portMAX_DELAY)) {
                SET_ERROR(ERROR_TX_FAILED);
                LOG_ERROR("CAN_tx failed\n");
                printf("[0x%02X] CAN_tx failed\n", error_flag);
                CAN_reset_bus(can1); // reset CAN bus, because bus might be in bus off state
            }
            else {
                tx_count++;
                tx_bytes += msg.frame_fields.data_len;
                if((error_flag & ERROR_TX_FAILED) == ERROR_TX_FAILED) {
                    RESET_ERROR(ERROR_TX_FAILED);
                }
            }
        }
    }
    else {
        msg.msg_id = msg_id.raw;
        msg.frame_fields.is_29bit = 0;
        msg.frame_fields.data_len = 0;

        if(false == CAN_tx(can1, &msg, portMAX_DELAY)){
            SET_ERROR(ERROR_TX_FAILED);
            LOG_ERROR("CAN_tx failed\n");
            printf("[0x%02X] CAN_tx failed\n", error_flag);
            CAN_reset_bus(can1); // reset CAN bus, because bus might be in bus off state
        }
        else {
            tx_count++;
            tx_bytes += msg.frame_fields.data_len;
            if((error_flag & ERROR_TX_FAILED) == ERROR_TX_FAILED) {
                RESET_ERROR(ERROR_TX_FAILED);
            }
        }
    }

    return true; // assuming that CAN_tx will never fail
}

bool can_controller:: can_recv(controller_id_t *src, msg_id_t *msg_num, uint8_t * data, uint16_t *len)
{
    can_msg_t msg = {0};
    can_msg_id_t msg_id = {0};

    if(false == CAN_rx(can1, &msg, portMAX_DELAY)) {
        SET_ERROR(ERROR_RX_FAILED);
        LOG_ERROR("CAN_rx failed\n");
        printf("[0x%02X] CAN_rx failed\n", error_flag);
        CAN_reset_bus(can1); // reset CAN bus, because bus might be in bus off state
        return false;
    }
    else {
        rx_count++;
        if((error_flag & ERROR_RX_FAILED) == ERROR_RX_FAILED) {
            RESET_ERROR(ERROR_RX_FAILED);
        }
    }
    LE.set(CAN_RX_LED, !(LE.getValues() & (1 << (CAN_RX_LED - 1))));

    uint16_t data_len = 0;
    uint16_t actual_len = 0;
    uint16_t read_len = 0;

    msg_id.raw = msg.msg_id;
    *src = (controller_id_t) msg_id.src;
    *msg_num = (msg_id_t) msg_id.msg_num;
    data_len = msg.frame_fields.data_len;

    /* check if there is some data */
    rx_bytes += data_len;
    if(data_len) {
        /* read 0th byte to find out actual len of data */
        actual_len = msg.data.bytes[0];
        if(actual_len > *len) {
            /* TODO: provided buffer is not enough to store incoming data! */
            printf("insufficient buffer to read incoming message\n");
            return false;
        }

        while(read_len < actual_len) {
            if(actual_len - read_len > 7) {
                /* we have more packets of data */
                for(int i = 0; i < 7; i++) {
                    data[i + read_len] = msg.data.bytes[i + 1];
                }
                read_len += 7;
            }
            else {
                /* last packet of data */
                for(uint16_t i = 0; i < (actual_len - read_len); i++) {
                    data[i + read_len] = msg.data.bytes[i + 1];
                }
                read_len = actual_len;
            }

            if(read_len < actual_len) {
                if(false == CAN_rx(can1, &msg, portMAX_DELAY)) {
                    SET_ERROR(ERROR_RX_FAILED);
                    LOG_ERROR("CAN_rx failed\n");
                    printf("[0x%02X] CAN_rx failed\n", error_flag);
                    CAN_reset_bus(can1); // reset CAN bus, because bus might be in bus off state
                }
                else {
                    rx_count++;
                    if((error_flag & ERROR_RX_FAILED) == ERROR_RX_FAILED) {
                        RESET_ERROR(ERROR_RX_FAILED);
                    }
                }
            }
        }
    }
    *len = read_len;

    return true; // assuming that CAN_rx will never fail
}

uint16_t can_controller :: get_rx_count(void)
{
    return rx_count;
}

uint16_t can_controller :: get_rx_bytes(void)
{
    return rx_bytes;
}

uint16_t can_controller :: get_tx_count(void)
{
    return tx_count;
}

uint16_t can_controller :: get_tx_bytes(void)
{
    return tx_bytes;
}

bool can_controller :: processBootSequence(uint8_t controller_version)
{
    msg_t msg;

    LE.on(4); // LED4 will be ON until can communication is setup
    while(1) {
        msg.len = sizeof(msg.data);
        can_recv(&msg.src, &msg.msg_num, msg.data, &msg.len);
        if(msg.msg_num == MSG_POWERUP_SYN) {
            delay_ms(controller_id * 10);
            msg.data[0] = controller_version;
            can_send(msg.src, MSG_POWERUP_SYN_ACK, msg.data, 1);
        }
        else if(msg.msg_num == MSG_POWERUP_ACK) {
            /* TODO: set date time received in "data" */
            LE.off(4); // turn OFF LED4 after can communication is setup
            break;
        }
        else if(msg.msg_num == MSG_RESET) {
            processReset();
            return true;
        }
        else {
            /* this should not get executed */
//            LE.on(1);
//            LOG_ERROR("Unknown msg %s from %s received.\n", message_id_str[msg_num], controller_id_str[src]);
        }
    }

    //delay_ms(controller_id * 100);

    return true;
}

void can_controller :: processReset(void)
{
    printf("RESET received, Rebooting System\n");
    LOG_FLUSH();
    //vTaskDelayMs(1000);
    sys_reboot();
}
