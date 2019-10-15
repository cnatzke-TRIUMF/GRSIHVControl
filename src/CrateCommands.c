
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <stdbool.h>

#include "CAENHVWrapper.h"
#include "GRSIVoltageControl.h"
#include "CrateCommands.h"

#define MAX_PASSWORD_LENGTH 20

//==============================================================================
// noHVPS - Checks if anything is logged in
// Internal Function
//==============================================================================
static int noHVPS(void)
{
   int i = 0;

   while(System[i].ID == -1 && i != (MAX_HVPS - 1)) i++;
   return ((i == MAX_HVPS - 1) ? 1 : 0);
}

//==============================================================================
// OneHVPS - Checks for one crate
// Internal Function
//==============================================================================
static int OneHVPS(void)
{
   int i, j, k;

   for(i = 0, k = 0; i < (MAX_HVPS -1); i++)
      if(System[i].ID != -1){
         j = i;
         k++;
      }
   return ((k != 1) ? -1 : j);
}

//==============================================================================
// HVSystemLogin - logs into HVCrate
//
// Params:
// hostname - Human readable crate name (e.g. grifhv03)
// userName - Login username 
// passwd   - Login password
//==============================================================================

void HVSystemLogin(const char * hostname, const char * userName, const char * passwd)
{
   char hostIP[30];
   int i, link;
   int sysHandle = -1;
   int sysType = -1;
   CAENHVRESULT ret;

   i=0;
   while( System[i].ID != -1 && i != (MAX_HVPS - 1)) i++;
   if ( i == MAX_HVPS -1 ){
      printf("Could not find system ID\n");
      return;
   }

   // matching hostname to host IP
   if(strcmp(hostname, "grifhv03") == 0){
      strcpy(hostIP, "142.90.121.131");
      sysType = 2;
   }
   else if(strcmp(hostname, "tighv00") == 0){
      strcpy(hostIP, "142.90.97.225");
      sysType = 0;
   }
   else {
      printf("No hostname found. Cannot complete login procedure.");
      return;
   }

   // TCP/IP connection to crates, more are available but we are only using TCP/IP
   // connections
   link = LINKTYPE_TCPIP;

   printf("::: Attempting to log into HV Crate\n");
   if( sysType == 0 || sysType == 2 ){

      ret = CAENHV_InitSystem((CAENHV_SYSTEM_TYPE_t)sysType, link, hostIP, userName, passwd, &sysHandle);

      printf("CAENHV_InitSystem: %s\n\n", CAENHV_GetError(sysHandle));

      if( ret == CAENHV_OK )
      {
         i = 0;
         while( System[i].ID != -1 ) i++;
         System[i].ID = ret;
         System[i].Handle = sysHandle;
      }
   }
   else{
      printf("ERROR: Bad system type. \n Did you connect to the right system?\n");
   }
}

//==============================================================================
// HVSystemLogout - logs out of HVCrate
//==============================================================================

void HVSystemLogout()
{
   int handle = -1;
   int i;
   CAENHVRESULT ret;

   printf("\n::: Attempting to log out of crate.\n");

   if( noHVPS() ) return;

   if( (i = OneHVPS()) >= 0) handle = System[i].Handle;

   ret = CAENHV_DeinitSystem(handle);
   if(ret == CAENHV_OK){
      printf("CAENHV_DeinitSystem: Connection closed\n");
   }else{
      printf("CAENHV_DeinitSystem: %s\n", CAENHV_GetError(handle));
   }

   if(ret == CAENHV_OK){
      for(i = 0; System[i].ID != -1; i++){
         System[i].ID = System[i+1].ID;
         System[i].Handle = System[i+1].Handle;
      }
   }

   printf("::: Logout successful.\n");
   return;
}

//==============================================================================
// WriteToXML - Writes status of entire crate to XML file
//
// Inputs: 
// hvSysName      - Human readable name of crate (e.g. grifhv03, tighv00, etc.)
// hvSysHandle    - Handle of hv system, contained in System list
// NumSlots       - Number of slots in crate
// ChannelsInSlot - List of channels in the slot
// filename       - Name of output file
//==============================================================================
void WriteToXML(const char *hvSysName, const int hvSysHandle, ushort NumSlots, ushort ChannelsInSlot[], const char *filename)
{
   int num_param;
   ushort i, j, k;
   ushort *chan_list;
   unsigned type;
   char (*chan_name_list)[MAX_CH_NAME];
   char *param_name_list, *param_ptr;
   CAENHVRESULT returnCode;
   struct Property property;
   FILE * outFile;

   printf("::: Generating Parameter Map for %s\n", (char *) hvSysName);
   // opens file named <filename>
   outFile = fopen(filename, "w");
   if( outFile == NULL){
      fprintf(stderr, "Could not write file %s.xml", (char *) hvSysName);
      return;
   }

   fprintf(outFile, "<crate name=\"%s\">\n", hvSysName);
   for(i = 0; i < NumSlots; i++){
      // allocating memory
      chan_list = malloc(ChannelsInSlot[i]*sizeof(ushort));
      chan_name_list = malloc(ChannelsInSlot[i]*MAX_CH_NAME*sizeof(char));

      // ensure memory was allocated
      if(chan_list == NULL){
         fprintf(stderr, "Could not allocate memory for chan_list in WriteToXML");
         return;
      }
      if(chan_name_list == NULL){
         fprintf(stderr, "Could not allocate memory for chan_name_list in WriteToXML");
         return;
      }

      // places channel number for each channel in chan_list
      for(j = 0; j < ChannelsInSlot[i]; j++) chan_list[j] = j;

      //ensure names are grabbed for slots with non-zero channels
      if(ChannelsInSlot[i]){
         fprintf(outFile, "\t<slot num=\"%u\">\n", i);
         returnCode = CAENHV_GetChName(hvSysHandle, i, ChannelsInSlot[i], chan_list, chan_name_list);
         if(returnCode){
            fprintf(stderr, "ERROR %#X: %s\n", returnCode, CAENHV_GetError(hvSysHandle));
            fprintf(stderr, "slot:%d chans:%d\n", i, ChannelsInSlot[i]);
            free(chan_list);
            free(chan_name_list);
            return;
         }
      }

      // loop over every channel in slot
      for(j = 0; j < ChannelsInSlot[i]; j++){
         fprintf(outFile, "\t\t<channel num=\"%u\">\n",j);
         fprintf(outFile, "\t\t\t<Name>%s</Name>\n",chan_name_list[j]);
         returnCode = CAENHV_GetChParamInfo(hvSysHandle, i, j, &param_name_list, &num_param);
         if(returnCode){
            fprintf(stderr, "ERROR %#X: %s\n", returnCode, CAENHV_GetError(hvSysHandle));
            fprintf(stderr, "chan\n");
            free(chan_list);
            free(chan_name_list);
            return;
         }

         // gets properties
         // starts param_ptr at beginning of list and advances by MAX_PAR_NAME
         for(k = 0, param_ptr = param_name_list; k < num_param; k++, param_ptr += MAX_PAR_NAME){
            // ensures everything is valid
            returnCode = CAENHV_GetChParamProp(hvSysHandle, i, j, param_ptr, "Type", &type);
            if(returnCode){
               fprintf(stderr, "ERROR %#X: %s\n", returnCode, CAENHV_GetError(hvSysHandle));
               fprintf(stderr, "slot:%u    chan:%u    k:%u    param:%s", i, j, k, param_ptr);
               free(chan_list);
               free(chan_name_list);
               return;
            }

            returnCode = CAENHV_GetChParam(hvSysHandle, i, param_ptr, 1, &j, &(property.value));
            if(returnCode){
               fprintf(stderr, "ERROR %#X: %s\n", returnCode, CAENHV_GetError(hvSysHandle));
               fprintf(stderr, "slot:%u    chan:%u    k:%u    param:%s", i, j, k, param_ptr);
               free(chan_list);
               free(chan_name_list);
               return;
            }

            // checks parameter type and outputs the value based on type
            // numerical -> floating point value
            if(type == PARAM_TYPE_NUMERIC){
               fprintf(outFile, "\t\t\t<%s>%.3f</%s>\n", param_ptr, property.value.numValue, param_ptr);
            }
            else if(type == PARAM_TYPE_ONOFF){
               // boolean write "ON for true, "OFF" for false
               if(property.value.bitValue){
                  fprintf(outFile, "\t\t\t<%s>ON</%s>\n", param_ptr, param_ptr);
               }
               else{
                  fprintf(outFile, "\t\t\t<%s>OFF</%s>\n", param_ptr, param_ptr);
               }
            }

            // outputs bit field if it is a channel status
            else if(type == PARAM_TYPE_CHSTATUS){
               fprintf(outFile, "\t\t\t<%s>%u</%s>\n", param_ptr, property.value.bitValue, param_ptr);
            }
         }
         fprintf(outFile, "\t\t</channel>\n");
      }

      if(ChannelsInSlot[i]){
         fprintf(outFile, "\t</slot>\n");
      }
   }

   // close file 
   fprintf(outFile, "\t</crate>\n");
   fclose(outFile);
   printf("::: Done\n");

}

//==============================================================================
// ChangeParameter
//
// description - Changes a parameter of a single channel
//==============================================================================
void ChangeParameter(const int hvSysHandle, unsigned short slotNum, unsigned short chNum, float chNew, const char * parName)
{
   CAENHVRESULT returnCode; 
   float chValue; 

   returnCode = CAENHV_GetChParam(hvSysHandle, slotNum, parName, 1, &chNum, (void *)&chValue);
   if(returnCode){
      fprintf(stderr, "ERROR %#X: %s\n", returnCode, CAENHV_GetError(hvSysHandle));
   }
   returnCode = CAENHV_SetChParam(hvSysHandle, slotNum, parName, 1, &chNum, (void *)&chNew);
   if(returnCode){
      fprintf(stderr, "ERROR %#X: %s\n", returnCode, CAENHV_GetError(hvSysHandle));
   }
   printf("Slot %u Channel %u %s Changed from %f to %f \n",slotNum, chNum, parName, chValue, chNew);
}

//==============================================================================
// ChangeVoltage
//
// description - Changes voltage  of a single channel
//==============================================================================
void ChangeVoltage(const int hvSysHandle, const char * hvSysName, const char * chanName, double chNew, unsigned short NrOfSlots, unsigned short ChList[])
{
   CAENHVRESULT returnCode; 
   char chName[12];
   const char * parName = "V0Set";
   float chValue; 
   unsigned short i = 1;
   unsigned short j = 0;
   bool chFound = false;

   // varifying we can find the channel of interest
   for (i = 0; i < NrOfSlots; i++){
      for (j = 0; j < ChList[i]; j++){
         returnCode = CAENHV_GetChName(hvSysHandle, i, 1, &j, (char (*)[12]) chName);
         if(returnCode){
            fprintf(stderr, "ERROR %#X: %s\n", returnCode, CAENHV_GetError(hvSysHandle));
         }
         if(strcmp(chName, chanName) == 0){
            printf("Found %s in %s Slot %i Channel %i \n", chanName, (char *) hvSysName, i, j);
            chFound = true;
         }
         if(chFound) break;
      }
      if(chFound) break;
   }
   
   if(!chFound){
      printf("%s not found in %s \n", chanName, (char *) hvSysName);
      return;
   }

   returnCode = CAENHV_GetChParam(hvSysHandle, i, parName, 1, &j, (void *) &chValue);
   if(returnCode){
      fprintf(stderr, "ERROR %#X: %s\n", returnCode, CAENHV_GetError(hvSysHandle));
   }
   returnCode = CAENHV_SetChParam(hvSysHandle, i, parName, 1, &j, (void *) &chNew);
   if(returnCode){
      fprintf(stderr, "ERROR %#X: %s\n", returnCode, CAENHV_GetError(hvSysHandle));
   }
   printf("%s Changed from %f to %f \n", parName, chValue, chNew);

}

//==============================================================================
// AdjustVoltage
//
// description - adjusts voltage  of a single channel
//==============================================================================
void AdjustVoltage(const int hvSysHandle, const char* hvSysName, const char * chanName, float chNew, unsigned short NrOfSlots, unsigned short ChList[])
{
   CAENHVRESULT returnCode; 
   char chName[12];
   const char * parName = "V0Set";
   float chValue; 
   unsigned short i = 1;
   unsigned short j = 0;
   bool chFound = false;

   printf("Here: %f\n", chNew);
   // varifying we can find the channel of interest
   for (i = 0; i < NrOfSlots; i++){
      for (j = 0; j < ChList[i]; j++){
         returnCode = CAENHV_GetChName(hvSysHandle, i, 1, &j, (char (*)[12]) chName);
         if(returnCode){
            fprintf(stderr, "ERROR %#X: %s\n", returnCode, CAENHV_GetError(hvSysHandle));
         }
         if(strcmp(chName, chanName) == 0){
            printf("Found %s in %s Slot %i Channel %i \n", chanName, (char *) hvSysName, i, j);
            chFound = true;
         }
         if(chFound) break;
      }
      if(chFound) break;
   }
   
   if(!chFound){
      printf("%s not found in %s \n", chanName, (char *) hvSysName);
      return;
   }

   returnCode = CAENHV_GetChParam(hvSysHandle, i, parName, 1, &j, (void *) &chValue);
   if(returnCode){
      fprintf(stderr, "ERROR %#X: %s\n", returnCode, CAENHV_GetError(hvSysHandle));
   }

   // Setting new voltage and setting lower limit of voltage
   chNew += chValue;
   if(chNew < 600) chNew = 600;

   returnCode = CAENHV_SetChParam(hvSysHandle, i, parName, 1, &j, &chNew);
   if(returnCode){
      fprintf(stderr, "ERROR %#X: %s\n", returnCode, CAENHV_GetError(hvSysHandle));
   }
   printf("%s Changed from %f to %f \n", parName, chValue, chNew);

}
