/*
 * can_common.hpp
 *
 *  Created on: Oct 22, 2014
 *      Author: Harita
 */

#ifndef CAN_COMMON_HPP_
#define CAN_COMMON_HPP_

#include "FreeRTOS.h"
#include "can.h"
#include "can_protocol.hpp"

//#define ENABLE_DUMMY
//#define ENABLE_MASTER
//#define ENABLE_SENSOR
#define ENABLE_MOTOR
//#define ENABLE_GEO
//#define ENABLE_IO
//#define ENABLE_BT_ANDROID

extern uint8_t error_flag;

/* basic structure to create/store 11bit CAN msg id,
 * used for creating filter list and for creating 11bit ids
 */
typedef union {
        struct{
                uint16_t src        : 3; // Source ID       (LSB)
                uint16_t dst        : 3; // Destination ID
                uint16_t msg_num    : 5; // Message Number  (MSB)
        };
        uint16_t raw;
} __attribute__((packed)) can_msg_id_t;

/* wrapper class for CAN framework */
class can_controller {
    private:
        controller_id_t controller_id;
        uint16_t        rx_count;
        uint16_t        rx_bytes;
        uint16_t        tx_count;
        uint16_t        tx_bytes;

        void enum_to_string(void);
        bool can_setup(can_msg_id_t *, size_t);

    public:
        /*
         * function get_rx_count: returns total can messages received so far
         */
        uint16_t get_rx_count(void);

        /*
         * function get_rx_bytes: returns total bytes received so far
         */
        uint16_t get_rx_bytes(void);

        /*
         * function get_tx_count: returns total can messages sent so far
         */
        uint16_t get_tx_count(void);

        /*
         * function get_tx_bytes: returns total bytes sent so far
         */
        uint16_t get_tx_bytes(void);

        /*
         * constructor can_controller: this function inits the can bus, sets up the filter list, and subscribes to other controllers
         *
         * @param val: ID of your controller for which you are creating this controller object
         *
         * @param filter_list: array of type can_msgID_t which lists what messages you are expecting
         *      e.g.,:
         *      can_msgID_t filter_list[] ={
         *              {CONTROLLER_MOTOR,        CONTROLLER_MASTER,    MSG_HEARTBEAT_ACK },
         *              {CONTROLLER_MOTOR ,       CONTROLLER_MASTER,    MSG_ENCODER_DATA },
         *      };
         *
         * @param filter_len: length of above list, typically sizeof(filter_list)/sizeof(can_msgID_t)
         *
         * NOTE:passing NULL to filter_list argument will bypass all the filters and acks all messages
         *
         */
        can_controller(controller_id_t val, can_msg_id_t *filter_list, size_t filter_len);

        /*
         * function can_send: send data over can bus
         *
         * @param dst: destination controller, usually of type controllerID_t
         *
         * @param msg_num: message to send, usually of type msgID_t
         *
         * @param data: pointer to data buffer, can be NULL if there is no data to send with the message
         *
         * @param len: size of the data
         */
        bool can_send(controller_id_t dst, msg_id_t msg_num, uint8_t *data, uint16_t len);

        /*
         * function can_recv: receive data from can bus
         *
         * @param src: after function successfully executes, stores/returns controller ID of source
         *
         * @param msg_num: after function successfully executes, stores/returns msg ID of the received message
         *
         * @param data: pointer to data buffer in which you want to store received data
         *
         * @paran len: while invoking the function should contain size of data buffer, after function executes successfully, stores/returns length of actual data received
         */
        bool can_recv(controller_id_t *src, msg_id_t *msg_num, uint8_t *data, uint16_t *len);

        bool processBootSequence(uint8_t controller_version);
        void processReset(void);
};

#endif /* CAN_COMMON_HPP_ */
