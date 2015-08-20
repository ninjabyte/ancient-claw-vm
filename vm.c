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
#include "bytecode.h"

#define NUM_STACKS 4
#define STACK_SIZE 1024 // in bytes
uint32_t pc = 0;
uint32_t sp[NUM_STACKS] = {0};
uint8_t stacks[NUM_STACKS][STACK_SIZE];

// these are actually used as a bool, their size doesn't matter as long as it's at least 1 bit wide
unsigned int flag_zero;
unsigned int flag_negative;

void updateFlags(int32_t value) {
  flag_zero = !value;
  flag_negative = value < 0;
}

void parseInstruction(uint16_t instruction, uint16_t* code, uint16_t* source, uint16_t* destination) {
  *destination = instruction & 3;
  *source = (instruction & 12) >> 2;
  *code = instruction >> 4;
}

uint32_t fetchLiteral(uint8_t* p, int size) {
  uint32_t r;
  switch(size) {
    case 8: // 8 bit literals will still take 16 bits, in order to keep instructions 16-bit aligned
      r = p[pc];
      break;
    case 16:
    default:
      r = p[pc] | p[pc + 1]<<8;
      break;
    case 32:
      r = p[pc] | p[pc + 1]<<8 | p[pc + 2]<<16 | p[pc + 3]<<24;
      break;
  }
  pc += size/8;
  return r;
}

void stackPushUint(unsigned int stack, int size, uint32_t value) {
  // TODO: check if stack is < NUM_STACKS
  // TODO: check for stack overflows
  printf("Push to stack %d value %u with size %d. cur sp: %u\n", stack, value, size, sp[stack]);
  switch(size) {
    case 8:
      stacks[stack][sp[stack]] = value & 0xff;
      sp[stack]++;
      break;
    case 16:
      stacks[stack][sp[stack]] = value & 0xff;
      stacks[stack][sp[stack]+1] = value >> 8;
      sp[stack] += 2;
      break;
    case 32:
      stacks[stack][sp[stack]] = value & 0xff;
      stacks[stack][sp[stack]+1] = (value & 0xff00) >> 8;
      stacks[stack][sp[stack]+2] = (value & 0xff0000) >> 16;
      stacks[stack][sp[stack]+3] = value >> 24;
      sp[stack] += 4;
      break;
  }
}

uint32_t stackPeekUint(unsigned int stack, int size) {
  // TODO: check if stack is < NUM_STACKS
  // TODO: check for stack underflows
  uint32_t value = 0;
  switch(size) {
    case 8:
      value = stacks[stack][sp[stack]-1];
      break;
    case 16:
      value = stacks[stack][sp[stack]-1] << 8;
      value |= stacks[stack][sp[stack]-2];
      break;
    case 32:
      value = stacks[stack][sp[stack]-1] << 24;
      value |= stacks[stack][sp[stack]-2] << 16;
      value |= stacks[stack][sp[stack]-3] << 8;
      value |= stacks[stack][sp[stack]-4];
      break;
  }
  return value;
}

uint32_t stackPopUint(unsigned int stack, int size) {
  // TODO: check if stack is < NUM_STACKS
  // TODO: check for stack underflows
  uint32_t value = 0;
  switch(size) {
    case 8:
      sp[stack]--;
      value = stacks[stack][sp[stack]];
      break;
    case 16:
      sp[stack]--;
      value = stacks[stack][sp[stack]] << 8;
      sp[stack]--;
      value |= stacks[stack][sp[stack]];
      break;
    case 32:
      sp[stack]--;
      value = stacks[stack][sp[stack]] << 24;
      sp[stack]--;
      value |= stacks[stack][sp[stack]] << 16;
      sp[stack]--;
      value |= stacks[stack][sp[stack]] << 8;
      sp[stack]--;
      value |= stacks[stack][sp[stack]];
      break;
  }
  printf("Pop from stack %d value %u with size %d\n", stack, value, size);
  return value;
}

typedef enum {OP_ADD = 0, OP_ADDF, OP_SUB, OP_SUBF, OP_MUL,OP_MULF, OP_DIV, OP_DIVF, OP_MOD, OP_MODF} MathOperations;
void mathIntInstruction(unsigned int src, unsigned int dest, int size, int operation) {
  // TODO make this work for signed
  uint32_t op1 = stackPopUint(src, size);
  uint32_t op2 = stackPopUint(src, size);
  int32_t result = 0;
  switch(operation) {
    case OP_ADD:
      result = op2+op1;
      break;
    case OP_SUB:
      result = op2-op1;
      break;
    case OP_MUL:
      result = op2*op1;
      break;
    case OP_DIV:
      result = op2/op1;
      break;
    case OP_MOD:
      result = op2%op1;
      break;
  }
  stackPushUint(dest, size, result);
  updateFlags(result);
}

typedef enum { OP_SR = 0, OP_SL, OP_SSR, OP_AND, OP_OR, OP_NOT, OP_NOR, OP_NAND, OP_XOR, OP_NEG } BitwiseOperations;
void bitwiseShiftInstruction(unsigned int src, unsigned int dest, int size, int operation) {
  uint8_t places = stackPopUint(src, 8);
  uint32_t value = stackPopUint(src, size);
  uint32_t result = 0;
  switch(operation) {
    case OP_SR:
      result = value >> places;
      break;
    case OP_SSR:
      // TODO: check this works correctly
      result = (int32_t)value >> places;
      break;
    case OP_SL:
      result = value << places;
      break;
  }
  stackPushUint(dest, size, result);
  updateFlags(result);
}

void bitwiseTwoOpInstruction(unsigned int src, unsigned int dest, int size, int operation) {
  uint32_t op1 = stackPopUint(src, size);
  uint32_t op2 = stackPopUint(src, size);
  uint32_t result = 0;
  switch(operation) {
    case OP_AND:
      result = op1 & op2;
      break;
    case OP_OR:
      result = op1 | op2;
      break;
    case OP_NOR:
      result = ~(op1 | op2);
      break;
    case OP_NAND:
      result = ~(op1 & op2);
      break;
    case OP_XOR:
      result = op1 ^ op2;
      break;
  }
  stackPushUint(dest, size, result);
  updateFlags(result);
}

void bitwiseOneOpInstruction(unsigned int src, unsigned int dest, int size, int operation) {
  uint32_t op = stackPopUint(src, size);
  uint32_t result = 0;
  switch(operation) {
    case OP_NOT:
      result = ~op;
      break;
    case OP_NEG:
      result = -op;
      break;
  }
  stackPushUint(dest, size, result);
  updateFlags(result);
}

void run(uint8_t* program, uint32_t buflen) {
  pc = 0;
  updateFlags(0); // reset flags
  while(pc < buflen) {
    uint16_t instruction = program[pc] | (program[pc + 1] << 8);
    pc += 2;
    uint16_t code, source, destination;
    parseInstruction(instruction, &code, &source, &destination);
    printf("PC 0x%u, instruction 0x%x, source %u, dest %u\n", pc-2, code, source, destination);
    switch(code) {
      case LET8:
        stackPushUint(destination, 8, fetchLiteral(program, 8));
        break;
      case LET16:
        stackPushUint(destination, 16, fetchLiteral(program, 16));
        break;
      case LET32:
        stackPushUint(destination, 32, fetchLiteral(program, 32));
        break;
      case CPY8:
        stackPushUint(destination, 8, stackPeekUint(source, 8));
        break;
      case CPY16:
        stackPushUint(destination, 16, stackPeekUint(source, 16));
        break;
      case CPY32:
        stackPushUint(destination, 32, stackPeekUint(source, 32));
        break;
      case MOV8:
        stackPushUint(destination, 8, stackPopUint(source, 8));
        break;
      case MOV16:
        stackPushUint(destination, 16, stackPopUint(source, 16));
        break;
      case MOV32:
        stackPushUint(destination, 32, stackPopUint(source, 32));
        break;
      case DEL8:
        stackPopUint(source, 8);
        break;
      case DEL16:
        stackPopUint(source, 16);
        break;
      case DEL32:
        stackPopUint(source, 32);
        break;

      // math
      case ADD8:
        mathIntInstruction(source, destination, 8, OP_ADD);
        break;
      case ADD16:
        mathIntInstruction(source, destination, 16, OP_ADD);
        break;
      case ADD32:
        mathIntInstruction(source, destination, 32, OP_ADD);
        break;
      case SUB8:
        mathIntInstruction(source, destination, 8, OP_SUB);
        break;
      case SUB16:
        mathIntInstruction(source, destination, 16, OP_SUB);
        break;
      case SUB32:
        mathIntInstruction(source, destination, 32, OP_SUB);
        break;
      case MUL8:
        mathIntInstruction(source, destination, 8, OP_MUL);
        break;
      case MUL16:
        mathIntInstruction(source, destination, 16, OP_MUL);
        break;
      case MUL32:
        mathIntInstruction(source, destination, 32, OP_MUL);
        break;
      case DIV8:
        mathIntInstruction(source, destination, 8, OP_DIV);
        break;
      case DIV16:
        mathIntInstruction(source, destination, 16, OP_DIV);
        break;
      case DIV32:
        mathIntInstruction(source, destination, 32, OP_DIV);
        break;
      case MOD8:
        mathIntInstruction(source, destination, 8, OP_MOD);
        break;
      case MOD16:
        mathIntInstruction(source, destination, 16, OP_MOD);
        break;
      case MOD32:
        mathIntInstruction(source, destination, 32, OP_MOD);
        break;

      // bitwise shifts
      case SR8:
        bitwiseShiftInstruction(source, destination, 8, OP_SR);
        break;
      case SR16:
        bitwiseShiftInstruction(source, destination, 16, OP_SR);
        break;
      case SR32:
        bitwiseShiftInstruction(source, destination, 32, OP_SR);
        break;
      case SSR8:
        bitwiseShiftInstruction(source, destination, 8, OP_SSR);
        break;
      case SSR16:
        bitwiseShiftInstruction(source, destination, 16, OP_SSR);
        break;
      case SSR32:
        bitwiseShiftInstruction(source, destination, 32, OP_SSR);
        break;
      case SL8:
        bitwiseShiftInstruction(source, destination, 8, OP_SL);
        break;
      case SL16:
        bitwiseShiftInstruction(source, destination, 16, OP_SL);
        break;
      case SL32:
        bitwiseShiftInstruction(source, destination, 32, OP_SL);
        break;

      // other bitwise operations with two operands
      case AND8:
        bitwiseTwoOpInstruction(source, destination, 8, OP_AND);
        break;
      case AND16:
        bitwiseTwoOpInstruction(source, destination, 16, OP_AND);
        break;
      case AND32:
        bitwiseTwoOpInstruction(source, destination, 32, OP_AND);
        break;
      case OR8:
        bitwiseTwoOpInstruction(source, destination, 8, OP_OR);
        break;
      case OR16:
        bitwiseTwoOpInstruction(source, destination, 16, OP_OR);
        break;
      case OR32:
        bitwiseTwoOpInstruction(source, destination, 32, OP_OR);
        break;
      case NOR8:
        bitwiseTwoOpInstruction(source, destination, 8, OP_OR);
        break;
      case NOR16:
        bitwiseTwoOpInstruction(source, destination, 16, OP_NOR);
        break;
      case NOR32:
        bitwiseTwoOpInstruction(source, destination, 32, OP_NOR);
        break;
      case NAND8:
        bitwiseTwoOpInstruction(source, destination, 8, OP_NAND);
        break;
      case NAND16:
        bitwiseTwoOpInstruction(source, destination, 16, OP_NAND);
        break;
      case NAND32:
        bitwiseTwoOpInstruction(source, destination, 32, OP_NAND);
        break;
      case XOR8:
        bitwiseTwoOpInstruction(source, destination, 8, OP_XOR);
        break;
      case XOR16:
        bitwiseTwoOpInstruction(source, destination, 16, OP_XOR);
        break;
      case XOR32:
        bitwiseTwoOpInstruction(source, destination, 32, OP_XOR);
        break;

      // bitwise operations with one operand
      case NOT8:
        bitwiseOneOpInstruction(source, destination, 8, OP_NOT);
        break;
      case NOT16:
        bitwiseOneOpInstruction(source, destination, 16, OP_NOT);
        break;
      case NOT32:
        bitwiseOneOpInstruction(source, destination, 32, OP_NOT);
        break;
      case NEG8:
        bitwiseOneOpInstruction(source, destination, 8, OP_NEG);
        break;
      case NEG16:
        bitwiseOneOpInstruction(source, destination, 16, OP_NEG);
        break;
      case NEG32:
        bitwiseOneOpInstruction(source, destination, 32, OP_NEG);
        break;

      case JMP:
        pc = stackPopUint(source, 32);
        break;
      case JMPZ:
      case JMPNZ:
      case JMPN:
      case JMPNN:
      {
        uint32_t loc = stackPopUint(source, 32);
        if((code == JMPZ && flag_zero) ||
           (code == JMPNZ && !flag_zero) ||
           (code == JMPN && flag_negative) ||
           (code == JMPNN && !flag_negative)) {
          pc = loc;
        }
        break;
      }
      case BR:
        pc += (int16_t)fetchLiteral(program, 16);
        break;
      case BRZ:
      case BRNZ:
      case BRN:
      case BRNN:
      {
        int16_t offset = fetchLiteral(program, 16);
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
      // default: nop
    }
  }
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
  return 0;
}