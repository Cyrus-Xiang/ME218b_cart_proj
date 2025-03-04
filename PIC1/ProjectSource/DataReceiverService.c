#include "ES_Configure.h"
#include "ES_Framework.h"
#include "dbprintf.h"
#include "DataReceiverService.h"
#include "SPIFollowerService.h"

static uint8_t MyPriority;
static uint8_t ReceivedData = 0;

bool InitDataReceiverService(uint8_t Priority);
bool PostDataReceiverService(ES_Event_t ThisEvent);
ES_Event_t RunDataReceiverService(ES_Event_t ThisEvent);

static NavigatorState_t CurrentState;
curentState = Init;

bool InitDataReceiverService(uint8_t Priority) {
    MyPriority = Priority;
    ES_Event_t InitEvent;
    InitEvent.EventType = ES_INIT;
    DB_printf("Start data receiver service!!!\n");
    return ES_PostToService(MyPriority, InitEvent);
}

bool PostDataReceiverService(ES_Event_t ThisEvent) {
    return ES_PostToService(MyPriority, ThisEvent);
}

ES_Event_t RunDataReceiverService(ES_Event_t ThisEvent) {
    ES_Event_t ReturnEvent;
    ReturnEvent.EventType = ES_NO_EVENT;

    if (ThisEvent.EventType == ES_DATA_RECEIVED) {
        ReceivedData = ThisEvent.EventParam;
        DB_printf("[DataReceiverService] Received data: %X\r\n", ReceivedData);
    }else if (ThisEvent.EventType == ES_NEW_PIC0_CMD){
        ReceivedData = ThisEvent.EventParam;
        DB_printf("[DataReceiver] Received SPI command %d\r\n", ReceivedData);
        ES_Event_t CmdEvent;
        if (ThisEvent.EventParam == MOTOR_MOVE_FORWARDS){
            CmdEvent.EventType = ES_MOTOR_FWD;
            CmdEvent.EventParam = 50;
            PostMotorService(CmdEvent);
        }
    }

    return ReturnEvent;
}


NavigatorState_t QueryDataReceiverServiceState(void) {
    return CurrentState;
}