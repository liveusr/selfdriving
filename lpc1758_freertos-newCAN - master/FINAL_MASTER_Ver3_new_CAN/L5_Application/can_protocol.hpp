/*
 * can_protocol.hpp
 *
 *  Created on: Nov 5, 2014
 *      Author: MANish
 */

#ifndef CAN_PROTOCOL_HPP_
#define CAN_PROTOCOL_HPP_

extern char controller_id_str[][32];
extern char message_id_str[][32];

#define ERROR_LED       1
#define CAN_RX_LED      2
#define CAN_TX_LED      3
#define MAX_DATA_LEN    32

#define SET_ERROR(error_code) \
        error_flag |= error_code; \
        (error_flag)? LE.on(ERROR_LED): LE.off(ERROR_LED);

#define RESET_ERROR(error_code) \
        error_flag &= ~(error_code); \
        (error_flag)? LE.on(ERROR_LED): LE.off(ERROR_LED);


/* enum of all available controllers */
typedef enum {
    CONTROLLER_ALL          = 0x00, // used for broadcast
    CONTROLLER_MOTOR        = 0x01,
    CONTROLLER_MASTER       = 0x02,
    CONTROLLER_SENSOR       = 0x03,
    CONTROLLER_GEO          = 0x04,
    CONTROLLER_BT_ANDROID   = 0x05,
    CONTROLLER_IO           = 0x06,
    CONTROLLER_DUMMY        = 0x07  // used for testing
} controller_id_t;

/* enum of all possible messages */
typedef enum {
    MSG_RESET               = 0x01, // DATA: NO DATA, IO to MASTER, when hardware reset button on car is pressed, MASTER to ALL when master boots
    MSG_POWERUP_SYN         = 0x02, // DATA: NO DATA, MASTER to ALL, after board boots up, every 1 sec until MSG_POWERUP_SYN_ACK received from ALL
    MSG_POWERUP_SYN_ACK     = 0x03, // DATA: powerup_syn_ack_data_t, ALL to MASTER, after MSG_POWERUP_SYN received from MASTER, hooked by IO/BT to show connection status, and module version number
    MSG_POWERUP_ACK         = 0x04, // DATA: powerup_ack_data_t, MASTER to ALL, after MSG_POWERUP_SYN_ACK received from ALL

    MSG_HEARTBEAT           = 0x05, // DATA: heartbeat_data_t, MASTER to ALL, periodic
    MSG_HEARTBEAT_ACK       = 0x06, // DATA: heartbeat_ack_data_t, ALL to MASTER, when MSG_HEARTBEAT received, IO/BT will hook to display Rx/Tx count

    MSG_DIST_SENSOR_DATA    = 0x0A, // DATA: dist_sensor_data_t, SENSOR to ALL, MASTER/IO/BT will hook, periodic
    MSG_OTHER_SENSOR_DATA   = 0x11, // DATA: other_sensor_data_t, SENSOR to ALL, IO/BT will hook, periodic

    MSG_GEO_DATA            = 0x0B, // DATA: geo_data_t, GEO to ALL, MASTER/IO/BT will hook, periodic

    MSG_SPEED_ENCODER_DATA  = 0x10, // DATA: speed_encoder_data_t, MOTOR to ALL, MASTER/IO/BT will hook, periodic

    MSG_CAR_PAUSE           = 0x07, // DATA: NO DATA, IO/BT to MASTER, MASTER/IO/BT will hook
    MSG_CAR_RESUME          = 0x08, // DATA: NO DATA, IO/BT to MASTER, MASTER/IO/BT will hook

    MSG_CHECKPOINT_REQUEST  = 0x0C, // DATA: checkpoint_request_data_t, GEO to BT, when current checkpoint expires
    MSG_CHECKPOINT_DATA     = 0x0D, // DATA: checkpoint_data_t, BT to GEO, when MSG_CHECKPOINT_REQUEST received, IO will hook to display progress bar
    MSG_DRIVE_MODE          = 0x0E, // DATA: drive_mode_data_t, IO/BT to ALL, MASTER/IO/BT/MOTOR will hook
    MSG_SPEED_DIR_COMMAND   = 0x09, // DATA: speed_dir_data_t, MASTER to MOTOR, periodic
    MSG_FREE_RUN_DIR        = 0x0F, // DATA: speed_dir_data_t, BT to MASTER, valid only in free run

    MSG_ERROR               = 0x1E, // DATA: error_data_t, ALL to ALL
    MSG_DUMMY               = 0x1F, // DATA: dummy_data_t, dummy message just to make even number of filter entries, if required
} msg_id_t;



/* used with MSG_POWERUP_SYN_ACK */
typedef struct {
        uint8_t  version;                   // in major.minor format (i.e. if version is 2.1, version = 0x21)
} powerup_syn_ack_data_t;

/* used with MSG_POWERUP_ACK */
typedef struct {
        uint8_t version;                    // in major.minor format (i.e. if version is 2.1, version = 0x21)
        struct {
                uint16_t    year;
                uint8_t     month;
                uint8_t     date;
                uint8_t     dayofweek;
                uint8_t     hour;
                uint8_t     minute;
                uint8_t     second;
        } time;
} powerup_ack_data_t;



/* used with MSG_HEARTBEAT_ACK */
typedef struct {
        uint16_t rx_count;                  // total number of can messages received (controller.get_rx_count())
        uint16_t rx_bytes;                  // total number of bytes received (controller.get_rx_bytes())
        uint16_t tx_count;                  // total number of can messages transmitted (controller.get_tx_count())
        uint16_t tx_bytes;                  // total number of bytes transmitted (controller.get_tx_bytes())
} heartbeat_ack_data_t;
typedef heartbeat_ack_data_t heartbeat_data_t;


/* used with MSG_DIST_SENSOR_DATA */
typedef struct {
        uint8_t  left;                      // in feet
        uint8_t  middle;                    // in feet
        uint8_t  right;                     // in feet
        uint8_t  back;                      // in feet
} dist_sensor_data_t;

/* used with MSG_OTHER_SENSOR_DATA */
typedef struct {
        uint8_t  battery;                   // in percentage
        uint8_t  light;                     // in percentage
} other_sensor_data_t;



///* MSG_GEO_DATA */
//typedef struct {
//        float    latitude;                  // convert convert GPS/MAP received string to float
//        float    longitude;                 // convert convert GPS/MAP received string to float
//        uint16_t dist_to_final_destination; // in feet
//        uint16_t dist_to_next_checkpoint;   // in feet
//        uint8_t  current_angle;             // zone number
//        uint8_t  desired_angle;             // zone number
//        uint8_t  is_valid : 1;              // true only if GPS location is fixed; if false, ignore above data
//} geo_data_t;

/* MSG_GEO_HEADING_DATA */
typedef struct {
        uint8_t  current_angle;             // zone number
        uint8_t  desired_angle;             // zone number
        uint8_t  destination_reached;       // true when destination reached
        uint8_t  is_valid;                  // true only if GPS location is fixed; if false, ignore above data
} geo_heading_data_t;

/* MSG_GEO_LOCATION_DATA */
typedef struct {
        float    latitude;                  // convert convert GPS/MAP received string to float
        float    longitude;                 // convert convert GPS/MAP received string to float
        uint16_t dist_to_final_destination; // in feet
        uint16_t dist_to_next_checkpoint;   // in feet
        uint8_t  is_valid : 1;              // true only if GPS location is fixed; if false, ignore above data
} geo_location_data_t;




/* used with MSG_SPEED_ENCODER_DATA */
typedef struct {
        uint8_t  speed;                     // in mph
} speed_encoder_data_t;



/* used with MSG_CHECKPOINT_REQUEST */
typedef struct {
        uint8_t  checkpoint_num;            // required checkpoint number
} checkpoint_request_data_t;

/* used with MSG_CHECKPOINT_DATA */
typedef struct {
        float    latitude;                  // convert convert convert GPS/MAP received string to float
        float    longitude;                 // convert convert convert GPS/MAP received string to float
        uint16_t total_distance;            // in feet
        uint8_t  checkpoint_num;            // checkpoint number of this checkpoint, should start with 1
        uint8_t  is_new_route        : 1;   // true if this is first checkpoint
        uint8_t  is_final_checkpoint : 1;   // true if this is final checkpoint
} checkpoint_data_t;



/* used in drive_mode_data_t */
typedef enum {
    MODE_FREE_RUN           = 0x01,         // default mode after powerup, and car should be in CAR_PAUSE state after powerup
    MODE_MAP                = 0x02,         // set by BT before sending first checkpoint
    MODE_HOME               = 0x03,         // set by BT/IO when home button is pressed
    MODE_SLOW               = 0x04,         // set by BT only, changes base speed
    MODE_NORMAL             = 0x05,         // set by BT only, changes base speed, default mode for MOTOR
    MODE_TURBO              = 0x06          // set by BT only, changes base speed
} drive_mode_t;

/* used with MSG_DRIVE_MODE */
typedef struct {
        uint8_t  mode;                      // of type drive_mode_t
} drive_mode_data_t;



/* used in speed_dir_data_t */
typedef enum {
    SPEED_STOP              = 0x01,         // stops smoothly
    SPEED_SLOW              = 0x02,         // base + slow speed
    SPEED_MEDIUM            = 0x03,         // base + medium speed
    SPEED_FAST              = 0x04,         // base + fast speed
    SPEED_EMERGENCY_STOP    = 0x05          // urgent brake!
} motor_speed_t;

/* used in speed_dir_data_t */
typedef enum {
    TURN_STRAIGHT           = 0x01,         // straight
    TURN_SLIGHT_LEFT        = 0x02,         // slight left turn
    TURN_HALF_LEFT          = 0x03,         // half left turn
    TURN_FULL_LEFT          = 0x04,         // full left turn
    TURN_SLIGHT_RIGHT       = 0x05,         // slight right turn
    TURN_HALF_RIGHT         = 0x06,         // half left turn
    TURN_FULL_RIGHT         = 0x07          // full right turn
} motor_turn_t;

/* used in speed_dir_data_t */
typedef enum {
    DIR_FWD                 = 0x01,         // forward
    DIR_REV                 = 0x02          // reverse
} motor_direction_t;

/* used with MSG_SPEED_DIR_COMMAND and MSG_FREE_RUN_DIR */
typedef struct {
        uint8_t  speed;                     // of type motor_speed_t
        uint8_t  turn;                      // of type motor_turn_t
        uint8_t  direction;                 // of type motor_direction_t
} speed_dir_data_t;



/* used in error_data_t */
typedef enum {
    ERROR_NO_ERROR          = 0x00,
    ERROR_BUS_OFF           = 0x01,
    ERROR_DATA_OVERRUN      = 0x02,
    ERROR_TX_FAILED         = 0x04,
    ERROR_RX_FAILED         = 0x08,
    ERROR_MSG_QUEUE_FULL    = 0x10,
    ERROR_UNKNOWN           = 0xFF          // unspecified error
} error_code_t;

/* used with MSG_ERROR */
typedef struct {
        uint8_t  error_code;                // of type error_code_t
} error_data_t;

/* used with MSG_DUMMY */
typedef struct {
        uint16_t i;                         // dummy data bytes
        uint8_t  c;                         // dummy data bytes
} dummy_data_t;



/* used to buffer the incoming message */
typedef struct {
        controller_id_t                     src;
        msg_id_t                            msg_num;
        union {
                uint8_t                     data[MAX_DATA_LEN];
                powerup_syn_ack_data_t      powerup_syn_ack_data;
                powerup_ack_data_t          powerup_ack_data;
                heartbeat_data_t            heartbeat_data;
                heartbeat_ack_data_t        heartbeat_ack_data;
                dist_sensor_data_t          dist_sensor_data;
                other_sensor_data_t         other_sensor_data;
                geo_heading_data_t          geo_heading_data;
                geo_location_data_t         geo_location_data;
                speed_encoder_data_t        speed_encoder_data;
                checkpoint_data_t           checkpoint_data;
                checkpoint_request_data_t   checkpoint_request_data;
                drive_mode_data_t           drive_mode_data;
                motor_direction_t           motor_direction;
                speed_dir_data_t            speed_dir_data;
                error_data_t                error_data;
                dummy_data_t                dummy_data;
        };
        uint16_t                            len;
} msg_t;

#endif /* CAN_PROTOCOL_HPP_ */
