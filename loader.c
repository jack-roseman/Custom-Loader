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
	//open binary file
	in_bin_file = (FILE*) malloc(sizeof(FILE));
    in_bin_file = fopen(filename, "rb");
	//check if file is valid
	if (!in_bin_file) {
		printf("Invalid argument, Cannot read file %s.\n", filename);
		return 0;
	}
    //parse sections
	while (1) { 
        
        if (fread(&dir, sizeof(unsigned short int), 1, in_bin_file) == 0) {
            if (feof(in_bin_file)) {
                break;
            }
            printf("Error. Cannot parse");
            return 0;
        }
        
		dir = (((dir >> 8) & 0x00ff) | ((dir << 8) & 0xff00)); //convert endianness
		if (dir == 0xCADE) {       //if next 2 bytes is CODE directive (0xCADE), call parseCODE()
			if (fread(&addr, sizeof(unsigned short int), 1, in_bin_file) != 1
				|| fread(&n, sizeof(unsigned short int), 1, in_bin_file) != 1) {
				printf("Error. Cannot parse directive code 2.\n");
				return 0;
			}
			addr = (((addr >> 8) & 0x00ff) | ((addr << 8) & 0xff00)); //convert endianness
			n = (((n >> 8) & 0x00ff) | ((n << 8) & 0xff00)); //convert endianness
            printf("CODE:\n");
			ParseCODE(addr, n, out_txt_file, in_bin_file, CPU);
		} else if (dir == 0xDADA) { //if next 2 bytes is DATA directive (0xDADA), call parseDATA()
			if (fread(&addr, sizeof(unsigned short int), 1, in_bin_file) != 1
				|| fread(&n, sizeof(unsigned short int), 1, in_bin_file) != 1) {
				printf("Error. Cannot parse directive code 2.\n");
				return 0;
			}
			addr = (((addr >> 8) & 0x00ff) | ((addr << 8) & 0xff00)); //convert endianness
			n = (((n >> 8) & 0x00ff) | ((n << 8) & 0xff00)); //convert endianness
            printf("DATA:\n");
            ParseDATA(addr, n, out_txt_file, in_bin_file, CPU);
		} else if (dir == 0xF17E) {//if next 2 bytes is FILENAME directive (0xF17E), call parseFILENAME()
			ParseFILENAME(out_txt_file, in_bin_file, CPU);
		}
    } 

	fclose(in_bin_file);
	return 1;
}

int ParseCODE(unsigned short int addr, unsigned short int n,
			  FILE* out_txt_file, FILE* in_bin_file, MachineState* CPU){
	int i;
	unsigned short int prog[n];
	unsigned short int data;
	for (i = 0; i < n; i++) {
		if (fread(&data, sizeof(unsigned short int), 1, in_bin_file) != 1) {
			printf("Error parsing CODE.\n");
			return 0;
		}
		data = (((data >> 8) & 0x00ff) | ((data << 8) & 0xff00));
		prog[i] = data;
        printf("address: 0x%X   data: %d\n", addr+i, data);
	}
	memcpy(CPU->memory + addr, prog, n);
	return 1;
}

int ParseDATA(unsigned short int addr, unsigned short int n,
              FILE* out_txt_file, FILE* in_bin_file, MachineState* CPU){
    int i;
	unsigned short int prog[n];
	unsigned short int data;
	for (i = 0; i < n; i++) {
		if (fread(&data, sizeof(unsigned short int), 1, in_bin_file) != 1) {
			printf("Error parsing CODE.\n");
			return 0;
		}
		data = (((data >> 8) & 0x00ff) | ((data << 8) & 0xff00));
		prog[i] = data;
        printf("address: 0x%X   data: %d\n", addr+i, data);
	}
	memcpy(CPU->memory + addr, prog, n);
    
	return 1;
}
int ParseFILENAME(FILE* out_txt_file, FILE* in_bin_file, MachineState* CPU){
	return 0;
}