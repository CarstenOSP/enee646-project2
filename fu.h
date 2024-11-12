
/*
 * fu.h

   This module was originally written by Paul Kohout.  It was later
   adapted for the RISC-V ISA.

 * 
 * Donald Yeung
 */


#include <stdio.h>

#define MAXMEMORY 65536 /* maximum number of data words in memory */
#define NUMREGS 32 /* number of machine registers */

#define FIELD_OPCODE(instr) ((unsigned int)((instr)&0x007F))
#define FIELD_FUNC3(instr)  ((unsigned int)(((instr)>>12)&0x07))
#define FIELD_FUNC7(instr)  ((unsigned int)(((instr)>>25)&0x07F))
#define FIELD_RS1(instr)    ((unsigned int)(((instr)>>15)&0x001F))
#define FIELD_RS2(instr)    ((unsigned int)(((instr)>>20)&0x001F))
#define FIELD_RD(instr)     ((unsigned int)(((instr)>>7)&0x001F))
#define FIELD_IMM_I(instr)  ((((signed int)instr)>>20))
#define FIELD_IMM_S(instr)  ((((((signed int)instr)>>25)<<5)|(((instr)>>7)&0x1F)))
#define FIELD_OFFSET(instr) ((((signed int)instr)>>12))

#define FU_GROUP_INT     0
#define FU_GROUP_ADD     1
#define FU_GROUP_MULT    2
#define FU_GROUP_DIV     3
#define FU_GROUP_MEM     4
#define FU_GROUP_BRANCH  5
#define FU_GROUP_NONE    -1
#define FU_GROUP_INVALID -2
#define FU_GROUP_HALT    -3

#define OPERATION_ADD   0
#define OPERATION_SUB   2
#define OPERATION_MULT  4
#define OPERATION_DIV   5
#define OPERATION_SLL   6
#define OPERATION_SRL   7
#define OPERATION_AND   8
#define OPERATION_OR    9
#define OPERATION_XOR   10
#define OPERATION_LOAD  11
#define OPERATION_STORE 12
#define OPERATION_JAL   14
#define OPERATION_JR    15
#define OPERATION_JALR  16
#define OPERATION_BEQ   17
#define OPERATION_BNE   18
#define OPERATION_SLT   19
#define OPERATION_SLTU  20
#define OPERATION_NONE  -1

#define DATA_TYPE_W    0
#define DATA_TYPE_F    1
#define DATA_TYPE_NONE -1

#define NOP 0x00000000

typedef struct _op_info_t {
  const char *name;
  const int fu_group_num;
  const int operation;
  const int data_type;
} op_info_t;

typedef struct _op_level1_t {
  const struct _op_info_t info;
  const struct _op_level2_t * const level2_table;
} op_level1_t;

typedef struct _op_level2_t {
  const struct _op_info_t info;
  const struct _op_level3_t * const level3_table;
} op_level2_t;

typedef struct _op_level3_t {
  const struct _op_info_t info;
} op_level3_t;

extern const op_level1_t op_table[];
extern const op_level2_t op_int_table[];
extern const op_level2_t op_fp_table[];
extern const op_level2_t op_br_table[];
extern const op_level2_t op_imm_table[];
extern const op_level2_t op_load_table[];
extern const op_level2_t op_store_table[];
extern const op_level3_t op_intrr_table[];
extern const op_level3_t op_fprr_table[];

extern const char fu_group_int_name[];
extern const char fu_group_mem_name[];
extern const char fu_group_add_name[];
extern const char fu_group_mult_name[];
extern const char fu_group_div_name[];


/* union to handle multiple fixed-point types */
typedef union _int_t {
  signed int w;
  unsigned int wu;
} int_t;

typedef union _operand_t {
  int_t integer;
  float flt;
} operand_t;


/* functional unit and memory port structures */
typedef struct _fu_int_stage_t {
  int num_cycles;
  int current_cycle;
  int tag;
  int branch;
  int link;
  struct _fu_int_stage_t *prev;
} fu_int_stage_t;

typedef struct _fu_int_t {
  char *name;
  fu_int_stage_t *stage_list;
  struct _fu_int_t *next;
} fu_int_t;


typedef struct _fu_mem_stage_t {
  int num_cycles;
  int current_cycle;
  int tag;
  int store;
  int float_mem;
  struct _fu_mem_stage_t *prev;
} fu_mem_stage_t;

typedef struct _fu_mem_t {
  char *name;
  fu_mem_stage_t *stage_list;
  struct _fu_mem_t *next;
} fu_mem_t;


typedef struct _fu_fp_stage_t {
  int num_cycles;
  int current_cycle;
  int tag;
  struct _fu_fp_stage_t *prev;
} fu_fp_stage_t;

typedef struct _fu_fp_t {
  char *name;
  fu_fp_stage_t *stage_list;
  struct _fu_fp_t *next;
} fu_fp_t;


/* writeback pipeline register */
typedef struct _wb_port_int_t {
  int tag;
} wb_port_int_t;

typedef struct _wb_port_fp_t {
  int tag;
} wb_port_fp_t;


extern int fu_int_read(fu_int_t **, FILE *);
extern int fu_mem_read(fu_mem_t **, FILE *);
extern int fu_fp_read(fu_fp_t **, FILE *);

extern int issue_fu_int(fu_int_t *, int, int, int);
extern int issue_fu_mem(fu_mem_t *, int, int, int);
extern int issue_fu_fp(fu_fp_t *,int);

extern void advance_fu_int(fu_int_t *, wb_port_int_t [], int, int *);
extern void advance_fu_mem(fu_mem_t *, wb_port_int_t [], int, wb_port_fp_t [], int);
extern void advance_fu_fp(fu_fp_t *, wb_port_fp_t [],int);

extern int wb_int(wb_port_int_t [], int, int);
extern int wb_fp(wb_port_fp_t [], int, int);

const op_info_t *decode_instr(int, int *);
const op_info_t *perform_operation(int, int, unsigned char *, int, operand_t, 
                                   operand_t, operand_t *, int *);
