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
	CPU = malloc(sizeof(CPU));
	
	ReadObjectFile(argv[1], CPU);
    free(CPU->memory);
	free(CPU);
    return 0;
}