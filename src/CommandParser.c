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

   unsigned short NrOfSlots = 0; // number of slots in crate
   unsigned short *NrOfChList = NULL; // number of channels in each slots
   char *ModelList = NULL; // board model 
   char *DescList = NULL; // board description
   unsigned short *SerialNumList = NULL; // serial number of board
   unsigned char *FmVerLSByte = NULL; // LSByte of firmware release
   unsigned char *FmVerMSByte = NULL; // MSByte of firmware release
   const char * chanName = NULL;
   const char * chanType = NULL;
   unsigned short slotN = NULL;
   unsigned short chanN = NULL;
   unsigned state;
   const char * inFile = NULL;

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
                           printf("ERROR: 5 arguments needed, %i given\n", argc);
                           return;
                        }
                        if (argc == 5){
                           chanName = argv[1];
                           chanV = atof(argv[2]);
                           printf("\n::: Attempting to adjust the voltage of channel %s.\n", chanName);
                           ChangeVoltage(hvSysHandle, hvSysName, chanName, (float)chanV, NrOfSlots, NrOfChList);
                           printf("::: Done\n");
                        }
                        break;
                     case 'a':
                        // Searches all channels for a specific name and
                        // adjusts the channel voltage.
                        if(argc != 5){
                           printf("::: AdjustVoltage \n");
                           printf("ERROR: 5 arguments needed, %i given\n", argc);
                           return;
                        }
                        if(argc == 5){
                           chanName = argv[1];
                           chanV = atof(argv[2]);
                           printf("\n::: Attempting to adjust the voltage of channel %s.\n", chanName);
                           AdjustVoltage(hvSysHandle, hvSysName, chanName, (float)chanV, NrOfSlots, NrOfChList);
                           printf("::: Done\n");
                        }
                        break;

                     // Searches all channels for a specific name and toggles
                     // power
                     case 'p':
                        if(argc != 5){
                           printf("::: ToggleChPower \n");
                           printf("ERROR: 5 arguments needed, %i given\n", argc);
                           return;
                        }
                        if(argc == 5){
                           chanName = argv[1];
                           state = atof(argv[2]);
                           printf("\n::: Attempting to toggle power of channel %s.\n", chanName);
                           ToggleChPower(hvSysHandle, hvSysName, chanName, state, NrOfSlots, NrOfChList);
                           printf("::: Done\n");
                        }
                        break;
                     // Searches all channels for a specific type (A/B)
                     // and toggles power
                     case 't':
                        if(argc != 5){
                           printf("::: ToggleUpChannels\n");
                           printf("ERROR: 5 arguments needed, %i given\n", argc);
                           return;
                        }
                        if(argc == 5){
                           chanType = argv[1];
                           state = atof(argv[2]);
                           printf("\n::: Attempting to toggle power of %s channels.\n", chanType);
                           ToggleUpChannels(hvSysHandle, hvSysName, chanType, state, NrOfSlots, NrOfChList);
                           printf("::: Done\n");
                        }
                        break;
                     // Changes names of channels in crate
                     case 'n':
                        if(argc != 6){
                           printf("::: ChangeName\n");
                           printf("ERROR: 6 arguments needed, %i given\n", argc);
                           return;
                        }
                        if(argc == 6){
                           slotN = atof(argv[1]);
                           chanN = atof(argv[2]);
                           chanName = argv[3];
                           if(NrOfChList[slotN] < chanN){
                              printf("Number of channels exceeds number of channels in slot. Aborting.\n");
                           }
                           else {
                              //printf("\n::: Attempting to name channels\n");
                              ChangeChName(hvSysHandle, slotN, chanN, chanName);
                              //printf("::: Done\n");
                           }
                        }
                        break;
                     // Changes names of channels in crate via file
                     case 'f':
                        if(argc != 4){
                           printf("::: ChangeName\n");
                           printf("ERROR: 4 arguments needed, %i given\n", argc);
                           return;
                        }
                        if(argc == 4){
                           inFile =argv[1];
                           //printf("\n::: Attempting to name channels\n");
                           ChangeName(hvSysHandle, inFile);
                           //printf("::: Done\n");
                        }
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
   
