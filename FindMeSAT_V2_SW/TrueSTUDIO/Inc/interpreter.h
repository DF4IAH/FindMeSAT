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


void interpreterPrintHelp(void);
void interpreterShowCursor(void);

void interpreterInterpreterTaskInit(void);
void interpreterInterpreterTaskLoop(void);


#ifdef __cplusplus
}
#endif
#endif /* INTERPRETER_H_ */
