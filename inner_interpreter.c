/*
   invoke preprocessor only
   ~/tcc/tcc.exe -E inner_interpreter.c | grep -v '^[#]'

   first line contains filename
*/

#define POP SD
#define PUSH s
#define WRITE .
#define FETCH(X) X m _0i @
#define STORE(x) x

// Instruction register: j (I)
// contains the address of thenext instruction in then
// threaded list of the current secondary

// Word address register: k (WA)
// contains the word address of the current keyword
// or the address of the first code bodylocation of the current keyword.

// Code address register: l (CA)
// Return stack register: r (RS)
// Stack pointer register: s?
// processor program counter register: p

7EXE     // Dictionary header for EXECUTE
0000     // Link address
_0[0000]0000
Tk       // POP SP->WA
_0000p   // JMP {RUN}

// SEMI
_0[0000]0000
Rj      // POP RS->I

// NEXT
Jm_0i@k // @I->WA
_4sJ+j  // I = I + 4

// RUN
_0[0000]0000
Km_0i@l // @WA->CA
_4sK+k  // WA = WA + 4
Lp      // CA->PC

// COLON
Jr      // PSH I->RS
Kj      // WA->I
p       // JMP
