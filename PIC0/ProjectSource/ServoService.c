
#include <xc.h>
// #include <proc/p32mx170f256b.h>

// Event & Services Framework
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "ES_DeferRecall.h"
#include "ES_Port.h"
#include "terminal.h"
#include "dbprintf.h"
#include "ServoService.h"
#include <sys/attribs.h>

/*---------------------------- Module Variables ---------------------------*/
// with the introduction of Gen2, we need a module level Priority variable
static uint8_t MyPriority;

#define ONE_SEC 1000
#define HALF_SEC (ONE_SEC / 2)
#define QUATER_SEC (HALF_SEC / 2)

#define GAME_TIME 60 * ONE_SEC


static Beacon_t DetectedBeacon;
static uint8_t DutyCycle;
// 50 , 1
#define CHANNEL4_PWM_FREQUENCY 50
#define PIC_FREQ 20000000 // PIC 20MHz
#define BLUE_PWM 9
#define GREEN_PWM 5
#define INIT_PWM 7

static void ConfigPWM_OC4();


bool InitServoService(uint8_t Priority)
{
    ES_Event_t ThisEvent;

    MyPriority = Priority;

    DB_printf("servo service is being initialized\r");

    TRISBbits.TRISB13 = 0; // set RB13 as PMW output pin
    ConfigPWM_OC4();
    DutyCycle = INIT_PWM;

    // post the initial transition event
    ThisEvent.EventType = ES_INIT;
    if (ES_PostToService(MyPriority, ThisEvent) == true)
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool PostServoService(ES_Event_t ThisEvent)
{
    return ES_PostToService(MyPriority, ThisEvent);
}

ES_Event_t RunServoService(ES_Event_t ThisEvent)
{
    ES_Event_t ReturnEvent;
    ReturnEvent.EventType = ES_NO_EVENT; // assume no errors
    switch (ThisEvent.EventType)
    {
    case ES_INIT:;
        OC4RS = (float)(PR3 + 1) * DutyCycle / 100;
        DB_printf("curent PR3 is %d\n", PR3);
        
    break;
    
    case ES_SIDE_DETECTED:
        DetectedBeacon =ThisEvent.EventParam;
        if (DetectedBeacon == BEACON_B || DetectedBeacon == BEACON_L){
            DutyCycle = BLUE_PWM;
        }else if (DetectedBeacon == BEACON_G || DetectedBeacon == BEACON_R){
            DutyCycle = GREEN_PWM;
        }
        OC4RS = (float)(PR3 + 1) * DutyCycle / 100;
        DB_printf("ES_SIDE_DETECTED is  %d\n", DetectedBeacon);
    break;
    case ES_SERVO_IND_RESET:
        DutyCycle = INIT_PWM;
        OC4RS = (float)(PR3 + 1) * DutyCycle / 100;
        DB_printf("servo side indicator is reset to neutral\n");
    break;
    default:
        break;
    } // end switch on Current State
    return ReturnEvent;
}

static void ConfigPWM_OC4() {

//     map OC2 to RB13
    RPB13R = 0b0101;
    //Clear OC2CON register: 
    OC4CON = 0;
    // Configure the Output Compare module for one of two PWM operation modes
    OC4CONbits.ON = 0;         // Turn off Output Compare module
    OC4CONbits.OCM = 0b110;    // PWM mode without fault pin
    OC4CONbits.OCTSEL = 1;     // Use Timer3 as the time base

    // Set the PWM duty cycle by writing to the OCxRS register
    OC4RS = PR3 * 0;       // Secondary Compare Register (for duty cycle)
    OC4R = PR3 * 0;        // Primary Compare Register (initial value)
    OC4CONbits.ON = 1;         // Enable Output Compare module
    
    return;
}
