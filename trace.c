/*
 * trace.c: location of main() to start the simulator
 */

#include "loader.h"

// Global variable defining the current state of the machine
MachineState* CPU;

int main(int argc, char** argv)
{
	if (argc == 0 || argc == 1) {
		printf("Invalid argument to main(). Expected <./trace output_filename.txt input1_filename.obj ...>");
		return 0;
	}
	printf("%s\n", argv[1]);
	CPU = (MachineState*) malloc(sizeof(CPU));
	//clear lc4 memory
	printf("CPU: %p\n", CPU);
    printf("PC: %p\n", &CPU->PC);
    printf("PSR: %p\n", &CPU->PSR);
    printf("R: %p\n", CPU->R);
    printf("rsMux_CTL: %p\n", &CPU->rsMux_CTL);
    printf("rtMux_CTL: %p\n", &CPU->rtMux_CTL);
    printf("rdMux_CTL: %p\n", &CPU->rdMux_CTL);
    printf("regFile_WE: %p\n", &CPU->regFile_WE);
    printf("NZP_WE: %p\n", &CPU->NZP_WE);
    printf("DATA_WE: %p\n", &CPU->DATA_WE);
    printf("regInputVal: %p\n", &CPU->regInputVal);
    printf("NZPVal: %p\n", &CPU->NZPVal);
    printf("dmemAddr: %p\n", &CPU->dmemAddr);
    printf("dmemValue: %p\n", &CPU->dmemValue);
    printf("Memory: %p\n", CPU->memory);
    
    memset(CPU->memory, '0', sizeof(CPU->memory));
	ReadObjectFile(argv[1], CPU);
    
//     free(&CPU->PC);
//     free(&CPU->PSR);
//     free(CPU->R);
//     free(&CPU->rsMux_CTL);
//     free(&CPU->rtMux_CTL);
//     free(&CPU->rdMux_CTL);
//     free(&CPU->rdMux_CTL);
//     free(&CPU->regFile_WE);
//     free(&CPU->NZP_WE);
//     free(&CPU->DATA_WE);
//     free(&CPU->regInputVal);
//     free(&CPU->NZPVal);
//     free(&CPU->dmemAddr);
//     free(&CPU->dmemValue);
	free(CPU);
    return 0;
}