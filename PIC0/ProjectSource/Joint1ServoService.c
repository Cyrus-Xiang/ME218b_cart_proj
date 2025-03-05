
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
#define PWM_0_DEG 2.9
#define PWM_90_DEG 8
#define PWM_10_DEG 13
#define JOINT1_TIME_STEP 50
//for configuring timer 2
#define PWM_freq 50 // wheel motor PWM frquency in Hz
#define PIC_freq 20000000
#define PIC_freq_kHz 20000
#define ns_per_tick 50 // nano-sec per tick for the PBC = 1/PIC_freq
#define prescalar_T2 8  // for PWM for the 2 motors

static void ConfigPWM_OC1() ;
static void ConfigTimer2();



bool InitJoint1ServoService(uint8_t Priority)
{
    ES_Event_t ThisEvent;

    MyPriority = Priority;

    // When doing testing, it is useful to announce just which program
    // is running.
    puts("\rStarting ServoService\r");
    ConfigTimer2();
    TRISBbits.TRISB15 = 0; // set RB15 as PMW output pin
    ConfigPWM_OC1();  
 
    DutyCycle = PWM_0_DEG;

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

bool PostJoint1ServoService(ES_Event_t ThisEvent)
{
    return ES_PostToService(MyPriority, ThisEvent);
}


ES_Event_t RunJoint1ServoService(ES_Event_t ThisEvent)
{
    ES_Event_t ReturnEvent;
    ReturnEvent.EventType = ES_NO_EVENT; // assume no errors
    static float currentPWM = PWM_0_DEG; // Start from PWM_0_DEG
    const float PWM_STEP = 0.2;
    static bool decreasing = false; // Track direction
    
    switch (ThisEvent.EventType)
    {
    case ES_INIT:
//        // Start at PWM_0_DEG
        currentPWM = PWM_0_DEG;
        OC1RS = (float)(PR2) * currentPWM / 100;
        DB_printf("Starting PWM at %f\n", currentPWM);
//        // Post a timeout event to start advancing
//        ES_Timer_InitTimer(JOINT1_SERVO_TIMER, JOINT1_TIME_STEP);
    break;
    
    case ES_ROTATE_ZERO_Joint1:
        // Start at PWM_0_DEG
        currentPWM = PWM_0_DEG;
        decreasing = false;
        ES_Timer_InitTimer(JOINT1_SERVO_TIMER, JOINT1_TIME_STEP);
        break;
    
    case ES_ROTATE_90_Joint1:
        decreasing = true;
        ES_Timer_InitTimer(JOINT1_SERVO_TIMER, JOINT1_TIME_STEP);
        break;
    case ES_ROTATE_180_Joint1:
        break;
    case ES_TIMEOUT:
        if (decreasing) {
            if (currentPWM > PWM_90_DEG) {
                currentPWM -= PWM_STEP;
                if (currentPWM < PWM_90_DEG) {
                    currentPWM = PWM_90_DEG; // Ensure it does not exceed limit
                }
                OC1RS = (float)(PR3 + 1) * currentPWM / 100;
                //DB_printf("Decreasing PWM: %f\n", currentPWM);
                ES_Timer_InitTimer(JOINT1_SERVO_TIMER, JOINT1_TIME_STEP);
            }
        } else {
            if (currentPWM < PWM_0_DEG) {
                currentPWM += PWM_STEP;
                if (currentPWM > PWM_0_DEG) {
                    currentPWM = PWM_0_DEG; // Ensure it does not exceed limit
                }
                OC1RS = (float)(PR3 + 1) * currentPWM / 100;
                //DB_printf("Increasing PWM: %f\n", currentPWM);
                ES_Timer_InitTimer(JOINT1_SERVO_TIMER, JOINT1_TIME_STEP*5);
            }
        }
        break;
    
    default:
        break;
    }
    return ReturnEvent;
}
static void ConfigPWM_OC1() {

//     OC1 to B15
    RPB15R = 0b0101;
    //Clear OC2CON register: 
    OC1CON = 0;
    // Configure the Output Compare module for one of two PWM operation modes
    OC1CONbits.ON = 0;         // Turn off Output Compare module
    OC1CONbits.OCM = 0b110;    // PWM mode without fault pin
    OC1CONbits.OCTSEL = 0;     // Use Timer2 as the time base

    // Set the PWM duty cycle by writing to the OCxRS register
    OC1RS = PR2 * 0;       // Secondary Compare Register (for duty cycle)
    OC1R = PR2 * 0;        // Primary Compare Register (initial value)
    OC1CONbits.ON = 1;         // Enable Output Compare module
    
    return;
}
static void ConfigTimer2()
{
  // Clear the ON control bit to disable the timer.
  T2CONbits.ON = 0;
  // Clear the TCS control bit to select the internal PBCLK source.
  T2CONbits.TCS = 0;
  // Set input clock prescale to be 8:1
  T2CONbits.TCKPS = 0b011;
  // Clear the timer register TMR2
  TMR2 = 0;
  // Load PR2 with desired 16-bit match value
  PR2 = PIC_freq / (PWM_freq * prescalar_T2) - 1;
  DB_printf("PR2 is set to %d \n", PR2);

  // Clear the T2IF interrupt flag bit in the IFS2 register
  IFS0CLR = _IFS0_T2IF_MASK;
  // Disable interrupts on Timer 2
  IEC0CLR = _IEC0_T2IE_MASK;
  //turn on the timer again
  T2CONbits.ON = 1;
  return;
}