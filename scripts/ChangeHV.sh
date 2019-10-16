#!/bin/bash 
################################################################################
# Purpose:  This script will update or set the voltage for the TIGRESS/GRIFFIN
#           HV crates. 
#
# Creation: October 2019
# Updated:  October 2019
# Auth:     S.Gillespie, C.Natzke
# 
# Input Files: (These are unique to TIGRESS or GRIFFIN)
#  crateMap.txt   -> Contains map of which crate houses which channels
#  hvMap.txt      -> Default voltage for each channel 
#  hvChange.txt   -> Change in voltage for each channel
################################################################################

# Change these variables to your preference
ARRAY=TIGRESS
MAPFILE=crateMap.txt
VOLTFILE=hvMap.txt
CHANGEFILE=hvChange.txt

declare -A crateMap

# crate map dictionary to map channel to crate
while read key value; do
   if [[ $key ]]; then 
      crateMap[$key]="${crateMap[$key]}${crateMap[$key]:+,}$value"
   fi
done < $MAPFILE

# if CHANGEFILE is more recent than VOLTFILE
if [[ $CHANGEFILE -nt $VOLTFILE ]]; then
   echo 
   echo ":::: Adjusting voltages"
   readarray rows < $CHANGEFILE

   # Changing TIGRESS Channels
   if [[ $ARRAY == 'TIGRESS' ]]; then 
      for row in "${rows[@]}"; do
         row_array=(${row})
         channel=${row_array[0]}
         deltaV=${row_array[1]}
         hvLocation=${channel%-*}

         # matches channel to high voltage crate and changes voltage
         echo "./GRSIHVControl $channel $deltaV -a ${crateMap[$hvLocation]}"
      done 
   fi

   # Changing TIGRESS Channels
   if [[ $ARRAY == 'GRIFFIN' ]]; then 
     echo "Changing GRIFFIN Voltages" 
   fi

# if the hvMap file is newer than the change file
else
   echo 
   echo ":::: Setting voltages"
   readarray rows < $VOLTFILE

   # Setting TIGRESS Channels
   if [[ $ARRAY == 'TIGRESS' ]]; then 
      for row in "${rows[@]}"; do
         row_array=(${row})
         channel=${row_array[0]}
         voltage=${row_array[1]}
         hvLocation=${channel%-*}

         # matches channel to high voltage crate and changes voltage
         echo "./GRSIHVControl $channel $voltage -v ${crateMap[$hvLocation]}"
      done 
   fi

   # Setting GRIFFIN Channels
   if [[ $ARRAY == 'GRIFFIN' ]]; then 
     echo "Setting GRIFFIN Voltages" 
   fi
fi

echo
echo ":::: Finished"
