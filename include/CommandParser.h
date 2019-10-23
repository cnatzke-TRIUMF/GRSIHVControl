#ifndef COMMANDPARSER_HEADER
#define COMMANDPARSER_HEADER

// INCLUDES
#include <signal.h>
#include <sys/types.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include "CAENHVWrapper.h"
#include "CrateCommands.h"

#define MAX_HVPS (5) // maximum of 5 HV crates at one time


// prototypes
void ParseInputs(int argc, char *argv[]);

// variables
int hvSysHandle;
float chanV;

// crate map variables
#endif
