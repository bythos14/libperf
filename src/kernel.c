#include <psp2kern/kernel/modulemgr.h>
#include <psp2kern/kernel/sysmem/uid_puid.h>
#include <psp2kern/kernel/sysroot.h>
#include <psp2kern/kernel/threadmgr.h>

#include <taihen.h>

#include "libperf.h"

enum
{
    PMON_ACTION_RESET,
    PMON_ACTION_SET_EVENT,
    PMON_ACTION_START,
    PMON_ACTION_STOP,
    PMON_ACTION_SET_COUNTER
};

static SceUID injectIds[2];

static int (*sceKernelSetInitialPMCR)(SceUID pid, SceUInt32 initialPMCR);
static int (*sceKernelSetInitialPMUSERENR)(SceUID pid, SceUInt32 PMUSERENR);
static int (*sceKernelPMonSetControlRegister)(SceUID thid, SceUInt32 PMCR);
static int (*sceKernelPMonSetUserEnableRegister)(SceUID thid, SceUInt32 PMUSERENR);
static int (*sceKernelPMonThreadSetEnableCounter)(SceUID thid, SceUInt32 PMCNTENSET);
static int (*sceKernelPMonThreadClearEnableCounter)(SceUID thid, SceUInt32 PMCNTENCLR);
static int (*sceKernelPMonThreadSetCounter)(SceUID thid, SceUInt32 counter, SceUInt32 value);
static int (*sceKernelPMonThreadSetEvent)(SceUID thid, SceUInt32 counter, SceUInt32 event);

int module_get_export_func(SceUID pid, const char *modname, uint32_t libnid, uint32_t funcnid, uintptr_t *func);

static int armPmonResetAllCounters(SceUID threadId)
{
    int ret = 0;
    for (int i = 0; i < 6; i++)
    {
        if ((ret = sceKernelPMonThreadSetCounter(threadId, i, 0)) < 0)
        {
            return ret;
        }
    }

    return sceKernelPMonThreadSetCounter(threadId, 0x1F, 0);
}

static int armPmonExecAllThread(SceUInt32 pmonAction, SceUInt32 arg0, SceUInt32 arg1)
{

    return 0;
}

int sceKernelPerfArmPmonOpen(void)
{
    int permission = 0, ret = 0;
    if ((permission = ksceKernelSetPermission(0x80)) < 0)
    {
        return permission;
    }

    if ((ret = sceKernelSetInitialPMCR(0, 0x11)) < 0)
    {
        goto exit;
    }

    if ((ret = sceKernelPMonSetControlRegister(0, 0x11)) < 0)
    {
        goto exit;
    }

    if ((ret = sceKernelSetInitialPMUSERENR(0, 0x1)) < 0)
    {
        goto exit;
    }

    if ((ret = sceKernelPMonSetUserEnableRegister(0, 0x1)) < 0)
    {
        goto exit;
    }

exit:
    ksceKernelSetPermission(permission);

    return ret;
}

int sceKernelPerfArmPmonClose(void)
{

    int permission = 0, ret = 0;
    if ((permission = ksceKernelSetPermission(0x80)) < 0)
    {
        return permission;
    }

    if ((ret = sceKernelSetInitialPMCR(0, 0)) < 0)
    {
        goto exit;
    }

    if ((ret = sceKernelPMonSetControlRegister(0, 0)) < 0)
    {
        goto exit;
    }

    if ((ret = sceKernelSetInitialPMUSERENR(0, 0)) < 0)
    {
        goto exit;
    }

    if ((ret = sceKernelPMonSetUserEnableRegister(0, 0)) < 0)
    {
        goto exit;
    }

exit:
    ksceKernelSetPermission(permission);

    return ret;
}

int sceKernelPerfArmPmonStart(SceUID threadId)
{
    if ((threadId != 0 && threadId != SCE_GUID_THREAD_ID_PROCESS_ALL && threadId != -1) &&
        (threadId = kscePUIDtoGUID(0, threadId)) < 0)
    {
        return threadId;
    }

    if (threadId != SCE_GUID_THREAD_ID_PROCESS_ALL && threadId != -1)
        return sceKernelPMonThreadSetEnableCounter(threadId, 0x8000003F);
    else
        return armPmonExecAllThread(PMON_ACTION_START, 0, 0);
}

int sceKernelPerfArmPmonStop(SceUID threadId)
{
    if ((threadId != 0 && threadId != SCE_GUID_THREAD_ID_PROCESS_ALL && threadId != -1) &&
        (threadId = kscePUIDtoGUID(0, threadId)) < 0)
    {
        return threadId;
    }

    if (threadId != SCE_GUID_THREAD_ID_PROCESS_ALL && threadId != -1)
        return sceKernelPMonThreadClearEnableCounter(threadId, 0x8000003F);
    else
        return armPmonExecAllThread(PMON_ACTION_STOP, 0, 0);
}

int sceKernelPerfArmPmonReset(SceUID threadId)
{
    if ((threadId != 0 && threadId != SCE_GUID_THREAD_ID_PROCESS_ALL && threadId != -1) && 
        (threadId = kscePUIDtoGUID(0, threadId)) < 0)
    {
        return threadId;
    }

    if (threadId != SCE_GUID_THREAD_ID_PROCESS_ALL && threadId != -1)
        return armPmonResetAllCounters(threadId);
    else
        return armPmonExecAllThread(PMON_ACTION_RESET, 0, 0);
}

int sceKernelPerfArmPmonSelectEvent(SceUID threadId, SceUInt32 counter, SceUInt8 eventCode)
{
    if ((threadId != 0 && threadId != SCE_GUID_THREAD_ID_PROCESS_ALL && threadId != -1) &&
        (threadId = kscePUIDtoGUID(0, threadId)) < 0)
    {
        return threadId;
    }

    if (threadId != SCE_GUID_THREAD_ID_PROCESS_ALL && threadId != -1)
        return sceKernelPMonThreadSetEvent(threadId, counter, eventCode);
    else
        return armPmonExecAllThread(PMON_ACTION_SET_EVENT, counter, eventCode);
}

int sceKernelPerfArmPmonSetCounterValue(SceUID threadId, SceUInt32 counter, SceUInt32 value)
{
    if ((threadId != 0 && threadId != SCE_GUID_THREAD_ID_PROCESS_ALL && threadId != -1) &&
        (threadId = kscePUIDtoGUID(0, threadId)) < 0)
    {
        return threadId;
    }

    if (threadId != SCE_GUID_THREAD_ID_PROCESS_ALL && threadId != -1)
        return sceKernelPMonThreadSetCounter(threadId, counter, value);
    else
        return armPmonExecAllThread(PMON_ACTION_SET_COUNTER, counter, value);
}

int module_start(SceSize argSize, void *argp)
{
    int swVersion = ksceKernelSysrootGetSystemSwVersion();
    if (swVersion < 0)
    {
        return SCE_KERNEL_START_FAILED;
    }

    module_get_export_func(KERNEL_PID, "SceProcessmgr", 0x746EC971, 0x61B9B6FA, (uintptr_t *)&sceKernelSetInitialPMCR);
    module_get_export_func(KERNEL_PID, "SceProcessmgr", 0x746EC971, 0xB1C3EFCA, (uintptr_t *)&sceKernelSetInitialPMUSERENR);
    module_get_export_func(KERNEL_PID, "SceKernelThreadMgr", 0xE2C40624, 0x1AAFA818, (uintptr_t *)&sceKernelPMonSetControlRegister);
    module_get_export_func(KERNEL_PID, "SceKernelThreadMgr", 0xE2C40624, 0x5053B005, (uintptr_t *)&sceKernelPMonSetUserEnableRegister);

    if (swVersion >= 0x03630011) // 3.65
    {
        module_get_export_func(KERNEL_PID, "SceKernelThreadMgr", 0x7F8593BA, 0x7F831213, (uintptr_t *)&sceKernelPMonThreadSetEnableCounter);
        module_get_export_func(KERNEL_PID, "SceKernelThreadMgr", 0x7F8593BA, 0x1D2A6815, (uintptr_t *)&sceKernelPMonThreadClearEnableCounter);
        module_get_export_func(KERNEL_PID, "SceKernelThreadMgr", 0x7F8593BA, 0x7B3368F1, (uintptr_t *)&sceKernelPMonThreadSetCounter);
        module_get_export_func(KERNEL_PID, "SceKernelThreadMgr", 0x7F8593BA, 0xFFB9CD24, (uintptr_t *)&sceKernelPMonThreadSetEvent);
    }
    else
    {
        module_get_export_func(KERNEL_PID, "SceKernelThreadMgr", 0xA8CA0EFD, 0x72E5DA4E, (uintptr_t *)&sceKernelPMonThreadSetEnableCounter);
        module_get_export_func(KERNEL_PID, "SceKernelThreadMgr", 0xA8CA0EFD, 0x43D13895, (uintptr_t *)&sceKernelPMonThreadClearEnableCounter);
        module_get_export_func(KERNEL_PID, "SceKernelThreadMgr", 0xA8CA0EFD, 0xD2BE5EFB, (uintptr_t *)&sceKernelPMonThreadSetCounter);
        module_get_export_func(KERNEL_PID, "SceKernelThreadMgr", 0xA8CA0EFD, 0x6ECCDCBD, (uintptr_t *)&sceKernelPMonThreadSetEvent);
    }

    tai_module_info_t moduleInfo = {0};
    moduleInfo.size = sizeof(moduleInfo);
    taiGetModuleInfoForKernel(KERNEL_PID, "SceProcessmgr", &moduleInfo);

    // Patch 2 specific checks for dipsw 0xe4 in SceProcessmgr
    uint16_t nops[3] = {0xBF00, 0xBF00, 0xBF00};
    injectIds[0] = taiInjectDataForKernel(KERNEL_PID, moduleInfo.modid, 0, 0x6102, nops, sizeof(nops)); // sceKernelSetInitialPMUSERENR
    if (injectIds[0] < 0)
        return SCE_KERNEL_START_FAILED;
    injectIds[1] = taiInjectDataForKernel(KERNEL_PID, moduleInfo.modid, 0, 0x61A2, nops, sizeof(nops)); // sceKernelSetInitialPMCR
    if (injectIds[1] < 0)
    {
        taiInjectReleaseForKernel(injectIds[0]);
        return SCE_KERNEL_START_FAILED;
    }

    return SCE_KERNEL_START_SUCCESS;
}

int module_stop(SceSize argSize, void *argp)
{
    taiInjectReleaseForKernel(injectIds[0]);
    taiInjectReleaseForKernel(injectIds[1]);
    return SCE_KERNEL_STOP_SUCCESS;
}