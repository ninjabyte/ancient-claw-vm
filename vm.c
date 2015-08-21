/*
CLAW bytecode virtual machine draft

Copyright (c) 2015, Gabriel Maia <gabriel@tny.im> / Segvault
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

This software can be relicensed on request; contact the author.
*/

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "bytecode.h"

#define NUM_STACKS 4
#define STACK_SIZE 1024 // in bytes
uint32_t pc = 0;
uint32_t sp[NUM_STACKS] = {0}; // stack pointers always point to the next free position
uint8_t stacks[NUM_STACKS][STACK_SIZE];

// these are actually used as a bool, their size doesn't matter as long as it's at least 1 bit wide
unsigned int flag_zero;
unsigned int flag_negative;

void updateFlags(int32_t value) {
  flag_zero = !value;
  flag_negative = value < 0;
}

typedef enum {
  NONE = 0,
  ERR_ARITHMETIC,
  ERR_STACK_OVERFLOW,
  ERR_STACK_UNDERFLOW,
  ERR_INSUFFICIENT_PERMISSIONS,
  ERR_TARGET, // PC out of bounds
} RuntimeError;
RuntimeError last_error = NONE;


// TODO make fetch check program bounds
static uint32_t fetch8bitLiteral(uint8_t* p) {
  return p[pc++];
}

static uint32_t fetch16bitLiteral(uint8_t* p) {
  pc += 2;
  return p[pc - 2] | p[pc - 1] << 8;
}

static uint32_t fetch32bitLiteral(uint8_t* p) {
  pc += 4;
  return p[pc - 4] | p[pc - 3]<<8 | p[pc - 2]<<16 | p[pc - 1]<<24;
}

static void stackPush8bit(unsigned int stack, uint8_t value) {
  if(++sp[stack] >= STACK_SIZE) {
    last_error = ERR_STACK_OVERFLOW;
    return;
  }
  stacks[stack][sp[stack] - 1] = value;
}

static void stackPush16bit(unsigned int stack, uint16_t value) {
  if(sp[stack] + sizeof(uint16_t) >= STACK_SIZE) {
    last_error = ERR_STACK_OVERFLOW;
    return;
  }
  memcpy(&stacks[stack][sp[stack]], &value, sizeof(uint16_t));
  sp[stack] += sizeof(uint16_t);
}

static void stackPush32bit(unsigned int stack, uint32_t value) {
  if(sp[stack] + sizeof(uint32_t) >= STACK_SIZE) {
    last_error = ERR_STACK_OVERFLOW;
    return;
  }
  memcpy(&stacks[stack][sp[stack]], &value, sizeof(uint32_t));
  sp[stack] += sizeof(uint32_t);
}

static uint8_t stackPeek8bit(unsigned int stack) {
  if(!sp[stack]) {
    last_error = ERR_STACK_UNDERFLOW;
    return 0;
  }
  return stacks[stack][sp[stack]-1];
}

static uint16_t stackPeek16bit(unsigned int stack) {
  if(sp[stack] < sizeof(uint16_t)) {
    last_error = ERR_STACK_UNDERFLOW;
    return 0;
  }
  uint16_t value;
  memcpy(&value, &stacks[stack][sp[stack]-sizeof(uint16_t)], sizeof(uint16_t));
  return value;
}

static uint32_t stackPeek32bit(unsigned int stack) {
  if(sp[stack] < sizeof(uint32_t)) {
    last_error = ERR_STACK_UNDERFLOW;
    return 0;
  }
  uint32_t value;
  memcpy(&value, &stacks[stack][sp[stack]-sizeof(uint32_t)], sizeof(uint32_t));
  return value;
}

static uint8_t stackPop8bit(unsigned int stack) {
  if(!sp[stack]) {
    last_error = ERR_STACK_UNDERFLOW;
    return 0;
  }
  return stacks[stack][--sp[stack]];
}

static uint16_t stackPop16bit(unsigned int stack) {
  if(sp[stack] < sizeof(uint16_t)) {
    last_error = ERR_STACK_UNDERFLOW;
    return 0;
  }
  uint16_t value;
  sp[stack] -= sizeof(uint16_t);
  memcpy(&value, &stacks[stack][sp[stack]], sizeof(uint16_t));
  return value;
}
static uint32_t stackPop32bit(unsigned int stack) {
  if(sp[stack] < sizeof(uint32_t)) {
    last_error = ERR_STACK_UNDERFLOW;
    return 0;
  }
  uint32_t value;
  sp[stack] -= sizeof(uint32_t);
  memcpy(&value, &stacks[stack][sp[stack]], sizeof(uint32_t));
  return value;
}

static void run(uint8_t* program, uint32_t buflen) {
  pc = 0;
  last_error = NONE;
  updateFlags(0); // reset flags
  while(pc < buflen) {
    if(last_error != NONE) {
      return;
    }
    uint16_t instruction = program[pc] | (program[pc + 1] << 8);
    pc += 2;
    uint16_t destination = instruction & 3;
    uint16_t source = (instruction & 12) >> 2;
    uint16_t code = instruction >> 4;

#ifdef DEBUG
    printf("PC 0x%u, instruction 0x%x, source %u, dest %u\n", pc-2, code, source, destination);
#endif
    switch(code) {
      case LET8:
        stackPush8bit(destination, fetch8bitLiteral(program));
        break;
      case LET16:
        stackPush16bit(destination, fetch16bitLiteral(program));
        break;
      case LET32:
        stackPush32bit(destination, fetch32bitLiteral(program));
        break;
      case CPY8:
        stackPush8bit(destination, stackPeek8bit(source));
        break;
      case CPY16:
        stackPush16bit(destination, stackPeek16bit(source));
        break;
      case CPY32:
        stackPush32bit(destination, stackPeek32bit(source));
        break;
      case MOV8:
        stackPush8bit(destination, stackPop8bit(source));
        break;
      case MOV16:
        stackPush16bit(destination, stackPop16bit(source));
        break;
      case MOV32:
        stackPush32bit(destination, stackPop32bit(source));
        break;
      case DEL8:
        stackPop8bit(source);
        break;
      case DEL16:
        stackPop16bit(source);
        break;
      case DEL32:
        stackPop32bit(source);
        break;

      // math
      case ADD8:
      {
        uint8_t r = stackPop8bit(source) + stackPop8bit(source);
        stackPush8bit(destination, r);
        updateFlags(r);
        break;
      }
      case ADD16:
      {
        uint16_t r = stackPop16bit(source) + stackPop16bit(source);
        stackPush16bit(destination, r);
        updateFlags(r);
        break;
      }
      case ADD32:
      {
        uint32_t r = stackPop32bit(source) + stackPop32bit(source);
        stackPush32bit(destination, r);
        updateFlags(r);
        break;
      }
      case SUB8:
      {
        uint8_t op1 = stackPop8bit(source);
        uint8_t r = stackPop8bit(source) - op1;
        stackPush8bit(destination, r);
        updateFlags(r);
        break;
      }
      case SUB16:
      {
        uint16_t op1 = stackPop16bit(source);
        uint16_t r = stackPop16bit(source) - op1;
        stackPush16bit(destination, r);
        updateFlags(r);
        break;
      }
      case SUB32:
      {
        uint32_t op1 = stackPop32bit(source);
        uint32_t r = stackPop32bit(source) - op1;
        stackPush32bit(destination, r);
        updateFlags(r);
        break;
      }
      case MUL8:
      {
        uint8_t r = stackPop8bit(source) * stackPop8bit(source);
        stackPush8bit(destination, r);
        updateFlags(r);
        break;
      }
      case MUL16:
      {
        uint16_t r = stackPop16bit(source) * stackPop16bit(source);
        stackPush16bit(destination, r);
        updateFlags(r);
        break;
      }
      case MUL32:
      {
        uint32_t r = stackPop32bit(source) * stackPop32bit(source);
        stackPush32bit(destination, r);
        updateFlags(r);
        break;
      }
      case DIV8:
      {
        uint8_t op1 = stackPop8bit(source);
        uint8_t r = stackPop8bit(source) / op1;
        stackPush8bit(destination, r);
        updateFlags(r);
        break;
      }
      case DIV16:
      {
        uint16_t op1 = stackPop16bit(source);
        uint16_t r = stackPop16bit(source) / op1;
        stackPush16bit(destination, r);
        updateFlags(r);
        break;
      }
      case DIV32:
      {
        uint32_t op1 = stackPop32bit(source);
        uint32_t r = stackPop32bit(source) / op1;
        stackPush32bit(destination, r);
        updateFlags(r);
        break;
      }
      case MOD8:
      {
        uint8_t op1 = stackPop8bit(source);
        uint8_t r = stackPop8bit(source) % op1;
        stackPush8bit(destination, r);
        updateFlags(r);
        break;
      }
      case MOD16:
      {
        uint16_t op1 = stackPop16bit(source);
        uint16_t r = stackPop16bit(source) % op1;
        stackPush16bit(destination, r);
        updateFlags(r);
        break;
      }
      case MOD32:
      {
        uint32_t op1 = stackPop32bit(source);
        uint32_t r = stackPop32bit(source) % op1;
        stackPush32bit(destination, r);
        updateFlags(r);
        break;
      }

      // bitwise shifts
      case SR8:
      {
        uint8_t places = stackPop8bit(source);
        uint8_t value = stackPop8bit(source) >> places;
        stackPush8bit(destination, value);
        updateFlags(value);
        break;
      }
      case SR16:
      {
        uint16_t places = stackPop16bit(source);
        uint16_t value = stackPop16bit(source) >> places;
        stackPush16bit(destination, value);
        updateFlags(value);
        break;
      }
      case SR32:
      {
        uint32_t places = stackPop32bit(source);
        uint32_t value = stackPop32bit(source) >> places;
        stackPush32bit(destination, value);
        updateFlags(value);
        break;
      }
      case SSR8:
      {
        uint8_t places = stackPop8bit(source);
        int8_t value = (int8_t)stackPop8bit(source) >> places;
        stackPush8bit(destination, value);
        updateFlags(value);
        break;
      }
      case SSR16:
      {
        uint16_t places = stackPop16bit(source);
        int16_t value = (int16_t)stackPop16bit(source) >> places;
        stackPush16bit(destination, value);
        updateFlags(value);
        break;
      }
      case SSR32:
      {
        uint32_t places = stackPop32bit(source);
        int32_t value = (int32_t)stackPop32bit(source) >> places;
        stackPush32bit(destination, value);
        updateFlags(value);
        break;
      }
      case SL8:
      {
        uint8_t places = stackPop8bit(source);
        int8_t value = (int8_t)stackPop8bit(source) << places;
        stackPush8bit(destination, value);
        updateFlags(value);
        break;
      }
      case SL16:
      {
        uint16_t places = stackPop16bit(source);
        int16_t value = (int16_t)stackPop16bit(source) << places;
        stackPush16bit(destination, value);
        updateFlags(value);
        break;
      }
      case SL32:
      {
        uint32_t places = stackPop32bit(source);
        int32_t value = (int32_t)stackPop32bit(source) << places;
        stackPush32bit(destination, value);
        updateFlags(value);
        break;
      }

      // other bitwise operations with two operands
      case AND8:
      {
        uint8_t v = stackPop8bit(source) & stackPop8bit(source);
        stackPush8bit(destination, v);
        updateFlags(v);
        break;
      }
      case AND16:
      {
        uint16_t v = stackPop16bit(source) & stackPop16bit(source);
        stackPush16bit(destination, v);
        updateFlags(v);
        break;
      }
      case AND32:
      {
        uint32_t v = stackPop32bit(source) & stackPop32bit(source);
        stackPush32bit(destination, v);
        updateFlags(v);
        break;
      }
      case OR8:
      {
        uint8_t v = stackPop8bit(source) | stackPop8bit(source);
        stackPush8bit(destination, v);
        updateFlags(v);
        break;
      }
      case OR16:
      {
        uint16_t v = stackPop16bit(source) | stackPop16bit(source);
        stackPush16bit(destination, v);
        updateFlags(v);
        break;
      }
      case OR32:
      {
        uint32_t v = stackPop32bit(source) | stackPop32bit(source);
        stackPush32bit(destination, v);
        updateFlags(v);
        break;
      }
      case NOR8:
      {
        uint8_t v = ~(stackPop8bit(source) | stackPop8bit(source));
        stackPush8bit(destination, v);
        updateFlags(v);
        break;
      }
      case NOR16:
      {
        uint16_t v = ~(stackPop16bit(source) | stackPop16bit(source));
        stackPush16bit(destination, v);
        updateFlags(v);
        break;
      }
      case NOR32:
      {
        uint32_t v = ~(stackPop32bit(source) | stackPop32bit(source));
        stackPush32bit(destination, v);
        updateFlags(v);
        break;
      }
      case NAND8:
      {
        uint8_t v = ~(stackPop8bit(source) & stackPop8bit(source));
        stackPush8bit(destination, v);
        updateFlags(v);
        break;
      }
      case NAND16:
      {
        uint16_t v = ~(stackPop16bit(source) & stackPop16bit(source));
        stackPush16bit(destination, v);
        updateFlags(v);
        break;
      }
      case NAND32:
      {
        uint32_t v = ~(stackPop32bit(source) & stackPop32bit(source));
        stackPush32bit(destination, v);
        updateFlags(v);
        break;
      }
      case XOR8:
      {
        uint8_t v = stackPop8bit(source) ^ stackPop8bit(source);
        stackPush8bit(destination, v);
        updateFlags(v);
        break;
      }
      case XOR16:
      {
        uint16_t v = stackPop16bit(source) ^ stackPop16bit(source);
        stackPush16bit(destination, v);
        updateFlags(v);
        break;
      }
      case XOR32:
      {
        uint32_t v = stackPop32bit(source) ^ stackPop32bit(source);
        stackPush32bit(destination, v);
        updateFlags(v);
        break;
      }

      // bitwise operations with one operand
      case NOT8:
      {
        uint8_t v = ~ stackPop8bit(source);
        stackPush8bit(destination, v);
        updateFlags(v);
        break;
      }
      case NOT16:
      {
        uint16_t v = ~ stackPop16bit(source);
        stackPush16bit(destination, v);
        updateFlags(v);
        break;
      }
      case NOT32:
      {
        uint32_t v = ~ stackPop32bit(source);
        stackPush32bit(destination, v);
        updateFlags(v);
        break;
      }
      case NEG8:
      {
        int8_t v = -(int8_t)stackPop8bit(source);
        stackPush8bit(destination, v);
        updateFlags(v);
        break;
      }
      case NEG16:
      {
        int16_t v = -(int16_t)stackPop16bit(source);
        stackPush16bit(destination, v);
        updateFlags(v);
        break;
      }
      case NEG32:
      {
        int32_t v = -(int32_t)stackPop32bit(source);
        stackPush32bit(destination, v);
        updateFlags(v);
        break;
      }

      // flow control
      case JMP:
        pc = stackPop32bit(source);
        break;
      case JMPZ:
      case JMPNZ:
      case JMPN:
      case JMPNN:
      {
        uint32_t loc = stackPop32bit(source);
        if((code == JMPZ && flag_zero) ||
           (code == JMPNZ && !flag_zero) ||
           (code == JMPN && flag_negative) ||
           (code == JMPNN && !flag_negative)) {
          pc = loc;
        }
        break;
      }
      case BR:
        pc += (int16_t)fetch16bitLiteral(program);
        break;
      case BRZ:
      case BRNZ:
      case BRN:
      case BRNN:
      {
        int16_t offset = fetch16bitLiteral(program);
        if((code == BRZ && flag_zero) ||
           (code == BRNZ && !flag_zero) ||
           (code == BRN && flag_negative) ||
           (code == BRNN && !flag_negative)) {
          pc += offset;
        }
        break;
      }
      case ENDZ:
        if(flag_zero)
          return;
        break;
      case ENDN:
        if(flag_negative)
          return;
        break;
      case END:
        return;

      // debug instructions
      case DMPSSTR:
      {
        char c;
        while((c = fetch8bitLiteral(program))) {
          putchar(c);
        }
        break;
      }
      case DMPN8:
        printf("%u", stackPop8bit(source));
        break;
      case DMPN16:
        printf("%u", stackPop16bit(source));
        break;
      case DMPN32:
        printf("%u", stackPop32bit(source));
        break;
      // default: nop
    }
  }
  last_error = ERR_TARGET;
}

int main(int argc, char *argv[]) {
  /* PASTEBIN SAMPLE
  LET8 A
  .db8u(101)
  LET8 A  
  .db8u(99)
  ADD8 A
  LET8 A  
  .db8u(200)
  SUB8 A
  BRZ A
  .db16(2)     
  END       
  MOV8 A B
  END
  */
  if(argc < 2) {
    printf("Give me an input file!\n");
    return 1;
  }
  FILE* f = fopen(argv[1], "r");
  if(f == NULL) {
    printf("Error opening input file\n");
    return 1;
  }

  // obtain file size:
  fseek(f , 0 , SEEK_END);
  size_t size = ftell (f);
  rewind(f);

  uint8_t* program = (uint8_t*)malloc(sizeof(uint8_t)*size);
  if (program == NULL) {fputs ("Memory error",stderr); exit (2);}

  size_t result = fread (program, 1, size, f);
  if (result != size) {fputs ("Reading error",stderr); exit (3);}

  fclose(f);
  run(program, size);
  if(last_error != NONE) {
    printf("Runtime error: %u\n", last_error);
  }
  return 0;
}