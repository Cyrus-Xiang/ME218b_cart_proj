/****************************************************************************

  Header file for Joint2Servo service
  based on the Gen 2 Events and Services Framework

 ****************************************************************************/

#ifndef ServJoint2Servo_H
#define ServJoint2Servo_H

#include "ES_Types.h"

// Public Function Prototypes

bool InitJoint2ServoService(uint8_t Priority);
bool PostJoint2ServoService(ES_Event_t ThisEvent);
ES_Event_t RunJoint2ServoService(ES_Event_t ThisEvent);

#endif /* ServJoint2Servo_H */

