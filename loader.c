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
    unsigned short int file_index;
	unsigned short int dir;           //either xCADE , 0xDADA, or 0xF17E
    unsigned short int line;
	FILE* in_bin_file = NULL;
    
	//open binary file
	in_bin_file = (FILE*) malloc(sizeof(FILE));
    in_bin_file = fopen(filename, "rb");
    
	//check if file is valid
	if (!in_bin_file) {
		printf("Invalid argument. Cannot read file %s.\n", filename);
		return 0;
	}
    
    //parse sections
	while (1) { 
        if (fread(&dir, sizeof(unsigned short int), 1, in_bin_file) == 0) {
            if (feof(in_bin_file)) {
                break;
            }
            printf("Error. Cannot parse 0.\n");
            return 0;
        }
        
		dir = (((dir >> 8) & 0x00ff) | ((dir << 8) & 0xff00)); //convert endianness
		if (dir == 0xCADE) {       //if next 2 bytes is CODE directive (0xCADE), call parseCODE()
			if (fread(&addr, sizeof(unsigned short int), 1, in_bin_file) != 1
				|| fread(&n, sizeof(unsigned short int), 1, in_bin_file) != 1) {
				printf("Error. Cannot parse header.\n");
                fclose(in_bin_file);
				return 0;
			}
			addr = (((addr >> 8) & 0x00ff) | ((addr << 8) & 0xff00)); //convert endianness
			n = (((n >> 8) & 0x00ff) | ((n << 8) & 0xff00)); //convert endianness
            printf("CODE:\n");
			ParseCODE(addr, n, in_bin_file, CPU);
		} else if (dir == 0xDADA) { //if next 2 bytes is DATA directive (0xDADA), call parseDATA()
			if (fread(&addr, sizeof(unsigned short int), 1, in_bin_file) != 1
				|| fread(&n, sizeof(unsigned short int), 1, in_bin_file) != 1) {
				printf("Error. Cannot parse DATA.\n");
                fclose(in_bin_file);
				return 0;
			}
			addr = (((addr >> 8) & 0x00ff) | ((addr << 8) & 0xff00)); //convert endianness
			n = (((n >> 8) & 0x00ff) | ((n << 8) & 0xff00)); //convert endianness
            printf("DATA:\n");
            ParseDATA(addr, n, in_bin_file, CPU);
		} else if (dir == 0xC3B7) {//if next 2 bytes is SYMBOL directive (0xC3B7), call ParseSYMBOL()
            if (fread(&addr, sizeof(unsigned short int), 1, in_bin_file) != 1
				|| fread(&n, sizeof(unsigned short int), 1, in_bin_file) != 1) {
				printf("Error. Cannot parse SYMBOL.\n");
                fclose(in_bin_file);
				return 0;
			}
            addr = (((addr >> 8) & 0x00ff) | ((addr << 8) & 0xff00)); //convert endianness
			n = (((n >> 8) & 0x00ff) | ((n << 8) & 0xff00)); //convert endianness
			ParseSYMBOL(addr, n, in_bin_file, CPU);
		} else if (dir == 0xF17E) {//if next 2 bytes is FILENAME directive (0xF17E), call ParseFILENAME()
            if (fread(&n, sizeof(unsigned short int), 1, in_bin_file) != 1) {
				printf("Error. Cannot parse FILENAME.\n");
                fclose(in_bin_file);
				return 0;
			}
            n = (((n >> 8) & 0x00ff) | ((n << 8) & 0xff00)); //convert endianness
            printf("FILENAME:\n");
			ParseFILENAME(n, in_bin_file, CPU);
		} else if (dir == 0x715E) {//if next 2 bytes is l directive (0x715E), call Parse LINENUM
            if (fread(&addr, sizeof(unsigned short int), 1, in_bin_file) != 1
				|| fread(&line, sizeof(unsigned short int), 1, in_bin_file) != 1
                || fread(&file_index, sizeof(unsigned short int), 1, in_bin_file) != 1) {
				printf("Error. Cannot parse LINENUM.\n");
                fclose(in_bin_file);
				return 0;
			}
            addr = (((addr >> 8) & 0x00ff) | ((addr << 8) & 0xff00)); //convert endianness
			line = (((line >> 8) & 0x00ff) | ((line << 8) & 0xff00)); //convert endianness
            file_index = (((file_index >> 8) & 0x00ff) | ((file_index << 8) & 0xff00)); //convert endianness
			printf("LINENUM: 0x%X", line); //do something with data
		}
    } 

	fclose(in_bin_file); //frees in_bin_file
	return 1;
}

int ParseCODE(unsigned short int addr, unsigned short int n,
			  FILE* in_bin_file, MachineState* CPU){
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
        printf("address: 0x%X   data: 0x%X\n", addr+i, prog[i]);
	} 

	memcpy(CPU->memory + addr, prog, sizeof(prog));
	return 1;
}

int ParseDATA(unsigned short int addr, unsigned short int n,
			  FILE* in_bin_file, MachineState* CPU){
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
        printf("address: 0x%X   data: 0x%X\n", addr+i, prog[i]);
	} 
	memcpy(CPU->memory + addr, prog, sizeof(prog));
	return 1;
}

int ParseSYMBOL(unsigned short int addr, unsigned short int n,
              FILE* in_bin_file, MachineState* CPU){
    int i;
	unsigned char symb[n + 1];
	unsigned char ch;
	for (i = 0; i < n; i++) {
		if (fread(&ch, sizeof(unsigned char), 1, in_bin_file) != 1) {
			printf("Error parsing SYMBOL.\n");
			return 0;
		}
		symb[i] = ch;
	}
    symb[n] = '\0';
    printf("label: %s\n", (unsigned char*) symb);
	return 1;
}

int ParseFILENAME(unsigned short n, FILE* in_bin_file, MachineState* CPU) {
    int i;
	unsigned short int filename[n + 1];
	unsigned char ch;
	for (i = 0; i < n; i++) {
		if (fread(&ch, sizeof(unsigned char), 1, in_bin_file) != 1) {
			printf("Error parsing CODE.\n");
			return 0;
		}
		filename[i] = ch;
	} 
    filename[n] = '\0';
	printf("filename: %s\n", (unsigned char*) filename);
	return 0;
}
