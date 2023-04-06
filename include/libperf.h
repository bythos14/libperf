#ifndef _PSP2_LIBPERF_H_
#define _PSP2_LIBPERF_H_

#include <psp2/types.h>

#ifdef __cplusplus
extern "C" {
#endif

int scePerfArmPmonReset(SceUID threadId);
int scePerfArmPmonSelectEvent(SceUID threadId, SceUInt32 counter, SceUInt8 eventCode);
int scePerfArmPmonStart(SceUID threadId);
int scePerfArmPmonStop(SceUID threadId);
int scePerfArmPmonGetCounterValue(SceUID threadId, SceUInt32 counter, SceUInt32 *value);
int scePerfArmPmonSetCounterValue(SceUID threadId, SceUInt32 counter, SceUInt32 value);
int scePerfArmPmonSoftwareIncrement(SceUInt32 mask);

#ifdef __cplusplus
}
#endif

#endif