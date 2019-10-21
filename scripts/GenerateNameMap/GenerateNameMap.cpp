/////////////////////////////////////////////////////////////////////////////////////////////////
// Auth:          C.Natzke
// Desc:          Generates .txt file mapping the slot and channel number of a high
//                voltage crate to human readable channel numbers
// Date:          October 2019
// Last Update:   October 2019
// Compile:       g++ GenerateNameMap.cpp -std=c++0x -o GenerateNameMap
/////////////////////////////////////////////////////////////////////////////////////////////////
#include <stdio.h>
#include <string.h>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <dirent.h>
using namespace std;

void GenerateNameMap(const char * out = "ChannelNameMap.txt")
{
   ofstream outFile;
   outFile.open(out);
   char colour[4] = {'B','G','R','W'};
   char chanType[2] = {'A','B'};
   string hostName[2] = {"grifhv03", "grifhv00"};
   int slotN;
   int detNum;
   int chanN = 0;

   for (int crateID = 0; crateID < 2; crateID++){
      for (int i = 0; i < 8; i++){
         if (crateID == 0) detNum = i + 1;
         if (crateID == 1) detNum = i + 9;
         slotN = 14 - 2 * i;

         while (chanN < 48){
            for (int j = 0; j < 4; j++){
               for (int k = 1; k < 6; k++){
                  // labels suppressor channels
                  if (chanN < 40){
                     for (int l = 0; l < 2; l++){
                        outFile << 
                           std::setfill('0') << std::setw(2) << slotN << '\t' << 
                           std::setfill('0') << std::setw(2) << chanN << '\t' << 
                           "GRS" << std::setfill('0') << std::setw(2) << detNum << colour[j] << 'N' 
                           << std::setfill('0') << std::setw(2) << k << chanType[l] << 
                           "\t" << hostName[crateID] << "\n";
                           chanN++;
                     } // end A/B 
                  } // end suppressor names

                  // set last 8 channels in slot as spare
                  else if (chanN < 48){
                     for (int l = 0; l < 8; l++){
                        outFile << 
                           std::setfill('0') << std::setw(2) << slotN << '\t' << 
                           std::setfill('0') << std::setw(2) << chanN << '\t' << 
                           "G" << std::setfill('0') << std::setw(2) << detNum << "SPARE"
                           << std::setfill('0') << std::setw(2) << l <<
                           "\t" << hostName[crateID] << "\n";
                           chanN++;
                     } // end spare names
                  } // end spare
               } // end suppressor number
            } // end colour 
         } // end chanN

         // resets channel for new crate
         chanN = 0;
      } // end detector number
   } // end crateID


   outFile.close();
} // end GenerateNameMap

int main(int argc, char * argv[])
{
   if (argc == 1) GenerateNameMap();
   else if (argc == 2) GenerateNameMap(argv[1]);

   return 0;
} // end main
