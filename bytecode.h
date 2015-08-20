typedef enum {
  NOP = 0x0,
  SLEEP = 0x2,
  LET8 = 0x4,
  LET16 = 0x5,
  LET32 = 0x6,
  LETA = 0x7,
  CPY8 = 0xd,
  CPY16 = 0xe,
  CPY32 = 0xf,
  CPYA = 0x10,
  MOV8 = 0x11,
  MOV16 = 0x12,
  MOV32 = 0x13,
  MOVA = 0x14,
  SWP8 = 0x15,
  SWP16 = 0x16,
  SWP32 = 0x17,
  SWPA = 0x18,
  PEEKD8 = 0x19,
  PEEKD16 = 0x1a,
  PEEKD32 = 0x1b,
  SPTR = 0x1c,
  DEL8 = 0x1d,
  DEL16 = 0x1e,
  DEL32 = 0x1f,
  MMCP = 0x2d,
  ADD8 = 0x30,
  ADD16 = 0x31,
  ADD32 = 0x32,
  ADDF = 0x33,
  SUB8 = 0x34,
  SUB16 = 0x35,
  SUB32 = 0x36,
  SUBF = 0x37,
  MUL8 = 0x38,
  MUL16 = 0x39,
  MUL32 = 0x3a,
  MULF = 0x3b,
  DIV8 = 0x3c,
  DIV16 = 0x3d,
  DIV32 = 0x3e,
  DIVF = 0x3f,
  DIVU8 = 0x40,
  DIVU16 = 0x41,
  DIVU32 = 0x42,
  MOD8 = 0x43,
  MOD16 = 0x44,
  MOD32 = 0x45,
  MODF = 0x46,
  MODU8 = 0x47,
  MODU16 = 0x48,
  MODU32 = 0x49,
  SR8 = 0x4a,
  SR16 = 0x4b,
  SR32 = 0x4c,
  SL8 = 0x4d,
  SL16 = 0x4e,
  SL32 = 0x4f,
  SSR8 = 0x50,
  SSR16 = 0x51,
  SSR32 = 0x52,
  AND8 = 0x53,
  AND16 = 0x54,
  AND32 = 0x55,
  OR8 = 0x56,
  OR16 = 0x57,
  OR32 = 0x58,
  NOT8 = 0x59,
  NOT16 = 0x5a,
  NOT32 = 0x5b,
  NOR8 = 0x5c,
  NOR16 = 0x5d,
  NOR32 = 0x5e,
  NAND8 = 0x5f,
  NAND16 = 0x60,
  NAND32 = 0x61,
  XOR8 = 0x62,
  XOR16 = 0x63,
  XOR32 = 0x64,
  NEG8 = 0x65,
  NEG16 = 0x66,
  NEG32 = 0x67,
  C8T16 = 0xfd,
  C8T32 = 0xfe,
  C16T8 = 0xff,
  C16T32 = 0x100,
  C32T8 = 0x101,
  C32T16 = 0x102,
  C8UT16U = 0x103,
  C8UT32U = 0x104,
  C16UT8U = 0x105,
  C16UT32U = 0x106,
  C32UT8U = 0x107,
  C32UT16U = 0x108,
  CFT32 = 0x109,
  C32TF = 0x10a,
  JMP = 0x110,
  JMPZ = 0x111,
  JMPNZ = 0x112,
  JMPN = 0x113,
  JMPNN = 0x114,
  BR = 0x118,
  BRZ = 0x119,
  BRNZ = 0x11a,
  BRN = 0x11b,
  BRNN = 0x11c,
  CALL = 0x120,
  RET = 0x121,
  END = 0x128,
  ENDZ = 0x129,
  ENDN = 0x12a,
  CLR = 0x139,
  OLED = 0x13a,
  GETPIX = 0x13b,
  FILL = 0x13c,
  FONT = 0x13e,
  PRINT = 0x13f,
  COLOR = 0x140,
  POINT = 0x141,
  HLINE = 0x142,
  VLINE = 0x143,
  LINE = 0x144,
  RECT = 0x145,
  LRECT = 0x146,
  ELIPS = 0x147,
  LELIPS = 0x148,
  CIRCL = 0x149,
  LCIRCL = 0x14a,
  SPRT = 0x14b,
  POLY = 0x14c,
  BITM = 0x14d,
  SWBUFF = 0x14e,
  GMODE = 0x14f,
  MIRROR = 0x150,
  CONST8_M1 = 0x159,
  CONST8_0 = 0x15a,
  CONST8_1 = 0x15b,
  CONST8_2 = 0x15c,
  CONST16_M1 = 0x15d,
  CONST16_0 = 0x15e,
  CONST16_1 = 0x15f,
  CONST16_2 = 0x160,
  CONST32_M1 = 0x161,
  CONST32_0 = 0x162,
  CONST32_1 = 0x163,
  CONST32_2 = 0x164,
  CONSTF_M1 = 0x165,
  CONSTF_0 = 0x166,
  CONSTF_1 = 0x167,
  CONSTF_2 = 0x168,
  PROCESSOR_ID = 0x1a1,
  WIFI_ID = 0x1a2,
  BTN = 0x1a6,
  STANDBY = 0x1aa,
  POWEROFF = 0x1ab,
  DOOM = 0x29a,
  RICK = 0x539
} InstructionSet;