// Logs into HV crate

#include "CAENHVWrapper.h"
#include "GRSIVoltageControl.h"

HV System[MAX_HVPS];

//#==============================================================================
//# MAIN
//#==============================================================================

int main(int argc, char *argv[])
{
   // variables
   int i = 0;
   int j = 0;
   int sys_iter;
   int returnCode;
   int ret; 
   char option;
   char userName[20] = "admin";
   char passwd[20] = "Kittygam";
   char hvSysName[80];
   char filename[80]; 

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

   // setting up instructions for pre-mature exit (SegFault, Ctrl-c)
   //(void)(*signal)(SIGSEGV, &SegFaultFinish);
   //(void)(*signal)(SIGINT, &InterruptFinish);
   
   // copies crate name to string so its modifiable (modifying argv = bad)
   strcpy(hvSysName, argv[argc - 1]);

   for( ret = 0; ret < MAX_HVPS; ret++ ){
      System[ret].ID = -1;
   }

   // login to HVCrate
   HVSystemLogin(hvSysName, (const char *)userName, (const char *)passwd);

   // check if any system has been logged into
   for( sys_iter = 0; sys_iter < MAX_HVPS; sys_iter++){
      if(System[sys_iter].ID != -1){
         returnCode = CAENHV_GetCrateMap(System[sys_iter].Handle, &NrOfSlots, &NrOfChList, &ModelList, &DescList, &SerialNumList, &FmVerLSByte, &FmVerMSByte);
         if(returnCode){
            fprintf(stderr, "ERROR %08x: %s\n", returnCode, CAENHV_GetError(System[sys_iter].Handle));
         }

         //building filename
         strcpy(filename, "./");
         strcat(filename, hvSysName);
         strcat(filename, ".xml");

         // writing file
         WriteToXML(hvSysName, System[sys_iter].Handle, NrOfSlots, NrOfChList, filename);
      }
   }

   // logout of crates
   HVSystemLogout();

   // deallocate memory
   free(DescList);
   free(SerialNumList);
   free(NrOfChList);
   free(FmVerLSByte);
   free(FmVerMSByte);
   free(ModelList);
   
   return 0;
}
