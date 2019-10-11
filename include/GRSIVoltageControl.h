#ifndef HVLOGIN_HEADER
#define HVLOGIN_HEADER

// INCLUDES
#include <signal.h>
#include <sys/types.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

// constants
#define MAX_HVPS (5) // maximum of 5 HV crates at one time
#define MAX_PAR_NAME 10
#define MAX_CH_NAME 12

// structure for information of each system
typedef struct sys
{
   int Handle;
   int ID;
} HV;

struct Property{
   unsigned long type, mode;
   float minval, maxval;
   unsigned short unit;
   short exp;
   char name[30], onstate[30], offstate[30];
   union Value{
      float numValue;      // if property has a numeric value
      int bitValue;        // if property has a boolean/bit value
   }value;
};

// prototypes
void HVSystemLogin(const char * hvSysName, const char * userName, const char * passwd);
void HVSystemLogout(void);
void WriteToXML(const char *hvSysName, const int hvSysHandle, ushort NumSlots, ushort ChannelsInSlot[], const char *filename);
void ChangeParameter(const int hvSysHandle, unsigned short slotNum, unsigned short chNum, float chNew, const char * parName);
void ChangeVoltage(const int hvSysHandle, const int * hvSysName, const char * chanName, float chNew, unsigned short NrOfSlots, unsigned short ChList[]);
void AdjustVoltage(const int hvSysHandle, const int * hvSysName, const char * chanName, float chNew, unsigned short NrOfSlots, unsigned short ChList[]);

int SegFaultFinish(int sig);
int InterruptFinish(int sig);

extern HV System[];

#endif
