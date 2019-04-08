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
	unsigned short int addr;
	unsigned short int n;
	unsigned short int dir;           //either xCADE , 0xDADA, or 0xF17E
	FILE* in_bin_file;
	FILE* out_txt_file;
	//open file
	in_bin_file = fopen(filename, "rb");
    
	//check if file is valid
	if (!in_bin_file) {
		printf("Invalid argument, Cannot read file %s.", filename);
		return 0;
	}
    
    //clear lc4 memory
    memset(CPU->memory, (unsigned short int) 0, sizeof(CPU->memory));
    
    //parse directives
	while (!feof(in_bin_file)) { 
        //read directive hex code
        if (fread(&dir, sizeof(unsigned short int), 1, in_bin_file) != 1) {
			printf("error. cannot read directive");
			return 0;
		}
        
		dir = (((dir >> 8) & 0x00ff) | ((dir << 8) & 0xff00)); //convert endianness
        
		if (dir == 0xCADE) {       //if next 2 bytes is CODE directive (0xCADE), call parseCODE()
			if (fread(&addr, sizeof(unsigned short int), 1, in_bin_file) != 1
				|| fread(&n, sizeof(unsigned short int), 1, in_bin_file) != 1) {
				printf("error. cannot read data");
				return 0;
			}
			addr = (((addr >> 8) & 0x00ff) | ((addr << 8) & 0xff00)); //convert endianness
			n = (((n >> 8) & 0x00ff) | ((n << 8) & 0xff00)); //convert endianness
			ParseCODE(addr, n, out_txt_file, in_bin_file, CPU);
		} else if (dir == 0xDADA) { //if next 2 bytes is DATA directive (0xDADA), call parseDATA()
			ParseDATA(out_txt_file, in_bin_file, CPU);
		} else if (dir == 0xF17E) {//if next 2 bytes is FILENAME directive (0xF17E), call parseFILENAME()
			ParseFILENAME(out_txt_file, in_bin_file, CPU);
		}
    } 
	
	fclose(in_bin_file);
	return 0;
}

int ParseCODE(unsigned short int addr, unsigned short int n,
			  FILE* out_txt_file, FILE* in_bin_file, MachineState* CPU){
	int i;
	unsigned short int prog[n];
	unsigned short int data;
	for (i = 0; i < n; i++) {
		if (fread(&data, sizeof(unsigned short int), 1, in_bin_file) != 1) {
			printf("error parsing CODE");
			return 0;
		}
		data = (((data >> 8) & 0x00ff) | ((data << 8) & 0xff00));
		printf("0x%X\n", data);
		prog[i] = data;
	}
	unsigned short int* mem = CPU->memory;
	memcpy(mem + addr, prog, sizeof(prog));
	return 1;
}
int ParseDATA(FILE* out_txt_file, FILE* in_bin_file, MachineState* CPU){
	return 0;
}
int ParseFILENAME(FILE* out_txt_file, FILE* in_bin_file, MachineState* CPU){
	return 0;
}