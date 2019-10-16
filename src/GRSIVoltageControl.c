// Logs into HV crate

#include "CAENHVWrapper.h"
#include "GRSIVoltageControl.h"
#include "CrateCommands.h"
#include "CommandParser.h"

HV System[MAX_HVPS];

//#==============================================================================
//# MAIN
//#==============================================================================

int main(int argc, char *argv[])
{
   char userName[20] = "admin";
   char passwd[20] = "Kittygam";
   char hvSysName[80];
   int ret;

   // copies crate name to string so its modifiable (modifying argv = bad)
   strcpy(hvSysName, argv[argc - 1]);

   for( ret = 0; ret < MAX_HVPS; ret++ ){
      System[ret].ID = -1;
   }

   // login to HVCrate
   HVSystemLogin(hvSysName, (const char *)userName, (const char *)passwd);

   ParseInputs(argc, argv);

   // logout of crate
   HVSystemLogout();

   return 0;
}
