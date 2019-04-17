/*
 * LC4.c: Defines simulator functions for executing instructions
 */

#include "LC4.h"
#include <stdio.h>
#include <stdint.h>

#define DEBUG 0

//macros
#define IMM4(I) (((I & 0xF) >> 3) % 2 == 1 ? (I & 0xF) - 16 : (I & 0xF))
#define IMM5(I) (((I & 0x1F) >> 4) % 2 == 1 ? (I & 0x1F) - 32 : (I & 0x1F))
#define IMM6(I) (((I & 0x3F) >> 5) % 2 == 1 ? (I & 0x3F) - 64 : (I & 0x3F))
#define IMM7(I) (((I & 0x7F) >> 6) % 2 == 1 ? (I & 0x7F) - 128 : (I & 0x7F))
#define IMM9(I) (((I & 0x1FF) >> 8) % 2 == 1 ? (I & 0x1FF) - 512 : (I & 0x1FF))
#define IMM11(I) (((I & 0x7FF) >> 10) % 2 == 1 ? (I & 0x7FF) - 2048 : (I & 0x7FF))
#define UIMM7(I) (I & 0x7F)
#define INSTR_15_12(I) (I >> 12)
#define INSTR_11_9(I) ((I >> 9) & 0x7)
#define INSTR_8_6(I) ((I >> 6) & 0x7)
#define INSTR_5_3(I) ((I >> 3) & 0x7)
#define INSTR_2_0(I) (I & 0x7)
#define PRIVILEDGE_BIT(I) (I >> 15)

/*
 * Reset the machine state as Pennsim would do
 */
void Reset(MachineState* CPU) {
    CPU->PC = (uint16_t) 0x8200;
    CPU->PSR = (uint16_t) 0x8002;
    memset(CPU->R, 0, 8);

    //clear signals
    ClearSignals(CPU);
    CPU->regInputVal = 0;
    CPU->NZPVal = 0;
    CPU->dmemAddr = 0;
    CPU->dmemValue = 0;
}


/*
 * Clear all of the control signals (set to 0)
 */
void ClearSignals(MachineState* CPU) {
    CPU->rsMux_CTL  = 0;
    CPU->rtMux_CTL  = 0;
    CPU->rdMux_CTL  = 0;
    CPU->regFile_WE = 0;
    CPU->NZP_WE     = 0;
    CPU->DATA_WE    = 0;
}


/*
 * This function should write out the current state of the CPU to the file output.
 */
void WriteOut(MachineState* CPU, FILE* output) {
    int i;
    char instr[17];
    uint16_t hex_instr = CPU->memory[CPU->PC];
    fprintf(output, "%04X ", CPU->PC);    //print 
    for (i = 0; i < 16; i++) {
        if ((hex_instr >> (15 - i)) % 2 == 1) {
            instr[i] = '1';
        } else {
            instr[i] = '0';
        }
    }
    instr[16] = '\0';
    fprintf(output, "%s ", instr);          //print instruction
    fprintf(output, "%d ", CPU->regFile_WE);
    if (CPU->regFile_WE) {
        if (INSTR_15_12(hex_instr) == 0xF) {
            fprintf(output, "%d ", 7);
        } else {
            fprintf(output, "%d ", INSTR_11_9(hex_instr));
        }
        fprintf(output, "%04X ", CPU->regInputVal);
    } else {
        fprintf(output, "%d ", 0);
        fprintf(output, "%04X ", 0);
    }
    fprintf(output, "%d ", CPU->NZP_WE);
    if (CPU->NZP_WE) {
        fprintf(output, "%d ", CPU->NZPVal);
    } else {
        fprintf(output, "%d ", 0);
    }
    fprintf(output, "%d ", CPU->DATA_WE);
    if (CPU->DATA_WE || (INSTR_15_12(hex_instr) == 6)) {
        fprintf(output, "%04X ", CPU->dmemAddr);
        fprintf(output, "%04X ", CPU->dmemValue);
    } else {
        fprintf(output, "%04X ", 0);
        fprintf(output, "%04X ", 0);
    }
    fprintf(output, "\n");
}


/*
 * This function should execute one LC4 datapath cycle.
 */
int UpdateMachineState(MachineState* CPU, FILE* output) {
    //load instruction
    int i;
    char instr[17];
    uint8_t rd;
    uint8_t rs;
    uint8_t rt;
    uint16_t cnst;
    uint16_t old_pc;
    uint16_t hex_instr = CPU->memory[CPU->PC];
    if (DEBUG) {
        //convert instruction to string of "1" and "0" for debuging
        for (i = 0; i < 16; i++) {
            if ((hex_instr >> (15 - i)) % 2 == 1) {
                instr[i] = '1';
            } else {
                instr[i] = '0';
            }
        }
        instr[16] = '\0';
        printf("PC: 0x%X\n", CPU->PC);
        printf("Control Word: %s\n", instr);
    }
    // hex_instr >> 12 will give us uint16 of hex_instr[15:12]that we can use to identify
    switch (INSTR_15_12(hex_instr)) {
        case 0: //BR
            CPU->regFile_WE = 0;
            CPU->NZP_WE = 0;
            CPU->DATA_WE = 0;
            WriteOut(CPU, output);
            BranchOp(CPU, output);
            break;
        case 1: //ARITH
            CPU->rsMux_CTL = 0; 
            CPU->rtMux_CTL = 0; 
            CPU->rdMux_CTL = 0; 
            CPU->regFile_WE = 1;
            CPU->NZP_WE = 1;
            CPU->DATA_WE = 0;
            ArithmeticOp(CPU, output);
            CPU->PC = CPU->PC + 1;
            break;
        case 2: //CMP
            CPU->rsMux_CTL = 2; 
            CPU->rtMux_CTL = 0;
            CPU->regFile_WE = 0;
            CPU->NZP_WE = 1;
            CPU->DATA_WE = 0;
            ComparativeOp(CPU, output);
            CPU->PC = CPU->PC + 1;
            break;
        case 4: //JSR / JSRR
            CPU->rdMux_CTL = 1; 
            CPU->regFile_WE = 1;
            CPU->NZP_WE = 0;
            CPU->DATA_WE = 0;
            old_pc = CPU->PC;
            JSROp(CPU, output);
            if ((old_pc < 0x2000) && (CPU->PC >= 0x2000)) {
                printf("Illegal JMP/JMPR: cannot jump out of USER CODE section.\n");
                return 0;
            }
            if(((old_pc >= 0x8000) && (old_pc < 0xA000)) && 
               ((CPU->PC < 0x8000) || (CPU->PC >= 0xA000))) {
                printf("Illegal JMP/JMPR: cannot jump out of OS CODE section.\n");
                return 0;
            }
            WriteOut(CPU, output);
            break;
        case 5: //LOGICAL
            CPU->rsMux_CTL = 0; 
            CPU->rtMux_CTL = 0; 
            CPU->rdMux_CTL = 0; 
            CPU->regFile_WE = 1;
            CPU->NZP_WE = 1;
            CPU->DATA_WE = 0;
            LogicalOp(CPU, output);
            CPU->PC = CPU->PC + 1;
            break;
        case 6: //LDR
            //check PSR AND BOUNDS!!!!
            CPU->rsMux_CTL = 0; 
            CPU->rdMux_CTL = 0; 
            CPU->regFile_WE = 1;
            CPU->NZP_WE = 1;
            CPU->DATA_WE = 0;
            rd = INSTR_11_9(hex_instr);
            rs = INSTR_8_6(hex_instr);
            CPU->dmemAddr = CPU->R[rs] + IMM6(hex_instr);
            if ((CPU->PC >= 0x2000 && CPU->PC < 0x8000) || 
                (CPU->PC >= 0xA000 && CPU->PC <= 0xFFFF)) {
                //in data section so program exits
                printf("Illegal LDR: cannot read/write while executing in data section.\n");
                return 0;
            }
            if ((CPU->dmemAddr < 0x2000) || (CPU->dmemAddr > 0x7FFF && CPU->dmemAddr < 0xA000)) {
                printf("Illegal LDR: read address is in CODE section.\n");
                return 0;
            }
            if ((PRIVILEDGE_BIT(CPU->PSR) == 0) && (CPU->dmemAddr >= 0x8000)) {
                printf("Illegal LDR: load address not in User section and priviledge bit is false.\n");
                return 0;
            }
            CPU->R[rd] = CPU->memory[CPU->dmemAddr];
            CPU->dmemValue = CPU->R[rd];
            SetNZP(CPU, CPU->R[rd]);
            WriteOut(CPU, output);
            CPU->PC = CPU->PC + 1;
            break;
        case 7: //STR
            //check PSR AND BOUNDS!!!!
            CPU->rsMux_CTL = 0; 
            CPU->rtMux_CTL = 1; 
            CPU->regFile_WE = 0;
            CPU->NZP_WE = 0;
            CPU->DATA_WE = 1;
            rt = INSTR_11_9(hex_instr);
            rs = INSTR_8_6(hex_instr);
            CPU->dmemAddr = CPU->R[rs] + IMM6(hex_instr);
            if ((CPU->PC >= 0x2000 && CPU->PC < 0x8000) || 
                (CPU->PC >= 0xA000 && CPU->PC <= 0xFFFF)) {
                //curently executing in data section so program exits
                printf("Illegal STR: cannot read/write while executing in data section.\n");
                return 0;
            }
            if ((CPU->dmemAddr < 0x2000) || (CPU->dmemAddr > 0x7FFF && CPU->dmemAddr < 0xA000)) {
                printf("Illegal STR: write address is in CODE section.\n");
                return 0;
            }
            if ((PRIVILEDGE_BIT(CPU->PSR) == 0) && (CPU->dmemAddr >= 0x8000)) {
                printf("Illegal STR: priviledge bit is false and write address is in OS section.\n");
                return 0;
            }
            CPU->memory[CPU->dmemAddr] = CPU->R[rt];
            CPU->dmemValue = CPU->R[rt];
            WriteOut(CPU, output);
            CPU->PC = CPU->PC + 1;
            break;
        case 8: //RTI
            if (DEBUG) {printf("RTI\n");}
            CPU->rsMux_CTL = 1; 
            CPU->regFile_WE = 0;
            CPU->NZP_WE = 0;
            CPU->DATA_WE = 0;
            WriteOut(CPU, output);
            CPU->PC = CPU->R[7];
            CPU->PSR = CPU->PSR & 0x7FFF;
            break;
        case 9: //CONST
            CPU->rdMux_CTL = 0; 
            CPU->regFile_WE = 1;
            CPU->NZP_WE = 1;
            CPU->DATA_WE = 0;
            rd = (hex_instr & 0xE00) >> 9;
            cnst = hex_instr & 0x1FF;
            if (DEBUG) {
                printf("CONST R%d, #%d\n", rd, cnst);
            }
            CPU->R[rd] = cnst;
            CPU->regInputVal = cnst;
            SetNZP(CPU, cnst);
            WriteOut(CPU, output);
            CPU->PC = CPU->PC + 1;
            break;
        case 10: //SHIFT
            CPU->rsMux_CTL = 0;
            CPU->rdMux_CTL = 0; 
            CPU->regFile_WE = 1;
            CPU->NZP_WE = 0;
            CPU->DATA_WE = 0;
            ShiftModOp(CPU, output);
            CPU->PC = CPU->PC + 1;
            break;
        case 12: //JMP / JMPR
            CPU->rsMux_CTL = 0; 
            CPU->regFile_WE = 0;
            CPU->NZP_WE = 0;
            CPU->DATA_WE = 0;
            old_pc = CPU->PC;
            JumpOp(CPU, output);
            if ((old_pc < 0x2000) && (CPU->PC >= 0x2000)) {
                printf("Illegal JMP/JMPR: cannot jump out of USER CODE section.\n");
                return 0;
            }
            if(((old_pc >= 0x8000) && (old_pc < 0xA000)) && 
               ((CPU->PC < 0x8000) || (CPU->PC >= 0xA000))) {
                printf("Illegal JMP/JMPR: cannot jump out of OS CODE section.\n");
                return 0;
            }
            WriteOut(CPU, output);
            break;
        case 13: //HICONST
            CPU->rdMux_CTL = 0; 
            CPU->regFile_WE = 1;
            CPU->NZP_WE = 1;
            CPU->DATA_WE = 0;
            rd = INSTR_11_9(hex_instr);
            CPU->R[rd] = (CPU->R[rd] & 0xFF) | ((hex_instr & 0xFF) << 8);
            CPU->regInputVal = CPU->R[rd];
            SetNZP(CPU, CPU->R[rd]);
            WriteOut(CPU, output);
            CPU->PC = CPU->PC + 1;
            break;
        case 15: //TRAP
            CPU->rdMux_CTL = 1; 
            CPU->regFile_WE = 1;
            CPU->NZP_WE = 1;
            CPU->DATA_WE = 0;
            CPU->R[7] = CPU->PC + 1; //R7 = PC + 1
            CPU->regInputVal = CPU->R[7];
            SetNZP(CPU, CPU->R[7]);
            WriteOut(CPU, output);
            if ((hex_instr & 0xFF) == 0xFF) { //HALT
                if (DEBUG) {
                    printf("\nTRAP xFF ... LC4 HALTED\n");
                }
                return 0;
            }
            CPU->PC = 0x8000 | (hex_instr & 0xFF); //PC = 0x8000 | UIMM8;
            CPU->PSR = CPU->PSR | 0x8000; //PSR[15] = 1
            break;
        default:
            if (DEBUG) {printf("Shouldn't be here. \n");}
            return 0;

    }
    if (DEBUG) {
        printf("\n");
    }
    ClearSignals(CPU);
    return 1;
}



//////////////// PARSING HELPER FUNCTIONS ///////////////////////////



/*
 * Parses rest of branch operation and updates state of machine.
 */
void BranchOp(MachineState* CPU, FILE* output) {
    uint16_t hex_instr = CPU->memory[CPU->PC];
    switch (INSTR_11_9(hex_instr)) {
        case 4: //N
            if ((CPU->PSR >> 2) % 2 == 1) {
                CPU->PC = CPU->PC + 1 + IMM9(hex_instr);
                if (DEBUG) {printf("BRn ... PC = PC + %d\n", IMM9(hex_instr) + 1);}
            } else {
                CPU->PC = CPU->PC + 1;
                if (DEBUG) {printf("BRn ... No Branch\n");}
            }
            break;
        case 6: //N|Z
            if ((CPU->PSR >> 2) % 2 == 1 || (CPU->PSR >> 1) % 2 == 1 ) {
                CPU->PC = CPU->PC + 1 + IMM9(hex_instr);
                if (DEBUG) {printf("BRnz ... PC = PC + %d\n", IMM9(hex_instr) + 1);}
            } else {
                CPU->PC = CPU->PC + 1;
                if (DEBUG) {printf("BRnz ... No Branch\n");}
            }
            break;
        case 5: //N|P
            if ((CPU->PSR >> 2) % 2 == 1 || (CPU->PSR % 2) == 1 ) {
                CPU->PC = CPU->PC + 1 + IMM9(hex_instr);
                if (DEBUG) {printf("BRnp ... PC = PC + %d\n", IMM9(hex_instr) + 1);}
            } else {
                CPU->PC = CPU->PC + 1;
                if (DEBUG) {printf("BRnp ... No Branch\n");}
            }
            break;
        case 2: //Z
            if ((CPU->PSR >> 1) % 2 == 1) {
                CPU->PC = CPU->PC + 1 + IMM9(hex_instr);
                if (DEBUG) {printf("BRz ... PC = PC + %d\n", IMM9(hex_instr) + 1);}
            } else {
                CPU->PC = CPU->PC + 1;
                if (DEBUG) {printf("BRz ... No Branch\n");}
            }
            break;
        case 3: //Z|P
            if ((CPU->PSR >> 1) % 2 == 1 || (CPU->PSR % 2) == 1 ) {
                CPU->PC = CPU->PC + 1 + IMM9(hex_instr);
                if (DEBUG) { printf("BRzp ... PC = PC + %d\n", IMM9(hex_instr) + 1);}
            } else {
                CPU->PC = CPU->PC + 1;
                if (DEBUG) {printf("BRzp ... No Branch\n");}
            }
            break;
        case 1: //P
            if (CPU->PSR % 2 == 1) {
                CPU->PC = CPU->PC + 1 + IMM9(hex_instr);
                if (DEBUG) {printf("BRp ... PC = PC + %d\n", IMM9(hex_instr) + 1);}
            } else {
                CPU->PC = CPU->PC + 1;
                if (DEBUG) {printf("BRp ... No Branch\n");}
            }
            break;
        case 7: //forced branch
            CPU->PC = CPU->PC + 1 + IMM9(hex_instr);
            if (DEBUG) {printf("BRnzp ... PC = PC + %d\n", IMM9(hex_instr) + 1);}
            break;
        default: //do nothing
            if (DEBUG) {
                printf("DO NOTHING\n");
            }
            CPU->PC = CPU->PC + 1;
            break;
    }
}

/*
 * Parses rest of arithmetic operation and prints out.
 */
void ArithmeticOp(MachineState* CPU, FILE* output) {
    uint16_t hex_instr = CPU->memory[CPU->PC];
    uint8_t rd = INSTR_11_9(hex_instr);
    uint8_t rs = INSTR_8_6(hex_instr);
    uint8_t rt = INSTR_2_0(hex_instr);
    uint8_t sub_op_code = INSTR_5_3(hex_instr);
    int8_t imm5 = IMM5(hex_instr);
    switch (sub_op_code) {
        case 0: //add
            CPU->R[rd] = CPU->R[rs] + CPU->R[rt];
            if (DEBUG) {
                printf("ADD R%d, R%d R%d\n", rd, rs, rt);
            }
            break;
        case 1: //mult
            CPU->R[rd] = CPU->R[rs] * CPU->R[rt];
            if (DEBUG) {
                printf("MUL R%d, R%d R%d\n", rd, rs, rt);
            }
            break;
        case 2: //sub
            CPU->R[rd] = CPU->R[rs] - CPU->R[rt];
            if (DEBUG) {
                printf("SUB R%d, R%d R%d\n", rd, rs, rt);
            }
            break;
        case 3: //div
            if (CPU->R[rt]) {
                CPU->R[rd] = CPU->R[rs] / CPU->R[rt];
                if (DEBUG) {
                    printf("DIV R%d, R%d R%d\n", rd, rs, rt);
                }
            }
            break;
        default: //else add immediate
            CPU->R[rd] = CPU->R[rs] + imm5;
            if (DEBUG) {
                printf("ADD R%d, R%d #%d\n", rd, rs, imm5);
            }
            break;
    }
    
    CPU->regInputVal = CPU->R[rd]; 
    SetNZP(CPU, CPU->regInputVal);
    WriteOut(CPU, output);
    if (DEBUG) {
        printf("PC = PC + 1\n");
    }
}

/*
 * Parses rest of comparative operation and prints out.
 */
void ComparativeOp(MachineState* CPU, FILE* output) {
    uint16_t hex_instr = CPU->memory[CPU->PC];
    uint8_t rs = INSTR_11_9(hex_instr);
    uint8_t rt = INSTR_2_0(hex_instr);
    switch ((hex_instr >> 7) & 0x3) {
        case 0: //CMP
            if (DEBUG) {printf("CMP R%d, R%d\n", rs, rt);}
            SetNZP(CPU, (int16_t) CPU->R[rs] - (int16_t) CPU->R[rt]);
            break;
        case 1: //CMPU
            if (DEBUG) {printf("CMPU R%d, R%d\n", rs, rt);}
            SetNZP(CPU, (uint16_t) CPU->R[rs] - (uint16_t) CPU->R[rt]);
            break;
        case 2: //CMPI
            if (DEBUG) {printf("CMPI R%d, #%hi\n", rs, (uint16_t) IMM7(hex_instr));}
            SetNZP(CPU, (int16_t) CPU->R[rs] - (uint16_t) IMM7(hex_instr));
            break;
        case 3: //CMPIU
            if (DEBUG) {printf("CMPIU R%d, #%hu\n", rs, (uint16_t) UIMM7(hex_instr));}
            SetNZP(CPU, (uint16_t) CPU->R[rs] - (uint16_t) UIMM7(hex_instr));
            break;
        default:
            break;
    }
    WriteOut(CPU, output);
}

/*
 * Parses rest of logical operation and prints out.
 */
void LogicalOp(MachineState* CPU, FILE* output) {
    uint16_t hex_instr = CPU->memory[CPU->PC];
    uint8_t rd = INSTR_11_9(hex_instr);
    uint8_t rs = INSTR_8_6(hex_instr);
    uint8_t rt = INSTR_2_0(hex_instr);
    switch (INSTR_5_3(hex_instr)) {
        case 0: //and
            CPU->R[rd] = CPU->R[rs] & CPU->R[rt];
            if (DEBUG) {printf("AND R%d, R%d R%d\n", rd, rs, rt);}
            break;
        case 1: //not
            CPU->R[rd] = ~CPU->R[rs];
            if (DEBUG) {printf("NOT R%d, R%d\n", rd, rs);}
            break;
        case 2: //or
            CPU->R[rd] = CPU->R[rs] | CPU->R[rt];
            if (DEBUG) {printf("OR R%d, R%d R%d\n", rd, rs, rt);}
            break;
        case 3: //xor
            CPU->R[rd] = CPU->R[rs] ^ CPU->R[rt];
            if (DEBUG) {printf("XOR R%d, R%d R%d\n", rd, rs, rt);}
            break;
        default: //else and immediate
            CPU->R[rd] = CPU->R[rs] & IMM5(hex_instr);
            if (DEBUG) {printf("AND R%d, R%d #%d\n", rd, rs, IMM5(hex_instr));}
            break;
    }

    CPU->regInputVal = CPU->R[rd]; 
    SetNZP(CPU, CPU->regInputVal);
    WriteOut(CPU, output);
}

/*
 * Parses rest of jump operation and prints out.
 */
void JumpOp(MachineState* CPU, FILE* output) {
    uint16_t hex_instr = CPU->memory[CPU->PC];
    uint8_t rs = INSTR_8_6(hex_instr);
    switch ((hex_instr >> 11) & 0x1) {
        case 0: //JMPR
            CPU->PC = CPU->R[rs];
            break;
        case 1: //JMP
            CPU->PC = CPU->PC + 1 + IMM11(hex_instr);
            break;
        default:
            break;
    }
}

/*
 * Parses rest of JSR operation and prints out.
 */
void JSROp(MachineState* CPU, FILE* output) {
    uint16_t hex_instr = CPU->memory[CPU->PC];
    uint8_t rs = INSTR_8_6(hex_instr);
    CPU->R[7] = CPU->PC + 1; //R7 = PC + 1
    CPU->regInputVal = CPU->R[7];
    switch ((hex_instr >> 11) & 0x1) {
        case 0: //JSR
            CPU->PC = (CPU->PC & 0x8000) | (IMM11(hex_instr) << 4);;  //PC = (PC & 0x8000) | (imm11 << 4)
            break;
        case 1: //JSRR
            CPU->PC = CPU->R[rs];  //PC = Rs
            break;
        default:
            break;
    }
}

/*
 * Parses rest of shift/mod operations and prints out.
 */
void ShiftModOp(MachineState* CPU, FILE* output) {
    uint16_t hex_instr = CPU->memory[CPU->PC];
    uint8_t rd = INSTR_11_9(hex_instr);
    uint8_t rs = INSTR_8_6(hex_instr);
    uint8_t rt = INSTR_2_0(hex_instr);
    switch((hex_instr >> 4) & 0x3) {
        case 0: //SLL
            CPU->R[rd] = CPU->R[rs] << IMM4(hex_instr);
            break;
        case 1: //SRA
            CPU->R[rd] = (CPU->R[rs] >> IMM4(hex_instr)) | (CPU->R[rs] | 0x8000);
            break;
        case 2: //SRL
            CPU->R[rd] = CPU->R[rs] >> IMM4(hex_instr);
            break;
        case 3: //MOD
            CPU->R[rd] = CPU->R[rs] % CPU->R[rt];
            break;
        default:
            break;
    }
    CPU->regInputVal = CPU->R[rd];
    SetNZP(CPU, CPU->regInputVal);
    WriteOut(CPU, output);
}

/*
 * Set the NZP bits in the PSR.
 */
void SetNZP(MachineState* CPU, short result) {
    uint16_t nzp = 0;
    if (result < 0) { //NZP = 100
        nzp = nzp | 0x4;
    } else if (result > 0) { //NZP = 001
        nzp = nzp | 0x1;
    } else { //NZP = 010
        nzp = nzp | 0x2;
    }
    CPU->NZPVal = nzp;
    CPU->PSR = (CPU->PSR & 0x8000) | nzp;
    if (DEBUG) {
        printf("PSR: 0x%X\n", CPU->PSR);
    }
}
