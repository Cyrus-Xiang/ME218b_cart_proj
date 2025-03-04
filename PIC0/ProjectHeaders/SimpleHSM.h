#ifndef SIMPLE_HSM_H
#define SIMPLE_HSM_H

#include "ES_Configure.h"
#include "ES_Framework.h"

bool InitSimpleHSM(uint8_t Priority);
bool PostSimpleHSM(ES_Event_t ThisEvent);
ES_Event_t RunSimpleHSM(ES_Event_t ThisEvent);

#endif // SIMPLE_HSM_H
