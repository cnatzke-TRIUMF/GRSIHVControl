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
#define LENGTH 30
// file parse size
#define MAX_SIZE 100

// prototypes
void HVSystemLogin(const char * hvSysName, const char * userName, const char * passwd);
void HVSystemLogout(void);
void WriteToXML(const char *hvSysName, const int hvSysHandle, ushort NumSlots, ushort ChannelsInSlot[], const char *filename);
void ChangeParameter(const int hvSysHandle, unsigned short slotNum, unsigned short chNum, float chNew, const char * parName);
int SetCrateVoltage(const int hvSysHandle, const char* hvSysName, const char * inFile, unsigned short NrOfSlots, unsigned short ChList[]);
void SetChannelVoltage(const int hvSysHandle, const char * hvSysName, const char * chanName, float chNew, unsigned short NrOfSlots, unsigned short ChList[]);
void AdjustChannelVoltage(const int hvSysHandle, const char * hvSysName, const char * chanName, float chNew, unsigned short NrOfSlots, unsigned short ChList[]);
int AdjustCrateVoltage(const int hvSysHandle, const char * inFile, unsigned short NrOfSlots, unsigned short ChList[]);
void ToggleUpChannels(const int hvSysHandle, const char* hvSysName, const char * chanType, unsigned state, unsigned short NrOfSlots, unsigned short ChList[]);
int ToggleTigChannels(const int hvSysHandle, const char* hvSysName, const char * fileName, const char * chanType, unsigned short NrOfSlots, unsigned short ChList[]);
void ToggleChPower(const int hvSysHandle, const char* hvSysName, const char * chanName, unsigned state, unsigned short NrOfSlots, unsigned short ChList[]);
void TogglePower(const int hvSysHandle, const char* hvSysName, const char * chanName, unsigned state, unsigned short i, unsigned short j);
void ChangeChannelName(const int hvSysHandle, unsigned short slotNum, unsigned short chNum, const char * parName);
int ChangeCrateName(const int hvSysHandle, const char * inFile);
int CountFileLines(FILE * inFile);
int CountFileColumns(FILE * inFile);

struct voltage_node {
   char * chName;
   int deltaV;
   char * hostName;
   struct voltage_node* next;
};
typedef struct voltage_node VoltageNode;

typedef struct Names{
   int slotNum;
   int chNum;
   char * parName;
   char * hostName;
   struct Names *next;
} Names_t;

#endif
