/****************************************************************************
 Module
   GameLogicFSM.c

 Revision
   1.0.1

 Description
   This is a GameLogic file for implementing flat state machines under the
   Gen2 Events and Services Framework.

 Notes

 History
 When           Who     What/Why
 -------------- ---     --------
 01/15/12 11:12 jec      revisions for Gen2 framework
 11/07/11 11:26 jec      made the queue static
 10/30/11 17:59 jec      fixed references to CurrentEvent in RunGameLogicSM()
 10/23/11 18:20 jec      began conversion from SMGameLogic.c (02/20/07 rev)
****************************************************************************/
/*----------------------------- Include Files -----------------------------*/
/* include header files for this state machine as well as any machines at the
   next lower level in the hierarchy that are sub-machines to this machine
*/
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "GameLogicFSM.h"
#include "dbprintf.h"
#include <sys/attribs.h>
#include "terminal.h"
#include "ServoService.h"
#include "BeaconIndicatorService.h"
#include "SPIMasterService.h"
/*----------------------------- Module Defines ----------------------------*/

/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this machine.They should be functions
   relevant to the behavior of this state machine
*/
static void exitGame(void);
static void enter_UnloadingCrate(void);
/*---------------------------- Module Variables ---------------------------*/
// everybody needs a state variable, you may need others as well.
// type of state variable should match htat of enum in header file
#define ActionTimeAllowed 2000
#define IdleTimeAtSetup 1000
#define InGameLED_LAT LATBbits.LATB3
#define GameTotalAllowedTime 10000
#define LinearStageSteps_unload 300 //we assume that it takes 300 steps to unload a crate
static GameLogicState_t CurrentState;

// with the introduction of Gen2, we need a module level Priority var as well
static uint8_t MyPriority;

/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
 Function
     InitGameLogicFSM

 Parameters
     uint8_t : the priorty of this service

 Returns
     bool, false if error in initialization, true otherwise

 Description
     Saves away the priority, sets up the initial transition and does any
     other required initialization for this state machine
 Notes

 Author
     J. Edward Carryer, 10/23/11, 18:55
****************************************************************************/
bool InitGameLogicFSM(uint8_t Priority)
{
  ES_Event_t ThisEvent;

  MyPriority = Priority;
  /**************************************
   * *****Initialize the pins
   *********************************/
  //for game ready button
  TRISBbits.TRISB2 = 1; //input
  ANSELBbits.ANSB2 = 0; //digital
  //for LED in game progress indicator
  TRISBbits.TRISB3 = 0; //output
  ANSELBbits.ANSB3 = 0; //digital
  LATBbits.LATB3 = 0;//initialize the led indicator as off
  // put us into the Initial PseudoState
  CurrentState = P_Init_Game_s;
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

/****************************************************************************
 Function
     PostGameLogicFSM

 Parameters
     EF_Event_t ThisEvent , the event to post to the queue

 Returns
     boolean False if the Enqueue operation failed, True otherwise

 Description
     Posts an event to this state machine's queue
 Notes

 Author
     J. Edward Carryer, 10/23/11, 19:25
****************************************************************************/
bool PostGameLogicFSM(ES_Event_t ThisEvent)
{
  return ES_PostToService(MyPriority, ThisEvent);
}

/****************************************************************************
 Function
    RunGameLogicFSM

 Parameters
   ES_Event_t : the event to process

 Returns
   ES_Event_t, ES_NO_EVENT if no error ES_ERROR otherwise

 Description
   add your description here
 Notes
   uses nested switch/case to implement the machine.
 Author
   J. Edward Carryer, 01/15/12, 15:23
****************************************************************************/
ES_Event_t RunGameLogicFSM(ES_Event_t ThisEvent)
{
  ES_Event_t ReturnEvent;
  ReturnEvent.EventType = ES_NO_EVENT; // assume no errors
  //the following runs no matter what state the game is in
  if (ThisEvent.EventType == ES_TIMEOUT && ThisEvent.EventParam == GameTotalTime_TIMER)
  {
    exitGame();
  }
  
  switch (CurrentState)
  {
    case P_Init_Game_s:        // If current state is initial Psedudo State
    {
      if (ThisEvent.EventType == ES_GAME_START_BUTTON_PRESSED)
      {
        InGameLED_LAT = 1;
        ES_Timer_InitTimer(GameTotalTime_TIMER,GameTotalAllowedTime);
        CurrentState = Wait4PIC1_Game_s;
        DB_printf("transition from P_init_game_s to Wait4PIC1_Game_s\n");
        ES_Event_t Event2Post = {ES_GAME_START_BUTTON_PRESSED, STATE_START_BUTTON_PRESSED};
        PostSPIMasterService(Event2Post); // tell PIC1 that game has started

      }
      
    }
    break;
    case Wait4PIC1_Game_s:{
      switch (ThisEvent.EventType)
      {
        case ES_SIDE_DETECTED:
        {
          PostServoService(ThisEvent);
          DB_printf("GameFSM posted side indication request to servo side indicator service \n");
          //disable the beacon detection interrupt to save CPU resources
          //IC1CONbits.ON = 0;
          ES_Event_t Event2Post = {ES_BEACON_FOUND, STATE_BEACON_FOUND};
          PostSPIMasterService(Event2Post); // tell PIC1 beacon has been detected
        }
        break;
        case ES_SPI_PIC1_UNLOADING_CUBE_S:
          CurrentState = UnloadingCrate_Game_s;
          DB_printf("transition from Wait4PIC1_Game_s to UnloadingCrate_Game_s\n");
          ES_Event_t Event2Post = {ES_SELF_TRANSITION, 0};
          PostGameLogicFSM(Event2Post);
        break;
        default:
        break;
      }
    }
    break;
    case UnloadingCrate_Game_s:
    {
      if (ThisEvent.EventType == ES_SELF_TRANSITION)
      {
        enter_UnloadingCrate();
      }else if (ThisEvent.EventType == ES_STEPPER_COMPLETE)
      {
        //CurrentState = Wait4PIC1_Game_s;
        DB_printf("stepper complete\n");
        DB_printf("transition from UnloadingCrate_Game_s to Wait4PIC1_Game_s\n");
        
      } 
    }
    break;
    default:
    break;
  }                                   // end switch on Current State
  return ReturnEvent;
}

/****************************************************************************
 Function
     QueryGameLogicSM

 Parameters
     None

 Returns
     GameLogicState_t The current state of the GameLogic state machine

 Description
     returns the current state of the GameLogic state machine
 Notes

 Author
     J. Edward Carryer, 10/23/11, 19:21
****************************************************************************/
GameLogicState_t QueryGameLogicFSM(void)
{
  return CurrentState;
}

/***************************************************************************
 private functions
 ***************************************************************************/
static void exitGame(void){
  InGameLED_LAT = 0;
  ES_Event_t Event2Post;
  Event2Post.EventType = ES_SERVO_IND_RESET;
  PostServoService(Event2Post);
  return;
}

static void enter_UnloadingCrate(void){
  ES_Event_t Event2Post = {ES_STEPPER_BWD, LinearStageSteps_unload};
  PostStepperService(Event2Post);
  return;
}