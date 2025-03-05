

#include <xc.h>
// #include <proc/p32mx170f256b.h>

// Event & Services Framework
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "ES_DeferRecall.h"
#include "ES_Port.h"
#include "terminal.h"

#include "dbprintf.h"
#include "Joint1ServoService.h"
#include <sys/attribs.h>

/*---------------------------- Module Variables ---------------------------*/
// with the introduction of Gen2, we need a module level Priority variable
static uint8_t MyPriority;

#define ONE_SEC 1000
#define HALF_SEC (ONE_SEC / 2)
#define QUATER_SEC (HALF_SEC / 2)



static uint8_t DutyCycle;
// 50 , 1

#define PIC_FREQ 20000000 // PIC 20MHz
// 9.5 for 180 degrees, 2 for 0 degree
#define PWM_0_DEG 2
#define PWM_180_DEG 9.5
#define JOINT2_TIME_STEP 50
static float currentPWM;
static bool increasing = false;

static void ConfigPWM_OC3();

bool InitJoint2ServoService(uint8_t Priority)
{
    ES_Event_t ThisEvent;

    MyPriority = Priority;
    puts("joint2 servo service is being initialized\r");
    TRISAbits.TRISA3 = 0; // set RA3 as PMW output pin
    ConfigPWM_OC3();
    DutyCycle = PWM_0_DEG;

    // Move to 0-degree position at startup
    //OC3RS = (float)(PR3 + 1) * PWM_0_DEG / 100;
    //DB_printf("Initializing at 0 degrees (PWM: %f)\n", PWM_0_DEG);
    
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

bool PostJoint2ServoService(ES_Event_t ThisEvent)
{
    return ES_PostToService(MyPriority, ThisEvent);
}

ES_Event_t RunJoint2ServoService(ES_Event_t ThisEvent)
{
    ES_Event_t ReturnEvent;
    ReturnEvent.EventType = ES_NO_EVENT; // assume no errors
//    static float currentPWM = PWM_180_DEG;
    const float PWM_STEP = 0.2;
//    static bool increasing = false;
    
    switch (ThisEvent.EventType)
    {
    case ES_INIT:
        currentPWM = PWM_0_DEG;
        increasing = true;
        OC3RS = (float)(PR3 + 1) * currentPWM / 100;
        DB_printf("[INIT] Servo at 180 degrees (PWM: %d)\n", (int)(currentPWM * 100));
        break;
    
    case ES_ROTATE_ZERO_Joint2:
        DB_printf("Recevie ES_ROTATE_ZERO");
        if (currentPWM == PWM_0_DEG) {
            DB_printf("[EVENT] Already at 0 degrees, no action needed.\n");
        } else {
            increasing = false;
            DB_printf("[EVENT] Moving to 0 degrees.\n");
            ES_Timer_InitTimer(JOINT2_SERVO_TIMER, JOINT2_TIME_STEP);
        }
        break;
    
    case ES_ROTATE_180_Joint2:
        DB_printf("Recevie ES_ROTATE_180");
        if (currentPWM == PWM_180_DEG) {
            DB_printf("[EVENT] Already at 180 degrees, no action needed.\n");
        } else {
            increasing = true;
            DB_printf("[EVENT] Moving to 180 degrees.\n");
            ES_Timer_InitTimer(JOINT2_SERVO_TIMER, JOINT2_TIME_STEP);
        }
        break;
    case ES_ROTATE_90_Joint2:
        break;
    case ES_TIMEOUT:
        if (increasing) {
            if (currentPWM < PWM_180_DEG) {
                currentPWM += PWM_STEP;
                if (currentPWM > PWM_180_DEG) {
                    currentPWM = PWM_180_DEG;
                }
                OC3RS = (float)(PR3 + 1) * currentPWM / 100;
                DB_printf("[TIMEOUT] Increasing PWM: %d\n", (int)(currentPWM * 100));
                ES_Timer_InitTimer(JOINT2_SERVO_TIMER, JOINT2_TIME_STEP);
            } else {
                DB_printf("[TIMEOUT] Reached 180 degrees. Stopping.\n");
            }
        } else {
            if (currentPWM > PWM_0_DEG) {
                currentPWM -= PWM_STEP;
                if (currentPWM < PWM_0_DEG) {
                    currentPWM = PWM_0_DEG;
                }
                OC3RS = (float)(PR3 + 1) * currentPWM / 100;
                DB_printf("[TIMEOUT] Decreasing PWM: %d\n", (int)(currentPWM * 100));
                ES_Timer_InitTimer(JOINT2_SERVO_TIMER, JOINT2_TIME_STEP);
            } else {
                DB_printf("[TIMEOUT] Reached 0 degrees. Stopping.\n");
            }
        }
        break;
    
    default:
        DB_printf("[UNKNOWN EVENT] Type: %d\n", ThisEvent.EventType);
        break;
    }
    return ReturnEvent;
}

static void ConfigPWM_OC3() {
    RPA3R = 0b0101;
    OC3CON = 0;
    OC3CONbits.ON = 0;
    OC3CONbits.OCM = 0b110;
    OC3CONbits.OCTSEL = 1;//uses Timer3
    OC3RS = PR3 * 0;
    OC3R = PR3 * 0;
    OC3CONbits.ON = 1;
    return;
}
