# GRSIHVControl
This project is designed to control the CAEN High Voltage mainframes from the
command line. It uses the CAENHVWrapper library to interface with the crates. 

## Table of Contents
  * [Installation](#installation)
  * [Running GRSIHVControl](#running-grsihvcontrol)
      - [Dump Crate/Channel Information](#dump-crate-information)
      - [Set Channel Voltage](#set-channel-voltage)
      - [Adjust Channel Voltage](#adjust-channel-voltage)
      - [Toggle Channel Power Status](#toggle-channel-power-status)
      - [Toggle GRIFFIN A/B Channels](#toggle-griffin-ab-channels)
      - [Toggle TIGRESS A/B Channels](#toggle-tigress-ab-channels)
      - [Name Channels](#name-channels)
  * [Helper scripts](#helper-scripts)
    + [ChangeHV.sh](#changehvsh)
    + [GenerateNameMap.sh](#generatenamemapsh)

# Installation
1. Get the code, either via tarball or from github
```
git clone https://github.com/cnatzke/GRSIHVControl.git
```
2. Add hostnames and host ip's to `src/CommandParser.c:68` and username/password to `src/GRSIHVControl.c:15`.
3. Make the GRSIHVControl simply by running 'make' in the program directory
 ``` 
 cd GRSIHVControl
 make
 ```
 
4. Enjoy controlling the HV crates from the comfort of the command line
```
./GRSIHVControl <arguments> <option> <hostname>
```

# Running GRSIHVControl
The general form of input is:
```
./GRSIHVControl <arguments> <option> <hostname>
```
where option is
```
-d    Dump crate/channel info
-v    Set channel voltage
-a    Adjust channel info
-p    Toggle channel power
-t    Toggle GRIFFIN A/B channels
-g    Toggle TIGRESS A/B channels
-n    Set channel name
```
The specific usage of each option is detailed below.

#### Dump Crate Information
```
./GRSIHVControl <-d> <hostname>
```
dumps the channel information and settings for the entire HV crate into a file called `<hostname>.xml`

#### Set Channel Voltage
To set the voltage of a single channel call 
```
./GRSIHVControl <name> <voltage> -v <host>
```
where `<name>` is the name of the channel (e.g. GRSxxyyzzA/B), `<voltage>` is the voltage of the channel, and `<host>` is the name of the crate. 

Alternatively, you can set the voltages for multiple channels at once using
```
  ./GRSIHVControl <voltage file> -v <host>
````
where `<voltage file>` is a csv text file containing the channel name and voltage in the form 
```
channel_name1, set_voltage1
channel_name2, set_voltage2
...
channel_nameN, set_voltageN
```

#### Adjust Channel Voltage 
To change the voltage of a single channel call 
```
./GRSIHVControl <name> <deltaV> -a <host>
```
where `<name>` is the name of the channel (e.g. GRSxxyyzzA/B), `<deltaV>` is the voltage change of the channel (this can be postive or negative), and `<host>` is the name of the crate. 

Alternatively, you can change the voltages for multiple channels at once using
```
  ./GRSIHVControl <voltage file> -a <host>
````
where `<voltage file>` is a csv text file containing the channel name and voltage change in the form 
```
channel_name1, deltaV1
channel_name2, deltaV2
...
channel_nameN, deltaVN
```
and must have the title `<hostname>.txt`

#### Toggle Channel Power Status
To change the power status (On/Off) of a single channel call 
```
./GRSIHVControl <channel> <1/0> -p <host>
```
where `<name>` is the name of the channel (e.g. GRSxxyyzzA/B), `<1/0>` is desired status (1->On, 0->Off), and `<host>` is the name of the crate. 

#### Toggle GRIFFIN AB Channels
To turn GRIFFIN A/B channels On/Off use
```
./GRSIHVControl <A/B> <1/0> -t <host>
```
where `<A/B>` is the type of channel, `<1/0>` is desired status (1->On, 0->Off), and `<host>` is the name of the crate. 

Using 
```
./GRSIHVControl A 1 -t <host>
```
will turn all of the A channels On for `<host>` and leave B unchanged; whereas, 
```
./GRSIHVControl B 0 -t <host>
```
will turn all B channels OFf and leave all A channels unchanged.  

#### Toggle TIGRESS AB Channels
To turn TIGRESS A/B channels On/Off use
```
./GRSIHVControl <A/B> -g <host>
```
where `<A/B>` is the type of channel and `<host>` is the name of the crate. 

```
./GRSIHVControl A -g <host>
```
will set all channels ending in A to 900 V and write the current voltage setting of the B channels to a file named `BGO_B_<host>.dat`.

#### Name Channels 
To change the name of a single channel call 
```
./GRSIHVControl <slot> <channel> <name> -n <host>
```
where `<slot>` is slot number of the channel, `<channel>` is the channel number, `<name>` is the name of the channel (e.g. GRSxxyyzzA/B), and `<host>` is the name of the crate. 

Alternatively, you can change the names of multiple channels at once using
```
  ./GRSIHVControl <name file> -n <host>
````
where `<name file>` is a tsv text file containing the channel name and voltage change in the form 
```
slot1  channel1   name1
slot2  channel2   name2
...
slotN  channelN  nameN
```

# Helper scripts
The 'scripts' directory contains helper scripts to aid the user in utilizing
the HV control program. 

### ChangeHV.sh
This script updates/sets the voltages of the TIGRESS and GRIFFIN crates. 

##### Adjusting Voltages
Adjust voltages via 
``` 
./ChangeHV.sh crateMap.txt HV_Change.txt
``` 
in the `GRSIHVControl` parent directory where `<crateMap.txt>` is a text file mapping the crates to the positions they control. Examples for both TIGRESS (`tigCrateMap.txt`) and GRIFFIN (`grifCrateMap.txt`) are included in the directory. `HV_Change.txt` is the output file from Stephen's Bgo_fit function. 

##### Setting Voltages
Set voltages via 
``` 
./ChangeHV.sh crateMap.txt HV_Change.txt -s
``` 

*Note* You can enable an optional safety stop in the code by changing the variable 
```
 SAFETYCHECK=false
``` 
to 
```
 SAFETYCHECK=true
``` 
at line 23 in the script.

### GenerateNameMap.sh
This script helps generate a text file containing the names for GRIFFIN channels for use in [Name Channels](#name-channels). It is still under construction and has some bugs. Use at your own risk.
