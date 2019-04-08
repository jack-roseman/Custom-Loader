/*
 * loader.c : Defines loader functions for opening and loading object files
 */

#include "loader.h"

// memory array location
unsigned short memoryAddress;

/*
 * Read an object file and modify the machine state as described in the writeup
 */
int ReadObjectFile(char* filename, MachineState* CPU) {
	unsigned short int buffer;
	FILE* in_bin_file;
	FILE* out_txt_file;
	//open file
	in_bin_file = fopen(filename, "rb");
	//check if file is valid
	if (!in_bin_file) {
		printf("Invalid argument, Cannot read file %s.", filename);
		return 0;
	}
	while (!feof(in_bin_file)) { 
        fread(&buffer, sizeof(unsigned short int), 1, in_bin_file); //read next 2 bytes into buffer
		buffer = (((buffer >> 8) & 0x00ff) | ((buffer << 8) & 0xff00));
		if (buffer == 0xCADE) { //if next 2 bytes is CODE directive (0xCADE), call parseCODE()
			ParseCODE(out_txt_file, in_bin_file, CPU);
			printf("0x%X\n", buffer);
		} else if (buffer == 0xDADA) { //if next 2 bytes is DATA directive (0xDADA), call parseDATA()
			ParseDATA(out_txt_file, in_bin_file, CPU);
			printf("0x%X\n", buffer);
		} else if (buffer == 0xF17E) {//if next 2 bytes is FILENAME directive (0xF17E), call parseFILENAME()
			ParseFILENAME(out_txt_file, in_bin_file, CPU);
			printf("0x%X\n", buffer);
		}
    } 
	
	
	
	fclose(in_bin_file);
  return 0;
}

int ParseCODE(FILE* out_txt_file, FILE* in_bin_file, MachineState* CPU){
	return 0;
}
int ParseDATA(FILE* out_txt_file, FILE* in_bin_file, MachineState* CPU){
	return 0;
}
int ParseFILENAME(FILE* out_txt_file, FILE* in_bin_file, MachineState* CPU){
	return 0;
}