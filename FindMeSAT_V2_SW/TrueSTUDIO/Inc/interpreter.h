/*
 * interpreter.h
 *
 *  Created on: 01.05.2018
 *      Author: DF4IAH
 */

#ifndef INTERPRETER_H_
#define INTERPRETER_H_
#ifdef __cplusplus
 extern "C" {
#endif


/* Command types for the interpreterOutQueue */
typedef enum InterOutQueueCmds {
  InterOutQueueCmds__NOP              = 0,
  InterOutQueueCmds__DoSendDataUp,
  InterOutQueueCmds__LinkCheckReq,
  InterOutQueueCmds__DeviceTimeReq,
  InterOutQueueCmds__ConfirmedPackets,
  InterOutQueueCmds__ADRset,
  InterOutQueueCmds__DRset,
  InterOutQueueCmds__PwrRedDb,
} InterOutQueueCmds_t;


void interpreterPrintHelp(void);
void interpreterShowCursor(void);

void interpreterInterpreterTaskInit(void);
void interpreterInterpreterTaskLoop(void);


#ifdef __cplusplus
}
#endif
#endif /* INTERPRETER_H_ */
