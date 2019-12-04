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
                     ///////////////////////////////////////////////////////////////////////////////////////
                     // Writes crate info to XML file
                     ///////////////////////////////////////////////////////////////////////////////////////
                     case 'd':
                        //building filename
                        strcpy(filename, "./");
                        strcat(filename, hvSysName);
                        strcat(filename, ".xml");
                        // writing file
                        WriteToXML(hvSysName, System[sys_iter].Handle, NrOfSlots, NrOfChList, filename);
                        break;
                     ///////////////////////////////////////////////////////////////////////////////////////
                     // Sets voltage for a channel or crate
                     ///////////////////////////////////////////////////////////////////////////////////////
                     case 'v':
                        // Searches all channels for a specific name and
                        // changes the voltage Value
                        if(argc != 5 && argc != 4){
                           printf("ERROR: 5 arguments needed for setting individual channel, 4 needed for setting with file, %i given\n\n", argc);
                           printf("./GRSIHVControl <name> <voltage change> -v <host>\n");
                           printf("./GRSIHVControl <voltage file> -v <host>\n\n");
                           return;
                        }
                        if (argc == 5){
                           chanName = argv[1];
                           chanV = atof(argv[2]);
                           printf("\n::: Attempting to set the voltage of channel %s.\n", chanName);
                           SetChannelVoltage(hvSysHandle, hvSysName, chanName, (float)chanV, NrOfSlots, NrOfChList);
                           printf("::: Done\n");
                        }
                        if (argc == 4){
                           inFile = argv[1];
                           SetCrateVoltage(hvSysHandle, hvSysName, inFile, NrOfSlots, NrOfChList);
                        }
                        break;
                     ///////////////////////////////////////////////////////////////////////////////////////
                     // Adjusting voltage values
                     ///////////////////////////////////////////////////////////////////////////////////////
                     case 'a':
                        // Searches all channels for a specific name and adjusts the channel voltage.
                        // or uses file to change entire crate at once
                        if(argc != 5 && argc != 4){
                           printf("ERROR: 5 arguments needed for individual channel adjustment, 4 needed for adjusting with file, %i given\n\n", argc);
                           printf("./GRSIHVControl <name> <voltage change> -a <host>\n");
                           printf("./GRSIHVControl <voltage file> -a <host>\n\n");
                           return;
                        }
                        if(argc == 5){
                           chanName = argv[1];
                           chanV = atof(argv[2]);
                           //printf("\n::: Attempting to adjust the voltage of channel %s.\n", chanName);
                           AdjustChannelVoltage(hvSysHandle, hvSysName, chanName, (float)chanV, NrOfSlots, NrOfChList);
                        }
                        if(argc == 4){
                           inFile = argv[1];
                           AdjustCrateVoltage(hvSysHandle, inFile, NrOfSlots, NrOfChList);
                        }
                        break;

                     ///////////////////////////////////////////////////////////////////////////////////////
                     // Toggles power of single channel
                     ///////////////////////////////////////////////////////////////////////////////////////
                     case 'p':
                        if(argc != 5){
                           printf("::: ToggleChPower \n");
                           printf("ERROR: 5 arguments needed, %i given\n\n", argc);
                           printf("./GRSIHVControl <channel> <1/0> -p <host>\n\n");
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
                     ///////////////////////////////////////////////////////////////////////////////////////
                     // Toggles power of A/B channels in GRIFFIN
                     ///////////////////////////////////////////////////////////////////////////////////////
                     case 't':
                        if(argc != 5){
                           printf("::: ToggleGriffinChannels\n");
                           printf("ERROR: 5 arguments needed, %i given\n\n", argc);
                           printf("./GRSIHVControl <A/B> <1/0> -t <host>\n\n");
                           return;
                        }
                        if(argc == 5){
                           chanType = argv[1];
                           state = atof(argv[2]);
                           printf("\n::: Attempting to toggle power of %s channels.\n", chanType);
                           ToggleGriffinChannels(hvSysHandle, hvSysName, chanType, state, NrOfSlots, NrOfChList);
                           printf("::: Done\n");
                        }
                        break;
                     ///////////////////////////////////////////////////////////////////////////////////////
                     // Toggles A/B channels for TIGRESS Suppressors
                     ///////////////////////////////////////////////////////////////////////////////////////
                     case 'g':
                        if(argc != 4){
                           //printf("::: ToggleTigChannels\n");
                           printf("ERROR: 4 arguments needed, %i given\n\n", argc);
                           printf("./GRSIHVControl <A/B> -g <host>\n\n");
                           return;
                        }
                        if (argc == 4){ 
                           char * bgo_file = NULL;
                           char buf[100];
                           chanType = argv[1];
                           if (strcmp(chanType, "A") == 0) bgo_file = "BGO_B_";
                           if (strcmp(chanType, "B") == 0) bgo_file = "BGO_A_";
                           //printf("\n::: Attempting to toggle power of %s channels.\n", chanType);
                           strcpy(buf, bgo_file);
                           strcat(buf, hvSysName);
                           strcat(buf, ".dat");
                           ToggleTigChannels(hvSysHandle, hvSysName, buf, chanType, NrOfSlots, NrOfChList);
                           //printf("::: Done\n");
                        }
                        break;
                     ///////////////////////////////////////////////////////////////////////////////////////
                     // Changes names of channels in a crate
                     ///////////////////////////////////////////////////////////////////////////////////////
                     case 'n':
                        if(argc != 6 && argc != 4){
                           //printf("::: ChangeName\n");
                           printf("ERROR: 6 arguments needed for individual channel change, 4 needed for changing with file, %i given\n\n", argc);
                           printf("./GRSIHVControl <slot> <channel> <name> -n <host>\n");
                           printf("./GRSIHVControl <nameFile> -n <host>\n\n");
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
                              ChangeChannelName(hvSysHandle, slotN, chanN, chanName);
                              //printf("::: Done\n");
                           }
                        }
                        if(argc == 4){
                           inFile =argv[1];
                           printf("\n::: Attempting to name channels using %s\n", inFile);
                           ChangeCrateName(hvSysHandle, inFile);
                           printf("::: Done\n");
                        }
                        break;

                     ///////////////////////////////////////////////////////////////////////////////////////
                     // No argument given
                     ///////////////////////////////////////////////////////////////////////////////////////
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
   
