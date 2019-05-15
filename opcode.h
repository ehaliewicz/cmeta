#ifndef OPCODE_H
#define OPCODE_H


// all branch targets are 2 bytes absolute reference, little endian

// T -> Test for literal char
// U -> Test for not literal char
// A -> test for Alpha char
// N -> test for Numeric char
// J -> jump to subroutine
// R -> Return from subroutine
// S -> Set flag
// B -> Branch unconditionally
// I -> branch If 
// E -> branch if Error 
// L -> copy Literal
// C -> Copy input char to output buffer
// H -> copy hex representation of input char to output buffer
// G -> Generate label
// F -> Fix left (for labels)
// O -> Output buffer
// D -> Delete buffer




#define OPCODES								\
  X(T, "TST") X(U, "UNTST") X(A, "ALPH") X(N, "NUM")			\
  X(J, "JSR") X(R, "RTS") X(S, "SET")					\
  X(B, "BRA") X(I, "BIF") X(E, "BERR") X(L, "LIT")			\
  X(C, "CPY") X(H, "HEXCPY")						\
  X(G, "GENLBL") X(F, "LBL") X(O, "OUT") X(D, "DEL")

typedef enum {
#define X(nm,lnm) nm,
  OPCODES
#undef X
} opcode;

char* opcode_names[] = {
#define X(nm,lnm) #nm,
  OPCODES
#undef X
};

char* opcode_long_names[] = {
#define X(nm,lnm) lnm,
  OPCODES
#undef X
};



#endif
