
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

   // 1527 -> 0, 4527 -> 2
   // matching hostname to host IP
   if(strcmp(hostname, "grifhv00") == 0){
      strcpy(hostIP, "142.90.119.252");
      sysType = 0;
   }
   else if(strcmp(hostname, "grifhv02") == 0){
      strcpy(hostIP, "142.90.127.153");
      sysType = 2;
   }
   else if(strcmp(hostname, "grifhv03") == 0){
      strcpy(hostIP, "142.90.121.131");
      sysType = 2;
   }
   else if(strcmp(hostname, "grifhv04") == 0){
      strcpy(hostIP, "142.90.121.137");
      sysType = 2;
   }
   else if(strcmp(hostname, "tighv00") == 0){
      strcpy(hostIP, "142.90.97.143");
      sysType = 0;
   }
   else if(strcmp(hostname, "tighv01") == 0){
      strcpy(hostIP, "142.90.97.225");
      sysType = 0;
   }
   else if(strcmp(hostname, "tighv02") == 0){
      strcpy(hostIP, "142.90.97.226");
      sysType = 0;
   }
   else {
      printf("No hostname found. Cannot complete login procedure.");
      return;
   }

   // TCP/IP connection to crates, more are available but we are only using TCP/IP
   // connections
   link = LINKTYPE_TCPIP;

   //printf("::: Attempting to log into HV Crate\n");
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

   //printf("\n::: Attempting to log out of crate.\n");

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

   //printf("::: Logout successful.\n");
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
// SetCrateVoltage
//
// Description - Sets the voltage of channels in a crate from a list
//
// @param hvSysHandle   The system handle
// @param fileName      Path of file containing voltage information
//==============================================================================
int SetCrateVoltage(const int hvSysHandle, const char * hvSysName, const char * fileName, unsigned short NrOfSlots, unsigned short ChList[])
{
   VoltageNode *first;     // Initial voltlist (contains junk)
   VoltageNode *voltList;  // VoltageNode being filled 
   VoltageNode *last;      // Previously filled VoltageNode
   char line[MAX_SIZE],chName[MAX_SIZE];
   int deltaV;

   FILE * inFile = fopen(fileName, "r");
   if (inFile == NULL){
      printf("Could not open file %s\n", fileName);
      return 1;
   }

   //int total_rows = CountFileLines(inFile);
   //printf("Found %i lines.\n", total_rows);

   // allocate new node to hold data
   first = malloc(sizeof(VoltageNode));
   if (first == NULL) {
      printf("Ran out of memory\n");
      return 1;
   }
   last = first;
   first->next = NULL;

   while (fgets(line, sizeof(line), inFile) != NULL){
      // safety Checks
      if (line[0] == '\0'){
         printf("Line too short\n");
         return 1;
      }
      if (line[strlen (line)-1] != '\n'){
         printf("Line starting with '%s' is too long\n", line);
         return 1;
      }

      line[strlen(line)-1] = '\0';

      // scan fields
      if (sscanf(line, "%[^,],%i", chName, &deltaV) != 2){
         printf("Line '%s' did not scan properly\n", line);
         return 1;
      }

      voltList = malloc(sizeof(VoltageNode));
      if (voltList == NULL) {
         printf("Ran out of memory\n");
         return 1;
      }
      voltList->chName = strdup(chName);
      voltList->deltaV = deltaV;
      voltList->hostName = strdup(hvSysName);
      voltList->next = NULL;
      if (first != NULL){
         last->next = voltList;
         last = voltList;
      } else {
         first = voltList;
         last = voltList;
      }
   }

   fclose(inFile);

   // output for debugging
   voltList = first;
   voltList = voltList->next; //quick fix
   while (voltList != NULL){
      //printf("SetCrateVoltage(hvSysHandle, %s, %s, %i, NrOfSlots, NrOfChList)\n", 
      //      voltList->hostName, voltList->chName, voltList->deltaV);
      SetChannelVoltage(hvSysHandle, voltList->hostName, voltList->chName, (float) voltList->deltaV, NrOfSlots, ChList);
      voltList = voltList->next;
   }

   // Freeing memory
   free(first);
   free(voltList);

   return 0;
} // end SetCrateVoltage

//==============================================================================
// SetChannelVoltage
//
// description - Changes voltage  of a single channel
//==============================================================================
void SetChannelVoltage(const int hvSysHandle, const char * hvSysName, const char * chanName, float chNew, unsigned short NrOfSlots, unsigned short ChList[])
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

   // getting old voltage value
   returnCode = CAENHV_GetChParam(hvSysHandle, i, parName, 1, &j, (void *) &chValue);
   if(returnCode){
      fprintf(stderr, "ERROR %#X: %s\n", returnCode, CAENHV_GetError(hvSysHandle));
   }

   // setting new voltage value
   returnCode = CAENHV_SetChParam(hvSysHandle, i, parName, 1, &j, &chNew);
   if(returnCode){
      fprintf(stderr, "ERROR %#X: %s\n", returnCode, CAENHV_GetError(hvSysHandle));
   }
   printf("%s Changed from %f to %f \n", parName, chValue, chNew);

} // SetChannelVoltage

//==============================================================================
// AdjustCrateVoltage
//
// Description - Adjusts the voltage of a channel from a list
//
// @param hvSysHandle   The system handle
// @param fileName      Path of file containing voltage information
//==============================================================================
int AdjustCrateVoltage(const int hvSysHandle, const char * fileName, unsigned short NrOfSlots, unsigned short ChList[])
{
   VoltageNode *first;     // Initial voltlist (contains junk)
   VoltageNode *voltList;  // VoltageNode being filled 
   VoltageNode *last;      // Previously filled VoltageNode
   char line[MAX_SIZE],chName[MAX_SIZE], hostName[MAX_SIZE];
   int deltaV;

   FILE * inFile = fopen(fileName, "r");
   if (inFile == NULL){
      printf("Could not open file %s\n", fileName);
      return 1;
   }

   //int total_rows = CountFileLines(inFile);
   //printf("Found %i lines.\n", total_rows);

   // extracting hostName from fileName
   strcpy(hostName, fileName);
   hostName[strlen(hostName) - 4] = 0;

   // allocate new node to hold data
   first = malloc(sizeof(VoltageNode));
   if (first == NULL) {
      printf("Ran out of memory\n");
      return 1;
   }
   last = first;
   first->next = NULL;

   while (fgets(line, sizeof(line), inFile) != NULL){
      // safety Checks
      if (line[0] == '\0'){
         printf("Line too short\n");
         return 1;
      }
      if (line[strlen (line)-1] != '\n'){
         printf("Line starting with '%s' is too long\n", line);
         return 1;
      }

      line[strlen(line)-1] = '\0';

      // scan fields
      if (sscanf(line, "%[^,],%i", chName, &deltaV) != 2){
         printf("Line '%s' did not scan properly\n", line);
         return 1;
      }

      voltList = malloc(sizeof(VoltageNode));
      if (voltList == NULL) {
         printf("Ran out of memory\n");
         return 1;
      }
      voltList->chName = strdup(chName);
      voltList->deltaV = deltaV;
      voltList->hostName = strdup(hostName);
      voltList->next = NULL;
      if (first != NULL){
         last->next = voltList;
         last = voltList;
      } else {
         first = voltList;
         last = voltList;
      }
   }

   fclose(inFile);

   // output for debugging
   voltList = first;
   voltList = voltList->next; //quick fix
   while (voltList != NULL){
      //printf("AdjustCrateVoltage(hvSysHandle, %s, %i, %s, NrOfSlots, NrOfChList)\n", 
      //      voltList->chName, voltList->deltaV, voltList->hostName);
      AdjustChannelVoltage(hvSysHandle, voltList->hostName, voltList->chName, (float) voltList->deltaV, NrOfSlots, ChList);
      voltList = voltList->next;
   }

   // Freeing memory
   free(first);
   free(voltList);

   return 0;
} // end AdjustCrateVoltage

//==============================================================================
// AdjustChannelVoltage
//
// description - adjusts voltage  of a single channel
//==============================================================================
void AdjustChannelVoltage(const int hvSysHandle, const char* hvSysName, const char * chanName, float chNew, unsigned short NrOfSlots, unsigned short ChList[])
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

   returnCode = CAENHV_GetChParam(hvSysHandle, i, parName, 1, &j,(void*)&chValue);
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

} // end AdjustChannelVoltage

//==============================================================================
// ToggleGriffinChannels
//
// Description - Toggles the power of all A/B channels in GRIFFIN
//
// @param hvSysHandle   The system handle
// @param hvSysName     The human readable system name
// @param chanType      A/B Channels 
// @param state         1/0 (On/Off)
// @param NrOfSlots     Number of slots in crate 
// @param ChList        List of channels in slot
//==============================================================================
void ToggleGriffinChannels(const int hvSysHandle, const char* hvSysName, const char * chanType, unsigned state, unsigned short NrOfSlots, unsigned short ChList[])
{
   CAENHVRESULT returnCode;
   char chName[12];
   const char * chType;
   int len;
   unsigned short i = 1;
   unsigned short j = 0;
   int changeCount = 0;

   // varifying we can find the channel of interest
   for (i = 0; i < NrOfSlots; i++){
      for (j = 0; j < ChList[i]; j++){
         returnCode = CAENHV_GetChName(hvSysHandle, i, 1, &j, (char (*)[12]) chName);
         if(returnCode){
            fprintf(stderr, "ERROR %#X: %s\n", returnCode, CAENHV_GetError(hvSysHandle));
         }
         // extract last character of channel name
         len = strlen(chName);
         chType = &chName[len - 1];

         if(strcmp(chanType, chType) == 0){
            printf("Found %s in %s Slot %i Channel %i \n", chName, (char *) hvSysName, i, j);
            TogglePower(hvSysHandle, hvSysName, chName, state, i, j);
            changeCount += 1;
         }
      }
   }

   // if channel is not found
   if (changeCount == 0){ 
      printf("ERROR: %s channels not found in %s \n Are the channels named properly?\n", chanType, (char *) hvSysName);
   }
} // ToggleGriffinChannels

//==============================================================================
// ToggleTigChannels
//
// Description - Toggles A/B channels for TIGRESS. TIGRESS cannnot turn off
// individual channels so an alternate method is used. 
//
// @param hvSysHandle   The system handle
// @param hvSysName     The human readable system name
// @param fileName      Output filename
// @param chanType      A/B Channels 
// @param NrOfSlots     Number of slots in crate 
// @param ChList        List of channels in slot
//==============================================================================
int ToggleTigChannels(const int hvSysHandle, const char* hvSysName, const char * fileName, const char * chanType, unsigned short NrOfSlots, unsigned short ChList[])
{
   CAENHVRESULT returnCode;
   FILE * outFile;
   const char *parName = "V0Set";
   char chName[12];
   unsigned short i =0;
   unsigned short j =0;
   float chValue;
   float chNew = 0;
   float chDefault = 900;
   const char * chType;
   int len;
   int changeCount = 0;

   // opens output file for recording current voltage settings
   outFile = fopen(fileName, "w");
   if (outFile == NULL){
      fprintf(stderr, "ERROR: Could not open/write file %s.dat", hvSysName);
      return 1;
   }

   // varifying we can find the channel of interest
   for (i = 0; i < NrOfSlots; i++){
      if(ChList[i] < 13) continue; // Skipping HPGe channels
      for (j = 0; j < ChList[i]; j++){
         returnCode = CAENHV_GetChName(hvSysHandle, i, 1, &j, (char (*)[12]) chName);
         if(returnCode){
            fprintf(stderr, "ERROR %#X: %s\n", returnCode, CAENHV_GetError(hvSysHandle));
         }
         // extract last character of channel name
         len = strlen(chName);
         chType = &chName[len - 1];

         // change voltage 
         if(strcmp(chanType, chType) == 0){
            printf("Found %s in %s Slot %i Channel %i \n", chName, (char *) hvSysName, i, j);
            returnCode = CAENHV_SetChParam(hvSysHandle, i, parName, 1, &j, &chDefault);
            if(returnCode){
               fprintf(stderr, "ERROR %#X: %s\n", returnCode, CAENHV_GetError(hvSysHandle));
            }
            printf("%s Changed to %f \n", parName, chDefault);
            changeCount += 1;
         }
         // Read out parameter to file 
         else { 
            // Get channel name
            returnCode = CAENHV_GetChName(hvSysHandle, i, 1, &j, (char (*)[12]) chName);
            if (returnCode){
               fprintf(stderr, "ERROR %#X: %s\n", returnCode, CAENHV_GetError(hvSysHandle));
            }
            
            // Get current voltage value
            returnCode = CAENHV_GetChParam(hvSysHandle, i, parName, 1, &j, (void *)&chValue);
            if (returnCode){
               fprintf(stderr, "ERROR %#X: %s\n", returnCode, CAENHV_GetError(hvSysHandle));
            }
            // write to XML file and zero channel voltage
            fprintf(outFile, "%s, %f \n", chName, chValue);
            returnCode = CAENHV_SetChParam(hvSysHandle, i, parName, 1, &j, (void *)&chNew);
            if (returnCode){
               fprintf(stderr, "ERROR %#X: %s\n", returnCode, CAENHV_GetError(hvSysHandle));
            }
         }
      }
   }

   fclose(outFile);

   if(strcmp(chanType, "A") == 0){
      printf("\nSuppressor B Channel data written to: %s\n", fileName);
   }
   else if(strcmp(chanType, "B") == 0){
      printf("\nSuppressor A Channel data written to: %s\n", fileName);
   } else {
      printf("\nWARNING: channel type not found. Channel data written to: %s\n", fileName);
   }

   // if channels are not found
   if (changeCount == 0){ 
      printf("ERROR: %s channels not found in %s \n Are the channels named properly?\n", chanType, (char *) hvSysName);
      return 1;
   }

   return 0;
} // ToggleTigChannels

//==============================================================================
// ToggleChPower
//
// Description - Toggles the power of a single channel. 
//
// @param hvSysHandle   The system handle
// @param hvSysName     The human readable system name
// @param chanName      Name of the channel
// @param state         1/0 (On/Off)
// @param NrOfSlots     Number of slots in crate
// @param ChList        List of channels in slot
//==============================================================================
void ToggleChPower(const int hvSysHandle, const char* hvSysName, const char * chanName, unsigned state, unsigned short NrOfSlots, unsigned short ChList[])
{
   CAENHVRESULT returnCode; 
   char chName[12];
   unsigned short i = 1;
   unsigned short j = 0;

   // varifying we can find the channel of interest
   for (i = 0; i < NrOfSlots; i++){
      for (j = 0; j < ChList[i]; j++){
         returnCode = CAENHV_GetChName(hvSysHandle, i, 1, &j, (char (*)[12]) chName);
         if(returnCode){
            fprintf(stderr, "ERROR %#X: %s\n", returnCode, CAENHV_GetError(hvSysHandle));
         }
         if(strcmp(chName, chanName) == 0){
            printf("Found %s in %s Slot %i Channel %i \n", chanName, (char *) hvSysName, i, j);
            TogglePower(hvSysHandle, hvSysName, chanName, state, i, j);
            return;
         }
      }
   }

   // if channel is not found
   printf("%s not found in %s \n", chanName, (char *) hvSysName);

} // end ToggleChPower

//==============================================================================
// TogglePower
//
// Description - Toggles the power of a channel 
//
// @param hvSysHandle   The system handle
// @param hvSysName     The human readable system name
// @param chanName      Name of the channel
// @param state         1/0 (On/Off)
// @param i             Channel index
// @param j             Slot index
//==============================================================================
void TogglePower(const int hvSysHandle, const char* hvSysName, const char * chanName, unsigned state, unsigned short i, unsigned short j)
{
   CAENHVRESULT returnCode; 
   const char * parName = "Pw";
   unsigned powerVal;

   // check status of power state and exit if it does not need to be changed
   returnCode = CAENHV_GetChParam(hvSysHandle, i, parName, 1, &j, (void *) &powerVal);
   if(returnCode){
      fprintf(stderr, "ERROR %#X: %s\n", returnCode, CAENHV_GetError(hvSysHandle));
   }

   if( powerVal == state && powerVal == 1 ){
      fprintf(stdout, "\tPower to channel %s is already ON.\n", chanName);
   }
   else if( powerVal == state && powerVal == 0 ){
      fprintf(stdout, "\tPower to channel %s is already OFF.\n", chanName);
   }
   else{ 
      returnCode = CAENHV_SetChParam(hvSysHandle, i, parName, 1, &j, &state);
      if(returnCode){
         fprintf(stderr, "ERROR %#X: %s\n", returnCode, CAENHV_GetError(hvSysHandle));
         return;
      }
      // success message
      printf("\tPower toggled for %s\n", chanName);
   }
}

//==============================================================================
// ChangeCrateName
//
// Description - Changes the name of a channel from a list
//
// @param hvSysHandle   The system handle
// @param fileName      Path of file containing naming information
//==============================================================================
int ChangeCrateName(const int hvSysHandle, const char * fileName)
{
   Names_t *first = NULL; Names_t *last = NULL;
   // opening file
   FILE * inFile = fopen(fileName, "r");
   if (inFile == NULL){
      printf("Could not open file %s\n", fileName);
      return 1;
   }

   // count number of lines 
   //int total_rows = CountFileLines(inFile);

   char line[MAX_SIZE], parName[MAX_SIZE], hostName[MAX_SIZE];
   int slotNum, chNum;
   Names_t *nameList;

   while (fgets(line, sizeof(line), inFile) != NULL){
      // safety Checks
      if (line[0] == '\0'){
         printf("Line too short\n");
         return 1;
      }
      if (line[strlen (line)-1] != '\n'){
         printf("Line starting with '%s' is too long\n", line);
         return 1;
      }

      line[strlen (line)-1] = '\0';

      // scan fields
      if (sscanf(line, "%i\t%i\t%s\t%s", &slotNum, &chNum, parName, hostName) != 4){
         printf("Line '%s' did not scan properly\n", line);
         return 1;
      }

      // allocate new node to hold data
      nameList = malloc(sizeof(Names_t));
      if (nameList == NULL) {
         printf("Ran out of memory\n");
         return 1;
      }

      nameList->slotNum = slotNum;
      nameList->chNum = chNum;
      nameList->parName = strdup(parName);
      nameList->hostName = strdup(hostName);
      nameList->next = NULL;
      if (first != NULL){
         last->next = nameList;
         last = nameList;
      } else {
         first = nameList;
         last = nameList;
      }
   }

   fclose(inFile);

   // output for debugging
   nameList = first;
   while (nameList != NULL){
      //printf("ChangeName(hvSysHandle, %i, %i, %s)\n", 
      //      nameList->slotNum, nameList->chNum, nameList->parName);
      ChangeChannelName(hvSysHandle, nameList->slotNum, nameList->chNum, nameList->parName);
      nameList = nameList->next;
   }

   free(nameList);
   return 0;

} // end ChangeCrateName

//==============================================================================
// ChangeChannelName
//
// Description - Changes the name of a single channel 
//
// @param hvSysHandle   The system handle
// @param slotNum       Slot number
// @param chNum         Channel number
// @param parName       New name of channel
//==============================================================================
void ChangeChannelName(const int hvSysHandle, unsigned short slotNum, unsigned short chNum, const char * parName)
{
   CAENHVRESULT returnCode;

   returnCode = CAENHV_SetChName(hvSysHandle, slotNum, 1, &chNum, parName);
   if(returnCode){
      fprintf(stderr, "ERROR %#X: %s\n", returnCode, CAENHV_GetError(hvSysHandle));
   }
}

//==============================================================================
// CountFileLines
//
// Description - Counts number of lines in file
//
// @param inFile   The input file
//==============================================================================
int CountFileLines(FILE * inFile)
{
   char c;
   int total_rows = 0;
   for (c = getc(inFile); c != EOF; c = getc(inFile)){
      if (c == '\n'){
         total_rows++;
      }
   };
   // rewinding file
   rewind(inFile);

   return total_rows;

} // end CountFileLines

//==============================================================================
// CountFileColumns
//
// Description - Counts number of columns in file
//
// @param inFile   The input file
// NEED TO BUILD IF NEEDED
//==============================================================================
int CountFileColumns(FILE * inFile)
{
   //   char line[1024];
   int total_columns = 0;
   //   if (fgets(line, sizeof(line), inFile) != NULL){
   //      char * p = strtok(line, "\t\n");
   //      while (p){
   //         ++total_columns;
   //         p = strtok(NULL, "\t\n");
   //      }
   //   }
   return total_columns;
} // end CountFileColumns
