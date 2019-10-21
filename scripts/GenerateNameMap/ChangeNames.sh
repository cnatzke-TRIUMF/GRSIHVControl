#!/bin/bash

# checking for the required arguments 
if [[ $# != 1 ]]; then 
   echo "ERROR: 1 argument required"
   exit
fi

readarray rows < $1
for row in "${rows[@]}"; do
   row_array=(${row})
   slot=${row_array[0]}
   channel=${row_array[1]}
   name=${row_array[2]}
   host=${row_array[3]}

   ./GRSIHVControl $slot $channel $name -n $host
done 
