/*
Stripped-down 6502 assembler.

Adapted from https://github.com/boeckmann/asm6502
Original by Bernd Boeckmann, BSD-3 license
*/

#pragma once

#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

#include <ctype.h>
#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* value specific preprocessor directives */
#define DEFINED(x) (((x).defined) != 0)
#define UNDEFINED(x) (((x).defined) == 0)
#define SET_DEFINED(v) ((v).defined = 1)
#define SET_UNDEFINED(v) ((v).defined = 0)
#define INFER_DEFINED(a, b) (a).defined = DEFINED(a) || DEFINED(b)
#define IS_END(p) (((!(p)) || (p) == 0x0a) || ((p) == 0x0d))

/* type specific preprocessor directives */
#define TYPE(v) ((v).t)
#define SET_TYPE(v, u) ((v).t = (u))
#define NUM_TYPE(x) (((x) < 0x100) ? TYPE_BYTE : TYPE_WORD)

namespace {

typedef unsigned char u8;
typedef unsigned short u16;

typedef struct instruction_desc {
  char mnemonic[5];
  u8 op[16];
} instruction_desc;

/* data type used when evaluating expressions */
/* the value may be undefined */
typedef struct value {
  u16 v;      /* the numeric value */
  u8 t;       /* type (none, byte or word) */
  u8 defined; /* defined or undefined */
} value;

enum { OP_RTS = 0x60, OP_JSR = 0x20, INV = 0xfc };
instruction_desc itbl_6502[56] = {
    {"ADC",
     {INV, INV, 0x69, INV, 0x65, 0x75, INV, 0x6d, 0x7d, 0x79, INV, 0x61, 0x71,
      INV, INV, INV}},
    {"AND",
     {INV, INV, 0x29, INV, 0x25, 0x35, INV, 0x2d, 0x3d, 0x39, INV, 0x21, 0x31,
      INV, INV, INV}},
    {"ASL",
     {0x0a, INV, INV, INV, 0x06, 0x16, INV, 0x0e, 0x1e, INV, INV, INV, INV, INV,
      INV, INV}},
    {"BCC",
     {INV, INV, INV, 0x90, INV, INV, INV, INV, INV, INV, INV, INV, INV, INV,
      INV, INV}},
    {"BCS",
     {INV, INV, INV, 0xb0, INV, INV, INV, INV, INV, INV, INV, INV, INV, INV,
      INV, INV}},
    {"BEQ",
     {INV, INV, INV, 0xf0, INV, INV, INV, INV, INV, INV, INV, INV, INV, INV,
      INV, INV}},
    {"BIT",
     {INV, INV, INV, INV, 0x24, INV, INV, 0x2c, INV, INV, INV, INV, INV, INV,
      INV, INV}},
    {"BMI",
     {INV, INV, INV, 0x30, INV, INV, INV, INV, INV, INV, INV, INV, INV, INV,
      INV, INV}},
    {"BNE",
     {INV, INV, INV, 0xd0, INV, INV, INV, INV, INV, INV, INV, INV, INV, INV,
      INV, INV}},
    {"BPL",
     {INV, INV, INV, 0x10, INV, INV, INV, INV, INV, INV, INV, INV, INV, INV,
      INV, INV}},
    {"BRK",
     {INV, 0x00, INV, INV, INV, INV, INV, INV, INV, INV, INV, INV, INV, INV,
      INV, INV}},
    {"BVC",
     {INV, INV, INV, 0x50, INV, INV, INV, INV, INV, INV, INV, INV, INV, INV,
      INV, INV}},
    {"BVS",
     {INV, INV, INV, 0x70, INV, INV, INV, INV, INV, INV, INV, INV, INV, INV,
      INV, INV}},
    {"CLC",
     {INV, 0x18, INV, INV, INV, INV, INV, INV, INV, INV, INV, INV, INV, INV,
      INV, INV}},
    {"CLD",
     {INV, 0xd8, INV, INV, INV, INV, INV, INV, INV, INV, INV, INV, INV, INV,
      INV, INV}},
    {"CLI",
     {INV, 0x58, INV, INV, INV, INV, INV, INV, INV, INV, INV, INV, INV, INV,
      INV, INV}},
    {"CLV",
     {INV, 0xb8, INV, INV, INV, INV, INV, INV, INV, INV, INV, INV, INV, INV,
      INV, INV}},
    {"CMP",
     {INV, INV, 0xc9, INV, 0xc5, 0xd5, INV, 0xcd, 0xdd, 0xd9, INV, 0xc1, 0xd1,
      INV, INV, INV}},
    {"CPX",
     {INV, INV, 0xe0, INV, 0xe4, INV, INV, 0xec, INV, INV, INV, INV, INV, INV,
      INV, INV}},
    {"CPY",
     {INV, INV, 0xc0, INV, 0xc4, INV, INV, 0xcc, INV, INV, INV, INV, INV, INV,
      INV, INV}},
    {"DEC",
     {INV, INV, INV, INV, 0xc6, 0xd6, INV, 0xce, 0xde, INV, INV, INV, INV, INV,
      INV, INV}},
    {"DEX",
     {INV, 0xca, INV, INV, INV, INV, INV, INV, INV, INV, INV, INV, INV, INV,
      INV, INV}},
    {"DEY",
     {INV, 0x88, INV, INV, INV, INV, INV, INV, INV, INV, INV, INV, INV, INV,
      INV, INV}},
    {"EOR",
     {INV, INV, 0x49, INV, 0x45, 0x55, INV, 0x4d, 0x5d, 0x59, INV, 0x41, 0x51,
      INV, INV, INV}},
    {"INC",
     {INV, INV, INV, INV, 0xe6, 0xf6, INV, 0xee, 0xfe, INV, INV, INV, INV, INV,
      INV, INV}},
    {"INX",
     {INV, 0xe8, INV, INV, INV, INV, INV, INV, INV, INV, INV, INV, INV, INV,
      INV, INV}},
    {"INY",
     {INV, 0xc8, INV, INV, INV, INV, INV, INV, INV, INV, INV, INV, INV, INV,
      INV, INV}},
    {"JMP",
     {INV, INV, INV, INV, INV, INV, INV, 0x4c, INV, INV, 0x6c, INV, INV, INV,
      INV, INV}},
    {"JSR",
     {INV, INV, INV, INV, INV, INV, INV, 0x20, INV, INV, INV, INV, INV, INV,
      INV, INV}},
    {"LDA",
     {INV, INV, 0xa9, INV, 0xa5, 0xb5, INV, 0xad, 0xbd, 0xb9, INV, 0xa1, 0xb1,
      INV, INV, INV}},
    {"LDX",
     {INV, INV, 0xa2, INV, 0xa6, INV, 0xb6, 0xae, INV, 0xbe, INV, INV, INV, INV,
      INV, INV}},
    {"LDY",
     {INV, INV, 0xa0, INV, 0xa4, 0xb4, INV, 0xac, 0xbc, INV, INV, INV, INV, INV,
      INV, INV}},
    {"LSR",
     {0x4a, INV, INV, INV, 0x46, 0x56, INV, 0x4e, 0x5e, INV, INV, INV, INV, INV,
      INV, INV}},
    {"NOP",
     {INV, 0xea, INV, INV, INV, INV, INV, INV, INV, INV, INV, INV, INV, INV,
      INV, INV}},
    {"ORA",
     {INV, INV, 0x09, INV, 0x05, 0x15, INV, 0x0d, 0x1d, 0x19, INV, 0x01, 0x11,
      INV, INV, INV}},
    {"PHA",
     {INV, 0x48, INV, INV, INV, INV, INV, INV, INV, INV, INV, INV, INV, INV,
      INV, INV}},
    {"PHP",
     {INV, 0x08, INV, INV, INV, INV, INV, INV, INV, INV, INV, INV, INV, INV,
      INV, INV}},
    {"PLA",
     {INV, 0x68, INV, INV, INV, INV, INV, INV, INV, INV, INV, INV, INV, INV,
      INV, INV}},
    {"PLP",
     {INV, 0x28, INV, INV, INV, INV, INV, INV, INV, INV, INV, INV, INV, INV,
      INV, INV}},
    {"ROL",
     {0x2a, INV, INV, INV, 0x26, 0x36, INV, 0x2e, 0x3e, INV, INV, INV, INV, INV,
      INV, INV}},
    {"ROR",
     {0x6a, INV, INV, INV, 0x66, 0x76, INV, 0x6e, 0x7e, INV, INV, INV, INV, INV,
      INV, INV}},
    {"RTI",
     {INV, 0x40, INV, INV, INV, INV, INV, INV, INV, INV, INV, INV, INV, INV,
      INV, INV}},
    {"RTS",
     {INV, 0x60, INV, INV, INV, INV, INV, INV, INV, INV, INV, INV, INV, INV,
      INV, INV}},
    {"SBC",
     {INV, INV, 0xe9, INV, 0xe5, 0xf5, INV, 0xed, 0xfd, 0xf9, INV, 0xe1, 0xf1,
      INV, INV, INV}},
    {"SEC",
     {INV, 0x38, INV, INV, INV, INV, INV, INV, INV, INV, INV, INV, INV, INV,
      INV, INV}},
    {"SED",
     {INV, 0xf8, INV, INV, INV, INV, INV, INV, INV, INV, INV, INV, INV, INV,
      INV, INV}},
    {"SEI",
     {INV, 0x78, INV, INV, INV, INV, INV, INV, INV, INV, INV, INV, INV, INV,
      INV, INV}},
    {"STA",
     {INV, INV, INV, INV, 0x85, 0x95, INV, 0x8d, 0x9d, 0x99, INV, 0x81, 0x91,
      INV, INV, INV}},
    {"STX",
     {INV, INV, INV, INV, 0x86, INV, 0x96, 0x8e, INV, INV, INV, INV, INV, INV,
      INV, INV}},
    {"STY",
     {INV, INV, INV, INV, 0x84, 0x94, INV, 0x8c, INV, INV, INV, INV, INV, INV,
      INV, INV}},
    {"TAX",
     {INV, 0xaa, INV, INV, INV, INV, INV, INV, INV, INV, INV, INV, INV, INV,
      INV, INV}},
    {"TAY",
     {INV, 0xa8, INV, INV, INV, INV, INV, INV, INV, INV, INV, INV, INV, INV,
      INV, INV}},
    {"TSX",
     {INV, 0xba, INV, INV, INV, INV, INV, INV, INV, INV, INV, INV, INV, INV,
      INV, INV}},
    {"TXA",
     {INV, 0x8a, INV, INV, INV, INV, INV, INV, INV, INV, INV, INV, INV, INV,
      INV, INV}},
    {"TXS",
     {INV, 0x9a, INV, INV, INV, INV, INV, INV, INV, INV, INV, INV, INV, INV,
      INV, INV}},
    {"TYA",
     {INV, 0x98, INV, INV, INV, INV, INV, INV, INV, INV, INV, INV, INV, INV,
      INV, INV}}};

/* addressing modes */
enum {
  AM_ACC = 0,  /*         accumulator                       */
  AM_IMP = 1,  /*         implied                           */
  AM_IMM = 2,  /* #       immediate addressing              */
  AM_REL = 3,  /* R       program counter relative          */
  AM_ZP = 4,   /* ZP      zero-page                         */
  AM_ZPX = 5,  /* ZP,X    zero-page indexed with X          */
  AM_ZPY = 6,  /* ZP,Y    zero-page indexed with Y          */
  AM_ABS = 7,  /* A       absolute                          */
  AM_ABX = 8,  /* A,X     absolute indexed with X           */
  AM_ABY = 9,  /* A,Y     absolute indexed with Y           */
  AM_AIN = 10, /* (A)     absolute indirect                 */
  AM_ZIX = 11, /* (ZP,X)  zero-page indexed indirect        */
  AM_ZIY = 12, /* (ZP),Y  zero-page indirect indexed with Y */
  AM_ZIN = 13, /* (ZP)    zero-page indirect                */
  AM_AIX = 14, /* (ABS,X) absolute indexed indirect         */
  AM_ZPR = 15, /* ZP,R    zero-page, relative               */
  AM_INV = 16
};

} // anonymous namespace

class Assembler {
private:
  instruction_desc *instruction_tbl = itbl_6502;
  int instruction_tbl_size = sizeof(itbl_6502) / sizeof(instruction_desc);

  u16 address_counter =
      0; /* program counter of currently assembled instruction */

  instruction_desc *get_instruction_descr(const char *p) {
    int l = 0, r = instruction_tbl_size, x;
    int cmp;

    while (r >= l) {
      x = l + ((r - l) >> 2);
      cmp = strcmp(p, instruction_tbl[x].mnemonic);
      if (cmp == 0)
        return &instruction_tbl[x];
      else if (cmp > 0)
        l = x + 1;
      else
        r = x - 1;
    }
    return NULL;
  }

  int pass_num = 2;
  int code_size = 100;
  std::vector<u8> code;

  enum {
    ERR_NUM = 1,
    ERR_UNBALANCED,
    ERR_ID,
    ERR_ID_LEN,
    ERR_STMT,
    ERR_EOL,
    ERR_REDEFINITION,
    ERR_INSTR,
    ERR_AM,
    ERR_CLOSING_PAREN,
    ERR_ZIX,
    ERR_ZIY,
    ERR_NO_DIRECTIVE,
    ERR_UNDEF,
    ERR_ILLEGAL_TYPE,
    ERR_RELATIVE_RANGE,
    ERR_STR_END,
    ERR_BYTE_RANGE,
    ERR_NO_GLOBAL,
    ERR_CHR,
    ERR_STRLEN,
    ERR_STR,
    ERR_OPEN,
    ERR_MAX_INC,
    ERR_NO_BYTE,
    ERR_NO_MEM,
    ERR_MISSING_IF,
    ERR_UNCLOSED_COND,
    ERR_MAX_COND,
    ERR_PHASE,
    ERR_PHASE_SIZE,
    ERR_DIV_BY_ZERO,
    ERR_CPU_UNSUPPORTED,
    ERR_MISSING_REP
  };

  int error_type = 0;
  jmp_buf error_jmp;

  enum {
    ERROR_NORM,
    ERROR_EXT,  /* extended error with additional message */
    ERROR_ABORT /* .error directive */
  };

  void error(int err) {
    error_type = ERROR_NORM;
    longjmp(error_jmp, err);
  }

  void emit_byte(u8 b) { code.push_back(b); }

  enum { TYPE_BYTE = 1, TYPE_WORD = 2 };

  void emit(const char *p, u16 len) {
    u16 i = 0;

    for (i = 0; i < len; i++) {
      code.push_back(p[i]);
    }
  }

  void emit_word(u16 w) {
    code.push_back(w & 0xff);
    code.push_back(w >> 8);
  }

  void skip_white(char **pp) {
    char *p = *pp;
    while ((*p == ' ') || (*p == '\t'))
      p++;
    *pp = p;
  }

  void skip_curr_and_white(char **p) {
    (*p)++;
    while ((**p == ' ') || (**p == '\t')) {
      (*p)++;
    }
  }

  value to_byte(value v) {
    if (DEFINED(v) && (v.v > 0xff))
      error(ERR_BYTE_RANGE);
    SET_TYPE(v, TYPE_BYTE);
    return v;
  }

  /* emit instruction without argument */
  void emit_instr_0(instruction_desc *instr, int am) {
    code.push_back(instr->op[am]);
  }

  /* emit instruction with byte argument */
  void emit_instr_1(instruction_desc *instr, int am, u8 o) {
    code.push_back(instr->op[am]);
    code.push_back(o);
  }

  /* emit instruction with word argument */
  void emit_instr_2(instruction_desc *instr, int am, u16 o) {
    code.push_back(instr->op[am]);
    code.push_back(o & 0xff);
    code.push_back(o >> 8);
  }

  /* emit instruction with two byte arguments */
  void emit_instr_2b(instruction_desc *instr, int am, u8 byte1, u8 byte2) {
    code.push_back(instr->op[am]);
    code.push_back(byte1);
    code.push_back(byte2);
  }

/* Parse a number, either in hex, binary or decimal */
#define IS_HEX_DIGIT(x)                                                        \
  (isdigit((x)) || (((x) >= 'a') && ((x) <= 'f')) ||                           \
   (((x) >= 'A') && ((x) <= 'F')))

  u16 digit(const char *p) {
    if (*p <= '9')
      return (u16)(*p - '0');
    if (*p <= 'F')
      return (u16)(*p + 10 - 'A');
    return (u16)(*p + 10 - 'a');
  }

  value number(char **p) {
    value num = {0};
    char *pt = *p;
    u8 typ;

    if (**p == '$') {
      (*p)++;
      if (!IS_HEX_DIGIT(**p))
        error(ERR_NUM);
      do {
        num.v = (num.v << 4) + digit((*p)++);
      } while (IS_HEX_DIGIT(**p));
      typ = ((*p - pt) > 3) ? TYPE_WORD : NUM_TYPE(num.v);
      SET_TYPE(num, typ);
      SET_DEFINED(num);
    } else if (**p == '%') {
      (*p)++;
      if ((**p != '0') && (**p != '1'))
        error(ERR_NUM);
      do {
        num.v = (num.v << 1) + (**p - '0');
        (*p)++;
      } while ((**p == '0') || (**p == '1'));
      typ = ((*p - pt) > 9) ? TYPE_WORD : NUM_TYPE(num.v);
      SET_TYPE(num, typ);
      SET_DEFINED(num);
    } else {
      if (!isdigit(**p))
        error(ERR_NUM);
      do {
        num.v = num.v * 10 + digit((*p)++);
      } while (isdigit(**p));
      SET_TYPE(num, ((*p - pt) > 3) ? TYPE_WORD : NUM_TYPE(num.v));
      SET_DEFINED(num);
    }

    return num;
  }

  int instruction_imp_acc(instruction_desc *instr) {
    int am = AM_INV;

    if (instr->op[AM_ACC] != INV)
      am = AM_ACC;
    else if (instr->op[AM_IMP] != INV)
      am = AM_IMP;
    else
      error(ERR_AM);

    emit_instr_0(instr, am);

    return am;
  }

  int instruction_imm(char **p, instruction_desc *instr) {
    int am = AM_IMM;
    value v;

    (*p)++;
    if (instr->op[am] == INV)
      error(ERR_AM);
    v = number(p);
    if (pass_num == 2) {
      if (UNDEFINED(v))
        error(ERR_UNDEF);
    }
    emit_instr_1(instr, am, (u8)to_byte(v).v);
    return am;
  }

  u16 calculate_offset(value v) {
    u16 off;

    /* relative branch offsets are in 2-complement */
    /* have to calculate it by hand avoiding implementation defined behaviour */
    /* using unsigned int because int may not be in 2-complement */

    if (v.v >= 0)
      off = v.v;
    else
      off = (u16)((~0u) + v.v + 1u);
    return off;
  }

  int instruction_rel(instruction_desc *instr, value v) {
    u16 off = calculate_offset(v);
    emit_instr_1(instr, AM_REL, off & 0xffu);
    return AM_REL;
  }

  void ident(char **pp, char *id, int numeric, int uppercase) {
    int i = 0;
    char *p = *pp;

    if ((!numeric && !isalpha(*p) && (*p != '_')) ||
        (!isalnum(*p) && (*p != '_')))
      error(ERR_ID);

    do {
      *id++ = (char)(uppercase ? toupper(*p++) : *p++);
      i++;
      if (i >= 32)
        error(ERR_ID_LEN);
    } while (isalnum(*p) || (*p == '_'));

    *id = '\0';
    *pp = p;
  }

#define AM_VALID(instr, am) ((instr).op[am] != INV)

  /* handle indirect addressing modes */
  int instruction_ind(char **p, instruction_desc *instr) {
    char id[32];
    int am = AM_INV;
    value v;

    (*p)++;
    v = number(p);
    skip_white(p);

    /* indirect X addressing mode? */
    if (**p == ',') {
      skip_curr_and_white(p);
      ident(p, id, 0, 1);
      if (strcmp(id, "X") != 0)
        error(ERR_ZIX);
      if (AM_VALID(*instr, AM_AIX))
        am = AM_AIX;
      else
        am = AM_ZIX;
      skip_white(p);
      if (**p != ')')
        error(ERR_CLOSING_PAREN);
      skip_curr_and_white(p);

    } else {
      if (**p != ')')
        error(ERR_CLOSING_PAREN);
      skip_curr_and_white(p);
      /* indirect Y addressing mode? */
      if (**p == ',') {
        skip_curr_and_white(p);
        ident(p, id, 0, 1);
        if (strcmp(id, "Y") != 0)
          error(ERR_ZIY);
        am = AM_ZIY;
      } else {
        if (AM_VALID(*instr, AM_ZIN))
          am = AM_ZIN;
        else
          am = AM_AIN;
      }
    }

    if ((instr->op[am]) == INV)
      error(ERR_AM);

    if (pass_num == 2) {
      if (UNDEFINED(v))
        error(ERR_UNDEF);
      if ((am == AM_ZIX || am == AM_ZIY || am == AM_ZIN) &&
          (TYPE(v) != TYPE_BYTE))
        error(ERR_ILLEGAL_TYPE);
    }

    if (am == AM_AIN || am == AM_AIX) {
      emit_instr_2(instr, am, v.v);
    } else {
      emit_instr_1(instr, am, (u8)v.v);
    }

    return am;
  }

  void print_notice(const char *s) {
    std::cout << "notice : " << s << std::endl;
  }
  /* handle absolute x and y, zero-page x and y addressing modes */
  int instruction_abxy_zpxy(char **p, instruction_desc *instr, value v) {
    char id[32];
    int am = AM_INV;

    if (pass_num == 2) {
      if (UNDEFINED(v))
        error(ERR_UNDEF);
    }

    ident(p, id, 0, 1);
    /* test for absolute and zero-page X addressing */
    if (!strcmp(id, "X")) {
      if ((TYPE(v) == TYPE_BYTE) && AM_VALID(*instr, AM_ZPX))
        am = AM_ZPX;
      else if (AM_VALID(*instr, AM_ABX)) {
        am = AM_ABX;
        if (pass_num == 2 && NUM_TYPE(v.v) == TYPE_BYTE &&
            AM_VALID(*instr, AM_ZPX))
          print_notice("can be zero-page,X adressing - is absolute,X");
      } else
        error(ERR_AM);
    }

    /* test for absolute and zero-page Y addressing */
    else if (!strcmp(id, "Y")) {
      if ((TYPE(v) == TYPE_BYTE) && AM_VALID(*instr, AM_ZPY))
        am = AM_ZPY;
      else if (AM_VALID(*instr, AM_ABY)) {
        am = AM_ABY;
        if (pass_num == 2 && NUM_TYPE(v.v) == TYPE_BYTE &&
            AM_VALID(*instr, AM_ZPY))
          print_notice("can be zero-page,Y adressing - is absolute,Y");
      } else
        error(ERR_AM);
    } else
      error(ERR_AM);

    if ((am == AM_ZPX) || (am == AM_ZPY)) {
      emit_instr_1(instr, am, (u8)v.v);
    } else {
      emit_instr_2(instr, am, v.v);
    }

    return am;
  }

  /* handle absolute and zero-page addressing modes */
  int instruction_abs_zp(instruction_desc *instr, value v) {
    int am = AM_INV;

    if ((TYPE(v) == TYPE_BYTE) && AM_VALID(*instr, AM_ZP)) {
      am = AM_ZP;
      if (pass_num == 2) {
        if (UNDEFINED(v))
          error(ERR_UNDEF);
      }
      emit_instr_1(instr, am, (u8)v.v);
    } else if (AM_VALID(*instr, AM_ABS)) {
      am = AM_ABS;
      if (pass_num == 2) {
        if (UNDEFINED(v))
          error(ERR_UNDEF);
        if (NUM_TYPE(v.v) == TYPE_BYTE && AM_VALID(*instr, AM_ZP))
          print_notice("can be zero-page adressing - is absolute");
      }
      emit_instr_2(instr, am, v.v);
    } else
      error(ERR_AM);
    return am;
  }

  /* bit branch, bit set/reset instructions */
  int instruction_zp_rel(char **p, instruction_desc *instr, value v) {
    u16 off;
    value rel;

    if (TYPE(v) != TYPE_BYTE)
      error(ERR_BYTE_RANGE);

    rel = number(p);
    off = calculate_offset(rel);

    if (pass_num == 2) {
      if (UNDEFINED(v) || UNDEFINED(rel))
        error(ERR_UNDEF);
    }
    emit_instr_2b(instr, AM_ZPR, (u8)v.v, off);

    return AM_ZPR;
  }

  u16 am_size[16] = {1, 1, 2, 2, 2, 2, 2, 3, 3, 3, 3, 2, 2, 2, 3, 3};
  inline void processInstruction(std::string &instruction) {
    char id[32];
    instruction_desc *instr;
    int am = 16; // Adressing mode
    value v;
    char *a = const_cast<char *>(instruction.c_str());
    char **p = &a;

    /* first get instruction for given mnemonic */
    ident(p, id, 0, 1);
    instr = get_instruction_descr(id);
    if (!instr)
      error(ERR_INSTR);

    /* if found get addressing mode */
    skip_white(p);
    if (IS_END(**p)) {
      am = instruction_imp_acc(instr);
    } else if (**p == '#') {
      am = instruction_imm(p, instr);
    }

    /* handle indirect addressing modes */
    else if (**p == '(') {
      am = instruction_ind(p, instr);
    }

    /* relative and absolute addressing modes */
    else {
      v = number(p);
      skip_white(p);
      /* relative instruction mode if instruction supports it */
      if (instr->op[AM_REL] != INV) {
        am = instruction_rel(instr, v);
      }
      /* else we go through the possible absolute addressing modes */
      else if (**p == ',') {
        skip_curr_and_white(p);
        if (AM_VALID(*instr, AM_ZPR))
          am = instruction_zp_rel(p, instr, v);
        else
          am = instruction_abxy_zpxy(p, instr, v);
      }
      /* must be absolute or zero-page addressing */
      else {
        am = instruction_abs_zp(instr, v);
      }
    }

    /* update program counter */
    if (am == AM_INV)
      error(ERR_AM);

    address_counter += am_size[am];
  }

public:
  inline std::vector<uint8_t> assemble(std::vector<std::string> &program) {
    for (auto instruction : program) {
      processInstruction(instruction);
    }
    return this->code;
  }
};