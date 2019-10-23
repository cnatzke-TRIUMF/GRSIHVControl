#ifndef HVLOGIN_HEADER
#define HVLOGIN_HEADER


#include "CrateCommands.h"
#include "CommandParser.h"

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

typedef struct Names{
   int slotNum;
   int chNum;
   char * parName;
   char * hostName;
   struct Names *next;
} Names_t;


extern HV System[];
extern Names_t nameList;

#endif
