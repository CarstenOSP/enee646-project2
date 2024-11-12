
/*
 * pipeline.c
 * 
 * Donald Yeung
 */


#include <stdlib.h>
#include <string.h>
#include "fu.h"
#include "pipeline.h"


int
commit(state_t *state, int *num_insn) {
    const op_info_t *op_info;
    int use_imm;
    int dest_reg;
    if (state->ROB[state->ROB_head].completed) {
        op_info = decode_instr(state->ROB[state->ROB_head].instr, &use_imm);
        if (op_info->fu_group_num == FU_GROUP_HALT) {
            (*num_insn)++;
            return 1;
        }
        dest_reg = FIELD_RD(state->ROB[state->ROB_head].instr);
        if (op_info->operation != OPERATION_STORE &&
            op_info->operation != OPERATION_BEQ &&
            op_info->operation != OPERATION_BNE) {
            if (op_info->data_type == DATA_TYPE_W) {
                state->rf_int.reg_int.integer[dest_reg] = state->ROB[state->ROB_head].result.integer;
                if (state->rf_int.tag[dest_reg] == state->ROB_head) {
                    state->rf_int.tag[dest_reg] = -1;
                }
            } else {
                state->rf_fp.reg_fp.flt[dest_reg] = state->ROB[state->ROB_head].result.flt;
                if (state->rf_fp.tag[dest_reg] == state->ROB_head) {
                    state->rf_fp.tag[dest_reg] = -1;
                }
            }
            (*num_insn)++;
            state->ROB_head = (state->ROB_head + 1) % ROB_SIZE;
        } else if (op_info->operation == OPERATION_STORE) {
            if (issue_fu_mem(state->fu_mem_list, state->ROB_head, 
                             op_info->data_type, TRUE) != -1) {
                if (op_info->data_type == DATA_TYPE_W) {
                    memcpy(&state->mem[state->ROB[state->ROB_head].target.integer.wu], 
                           &state->ROB[state->ROB_head].result.integer,
                           sizeof(int_t));
                } else {
                    memcpy(&state->mem[state->ROB[state->ROB_head].target.integer.wu], 
                           &state->ROB[state->ROB_head].result.flt,
                           sizeof(float));
                }
                state->ROB_head = (state->ROB_head + 1) % ROB_SIZE;
                (*num_insn)++;
            }
        } else {
            state->ROB_head = (state->ROB_head + 1) % ROB_SIZE;
            (*num_insn)++;
        }
    }
    state->rf_int.reg_int.integer[0].wu = 0;
    return 0;
}


void
writeback(state_t *state) {
    int wb_port;
    int curr_IQ, curr_CQ;
    for (wb_port = 0; wb_port < state->wb_port_int_num; wb_port++) {
        if (state->wb_port_int[wb_port].tag != -1) {
            if (state->wb_port_int[wb_port].tag < 64) {
                state->ROB[state->wb_port_int[wb_port].tag].completed = TRUE;
                for (curr_IQ = state->IQ_head; curr_IQ != state->IQ_tail; curr_IQ = (curr_IQ + 1) % IQ_SIZE) {
                    if (state->IQ[curr_IQ].tag1 == state->wb_port_int[wb_port].tag) {
                        state->IQ[curr_IQ].operand1 = state->ROB[state->IQ[curr_IQ].tag1].result;
                        state->IQ[curr_IQ].tag1 = -1;
                    }
                    if (state->IQ[curr_IQ].tag2 == state->wb_port_int[wb_port].tag) {
                        state->IQ[curr_IQ].operand2 = state->ROB[state->IQ[curr_IQ].tag2].result;
                        state->IQ[curr_IQ].tag2 = -1;
                    }
                }
                for (curr_CQ = state->CQ_head; curr_CQ != state->CQ_tail; curr_CQ = (curr_CQ + 1) % CQ_SIZE) {
                    if (state->CQ[curr_CQ].tag2 == state->wb_port_int[wb_port].tag)  {
                        state->CQ[curr_CQ].result = state->ROB[state->CQ[curr_CQ].tag2].result;
                        state->CQ[curr_CQ].tag2 = -1;
                    }
                }
            } else {
                for (curr_CQ = state->CQ_head; curr_CQ != state->CQ_tail; curr_CQ = (curr_CQ + 1) % CQ_SIZE) {
                    if (state->CQ[curr_CQ].tag1 == state->wb_port_int[wb_port].tag)  {
                        state->CQ[curr_CQ].address = state->ROB[state->CQ[curr_CQ].tag1 - ROB_SIZE].target;
                        state->CQ[curr_CQ].tag1 = -1;
                    }
                }
            }
            state->wb_port_int[wb_port].tag = -1;
        }     
    }
    for (wb_port = 0; wb_port < state->wb_port_fp_num; wb_port++) {
        if (state->wb_port_fp[wb_port].tag != -1) {
            if (state->wb_port_fp[wb_port].tag < 64) {
                state->ROB[state->wb_port_fp[wb_port].tag].completed = TRUE;
                for (curr_IQ = state->IQ_head; curr_IQ != state->IQ_tail; curr_IQ = (curr_IQ + 1) % IQ_SIZE) {
                    if (state->IQ[curr_IQ].tag1 == state->wb_port_fp[wb_port].tag) {
                        state->IQ[curr_IQ].operand1 = state->ROB[state->IQ[curr_IQ].tag1].result;
                        state->IQ[curr_IQ].tag1 = -1;
                    }
                    if (state->IQ[curr_IQ].tag2 == state->wb_port_fp[wb_port].tag) {
                        state->IQ[curr_IQ].operand2 = state->ROB[state->IQ[curr_IQ].tag2].result;
                        state->IQ[curr_IQ].tag2 = -1;
                    }
                }
                for (curr_CQ = state->CQ_head; curr_CQ != state->CQ_tail; curr_CQ = (curr_CQ + 1) % CQ_SIZE) {
                    if (state->CQ[curr_CQ].tag2 == state->wb_port_fp[wb_port].tag)  {
                        state->CQ[curr_CQ].result = state->ROB[state->CQ[curr_CQ].tag2].result;
                        state->CQ[curr_CQ].tag2 = -1;
                    }
                }
            } else {
                for (curr_CQ = state->CQ_head; curr_CQ != state->CQ_tail; curr_CQ = (curr_CQ + 1) % CQ_SIZE) {
                    if (state->CQ[curr_CQ].tag1 == state->wb_port_fp[wb_port].tag)  {
                        state->CQ[curr_CQ].address = state->ROB[state->CQ[curr_CQ].tag1 - ROB_SIZE].target;
                        state->CQ[curr_CQ].tag1 = -1;
                    }
                }
            }
            state->wb_port_fp[wb_port].tag = -1;
        }    
    }
    if (state->branch_tag != -1) {
        state->ROB[state->branch_tag].completed = TRUE;
        if (state->ROB[state->branch_tag].result.integer.wu) {
            state->if_id.instr = NOP;
            state->pc = state->ROB[state->branch_tag].target.integer.wu;
        }
        state->branch_tag = -1;
        state->branch_lock = FALSE;
    }
}


void
execute(state_t *state) {
    advance_fu_fp(state->fu_add_list, state->wb_port_fp, state->wb_port_fp_num);
    advance_fu_fp(state->fu_mult_list, state->wb_port_fp, state->wb_port_fp_num);
    advance_fu_fp(state->fu_div_list, state->wb_port_fp, state->wb_port_fp_num);
    advance_fu_int(state->fu_int_list, state->wb_port_int, state->wb_port_int_num, &(state->branch_tag));
    advance_fu_mem(state->fu_mem_list, state->wb_port_int, state->wb_port_int_num,
                   state->wb_port_fp, state->wb_port_fp_num);
}


int
memory_disambiguation(state_t *state) {
    operand_t result;
    const op_info_t *op_info;
    int use_imm;
    int data_type;
    int curr = state->CQ_head;
    int load_curr;
    int conflict = FALSE;
    int issued = FALSE;
    int target;
    while (curr != state->CQ_tail && !issued) {
        if (state->CQ[curr].store && state->CQ[curr].tag1 != -1) {
            return 1;
        }
        if (state->CQ[curr].store && state->CQ[curr].tag1 == -1
            && state->CQ[curr].tag2 == -1) {
            op_info = perform_operation(state->CQ[curr].instr, -1, state->mem, FALSE,
                                        state->CQ[curr].address, state->CQ[curr].result, &result, 
                                        &target);
            state->ROB[state->CQ[curr].ROB_index].result = result;
            state->ROB[state->CQ[curr].ROB_index].completed = TRUE;
            issued = 1;
        } else if (!state->CQ[curr].store && state->CQ[curr].tag1 == -1) {
            load_curr = state->CQ_head;
            while (load_curr != curr && !conflict) {
                if (state->CQ[load_curr].store && state->CQ[load_curr].address.integer.w ==
                                                  state->CQ[curr].address.integer.w) {
                    conflict = TRUE;
                }
                load_curr = (load_curr + 1) % IQ_SIZE;
            }
            if (!conflict) {
                for (load_curr = state->ROB_head; load_curr < state->ROB_tail; load_curr++) {
                    op_info = decode_instr(state->ROB[load_curr].instr, &use_imm);
                    if (op_info->operation == OPERATION_STORE &&
                        state->ROB[load_curr].target.integer.w == state->CQ[curr].address.integer.w) {
                        result = state->ROB[load_curr].target;
                    }
                }
                op_info = perform_operation(state->CQ[curr].instr, -1, state->mem, FALSE,
                                            state->CQ[curr].address, state->CQ[curr].result, &result, 
                                            &target);
                state->ROB[state->CQ[curr].ROB_index].result = result;
                if (issue_fu_mem(state->fu_mem_list, state->CQ[curr].ROB_index, 
                                 op_info->data_type, FALSE) != -1) {
                    state->ROB[state->CQ[curr].ROB_index].result = result;
                    issued = TRUE;
                }
            }
        }
        state->CQ[curr].issued = issued;
        curr = (curr + 1) % IQ_SIZE;
    }

    while (state->CQ[state->CQ_head].issued && state->CQ_head != state->CQ_tail) {
        state->CQ_head = (state->CQ_head + 1) % CQ_SIZE;
    }
}


int
issue(state_t *state) {
    operand_t result;
    const op_info_t *op_info;
    int target;
    int curr = state->IQ_head;
    int issued = FALSE;
    while (curr != state->IQ_tail && !issued) {
        if (state->IQ[curr].tag1 == -1 && state->IQ[curr].tag2 == -1 && !state->IQ[curr].issued) {
            op_info = perform_operation(state->IQ[curr].instr, state->IQ[curr].pc, state->mem, TRUE,
                                        state->IQ[curr].operand1, state->IQ[curr].operand2, &result, 
                                        &target);
            switch (op_info->fu_group_num) {
                case FU_GROUP_ADD:
                    if (issue_fu_fp(state->fu_add_list, state->IQ[curr].ROB_index) != -1) {
                        state->ROB[state->IQ[curr].ROB_index].result = result;
                        issued = TRUE;
                    }
                    break;
                case FU_GROUP_DIV:
                    if (issue_fu_fp(state->fu_div_list, state->IQ[curr].ROB_index) != -1) {
                        state->ROB[state->IQ[curr].ROB_index].result = result;
                        issued = TRUE;
                    }
                    break;
                case FU_GROUP_MULT:
                    if (issue_fu_fp(state->fu_mult_list, state->IQ[curr].ROB_index) != -1) {
                        state->ROB[state->IQ[curr].ROB_index].result = result;
                        issued = TRUE;
                    }
                    break;
                case FU_GROUP_INT:
                    if (issue_fu_int(state->fu_int_list, state->IQ[curr].ROB_index,
                                     0, 0) != -1) {
                        state->ROB[state->IQ[curr].ROB_index].result = result;
                        issued = TRUE;
                    }
                    break;
                case FU_GROUP_MEM:
                    if (issue_fu_int(state->fu_int_list, state->IQ[curr].ROB_index + 64,
                                     0, 0) != -1) {
                        state->ROB[state->IQ[curr].ROB_index].target.integer.wu = target;
                        issued = TRUE;
                    }
                    break;
                case FU_GROUP_BRANCH:
                    if (op_info->operation == OPERATION_JAL ||
                        op_info->operation == OPERATION_JALR) {
                        if (issue_fu_int(state->fu_int_list, state->IQ[curr].ROB_index,
                                     1, 1) != -1) {
                            state->ROB[state->IQ[curr].ROB_index].target.integer.wu = target;
                            state->ROB[state->IQ[curr].ROB_index].result = result;
                            issued = TRUE;
                        }
                    } else {
                        if (issue_fu_int(state->fu_int_list, state->IQ[curr].ROB_index,
                                         1, 0) != -1) {
                            state->ROB[state->IQ[curr].ROB_index].target.integer.wu = target;
                            state->ROB[state->IQ[curr].ROB_index].result = result;
                            issued = TRUE;
                        }
                    }
                    break;
            }
            state->IQ[curr].issued = issued;
        }
        curr = (curr + 1) % IQ_SIZE;
    }

    while (state->IQ[state->IQ_head].issued && state->IQ_head != state->IQ_tail) {
        state->IQ_head = (state->IQ_head + 1) % IQ_SIZE;
    }
}

int
dispatch_struct_hazard(state_t *state, const op_info_t *op_info) {
    if ((state->IQ_tail + 1) % IQ_SIZE == state->IQ_head ||
        (state->ROB_tail + 1) % ROB_SIZE == state->ROB_head ||
        (op_info->fu_group_num == FU_GROUP_MEM &&
        (state->CQ_tail + 1) % CQ_SIZE == state->CQ_head)) {
        return 1;
    }
    return 0;
}

int
grab_operand(state_t *state, int reg, int fp, operand_t *result) {
    if (fp) {
        if (state->rf_fp.tag[reg] == -1) {
            result->flt = state->rf_fp.reg_fp.flt[reg];
            return 0;
        } else {
            if (state->ROB[state->rf_fp.tag[reg]].completed) {
                result->flt = state->ROB[state->rf_fp.tag[reg]].result.flt;
                return 0;
            }
            return 1;
        }
    } else {
        if (state->rf_int.tag[reg] == -1) {
            result->integer = state->rf_int.reg_int.integer[reg];
            return 0;
        } else {
            if (state->ROB[state->rf_int.tag[reg]].completed) {
                result->integer = state->ROB[state->rf_int.tag[reg]].result.integer;
                return 0;
            }
            return 1;
        }
    }
}

int
dispatch(state_t *state) {
    int use_imm, data_type;
    int ROB_index;
    int instr = state->if_id.instr;
    const op_info_t *op_info = decode_instr(instr, &use_imm);
    operand_t operand1, operand2;
    if (instr != NOP) {
        if (op_info->fu_group_num == FU_GROUP_HALT) {
            state->ROB[state->ROB_tail].instr = instr;
            state->ROB[state->ROB_tail].completed = TRUE;
            state->ROB_tail = (state->ROB_tail + 1) % ROB_SIZE;
            state->halt = TRUE;
            return 0;
        }
        if (dispatch_struct_hazard(state, op_info)) {
            state->fetch_lock = TRUE;
            return 1;
        }
        state->fetch_lock = FALSE;
        state->ROB[state->ROB_tail].instr = instr;
        state->ROB[state->ROB_tail].completed = FALSE;
        state->IQ[state->IQ_tail].instr = instr;
        state->IQ[state->IQ_tail].pc = state->if_id.pc;
        state->IQ[state->IQ_tail].issued = FALSE;
        state->IQ[state->IQ_tail].ROB_index = state->ROB_tail;
        ROB_index = state->ROB_tail;
        
        switch (op_info->fu_group_num) {
            case FU_GROUP_BRANCH:
                state->branch_lock = TRUE;
                if (op_info->operation == OPERATION_JAL) {
                    state->IQ[state->IQ_tail].tag1 = -1;
                    state->IQ[state->IQ_tail].tag2 = -1;
                } else if (op_info->operation == OPERATION_JALR) {
                    if (grab_operand(state, FIELD_RS1(instr), op_info->data_type, &operand1)) {
                        state->IQ[state->IQ_tail].tag1 = state->rf_int.tag[FIELD_RS1(instr)];
                    } else {
                        state->IQ[state->IQ_tail].operand1 = operand1;
                        state->IQ[state->IQ_tail].tag1 = -1;
                        state->IQ[state->IQ_tail].tag2 = -1;
                    }
                } else {
                    if (grab_operand(state, FIELD_RS1(instr), op_info->data_type, &operand1)) {
                        if (op_info->data_type == DATA_TYPE_W) {
                            state->IQ[state->IQ_tail].tag1 = state->rf_int.tag[FIELD_RS1(instr)];
                        } else {
                            state->IQ[state->IQ_tail].tag1 = state->rf_fp.tag[FIELD_RS1(instr)];
                        }
                    } else {
                        state->IQ[state->IQ_tail].operand1 = operand1;
                        state->IQ[state->IQ_tail].tag1 = -1;
                    }
                    if(grab_operand(state, FIELD_RS2(instr), op_info->data_type, &operand2)) {
                        if (op_info->data_type == DATA_TYPE_W) {
                            state->IQ[state->IQ_tail].tag2 = state->rf_int.tag[FIELD_RS2(instr)];
                        } else {
                            state->IQ[state->IQ_tail].tag2 = state->rf_fp.tag[FIELD_RS2(instr)];
                        }
                    } else {
                        state->IQ[state->IQ_tail].operand2 = operand2;
                        state->IQ[state->IQ_tail].tag2 = -1;
                    }
                }
                break;
            case FU_GROUP_MEM:
                state->CQ[state->CQ_tail].issued = FALSE;
                state->CQ[state->CQ_tail].instr = instr;
                state->CQ[state->CQ_tail].ROB_index = ROB_index;
                ROB_index = ROB_index + ROB_SIZE;
                if (op_info->operation == OPERATION_LOAD) {
                    state->CQ[state->CQ_tail].store = FALSE;
                    state->CQ[state->CQ_tail].tag1 = ROB_index;
                    state->CQ[state->CQ_tail].tag2 = -1;
                } else {
                    state->CQ[state->CQ_tail].store = TRUE;
                    state->CQ[state->CQ_tail].tag1 = ROB_index;
                    if (grab_operand(state, FIELD_RS2(instr), op_info->data_type, &operand2)) {
                        if (op_info->data_type == DATA_TYPE_W) {
                            state->CQ[state->CQ_tail].tag2 = state->rf_int.tag[FIELD_RS2(instr)];
                        } else {
                            state->CQ[state->CQ_tail].tag2 = state->rf_fp.tag[FIELD_RS2(instr)];
                        }
                    } else {
                        state->CQ[state->CQ_tail].result = operand2;
                        state->CQ[state->CQ_tail].tag2 = -1;
                    }
                }
                state->CQ_tail = (state->CQ_tail + 1) % CQ_SIZE;
            case FU_GROUP_ADD:
            case FU_GROUP_DIV:
            case FU_GROUP_MULT:
            case FU_GROUP_INT:
                if (op_info->operation== OPERATION_LOAD || op_info->operation == OPERATION_STORE) {
                    data_type = DATA_TYPE_W;
                } else {
                    data_type = op_info->data_type;
                }
                if (grab_operand(state, FIELD_RS1(instr), data_type, &operand1)) {
                    if (op_info->data_type == DATA_TYPE_W) {
                        state->IQ[state->IQ_tail].tag1 = state->rf_int.tag[FIELD_RS1(instr)];
                    } else {
                        state->IQ[state->IQ_tail].tag1 = state->rf_fp.tag[FIELD_RS1(instr)];
                    }
                } else {
                    state->IQ[state->IQ_tail].operand1 = operand1;
                    state->IQ[state->IQ_tail].tag1 = -1;
                }
                if (use_imm) {
                    if (op_info->operation == OPERATION_STORE) {
                        state->IQ[state->IQ_tail].operand2.integer.w = FIELD_IMM_S(instr);
                        state->IQ[state->IQ_tail].tag2 = -1;
                    } else {
                        state->IQ[state->IQ_tail].operand2.integer.w = FIELD_IMM_I(instr);
                        state->IQ[state->IQ_tail].tag2 = -1;
                    }
                } else {
                    if(grab_operand(state, FIELD_RS2(instr), op_info->data_type, &operand2)) {
                        if (op_info->data_type == DATA_TYPE_W) {
                            state->IQ[state->IQ_tail].tag2 = state->rf_int.tag[FIELD_RS2(instr)];
                        } else {
                            state->IQ[state->IQ_tail].tag2 = state->rf_fp.tag[FIELD_RS2(instr)];
                        }
                    } else {
                        state->IQ[state->IQ_tail].operand2 = operand2;
                        state->IQ[state->IQ_tail].tag2 = -1;
                    }
                }
                break;
        }
        if (op_info->operation != OPERATION_STORE &&
            op_info->operation != OPERATION_BEQ &&
            op_info->operation != OPERATION_BNE) {
            if (op_info->data_type == DATA_TYPE_W) {
                state->rf_int.tag[FIELD_RD(instr)] = state->ROB_tail;
            } else {
                state->rf_fp.tag[FIELD_RD(instr)] = state->ROB_tail;
            }
        }
        state->IQ_tail = (state->IQ_tail + 1) % IQ_SIZE;
        state->ROB_tail = (state->ROB_tail + 1) % ROB_SIZE;
        state->rf_int.tag[0] = -1;
    }
    
}

void
fetch(state_t *state) {
    state->if_id.pc = state->pc;
    memcpy(&state->if_id.instr, &state->mem[state->pc], 4);
    state->pc = state->pc + 4;
}
