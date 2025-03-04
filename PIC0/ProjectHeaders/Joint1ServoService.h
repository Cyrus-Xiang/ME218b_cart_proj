/****************************************************************************

  Header file for Joint1Servo service
  based on the Gen 2 Events and Services Framework

 ****************************************************************************/

#ifndef ServJoint1Servo_H
#define ServJoint1Servo_H

#include "ES_Types.h"

// Public Function Prototypes

bool InitJoint1ServoService(uint8_t Priority);
bool PostJoint1ServoService(ES_Event_t ThisEvent);
ES_Event_t RunJoint1ServoService(ES_Event_t ThisEvent);

#endif /* ServJoint1Servo_H */

