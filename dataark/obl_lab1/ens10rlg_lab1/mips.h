/*
 * main.h
 *
 *  Created on: Nov 18, 2013
 *      Author: lundburgerr
 */

#ifndef MAIN_H_
#define MAIN_H_

//Constant definitions
#define INVALIDFUNC -1
#define NO_INSTRUCTION "not a valid instruction!"



enum format{R, I, J, INVALIDOP};

//Add format of print out
struct funccode_t{
	int funcnum;
	char *name;
};

//Add funcstart in opcode
struct opcode_t{
	enum format type;
	int opnum;
	struct funccode_t *func;
	int funcstart;
	unsigned int maxfunc;
};

char *registers[32] = {"$zero", "$at", "$v0", "$v1", "$a0", "$a1", "$a2", "$a0", \
		"$t0", "$t1", "$t2", "$t3", "$t4", "$t5", "$t6", "$t7", \
		"$s0", "$s1", "$s2", "$s3", "$s4", "$s5", "$s6", "$s7", \
		"$t8", "$t9", "$k0", "$k1", "$gp", "$sp", "$fp", "$ra"
};

struct funccode_t funk2op0[] = {
		{0x0, "sll %rd%, %rt, %shamt%"},
		{0x1, "movf %rd%, %rs%, %rt%"},
		{0x2, "srl %rd%, %rt%, %shamt%"},
		{0x3, "sra %rd%, %rt%, %shamt%"},
		{0x4, "sllv %rd%, %rt%, %rs%"},
		{INVALIDFUNC, NO_INSTRUCTION},
		{0x6, "srlv %rd%, %rt%, %rs%"},
		{0x7, "srav %rd%, %rt%, %rs%"},
		{0x8, "jr %rs%"},
		{0x9, "jalr %rs%, %rd%"},
		{0xa, "movz %rd%, %rs%, %rt%"},
		{0xb, "movn %rd%, %rs%, %rt%"},
		{0xc, "syscall"},
		{0xd, "break"},
		{INVALIDFUNC, NO_INSTRUCTION},
		{INVALIDFUNC, NO_INSTRUCTION},
		{0x10, "mfhi %rd%"},
		{0x11, "mthi %rs%"},
		{0x12, "mflo %rd%"},
		{0x13, "mtlo %rs%"},
		{INVALIDFUNC, NO_INSTRUCTION},
		{INVALIDFUNC, NO_INSTRUCTION},
		{INVALIDFUNC, NO_INSTRUCTION},
		{INVALIDFUNC, NO_INSTRUCTION},
		{0x18, "mult %rs%, %rt%"},
		{0x19, "multu %rs%, %rt%"},
		{0x1a, "div %rs%, %rt%"},
		{0x1b, "divu %rs%, %rt%"},
		{INVALIDFUNC, NO_INSTRUCTION},
		{INVALIDFUNC, NO_INSTRUCTION},
		{INVALIDFUNC, NO_INSTRUCTION},
		{INVALIDFUNC, NO_INSTRUCTION},
		{0x20, "add %rd%, %rs%, %rt%"},
		{0x21, "addu %rd%, %rs%, %rt%"},
		{0x22, "sub %rd%, %rs%, %rt%"},
		{0x23, "subu %rd%, %rs%, %rt%"},
		{0x24, "and %rd%, %rs%, %rt%"},
		{0x25, "or %rd%, %rs%, %rt%"},
		{0x26, "xor %rd%, %rs%, %rt%"},
		{0x27, "nor %rd%, %rs%, %rt%"},
		{INVALIDFUNC, NO_INSTRUCTION},
		{INVALIDFUNC, NO_INSTRUCTION},
		{0x2a, "slt %rd%, %rs%, %rt%"},
		{0x2b, "sltu %rd%, %rs%, %rt%"},
		{INVALIDFUNC, NO_INSTRUCTION},
		{INVALIDFUNC, NO_INSTRUCTION},
		{INVALIDFUNC, NO_INSTRUCTION},
		{INVALIDFUNC, NO_INSTRUCTION},
		{0x30, "tge %rs%, %rt%"},
		{0x31, "tgeu %rs%, %rt%"},
		{0x32, "tlt %rs%, %rt%"},
		{0x33, "tltu %rs%, %rt%"},
		{0x34, "teq %rs%, %rt%"},
		{INVALIDFUNC, NO_INSTRUCTION},
		{0x36, "tne %rs%, %rt%"}
};
struct funccode_t funk2op1[] = {
		{0x0, "bltz %rs%, %imm%"},
		{0x1, "bgez %rs%, %imm%"},
		{INVALIDFUNC, NO_INSTRUCTION},
		{INVALIDFUNC, NO_INSTRUCTION},
		{INVALIDFUNC, NO_INSTRUCTION},
		{INVALIDFUNC, NO_INSTRUCTION},
		{INVALIDFUNC, NO_INSTRUCTION},
		{INVALIDFUNC, NO_INSTRUCTION},
		{0x8, "tgei %rs%, %imm%"},
		{0x9, "tgeiu %rs%, %imm%"},
		{0xa, "tlti %rs%, %imm%"},
		{0xb, "tltiu %rs%, %imm%"},
		{0xc, "teqi %rs%, %imm%"},
		{INVALIDFUNC, NO_INSTRUCTION},
		{0xe, "tnei %rs%, %imm%"},
		{INVALIDFUNC, NO_INSTRUCTION},
		{0x10, "bltzal %rs%, %imm%"},
		{0x11, "bgezal %rs%, %imm%"},
};
struct funccode_t funk2op2[] = {
		{0, "j %address%"}
};
struct funccode_t funk2op3[] = {
		{0, "jal %address%"}
};
struct funccode_t funk2op4[] = {
		{0, "beq %rs%, %rt%, %imm%"}
};
struct funccode_t funk2op5[] = {
		{0, "bne %rs%, %rt%, %imm%"}
};
struct funccode_t funk2op6[] = {
		{0, "blez %rs%, %imm%"}
};
struct funccode_t funk2op7[] = {
		{0, "bgtz %rs%, %imm%"}
};
struct funccode_t funk2op8[] = {
		{0, "addi %rt%, %rs%, %imm%"}
};
struct funccode_t funk2op9[] = {
		{0, "addiu %rt%, %rs%, %imm%"}
};
struct funccode_t funk2opa[] = {
		{0, "slti %rt%, %rs%, %imm%"}
};
struct funccode_t funk2opb[] = {
		{0, "sltiu %rt%, %rs%, %imm%"}
};
struct funccode_t funk2opc[] = {
		{0, "andi %rt%, %rs%, %imm%"}
};
struct funccode_t funk2opd[] = {
		{0, "ori %rt%, %rs%, %imm%"}
};
struct funccode_t funk2ope[] = {
		{0, "xori %rt%, %rs%, %imm%"}
};
struct funccode_t funk2opf[] = {
		{0, "lui %rt%, %imm%"}
};
struct funccode_t funk2op11[] = {
		{0x0, "bclf %rt%, %imm%"}, //rs = 8, xx=0
		{0x1, "bclt %rt%, %imm%"}, //rs = 8, xx=1
		{0x2, "mfc1 %rt%, %rd%"}, //rs=0
		{0x3, "mtc1 %rt%, %rd%"},//rs=4
};
struct funccode_t funk2op1c[] = {
		{0x0, "madd %rs%, %rt%"},
		{0x1, "maddu %rs%, %rt%"},
		{0x2, "mul %rd%, %rs%, %rt%"},
		{INVALIDFUNC, NO_INSTRUCTION},
		{0x4, "msub %rs%, %rt%"},
		{0x5, "msubu %rs%, %rt%"},
		{INVALIDFUNC, NO_INSTRUCTION},
		{INVALIDFUNC, NO_INSTRUCTION},
		{INVALIDFUNC, NO_INSTRUCTION},
		{INVALIDFUNC, NO_INSTRUCTION},
		{INVALIDFUNC, NO_INSTRUCTION},
		{INVALIDFUNC, NO_INSTRUCTION},
		{INVALIDFUNC, NO_INSTRUCTION},
		{INVALIDFUNC, NO_INSTRUCTION},
		{INVALIDFUNC, NO_INSTRUCTION},
		{INVALIDFUNC, NO_INSTRUCTION},
		{INVALIDFUNC, NO_INSTRUCTION},
		{INVALIDFUNC, NO_INSTRUCTION},
		{INVALIDFUNC, NO_INSTRUCTION},
		{INVALIDFUNC, NO_INSTRUCTION},
		{INVALIDFUNC, NO_INSTRUCTION},
		{INVALIDFUNC, NO_INSTRUCTION},
		{INVALIDFUNC, NO_INSTRUCTION},
		{INVALIDFUNC, NO_INSTRUCTION},
		{INVALIDFUNC, NO_INSTRUCTION},
		{INVALIDFUNC, NO_INSTRUCTION},
		{INVALIDFUNC, NO_INSTRUCTION},
		{INVALIDFUNC, NO_INSTRUCTION},
		{INVALIDFUNC, NO_INSTRUCTION},
		{INVALIDFUNC, NO_INSTRUCTION},
		{INVALIDFUNC, NO_INSTRUCTION},
		{INVALIDFUNC, NO_INSTRUCTION},
		{0x20, "clz %rd%, %rs%"},
		{0x21, "clo %rd%, %rs%"},
};
struct funccode_t funk2op20[] = {
		{0, "lb %rt%, %imm%(%rs%)"}
};
struct funccode_t funk2op21[] = {
		{0, "lh %rt%, %imm%(%rs%)"}
};
struct funccode_t funk2op22[] = {
		{0, "lwl %rt%, %imm%(%rs%)"}
};
struct funccode_t funk2op23[] = {
		{0, "lw %rt%, %imm%(%rs%)"}
};
struct funccode_t funk2op24[] = {
		{0, "lbu %rt%, %imm%(%rs%)"}
};
struct funccode_t funk2op25[] = {
		{0, "lhu %rt%, %imm%(%rs%)"}
};
struct funccode_t funk2op26[] = {
		{0, "lwr %rt%, %imm%(%rs%)"}
};
struct funccode_t funk2op28[] = {
		{0, "sb %rt%, %imm%(%rs%)"}
};
struct funccode_t funk2op29[] = {
		{0, "sh %rt%, %imm%(%rs%)"}
};
struct funccode_t funk2op2a[] = {
		{0, "swl %rt%, %imm%(%rs%)"}
};
struct funccode_t funk2op2b[] = {
		{0, "sw %rt%, %imm%(%rs%)"}
};
struct funccode_t funk2op2e[] = {
		{0, "swr %rt%, %imm%(%rs%)"}
};
struct funccode_t funk2op30[] = {
		{0, "ll %rt%, %imm%(%rs%)"}
};
struct funccode_t funk2op31[] = {
		{0, "lwc1 %rt%, %imm%(%rs%)"}
};
struct funccode_t funk2op38[] = {
		{0, "sc %rt%, %imm%(%rs%)"}
};
struct funccode_t funk2op39[] = {
		{0, "swc1 %rt%, %imm%(%rs%)"}
};
struct funccode_t funk2op3d[] = {
		{0, "sdc1 %rt%, %imm%(%rs%)"}
};

struct opcode_t operations[] =
{
		{R, 0x0, funk2op0, 6, sizeof(funk2op0)/sizeof(struct funccode_t)},
		{I, 0x1, funk2op1, 21, sizeof(funk2op1)/sizeof(struct funccode_t)},
		{J, 0x2, funk2op2, 0, sizeof(funk2op2)/sizeof(struct funccode_t)},
		{J, 0x3, funk2op3, 0, sizeof(funk2op3)/sizeof(struct funccode_t)},
		{I, 0x4, funk2op4, 0, sizeof(funk2op4)/sizeof(struct funccode_t)},
		{I, 0x5, funk2op5, 0, sizeof(funk2op5)/sizeof(struct funccode_t)},
		{I, 0x6, funk2op6, 0, sizeof(funk2op6)/sizeof(struct funccode_t)},
		{I, 0x7, funk2op7, 0, sizeof(funk2op7)/sizeof(struct funccode_t)},
		{I, 0x8, funk2op8, 0, sizeof(funk2op8)/sizeof(struct funccode_t)},
		{I, 0x9, funk2op9, 0, sizeof(funk2op9)/sizeof(struct funccode_t)},
		{I, 0xa, funk2opa, 0, sizeof(funk2opa)/sizeof(struct funccode_t)},
		{I, 0xb, funk2opb, 0, sizeof(funk2opb)/sizeof(struct funccode_t)},
		{I, 0xc, funk2opc, 0, sizeof(funk2opc)/sizeof(struct funccode_t)},
		{I, 0xd, funk2opd, 0, sizeof(funk2opd)/sizeof(struct funccode_t)},
		{I, 0xe, funk2ope, 0, sizeof(funk2ope)/sizeof(struct funccode_t)},
		{I, 0xf, funk2opf, 0, sizeof(funk2opf)/sizeof(struct funccode_t)},
		{INVALIDOP, 0x10},
		{I, 0x11, funk2op11, 0, sizeof(funk2op11)/sizeof(struct funccode_t)},
		{INVALIDOP, 0x12},
		{INVALIDOP, 0x13},
		{INVALIDOP, 0x14},
		{INVALIDOP, 0x15},
		{INVALIDOP, 0x16},
		{INVALIDOP, 0x17},
		{INVALIDOP, 0x18},
		{INVALIDOP, 0x19},
		{INVALIDOP, 0x1a},
		{INVALIDOP, 0x1b},
		{R, 0x1c, funk2op1c, 5, sizeof(funk2op1c)/sizeof(struct funccode_t)},
		{INVALIDOP, 0x1d},
		{INVALIDOP, 0x1e},
		{INVALIDOP, 0x1f},
		{I, 0x20, funk2op20, 0, sizeof(funk2op20)/sizeof(struct funccode_t)},
		{I, 0x21, funk2op21, 0, sizeof(funk2op21)/sizeof(struct funccode_t)},
		{I, 0x22, funk2op22, 0, sizeof(funk2op22)/sizeof(struct funccode_t)},
		{I, 0x23, funk2op23, 0, sizeof(funk2op23)/sizeof(struct funccode_t)},
		{I, 0x24, funk2op24, 0, sizeof(funk2op24)/sizeof(struct funccode_t)},
		{I, 0x25, funk2op25, 0, sizeof(funk2op25)/sizeof(struct funccode_t)},
		{I, 0x26, funk2op26, 0, sizeof(funk2op26)/sizeof(struct funccode_t)},
		{INVALIDOP, 0x27},
		{I, 0x28, funk2op28, 0, sizeof(funk2op28)/sizeof(struct funccode_t)},
		{I, 0x29, funk2op29, 0, sizeof(funk2op29)/sizeof(struct funccode_t)},
		{I, 0x2a, funk2op2a, 0, sizeof(funk2op2a)/sizeof(struct funccode_t)},
		{I, 0x2b, funk2op2b, 0, sizeof(funk2op2b)/sizeof(struct funccode_t)},
		{INVALIDOP, 0x2c},
		{INVALIDOP, 0x2d},
		{I, 0x2e, funk2op2e, 0, sizeof(funk2op2e)/sizeof(struct funccode_t)},
		{INVALIDOP, 0x2f},
		{I, 0x30, funk2op30, 0, sizeof(funk2op30)/sizeof(struct funccode_t)},
		{I, 0x31, funk2op31, 0, sizeof(funk2op31)/sizeof(struct funccode_t)},
		{INVALIDOP, 0x32},
		{INVALIDOP, 0x33},
		{INVALIDOP, 0x34},
		{INVALIDOP, 0x35},
		{INVALIDOP, 0x36},
		{INVALIDOP, 0x37},
		{I, 0x38, funk2op38, 0, sizeof(funk2op38)/sizeof(struct funccode_t)},
		{I, 0x39, funk2op39, 0, sizeof(funk2op39)/sizeof(struct funccode_t)},
		{INVALIDOP, 0x3a},
		{INVALIDOP, 0x3b},
		{INVALIDOP, 0x3c},
		{I, 0x3d, funk2op3d, 0, sizeof(funk2op3d)/sizeof(struct funccode_t)},
};

#endif /* MAIN_H_ */
