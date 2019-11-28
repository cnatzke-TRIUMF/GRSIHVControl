#!/bin/bash 
################################################################################
# Purpose:  This script will update or set the voltage for the TIGRESS/GRIFFIN
#           HV crates. 
#
# Creation: October 2019
# Updated:  October 2019
# Auth:     S.Gillespie, C.Natzke
# 
# Usage: 
# To change voltage
#  ./ChangeHV.sh crateMap.txt HV_Change.txt
# To set voltage
#  ./ChangeHV.sh crateMap.txt HV_Change.txt 1
#
# Input Files: (These are unique to TIGRESS or GRIFFIN)
#  crateMap.txt   -> Contains map of which crate houses which channels
#  HV_Change.txt  -> Change in voltage for each channel
################################################################################

################################################################################
# ---> USER CONFIGS <---
################################################################################
SAFETYCHECK=false # Toggles safety check stage
################################################################################

if [[ $# == 2 ]]; then
   OPTION="a"
   STATUS_OPTION="ADJUSTING"
elif [[ $# == 3 && $3 == "-s" ]]; then 
   OPTION="s"
   STATUS_OPTION="SETTING"
elif [[ $# == 3 && $3 == "-s" ]]; then 
   echo "Setting Votages"
   exit
else
   echo "---------------------------------------------------"
   echo "2 parameters needed for adjusting voltage, $# given"
   echo "./ChangeHV.sh <crateMap.txt> <HV_Change.txt>"
   echo
   echo "3 parameters needed for setting voltage, $# given"
   echo "./ChangeHV.sh <crateMap.txt> <HV_Set.txt> -s"
   echo "---------------------------------------------------"
   exit
fi

# finds unique crate names 
crateNames=($(awk '{ a[$2]++ } END { for (b in a) { print b } }' $1))
# SAFETY CHECK
if [[ $SAFETYCHECK == true ]]; then
   echo "Changing voltage for ${#crateNames[@]} HV crates: ${crateNames[@]}"
   echo "Do you wish to continue?"
   select yn in "Yes" "No"; do
      case $yn in 
         Yes ) break;;
         No ) echo "Choice: No"; echo "Exiting..."; exit;;
      esac
   done
fi

# SETTING UP STUFF AND THINGS
for crate_name in "${crateNames[@]}"; do 
   touch ${crate_name}.txt
done
# crate map dictionary to map channel to crate
declare -A crateMap
while read key value; do
   if [[ $key ]]; then 
      crateMap[$key]="${crateMap[$key]}${crateMap[$key]:+,}$value"
   fi
done < $1

# PARSING HV_CHANGE FILE
readarray rows < $2
for row in "${rows[@]}"; do
   row_array=(${row}) # complete row
   channel=${row_array[0]} # channel name
   array_position=${channel:3:2}
   crate=${crateMap[$array_position]}
   # creates file for each crate
   echo "${row_array[0]},${row_array[1]}" >> ${crate}.txt
done 

# CHANGING VOLTAGE VALUES
echo 
echo ":::: $STATUS_OPTION VOLTAGES"

# looping through hv crates
for crate_file in "${crateNames[@]}"; do 
   if [[ -s ${crate_file}.txt ]]; then 
      echo "./GRSIHVControl ${crate_file}.txt -$OPTION ${crate_file}"
      echo "rm ${crate_file}.txt"
   else
      rm ${crate_file}.txt 
   fi
done
#
echo
echo ":::: FINISHED"
