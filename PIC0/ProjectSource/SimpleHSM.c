#include "ES_Configure.h"
#include "ES_Framework.h"
#include "SPIMasterService.h"
#include "dbprintf.h"
#include "SimpleHSM.h"

/*----------------------------- Module Variables ---------------------------*/
static uint8_t MyPriority;
static enum { INIT_STATE, SEND_STATE, WAIT_STATE } CurrentState;

/*----------------------------- Function Prototypes ------------------------*/
bool InitSimpleHSM(uint8_t Priority);
bool PostSimpleHSM(ES_Event_t ThisEvent);
ES_Event_t RunSimpleHSM(ES_Event_t ThisEvent);

/*----------------------------- Initialization -----------------------------*/
bool InitSimpleHSM(uint8_t Priority) {
    MyPriority = Priority;
    CurrentState = INIT_STATE;

    ES_Event_t InitEvent;
    InitEvent.EventType = ES_ENTRY;
    return PostSimpleHSM(InitEvent);
}

bool PostSimpleHSM(ES_Event_t ThisEvent) {
    return ES_PostToService(MyPriority, ThisEvent);
}

/*----------------------------- State Machine -----------------------------*/
ES_Event_t RunSimpleHSM(ES_Event_t ThisEvent) {
    ES_Event_t ReturnEvent;
    ReturnEvent.EventType = ES_NO_EVENT; // Assume no errors

    switch (CurrentState) {
        case INIT_STATE:
            if (ThisEvent.EventType == ES_ENTRY) {
                DB_printf("[HSM] Initializing...\r\n");
                CurrentState = SEND_STATE;
                ES_Event_t SendEvent;
                SendEvent.EventType = ES_SEND_DATA;
                PostSimpleHSM(SendEvent);
            }
            break;

        case SEND_STATE:
            if (ThisEvent.EventType == ES_SEND_DATA) {
                DB_printf("[HSM] Sending SPI Command...\r\n");
                ES_Event_t SpiEvent;
                SpiEvent.EventType = ES_NEW_NAV_CMD;
                SpiEvent.EventParam = 0x4; // Example byte to send
                PostSPIMasterService(SpiEvent);
                CurrentState = WAIT_STATE;
            }
            break;

        case WAIT_STATE:
            if (ThisEvent.EventType == ES_NAVIGATOR_STATUS_CHANGE) {
                DB_printf("[HSM] Received Response: %X\r\n", ThisEvent.EventParam);
                CurrentState = SEND_STATE;
                ES_Event_t SendEvent;
                SendEvent.EventType = ES_SEND_DATA;
                PostSimpleHSM(SendEvent);
            }
            break;

        default:
            break;
    }
    return ReturnEvent;
}
