/*
 * gps.cpp
 *
 *  Created on: Oct 13, 2014
 *      Author: Tanuj Chirimar
 */


#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "gps.hpp"
#include "uart3.hpp"
#include "str.hpp"
#include "controller_geo.hpp"
#include "io.hpp"
#include "utilities.h"

//GLOBAL VARIABLES



float start_latitude;
float start_longitude;
uint16_t back_distance=0;
float desired_heading;

//extern geo_data_t periodic_geo_data;
extern geo_heading_data_t periodic_geo_heading_data;
extern geo_location_data_t periodic_geo_location_data;

extern checkpoint_data_t current_checkpoint;
extern checkpoint_data_t next_checkpoint;
extern can_controller controller;
extern checkpoint_request_data_t checkpoint_number;
extern float current_angle;

gpsTask::gpsTask(uint8_t priority) :scheduler_task("GPS", 5*2048, priority)
{
    //UART INITIALIZATION
    Uart3 &u3= Uart3::getInstance();
    u3.init(38400,2000,10); //Baud Rate, Rx Queue size, Tx Queue Size
    setRunDuration(199);
    LD.init();
    periodic_geo_location_data.is_valid=false;
    periodic_geo_heading_data.is_valid=false;
}

float gpsTask::ToRadians(float degrees)
{
    float radians = degrees * M_PI / 180.0;
    return radians;
}

float gpsTask::ToDegrees(float radians)
{
    float degrees = radians * 180.0/M_PI;
    return degrees;
}



float gpsTask::get_bearing(float lat1, float lng1, float lat2, float lng2)
{

#if 1
   float dLng = ToRadians(lng2-lng1);
    lat1=ToRadians(lat1);
    lat2=ToRadians(lat2);
    lng1=ToRadians(lng1);
    lng2=ToRadians(lng2);
    float y= sin(dLng)*cos(lat2);
    float x= cos(lat1)*sin(lat2)-sin(lat1)*cos(lat2)*cos(dLng);
    float bearing=0;
    bearing= atan2(y,x);
    bearing = ToDegrees(bearing);
    bearing= fmodf((bearing+360),360);
    return bearing;
#endif

#if 0
//    printf("#########  Actual: %4.2f %2d  ",bearing,getZone(bearing));

/*
    if((bearing>20&&bearing<70)||(bearing>200&&bearing<250))
        bearing = (bearing-11 < 0)? (bearing-11+360): (bearing-11);
    else
        bearing = (bearing-22.5 < 0)? (bearing-22.5+360): (bearing-22.5);
*/


/*    if(bearing>180 && bearing<360)
         bearing+=45;//70

    bearing = (bearing-45 < 0)? (bearing-45+360): (bearing-45);
  */
/*
    if(bearing>180 && bearing<360)
         bearing+=36;//70

    bearing = (bearing-36 < 0)? (bearing-36+360): (bearing-36);
*/
   //fmodf(bearing+360,360);
//
    /*
    if(bearing>220 ||(bearing >=0 && bearing<=20))
         bearing+=70;

     bearing = (bearing-45 < 0)? (bearing-45+360): (bearing-45);
*/

  //  printf("Correction: %4.2f %2d ",bearing,getZone(bearing));

   // printf("Current: %4.2f %2d  \n",current_angle,getZone(current_angle));
#endif


#if 0
      lat1 = ToRadians(lat1);
      lng1 = ToRadians(lng1);
      lat2 = ToRadians(lat2);
      lng2 = ToRadians(lng2);

      float dLong = lng2 - lng1;

      float dPhi = log(tan(lat2/2.0+M_PI/4.0)/tan(lat1/2.0+M_PI/4.0));
      if (abs(dLong) > M_PI){
        if (dLong > 0.0)
           dLong = -(2.0 * M_PI - dLong);
        else
           dLong = (2.0 * M_PI + dLong);
      }
      printf("bearing = %lf", fmodf((ToDegrees(atan2(dLong, dPhi)) + 360.0),360));
      return fmodf((ToDegrees(atan2(dLong, dPhi)) + 360.0),360);
#endif

#if 0
    float dlat = lat2 - lat1;
    float dlon = lng2 - lng1;
    float y = sin(ToRadians(lng2)-ToRadians(lng1))*cos(ToRadians(lat2));
    float x = cos(ToRadians(lat1))*sin(ToRadians(lat2))-sin(ToRadians(lat1))*cos(ToRadians(lat2))*cos(ToRadians(lng2)-ToRadians(lng1));

    float bearing=0;
    if (y > 0)
    {
      if( x > 0)
          bearing = ToDegrees(atan(y/x));
      if (x < 0)
          bearing = 180-ToDegrees(atan(-y/x));
      if (x == 0)
          bearing = 90;
    }
    if (y < 0)
    {
      if (x > 0)
          bearing= -ToDegrees(atan(-y/x));
      if (x < 0)
          bearing = ToDegrees(atan(y/x))-180;
      if (x == 0)
          bearing = 270;
    }
    if (y == 0)
    {
      if (x > 0)
          bearing = 0;
      if (x < 0 )
          bearing= 180;
      //if (x == 0) then [the 2 points are the same]
    }

    printf("bearing = %f", ToDegrees(bearing));

    return ToDegrees(bearing);

#endif

}

uint8_t gpsTask::getZone(float compass_bearing)
{
    return (uint8_t)(compass_bearing/18.0);
}

void gpsTask::calculate_midpoint(float lat1, float lng1, float lat2, float lng2, float& latmid, float& lngmid)
{
    float Bx = cos(lat2) * cos(lng2-lng1);
    float By = cos(lat2) * sin(lng2-lng1);
    latmid = atan2(sin(lat1) + sin(lat2), sqrt( (cos(lat1)+Bx)*(cos(lat1)+Bx) + By*By ) );
    lngmid = lng1 + atan2(By, (cos(lat1) + Bx));

}

float gpsTask::calculate_distance(float lat1, float lng1, float lat2, float lng2)
{

  float earthRadius = 6371.0; //kilometers

  float dLat = ToRadians(lat2-lat1);
  float dLng = ToRadians(lng2-lng1);

  float a = sin(dLat/2.0) * sin(dLat/2.0) +
             cos(ToRadians(lat1)) * cos(ToRadians(lat2)) * sin(dLng/2.0) * sin(dLng/2.0);

  float c = 2.0 * atan2(sqrt(a),sqrt(1-a));

  float dist = (float) (earthRadius * c);

  return dist*3280.8399;
}

bool gpsTask::init(void)
{
    printf("init");
    periodic_geo_location_data.dist_to_final_destination = 100;
    return true;
}

void gpsTask::print(void)
{
    printf(" Lat: %f   Lon: %f\n",current_checkpoint.latitude,current_checkpoint.longitude);
    printf("LATITUDE %f ",periodic_geo_location_data.latitude);
    printf("LONGITUDE %f\n",periodic_geo_location_data.longitude);
    printf("DESIRED ZONE: %d  %f\n",periodic_geo_heading_data.desired_angle,desired_heading);
    printf("DISTANCE TO DESTINATION: %d\n",periodic_geo_location_data.dist_to_final_destination);
    printf("DISTANCE TO CHECKPOINT: %d\n",periodic_geo_location_data.dist_to_next_checkpoint);
    printf("CHECHPOINT NUMBER: %d \n",checkpoint_number.checkpoint_num);

}
bool gpsTask::run(void *p)
{

    char* rxtemp;

    Uart3 &u33= Uart3::getInstance();

    //****FOR DEBUGGING PURPOSES

 //   printf("#############%d %d %d %d %f %f\n",periodic_geo_data.dist_to_final_destination,current_checkpoint.is_new_route,current_checkpoint.is_final_checkpoint,current_checkpoint.total_distance,current_checkpoint.latitude,current_checkpoint.longitude);

        //RECIEVE BUFFER
        char rx[300]={0};

        //UART INITIALIZATION

        u33.gets(rx,300,portMAX_DELAY);

//
//current_checkpoint.latitude=37.336013;/// event centre37.335039;
//current_checkpoint.longitude=-121.881841;//-121.879689;
//current_checkpoint.is_final_checkpoint=true;
//current_checkpoint.is_new_route=true;/
//periodic_geo_heading_data.is_valid=true;
//periodic_geo_heading_data.destination_reached=true;
//periodic_geo_location_data.is_valid=true;
//periodic_geo_location_data.dist_to_next_checkpoint=80;

        if(rxtemp=strstr(rx,"$GPGGA"))
        {


            char b[]=",,";      //FIND
            char c[]=",NULL,";  //REPLACE WITH
            char delim[]=",";   //DELIMITER TO SPLIT/TOKENIZE STRING
            char *token;
            char *token_array[15];
            int i=0;
            //TO replace ,, with ,NULL, to ensure no token is missed
            str gps_string=rxtemp;
            gps_string.replaceAll(b,c);
            gps_string.scanf("%s",rxtemp);

            //TO PARSE GPRMC STRING
            token = strtok(rxtemp, delim);

            // walk through other tokens
            while( token != NULL )
            {
                token_array[i]=token;
                token = strtok(NULL, delim);
                i++;
            }

            if(!strcmp(token_array[0],"$GPGGA") && (strcmp(token_array[6],"0"))  && (!strcmp(token_array[3],"N")) && (!strcmp(token_array[5],"W")))
            {

                periodic_geo_location_data.is_valid=true;
                //GPS STATUS
                if(current_checkpoint.latitude!=0.0 && current_checkpoint.longitude!=0.0)
                {
                    LE.on(4);
                    periodic_geo_heading_data.is_valid=true;
                }
                //****LATITUDE COORDINATES****
                char lat_degree[3]={0},lat_minutes[7]={0};
                float lat_deg,lat_min,final_lat;
                char *latitude=token_array[2];
                strncpy(lat_degree,latitude,2);
                sscanf(lat_degree, "%f", &lat_deg);
                strncpy(lat_minutes,latitude+2,7);
                sscanf(lat_minutes, "%f", &lat_min);
                final_lat=lat_deg+(lat_min/60.0);
                char *latitude_dir=token_array[3];

                if(!strcmp(latitude_dir,"S"))
                    final_lat*=-1.0;

                periodic_geo_location_data.latitude=(float)final_lat;

                //****LONGITUDE COORDINATES
                char long_degree[4]={0},long_minutes[7]={0};
                float long_deg,long_min,final_long;
                char *longitude=token_array[4];
                strncpy(long_degree,longitude,3);
                sscanf(long_degree, "%f", &long_deg);
                strncpy(long_minutes,longitude+3,7);
                sscanf(long_minutes, "%f", &long_min);
                final_long=long_deg+(long_min/60.0);
                char *longitude_dir=token_array[5];

                if(!strcmp(longitude_dir,"W"))
                    final_long*=-1.0;

                periodic_geo_location_data.longitude=(float)final_long;

                //If it is the first checkpoint set start position
                if(current_checkpoint.is_new_route)
                {
                    start_latitude=periodic_geo_location_data.latitude;
                    start_longitude=periodic_geo_location_data.longitude;
                    current_checkpoint.is_new_route=false;
                }

                //Calculate Desired Compass Heading
                desired_heading=get_bearing(periodic_geo_location_data.latitude,periodic_geo_location_data.longitude,current_checkpoint.latitude,current_checkpoint.longitude);///from pt A to B

                //Calculate Desired Zone
                periodic_geo_heading_data.desired_angle=getZone(desired_heading);

                //Calculate Distance Travelled
                float distance_travelled=calculate_distance(start_latitude,start_longitude,periodic_geo_location_data.latitude,periodic_geo_location_data.longitude) + back_distance;

                //Calculate Distance to Destination
                periodic_geo_location_data.dist_to_final_destination = current_checkpoint.total_distance-distance_travelled;

                //Calculate Distance to next checkpoint
                periodic_geo_location_data.dist_to_next_checkpoint=calculate_distance(periodic_geo_location_data.latitude,periodic_geo_location_data.longitude,current_checkpoint.latitude,current_checkpoint.longitude);



                ////////////////////////////////////
                print();
                ////////////////////////////////////

                 //Calculation when Current Checkpoint expires and to request new checkpoint


                 if(periodic_geo_location_data.dist_to_next_checkpoint<20)
                 {
                     printf("reached checkpoint %d\n", current_checkpoint.checkpoint_num);
                     //printf("send  current %d, next %d ,global %d \n",current_checkpoint.checkpoint_num,next_checkpoint.checkpoint_num,checkpoint_number.checkpoint_num);
                     if(!current_checkpoint.is_final_checkpoint)
                     {
                         start_latitude=current_checkpoint.latitude;
                         start_longitude=current_checkpoint.longitude;
                         current_checkpoint=next_checkpoint;
                         back_distance+=distance_travelled-20;
                         if(!next_checkpoint.is_final_checkpoint) {
                             controller.can_send(CONTROLLER_BT_ANDROID,MSG_CHECKPOINT_REQUEST, (uint8_t*)&checkpoint_number,(uint16_t)sizeof(checkpoint_number));
                             printf("requesting new checkpoint %d\n", checkpoint_number.checkpoint_num);
                         }
                     }
                     else
                     {
                         periodic_geo_heading_data.is_valid=false;
                         periodic_geo_heading_data.destination_reached=true;
                     }
                 }

#if 0
                if(periodic_geo_location_data.dist_to_next_checkpoint<20)
                {

                    start_latitude=current_checkpoint.latitude;
                    start_longitude=current_checkpoint.longitude;
                    current_checkpoint=next_checkpoint;
                    back_distance+=distance_travelled-20;

                    if(!next_checkpoint.is_final_checkpoint)
                        controller.can_send(CONTROLLER_BT_ANDROID,MSG_CHECKPOINT_REQUEST, &checkpoint_number.checkpoint_num,(uint16_t)sizeof(checkpoint_number.checkpoint_num));
                }

                if(current_checkpoint.is_final_checkpoint && periodic_geo_location_data.dist_to_next_checkpoint<20)
                 {
                     periodic_geo_location_data.dist_to_final_destination = 0;
                     periodic_geo_heading_data.is_valid=false;
                     periodic_geo_heading_data.destination_reached=true;
                 }
#endif
            }

        }

        return true;
}
