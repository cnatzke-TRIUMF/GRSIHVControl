#ifndef CRATECOMMAND_HEADER
#define CRATECOMMAND_HEADER

// INCLUDES
#include <signal.h>
#include <sys/types.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>

#include "CAENHVWrapper.h"

// constants
#define MAX_HVPS (5) // maximum of 5 HV crates at one time
#define MAX_PAR_NAME 10
#define MAX_CH_NAME 12

// prototypes
void HVSystemLogin(const char * hvSysName, const char * userName, const char * passwd);
void HVSystemLogout(void);
void WriteToXML(const char *hvSysName, const int hvSysHandle, ushort NumSlots, ushort ChannelsInSlot[], const char *filename);
void ChangeParameter(const int hvSysHandle, unsigned short slotNum, unsigned short chNum, float chNew, const char * parName);
void ChangeVoltage(const int hvSysHandle, const char * hvSysName, const char * chanName, double chNew, unsigned short NrOfSlots, unsigned short ChList[]);
void AdjustVoltage(const int hvSysHandle, const char * hvSysName, const char * chanName, float chNew, unsigned short NrOfSlots, unsigned short ChList[]);

#endif
