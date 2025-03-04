#ifndef DATA_RECEIVER_SERVICE_H
#define DATA_RECEIVER_SERVICE_H

#include "ES_Configure.h"
#include "ES_Framework.h"


typedef enum {
    Init,
    Idle,
    LineFollowForward,
    LineFollowBackward,
    AlignBeacon,
    CheckIntersection,
    TurnLeft,
    TurnRight,
    LineDiscover,
    CheckCrate,
    AlignTape,
    TapeAligned,
    LineDiscoverFail
} NavigatorState_t;


bool InitDataReceiverService(uint8_t Priority);
bool PostDataReceiverService(ES_Event_t ThisEvent);
ES_Event_t RunDataReceiverService(ES_Event_t ThisEvent);

#endif // DATA_RECEIVER_SERVICE_H
