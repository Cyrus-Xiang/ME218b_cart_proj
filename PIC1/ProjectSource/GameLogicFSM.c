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
#include "MotorService.h"
#include "TapeFSM.h"

/*----------------------------- Module Defines ----------------------------*/

/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this machine.They should be functions
   relevant to the behavior of this state machine
*/

/*---------------------------- Module Variables ---------------------------*/
// everybody needs a state variable, you may need others as well.
// type of state variable should match htat of enum in header file
#define HitWallDetectTime 5000 //after this amount of time, if nothing happens we say that we have hit the wall
#define IdleTimeAtSetup 3000
#define RotateGuranteeTime 1000 //for the time between we send out rotate command and find tape command in aligning to stack state
#define TapeFollowGuranteeTime 800 //time that is guranteed for tape following to be executed 
#define tape_follow_speed 65 // speed for tape following in duty cycle (max=100)
#define rotate_speed 35 // speed for rotating in duty cycle (max=100)

static GameLogicState_t CurrentState;

// with the introduction of Gen2, we need a module level Priority var as well
static bool RotateDir = 0; //0 means CCW and 1 means CW
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
  // put us into the Initial PseudoState
  CurrentState = P_Init_Game_s;
  //set TRIS and LATS
  TRISBbits.TRISB11 = 1; //set B11 as input
  //ES_Timer_InitTimer(IdleSetup_TIMER, IdleTimeAtSetup);
  ES_Timer_InitTimer(GameLogicTest_TIMER,IdleTimeAtSetup+800);
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
  // if (ThisEvent.EventParam == GameLogicTest_TIMER)
  // {
  //   ES_Event_t Event2Post;
  //   Event2Post.EventType = ES_GAME_START_BUTTON_PRESSED;
  //   PostGameLogicFSM(Event2Post);
  //   Event2Post.EventType = ES_BEACON_FOUND;
  //   PostGameLogicFSM(Event2Post);
  // }
  
  switch (CurrentState)
  {
    case P_Init_Game_s:        // If current state is initial Psedudo State
    {
      // if (ThisEvent.EventType == ES_TIMEOUT && ThisEvent.EventParam == IdleSetup_TIMER)
        if (ThisEvent.EventType == ES_GAME_START_BUTTON_PRESSED)
        {
        CurrentState = Setup_Game_s;
        DB_printf("GameLogicFSM: Transition to Setup_Game_s\n");
        ES_Timer_InitTimer(ActionAllowedTime_TIMER, HitWallDetectTime);
        ES_Event_t Event2Post;
        Event2Post.EventType = ES_MOTOR_CW_CONTINUOUS;
        Event2Post.EventParam = rotate_speed;
        PostMotorService(Event2Post);
        DB_printf("motor service posted, turning cw\n");
        ES_Timer_InitTimer(ActionAllowedTime_TIMER, HitWallDetectTime);
        DB_printf("action allowed timer started with %d ms\n", HitWallDetectTime);
      }
      
    }
    case Setup_Game_s:        // If current state is initial Psedudo State
    {
      if (ThisEvent.EventType == ES_TIMEOUT && ThisEvent.EventParam == ActionAllowedTime_TIMER)
      {
        ES_Event_t Event2Post;
        Event2Post.EventType = ES_MOTOR_CCW_CONTINUOUS;
        Event2Post.EventParam = rotate_speed;
        PostMotorService(Event2Post);
        DB_printf("motor service posted, turning ccw\n");
      }else if (ThisEvent.EventType == ES_BEACON_FOUND) 
      {
        CurrentState = FindTape_Game_s;
        ES_Timer_StopTimer(ActionAllowedTime_TIMER);
        DB_printf("Transition from Setup_Game_s to FindTape_Game_s\n");
        ES_Event_t Event2Post;
        Event2Post.EventType = ES_TAPE_LookForTape;
        PostTapeFSM(Event2Post);
        DB_printf("tape service posted, looking for tape\n");
        Event2Post.EventType = ES_MOTOR_CW_CONTINUOUS;
        Event2Post.EventParam = rotate_speed;
        PostMotorService(Event2Post);
        DB_printf("motor service posted, turning cw\n");
        //start the timer again for detecting hitting the wall and this time we look for tape instead of beacon
        ES_Timer_InitTimer(ActionAllowedTime_TIMER, HitWallDetectTime);
      }
    }
    break;
    case FindTape_Game_s:        // If current state is state one
    {
      if (ThisEvent.EventType == ES_TAPE_FOUND)
      {
        CurrentState = GoToInterB_Game_s;
        DB_printf("Transition from FindTape_Game_s to GoToInterB_Game_s\n");
        ES_Event_t Event2Post;
        Event2Post.EventType = ES_MOTOR_STOP;
        PostMotorService(Event2Post);
        DB_printf("motor service posted, stopping\n");
        Event2Post.EventType = ES_TAPE_STOP;
        PostTapeFSM(Event2Post);
        Event2Post.EventType = ES_TAPE_FOLLOW_REV;
        Event2Post.EventParam = tape_follow_speed;
        PostTapeFSM(Event2Post);
        DB_printf("tape service posted, following tape in reverse\n");
        ES_Timer_InitTimer(ActionAllowedTime_TIMER, TapeFollowGuranteeTime);
      }else if (ThisEvent.EventType == ES_TIMEOUT && ThisEvent.EventParam == ActionAllowedTime_TIMER)
      {
        ES_Event_t Event2Post = {ES_MOTOR_CCW_CONTINUOUS, rotate_speed};
        PostMotorService(Event2Post);
        DB_printf("hit the wall detected, motor service posted, turning ccw\n");
      }
      
      
    }
    break;
    case AligningToLine_Game_s:
    {
      if (ThisEvent.EventType == ES_TIMEOUT && ThisEvent.EventParam == ActionAllowedTime_TIMER)
      {
        CurrentState = GoToInterB_Game_s;
        DB_printf("cart is assumed to be aligned to line (tape) already\n");
        DB_printf("Transition from AligningToLine_Game_s to GoToInterB_Game_s\n");
      }
    }
    case GoToInterB_Game_s:
    {
      DB_printf("GoToInterB_Game_s and received an event\n");
      if (ThisEvent.EventType == ES_RIGHT_INTERSECTION_DETECT)
      {
        CurrentState = AligningToStack_Game_s;
        DB_printf("Transition from GoToInterB_Game_s to AligningToStack_Game_s\n");
        ES_Event_t Event2Post;
        Event2Post.EventType = ES_TAPE_STOP;
        PostTapeFSM(Event2Post);
        DB_printf("tape service posted, stopping\n");
        Event2Post.EventType = ES_MOTOR_CCW_CONTINUOUS;
        Event2Post.EventParam = rotate_speed;
        PostMotorService(Event2Post);
        DB_printf("motor service posted, turning ccw\n");
        ES_Timer_InitTimer(ActionAllowedTime_TIMER, RotateGuranteeTime);
        DB_printf("rotate action allowed timer started with %d ms\n", RotateGuranteeTime);

      }
      
    }
    break;
    case AligningToStack_Game_s:
    {
      DB_printf("an event received while in AligningToStack_Game_s\n");
      if (ThisEvent.EventType == ES_TIMEOUT && ThisEvent.EventParam == ActionAllowedTime_TIMER)
      {
        ES_Event_t Event2Post;
        Event2Post.EventType = ES_TAPE_LookForTape;
        PostTapeFSM(Event2Post);
        DB_printf("rotate action allowed  timer expired \n");
        DB_printf("tape service posted, looking for tape\n");
      }

      if (ThisEvent.EventType == ES_TAPE_FOUND)
      {
        DB_printf("tape found EVENT received\n");
        CurrentState = GoingToStack_Game_s;
        DB_printf("GameLogicFSM: Transition to GoingToStack_Game_s\n");
        ES_Event_t Event2Post;
        Event2Post.EventType = ES_MOTOR_STOP;
        PostMotorService(Event2Post);
        DB_printf("motor service posted, stopping\n");
        Event2Post.EventType = ES_TAPE_FOLLOW_REV;
        Event2Post.EventParam = tape_follow_speed;
        PostTapeFSM(Event2Post);
        DB_printf("tape service posted, following tape in reverse\n");
      }
      
    }
    break;
    case GoingToStack_Game_s:
    {
      if (ThisEvent.EventType == ES_TAPE_FAIL)
      {
        DB_printf("taking a baby step\n");
        ES_Event_t Event2Post = {ES_MOTOR_BABY_STEP, 0};
        PostMotorService(Event2Post);
      }else if (ThisEvent.EventType==ES_MOTOR_MISSION_COMPLETE)
      {
        /* code */
      }
      
      
    }
    break;
    case UnloadingCrate_Game_s:
    {

    }
    break;
    case PickingUpCrate_Game_s:
    {

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

