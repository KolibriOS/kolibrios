/*
** Starscream 680x0 emulation library
** Copyright 1997, 1998, 1999 Neill Corlett
**
** Refer to STARDOC.TXT for terms of use, API reference, and directions on
** how to compile.
*/

#ifndef __CPUDEBUG_H__
#define __CPUDEBUG_H__

#ifdef __cplusplus
extern "C" {
#endif

int cpudebug_disabled(void);
int cpudebug_interactive(
	int cpun,
	void (*put)(const char*),
	void (*get)(char*, int),
	void (*execstep)(void),
	void (*dump)(void)
);

#ifdef __cplusplus
}
#endif

#endif
