#include "CommandParser.h"
#include "GRSIVoltageControl.h"

void ParseInputs(int argc, char *argv[])
{
   
   // variables
   int i = 0;
   int j = 0;
   int sys_iter;
   int returnCode;
   char option;
   char filename[80]; 
   char hvSysName[80];

   // crate map variables
   unsigned short NrOfSlots = 0; // number of slots in crate
   unsigned short *NrOfChList = NULL; // number of channels in each slots
   char *ModelList = NULL; // board model 
   char *DescList = NULL; // board description
   unsigned short *SerialNumList = NULL; // serial number of board
   unsigned char *FmVerLSByte = NULL; // LSByte of firmware release
   unsigned char *FmVerMSByte = NULL; // MSByte of firmware release
   const char * chanName = NULL;
   const char * parName = NULL;

   // copies crate name to string so its modifiable (modifying argv = bad)
   strcpy(hvSysName, argv[argc - 1]);

   // check if any system has been logged into
   for( sys_iter = 0; sys_iter < MAX_HVPS; sys_iter++){
      if(System[sys_iter].ID != -1){
         returnCode = CAENHV_GetCrateMap(System[sys_iter].Handle, &NrOfSlots, &NrOfChList, &ModelList, &DescList, &SerialNumList, &FmVerLSByte, &FmVerMSByte);
         if(returnCode){
            fprintf(stderr, "ERROR %08x: %s\n", returnCode, CAENHV_GetError(System[sys_iter].Handle));
         }

         // look for and parse command line arguments
         for(i = 0; i < argc; i++){
            if(*(argv[i]) == '-'){
               for(j = 1; *(argv[i] +j) != '\0'; j++){
                  option = *(argv[i] + j);

                  switch(option){
                     case 'd':
                        //building filename
                        strcpy(filename, "./");
                        strcat(filename, hvSysName);
                        strcat(filename, ".xml");
                        // writing file
                        WriteToXML(hvSysName, System[sys_iter].Handle, NrOfSlots, NrOfChList, filename);
                        break;
                     case 'v':
                        // Searches all channels for a specific name and
                        // changes the voltage Value
                        if(argc != 5){
                           printf("::: ChangeVoltage \n");
                           printf("5 arguments needed, %i given\n", argc);
                           return;
                        }
                        if (argc == 5){
                           chanName = argv[1];
                           chanV = atof(argv[2]);
                           printf("\n::: Attempting to change voltage of channel %i.\n", chanName);
                           ChangeVoltage(hvSysHandle, hvSysName, chanName, chanV, NrOfSlots, NrOfChList);
                        }
                        break;
                     case 'a': 
                        printf("Adjusting Voltage");
                        break;
                     default: 
                        printf("Please pass an argument\n");
                  }
               }
            }
         }
      }
   }

   // deallocate memory
   free(DescList);
   free(SerialNumList);
   free(NrOfChList);
   free(FmVerLSByte);
   free(FmVerMSByte);
   free(ModelList);

}
   
