# GRSIHVControl
This project is designed to control the CAEN High Voltage mainframes from the
command line. It uses the CAENHVWrapper library to interface with the crates. 

## Installation
1. Get the code, either via tarball or from github
2. Make the GRSIHVControl simply by running 'make' in the program directory
3. Enjoy controlling the HV crates from the comfort of the command line

## Helper scripts
The 'scripts' directory contains helper scripts to aid the user in utilizing
the HV control program. 

### ChangeHV.sh
This script updates/sets the voltages of the TIGRESS and GRIFFIN crates. It
requires the user to specify which array they would like to change and the user
must provide a text file detailing the channels and voltages. The name of these
files can be changed by the user, but their default names are: 'hvMap.txt' and
'hvChange.sh'. Examples for TIGRESS are provided in the scripts directory
