/*
 * loader.h: Declares loader functions for opening and loading object files
 */

#include <stdio.h>
#include "LC4.h"

// Read an object file and modify the machine state as described in the writeup
int ReadObjectFile(char* filename, MachineState* CPU);
int ParseCODE(unsigned short int addr, unsigned short int n,
			  FILE* out_txt_file, FILE* in_bin_file, 
			  MachineState* CPU);
int ParseDATA(unsigned short int addr, unsigned short int n,
              FILE* out_txt_file, FILE* in_bin_file, MachineState* CPU);
int ParseFILENAME(FILE* out_txt_file, FILE* in_bin_file, MachineState* CPU);