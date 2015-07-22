/*
 * gps.hpp
 *
 *  Created on: Oct 13, 2014
 *      Author: Tanuj Chirimar
 */

#ifndef GPS_HPP_
#define GPS_HPP_

#include "scheduler_task.hpp"

extern float start_latitude;
extern float start_longitude;
extern uint16_t back_distance;
extern float desired_heading;


class gpsTask : public scheduler_task
{

    private:
        float get_bearing(float lat1, float lng1, float lat2, float lng2);
        float ToDegrees(float radians);
        float ToRadians(float degrees);
        float calculate_distance(float lat1, float lng1, float lat2, float lng2);
        void calculate_midpoint(float lat1, float lng1, float lat2, float lng2, float& latmid, float& lngmid);
        void print(void);
    public:
        gpsTask(uint8_t priority);
        uint8_t getZone(float compass_bearing);
        bool run(void *p);
        bool init(void);


};

#endif /* GPS_HPP_ */
