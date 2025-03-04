/****************************************************************************

  Header file for GameLogic Flat Sate Machine
  based on the Gen2 Events and Services Framework

 ****************************************************************************/

#ifndef FSMGameLogic_H
#define FSMGameLogic_H

// Event Definitions
#include "ES_Configure.h" /* gets us event definitions */
#include "ES_Types.h"     /* gets bool type for returns */

// typedefs for the states
// State definitions for use with the query function
typedef enum
{
  P_Init_Game_s,Wait4PIC1_Game_s, UnloadingCrate_Game_s
}GameLogicState_t;

// Public Function Prototypes

bool InitGameLogicFSM(uint8_t Priority);
bool PostGameLogicFSM(ES_Event_t ThisEvent);
ES_Event_t RunGameLogicFSM(ES_Event_t ThisEvent);
GameLogicState_t QueryGameLogicSM(void);

#endif /* FSMGameLogic_H */

