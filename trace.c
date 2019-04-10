/*
 * trace.c: location of main() to start the simulator
 */

#include "loader.h"

// Global variable defining the current state of the machine
MachineState* CPU;

int main(int argc, char** argv){
    int i;
    int j;
    char ext[5];
    char* d;
    FILE* out_txt_file;
    
	if (argc == 1 || argc == 2) {
		printf("Invalid argument to main(). Expected <./trace <filename>.txt <filename>.obj <filename>.obj...>\n");
		return 0;
	}
    
    
    for (i = 0; i < 4; i++) {
        ext[i] = argv[1][strlen(argv[1]) - i - 1];
    }
    ext[4] = '\0';
    if (strncmp(ext, "txt.", 4) != 0) {
        printf("Invalid argument to main(). Missing output .txt file.\n");
        return 0;
    }
    
	CPU = (MachineState*) malloc(sizeof(CPU));
    //clear lc4 memory
    memset(CPU->memory, 0, sizeof(CPU->memory));
    
    //make sure files end with .obj before reading
    for (i = 2; i < argc; i++) {
        for (j = 0; j < 4; j++) {
            ext[j] = argv[i][strlen(argv[i]) - j - 1];
        }
        if (strncmp(ext, "jbo.", 4) != 0) {
            printf("Invalid argument to main(). Missing input .obj file.\n");
            return 0;
        }
        //populate LC4 memory from binary files
        ReadObjectFile(argv[i], CPU);
    }
    
    //write memory to output .txt file
    out_txt_file = (FILE*) malloc(sizeof(FILE));
    out_txt_file = fopen(argv[1], "w");
    //print out data at every address
    for (i = 0; i < 6500; i++) {
        unsigned short int* mem = CPU->memory;
            fprintf(out_txt_file, "address: %d%d%d%d%d contents: 0x%X\n", 
               (i / 10000) % 10 ,
               (i / 1000) % 10,
               (i / 100) % 10, 
               (i / 10) % 10 , 
               i % 10, mem[i]);
        //}
    }
    
    fclose(out_txt_file);
    
	free(CPU);
    return 0;
}