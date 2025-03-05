#include "ES_Configure.h"
#include "ES_Framework.h"
#include "DCService.h"
#include <xc.h>
#include "DCService.h"


/*----------------------------- Module Defines ----------------------------*/
#define ONE_SEC 1000 // 1000 ms
static uint16_t interval = 5;

/*---------------------------- Module Variables ---------------------------*/
static uint8_t MyPriority;
#define DC_1 LATAbits.LATA0
#define DC_2 LATAbits.LATA1
//#define DIR_VAL PORTAbits.RA3
#define DIR_VAL 0
/*---------------------------- Module Functions ---------------------------*/
//static void SetMotorDirection(bool direction);


/*------------------------------ Module Code ------------------------------*/

/****************************************************************************
 Function
     InitDCMotorService

 Parameters
     uint8_t : the priority of this service

 Returns
     bool, false if error in initialization, true otherwise

 Description
     Initializes the DC motor service, sets up PWM, and configures the motor.
****************************************************************************/
bool InitDCMotorService(uint8_t Priority)
{
    MyPriority = Priority;

    // Initialize motor control pins as digital outputs
    TRISAbits.TRISA0 = 0; // RA0 as output
    TRISAbits.TRISA1 = 0; // RA1 as output

    
    ANSELAbits.ANSA0 = 0; // Disable analog on RA0
    ANSELAbits.ANSA1 = 0; // Disable analog on RA0
    // Post the initial ES_INIT event
    ES_Event_t ThisEvent;
    ThisEvent.EventType = ES_INIT;
   
    DC_1 = 0;
    DC_2 = 0;

    return ES_PostToService(MyPriority, ThisEvent);
}

/****************************************************************************
 Function
     PostDCMotorService

 Parameters
     ES_Event ThisEvent : the event to post

 Returns
     bool, true if post succeeds, false otherwise
****************************************************************************/
bool PostDCMotorService(ES_Event_t ThisEvent)
{
    return ES_PostToService(MyPriority, ThisEvent);
}

/****************************************************************************
 Function
     RunDCMotorService

 Parameters
     ES_Event : the event to process

 Returns
     ES_Event, ES_NO_EVENT if no error
****************************************************************************/
ES_Event_t RunDCMotorService(ES_Event_t ThisEvent)
{
    ES_Event_t ReturnEvent;
    ReturnEvent.EventType = ES_NO_EVENT; // Assume no errors

    switch (ThisEvent.EventType)
    {
    case ES_LINEAR_ACTUATOR_BWD:
        DC_1 = 1;
        DC_2 = 0;
        break;
    case ES_LINEAR_ACTUATOR_FWD:
        DC_1 = 0;
        DC_2 = 1;
        break;
    case ES_LINEAR_ACTUATOR_STOP:
        DC_1 = 0;
        DC_2 = 0;
        break;
    default:
        break;
    }

    return ReturnEvent;
}
