#include "translate.h"

/// @brief Get addressing mode
/// @param a address after mod reg r/m byte
void get_adm(const unsigned char *p, size_t a, unsigned char mod,
             unsigned char rm, char *ea, size_t *ds) {

  size_t disp = 0;
  if (mod == 0b00 && rm == 0b110) {
    *ds = 2;
    disp = p[a] + (p[a + 1] << 8);
    sprintf(ea, "[%04lx]", disp);
    return;
  }

  char *dest;
  switch (rm) {
  case 0b000:
    dest = "bx+si";
    break;
  case 0b001:
    dest = "bx+di";
    break;
  case 0b010:
    dest = "bp+si";
    break;
  case 0b011:
    dest = "bp+di";
    break;
  case 0b100:
    dest = "si";
    break;
  case 0b101:
    dest = "di";
    break;
  case 0b110:
    dest = "bp";
    break;
  case 0b111:
    dest = "bx";
    break;
  }

  switch (mod) {
  case 0b01:
    *ds = 1;
    disp = p[a]; // to maybe define as signed char
    unsigned char adisp = (disp & 0x80) ? (~disp + 1) : disp;
    sprintf(ea, "[%s%c%x]", dest, (disp & 0x80) ? '-' : '+', adisp);
    break;

  case 0b10:
    *ds = 2;
    disp = p[a] + (p[a + 1] << 8);
    sprintf(ea, "[%s+%lx]", dest, disp);
    break;

  case 0b00:
  case 0b11:
    *ds = 0;
    sprintf(ea, !mod ? "[%s]" : "%s", dest); // to recheck maybe switch cases
    break;
  }
}

void translate_bin(const unsigned char *p, size_t p_size) {
  unsigned char op;
  size_t a = 0, ip = 0, pip = ip;

  void (*opcodes[16])(const unsigned char *, size_t, unsigned char,
                      size_t *) = {f0x0, f0x1, f0x2, f0x3, f0x4, f0x5,
                                   f0x6, f0x7, f0x8, f0x9, f0xa, f0xb,
                                   f0xc, f0xd, f0xe, f0xf};

  while (a < p_size) {
    op = p[a] >> 4;

    if (!op) {
      if (a + 1 >= p_size) {
        print4b(p, a, 1, &ip);
        printf("(undefined)\n");
        return; // exit if out of bounds, recheck if ends with something else
      }
    }

    opcodes[op](p, a, p[a] & 0b1111, &ip);

    if (pip == ip) {
      printf("(undefined) %02x\n", p[a]);
      ip++;
    }
    a = ip;
    pip = ip;
  }
}