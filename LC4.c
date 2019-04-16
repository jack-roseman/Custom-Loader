/*
 * LC4.c: Defines simulator functions for executing instructions
 */

#include "LC4.h"
#include <stdio.h>

/*
 * Reset the machine state as Pennsim would do
 */
void Reset(MachineState* CPU) {
	CPU->PC = (unsigned short int) 0x8200;
    CPU->PSR = (unsigned short int) 0x8002;
    memset(CPU->R, (unsigned short int) 0x0000, 8);
    ClearSignals(CPU);
    CPU->regInputVal = (unsigned short int) 0x0000;
    CPU->NZPVal = (unsigned short int) 0x0000;
    CPU->dmemAddr = (unsigned short int) 0x0000;
    CPU->dmemValue = (unsigned short int) 0x0000;
}


/*
 * Clear all of the control signals (set to 0)
 */
void ClearSignals(MachineState* CPU) {
    CPU->rsMux_CTL = (unsigned char) '0';
    CPU->rtMux_CTL = (unsigned char) '0';
    CPU->rdMux_CTL = (unsigned char) '0';
    CPU->regFile_WE = (unsigned char) '0';
    CPU->NZP_WE = (unsigned char) '0';
    CPU->DATA_WE= (unsigned char) '0';
}


/*
 * This function should write out the current state of the CPU to the file output.
 */
void WriteOut(MachineState* CPU, FILE* output) {
    
}


/*
 * This function should execute one LC4 datapath cycle.
 */
int UpdateMachineState(MachineState* CPU, FILE* output) {
    //load instruction
    int i;
    char instr[17];
    unsigned short int rd;
    unsigned short int rs;
    unsigned short int rt;
    unsigned short int val;
    unsigned short int hex_instr = CPU->memory[CPU->PC];
    
     //convert instruction to string of "1" and "0"
    for (i = 0; i < 16; i++) {
        if ((hex_instr >> (15 - i)) % 2 == 1) {
            instr[i] = '1';
        } else {
            instr[i] = '0';
        }
    }
    instr[16] = '\0';
    
    
    printf("PC: 0x%X\n", CPU->PC);
    //printf("Control Word: %s\n", instr);
    
    switch (hex_instr >> 12) {
        case 0: //BR
            BranchOp(CPU, output);
            break;
        case 1: //ARITH
            ArithmeticOp(CPU, output);
            break;
        case 2: //CMP
            ComparativeOp(CPU, output);
            break;
        case 4: //JSR / JSRR
            JSROp(CPU, output);
            break;
        case 5: //LOGICAL
            LogicalOp(CPU, output);
            break;
        case 6: //LDR
            break;
        case 7: //STR
            break;
        case 8: //RTI
            printf("RTI\n");
            memcpy(&CPU->PC, &CPU->R[7], 2);
            CPU->PSR = CPU->PSR & 0x7FFF;
            break;
        case 9: //CONST
            rd = (hex_instr & 0x0E00) >> 9;
            val = hex_instr & 0x01FF;
            printf("CONST R%d, #%d\n", rd, val);
            CPU->R[rd] = val;
            CPU->PC = CPU->PC + 1;
            break;
        case 10: //SHIFT
            ShiftModOp(CPU, output);
            break;
        case 12: //JMP / JMPR
            JumpOp(CPU, output);
            break;
        case 13: //HICONST
            break;
        case 15: //TRAP
            break;
        default:
            break;
            
    }
    printf("\n");
    WriteOut(CPU, output);
    return 0;
}



//////////////// PARSING HELPER FUNCTIONS ///////////////////////////



/*
 * Parses rest of branch operation and updates state of machine.
 */
void BranchOp(MachineState* CPU, FILE* output) {
    unsigned short int hex_instr = CPU->memory[CPU->PC];
    unsigned short int nzp = hex_instr & 0x0E00 >> 9;
    short int imm9 = (short int) hex_instr & 0x1FF;
    printf("0x%X\n", hex_instr);
    switch (nzp) {
        case 4: //N
            if ((CPU->PSR >> 2) % 2 == 1) {
                CPU->PC = CPU->PC + 1 + imm9;
                printf("BRn: PC = 0x%X\n", CPU->PC);
            } else {
                CPU->PC = CPU->PC + 1;
            }
            break;
        case 6: //N|Z
            if ((CPU->PSR >> 2) % 2 == 1 || (CPU->PSR >> 1) % 2 == 1 ) {
                CPU->PC = CPU->PC + 1 + imm9;
                 printf("BRnz: PC = 0x%X\n", CPU->PC);
            } else {
                CPU->PC = CPU->PC + 1;
            }
            break;
        case 5: //N|P
            if ((CPU->PSR >> 2) % 2 == 1 || (CPU->PSR % 2) == 1 ) {
                CPU->PC = CPU->PC + 1 + imm9;
                 printf("BRnp: PC = 0x%X\n", CPU->PC);
            } else {
                CPU->PC = CPU->PC + 1;
            }
            break;
        case 2: //Z
            if ((CPU->PSR >> 1) % 2 == 1) {
                CPU->PC = CPU->PC + 1 + imm9;
                 printf("BRz: PC = 0x%X\n", CPU->PC);
            } else {
                CPU->PC = CPU->PC + 1;
            }
            break;
        case 3: //Z|P
            if ((CPU->PSR >> 1) % 2 == 1 || (CPU->PSR % 2) == 1 ) {
                CPU->PC = CPU->PC + 1 + imm9;
                 printf("BRzp: PC = 0x%X\n", CPU->PC);
            } else {
                CPU->PC = CPU->PC + 1;
            }
            break;
        case 1: //P
            if (CPU->PSR % 2 == 1) {
                CPU->PC = CPU->PC + 1 + imm9;
                 printf("BRp: PC = 0x%X\n", CPU->PC);
            } else {
                CPU->PC = CPU->PC + 1;
            }
            break;
        case 7: //forced branch
            CPU->PC = CPU->PC + 1 + imm9;
            printf("BRnzp: PC = 0x%X\n", CPU->PC);
            break;
        default: //do nothing
            break;
    }
}

/*
 * Parses rest of arithmetic operation and prints out.
 */
void ArithmeticOp(MachineState* CPU, FILE* output) {
    unsigned short int hex_instr = CPU->memory[CPU->PC];
    unsigned short int rd = (hex_instr & 0x0E00) >> 9;
    unsigned short int rs = (hex_instr & 0x01C0) >> 6;
    unsigned short int rt = hex_instr & 0x0007;
    unsigned short int op = (hex_instr >> 3) & 0x0007;
    short int imm5 = (hex_instr & 0x001F) - 32;

    switch (op) {
        case 0: //add
            CPU->R[rd] = CPU->R[rs] + CPU->R[rt];
            printf("ADD R%d, R%d R%d\n", rd, rs, rt);
            break;
        case 1: //mult
            CPU->R[rd] = CPU->R[rs] * CPU->R[rt];
            printf("MUL R%d, R%d R%d\n", rd, rs, rt);
            break;
        case 2: //sub
            CPU->R[rd] = CPU->R[rs] - CPU->R[rt];
             printf("SUB R%d, R%d R%d\n", rd, rs, rt);
            break;
        case 3: //div
            CPU->R[rd] = CPU->R[rs] / CPU->R[rt];
            printf("DIV R%d, R%d R%d\n", rd, rs, rt);
            break;
        default: //else add immediate
            CPU->R[rd] = CPU->R[rs] + imm5;
            printf("ADD R%d, R%d #%d\n", rd, rs, imm5);
            break;
    }
    
    CPU->PC = CPU->PC + 1;
}

/*
 * Parses rest of comparative operation and prints out.
 */
void ComparativeOp(MachineState* CPU, FILE* output) {
    unsigned short int hex_instr = CPU->memory[CPU->PC];
    unsigned short int rs = (hex_instr & 0x0E00) >> 9;
    unsigned short int rt = hex_instr & 0x0007;
    unsigned short int op = (hex_instr >> 7) & 3;
    short int imm7 = (short int) (hex_instr & 0x007F);
    unsigned short int uimm7 = hex_instr & 0x007F;
    short nzp = 0x0000;
    switch (op) {
        case 0: //CMP
            printf("CMP R%d, R%d\n", rs, rt);
            if ((short) CPU->R[rs] - (short) CPU->R[rt] < 0) { //NZP = 100
                nzp = nzp | 0x0004;
            } else if ((short) CPU->R[rs] - (short) CPU->R[rt] > 0) { //NZP = 001
                nzp = nzp | 0x0001;
            } else { //NZP = 010
                nzp = nzp | 0x0002;
            }
            break;
        case 1: //CMPU
            printf("CMPU R%d, R%d\n", rs, rt);
            if (CPU->R[rs] - CPU->R[rt] < 0) { //NZP = 100
                nzp = nzp | 0x0004;
            } else if (CPU->R[rs] - CPU->R[rt] > 0) { //NZP = 001
                nzp = nzp | 0x0001;
            } else { //NZP = 010
                nzp = nzp | 0x0002;
            }
            break;
        case 2: //CMPI
            printf("CMPI R%d, #%d\n", rs, imm7);
            if ((short) CPU->R[rs] - imm7 < 0) { //NZP = 100
                nzp = nzp | 0x0004;
            } else if ((short) CPU->R[rs] - imm7 > 0) { //NZP = 001
                nzp = nzp | 0x0001;
            } else { //NZP = 010
                nzp = nzp | 0x0002;
            }
            break;
        case 3: //CMPIU
            printf("CMPIU R%d, #%d\n", rs, uimm7);
            if (CPU->R[rs] - uimm7 < 0) { //NZP = 100
                nzp = nzp | 0x0004;
            } else if (CPU->R[rs] - uimm7 > 0) { //NZP = 001
                nzp = nzp | 0x0001;
            } else { //NZP = 010
                nzp = nzp | 0x0002;
            }
            break;
        default:
            break;
    }
    SetNZP(CPU, nzp);
    CPU->PC = CPU->PC + 1;
}

/*
 * Parses rest of logical operation and prints out.
 */
void LogicalOp(MachineState* CPU, FILE* output) {
    unsigned short int hex_instr = CPU->memory[CPU->PC];
    unsigned short int rd = (hex_instr & 0x0E00) >> 9;
    unsigned short int rs = (hex_instr & 0x01C0) >> 6;
    unsigned short int rt = hex_instr & 0x0007;
    unsigned short int op = hex_instr & 0x0038 >> 3;
    short int imm5 = (hex_instr & 0x001F) - 32;

    switch (op) {
        case 0: //and
            CPU->R[rd] = CPU->R[rs] & CPU->R[rt];
            printf("AND R%d, R%d R%d\n", rd, rs, rt);
            break;
        case 1: //not
            CPU->R[rd] = ~CPU->R[rs];
            printf("NOT R%d, R%d\n", rd, rs);
            break;
        case 2: //or
            CPU->R[rd] = CPU->R[rs] | CPU->R[rt];
             printf("OR R%d, R%d R%d\n", rd, rs, rt);
            break;
        case 3: //xor
            CPU->R[rd] = CPU->R[rs] ^ CPU->R[rt];
            printf("XOR R%d, R%d R%d\n", rd, rs, rt);
            break;
        default: //else and immediate
            CPU->R[rd] = CPU->R[rs] & imm5;
            printf("AND R%d, R%d #%d\n", rd, rs, imm5);
            break;
    }
    
    CPU->PC = CPU->PC + 1;
}

/*
 * Parses rest of jump operation and prints out.
 */
void JumpOp(MachineState* CPU, FILE* output) {
    
}

/*
 * Parses rest of JSR operation and prints out.
 */
void JSROp(MachineState* CPU, FILE* output) {
    
}

/*
 * Parses rest of shift/mod operations and prints out.
 */
void ShiftModOp(MachineState* CPU, FILE* output) {
    
}

/*
 * Set the NZP bits in the PSR.
 */
void SetNZP(MachineState* CPU, short result) {
    CPU->PSR = (CPU->PSR & 0x8000) | result;
    printf("PSR: 0x%X\n", CPU->PSR);
}
