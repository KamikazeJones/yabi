#!/c/Users/ninjakuza/tcc/tcc.exe -run

/* yet another bytecode interpreter */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void intro();
void usage();
int x_num(char char_digit, int val);
int get_program(int index);

char *run(char *bc, int addr, int patch, int debug);
char *bc = "";

char *check=NULL;

int pcadd; // flag: if 0 then the pc is not incremented for one turn
char *program;
int debug = 0;
int patch = 0;
char result[500];

void patchJumpAddresses(int len, int startaddr, int debug) {
    // patch jump addresses
    char addrstr[10];
    int dest = 0;
    int n=5;

    debug = 0;
    if(debug) printf("patchJumpAdresses");
    for(int i=startaddr; i<startaddr+len; i++) {
        if(debug) printf("%c",program[i]);
        if(program[i]=='[') {
            dest=0;
            for(int k=1;k<5;k++) {
              dest = x_num(get_program(i+k), dest);
              if(debug) printf("%c",get_program(i+k));
            }
            if(dest == 0) { //not patched yet
              int c=0; n=5;
              while(!(get_program(i+n)==']' && c==0)) {
                if(get_program(i+n)==']') c--;
                else if(get_program(i+n)=='[') c++;
                n++;
              }
              // Rücksprungadresse gefunden
              dest=n+5;
              sprintf(addrstr,"%04x", dest);
              if(debug) printf("\n%d destiny:%d\n", i+1, dest);
              strncpy(&program[i+1], addrstr, 4);
              sprintf(addrstr,"%04x", n-5);
              if(debug) printf("\n%d start:%d\n", n+i, dest);
              strncpy(&program[i+n+1], addrstr, 4);
            }
        }
    }
}

typedef struct {
  int pc;
  int accu;
  int S;
  int SP;
  int R;
  int RP;
  int reg_i;
  int reg_j;
  int reg_k;
  int reg_l;
  int reg_m;
  int reg_n;
} CPU;

CPU cpu;

int main(int argc, char **argv) {
  int retval = 0;
  int skip=0;
  patch = 0;
  debug = 0;

/*
  printf("argc: %03d\n", argc);

  for(int i=0; i<argc; i++) {
      printf("%03d: %s\n",i,argv[i]);
  }
*/

  for(int i=1; i<argc; i++) {
      if(skip) {
          skip=0;
          continue;
      }

      if(strcmp(argv[i],"-u")==0) {
        usage();
        bc = "";
        break;
      }

      else if(strcmp(argv[i],"-x")==0) {
        bc = argv[i+1];
        skip=1;
      }

      else if(strcmp(argv[i],"-c")==0) {
        check = argv[i+1];
        skip=1;
      }
      else if(strcmp(argv[i],"-d")==0) {
        debug=1;
      }
      else if(strcmp(argv[i],"-p")==0) {
        patch=1;
      }
      else if(strcmp(argv[i],"-P")==0) {
        patch=2;
      }
      else if(strcmp(argv[i],"-i")==0) {
        intro();
      }
  }

  if(strlen(bc)>0) {
    run(bc, 256, patch, debug);

    printf("%s\n", result);

    if(check!=NULL) {
      if(strcmp(result, check)==0) {
        printf("  -- check Ok\n");
      }
      else {
        printf("  -- check ERROR, expected: %s\n", check);
        printf("                   program: %.*s\n", strlen(bc), program+256);
      }
      retval=1;
    }
  }
  return retval;

}

void print_memory(int addr, int n, char delim) {
  for(int i=0; i<n; i++) {
    if(i>0 && delim != 0) printf("%c", delim);
     printf("%c", get_program(addr+i));
  }
}

void print_r_stack(int addr, int n) {
  int i=0;
  while(i<n) {
      if(i>0) printf(" ");
      print_memory(addr+4*i, 4, 0);
      i+=4;
  }
}

void print_s_stack(int addr, int n) {
      print_memory(addr, n, " ");
}

void print_status(char byte, char *result) {
      printf("pc:%04x bc:%c ac:%04x i:%04x j:%04x k:%04x result:%s S-Stack:",
      cpu.pc, bc, cpu.accu, cpu.reg_i, cpu.reg_j, cpu.reg_k, result);
      print_s_stack(cpu.S, cpu.SP);
      printf(" R-Stack:");
      print_r_stack(cpu.R, cpu.RP);
      printf("\n");
}

/* INTERPRETER */

int get_num(char c) {
  int num;
  num = c - '0';
  if(c >= 'a') num = c - 'a' + 10;
  return num;
}

int get_char(int num) {
  int c;
  c = num + '0';
  if(num >= 0x0a) c = 'a' + num;
  return c;
}

int x_num(char char_digit, int val) {
   int v = ((val << 4) & 0xffff); v |= get_num(char_digit);
   return v;
}

#define x_store_accu(X) { X = cpu.accu; }
#define x_load_accu(X) { cpu.accu = X; }

#define x_print(X){ result[resp] = (X & 0xff); resp++; result[resp] = '\0';}

#define x_push(STACK) { program[STACK + STACK ## P ++] = cpu.accu; }
#define x_pop(STACK) { cpu.accu = program[STACK + -- STACK ## P]; }
#define x_add(STACK) { cpu.accu += program[STACK + STACK ## P - 1]; }
#define x_sub(STACK) { cpu.accu -= program[STACK + STACK ## P - 1]; }
#define x_clear { cpu.accu = 0; }
#define x_drop(STACK) { -- STACK ## P; }

#define x_and(STACK) {cpu.accu &= program[STACK + STACK ## P - 1]; }
#define x_or(STACK)  {cpu.accu |= program[STACK + STACK ## P - 1]; }
#define x_xor(STACK) {cpu.accu ^= program[STACK + STACK ## P - 1]; }
#define x_not {cpu.accu = (~cpu.accu) & 0xffff; }
#define x_lshift { cpu.accu = cpu.accu << 1; }
#define x_rshift { cpu.accu = cpu.accu >> 1; }

int x_load(int addr) {
  int a = 0;
  for(int i=0; i<4;i++) {
    a = x_num(get_program(addr + i), a);
  }
  return a;
}

void x_store(int val, int addr) {
  for(int i=3; i<=0; i++) {
     program[addr+i] = val & 0xff;
     val >> 4;
  }
  return;
}

int get_program(int index) {
    if(index < 0 || index >= 5000) {
        printf("access violation : program index: %d", index);
        exit(1);
    }
    return program[index];
}

void x_while(condition) {
    char deststr[10];
    int dest = 0;
    if(condition) {
        dest = 0;
        if(debug) printf("nibble:");
        for(int i=1;i<5;i++) {
            dest = x_num(get_program(cpu.pc+i), dest);
            if(debug) printf("%c",get_program(cpu.pc+i));
        }
        printf("\n");
        if(dest != 0) {
            cpu.pc =cpu. pc + dest;
            if(debug) printf("goto dest:%02ws",pc + dest);
        }
        else {
            int c=0; int n=5;
            while(!(get_program(pc+n)==']' && c==0)) {
                if(get_program(pc+n)==']') c--;
                else if(get_program(pc+n)=='[') c++;
                n++;
            }
            // Rücksprungadresse gefunden
            dest=n+5;
            sprintf(deststr,"%04x", dest);
            strncpy(&program[pc+1], deststr, 4);
            pc = pc + dest;
        }
    }
    else pc+=5;
    pcadd = 0;
}

void x_wend(condition) {
    char deststr[10];
    int dest = 0;
    if(condition) {
        dest = 0;
        if(debug) printf("nibble:");
        for(int i=1;i<5;i++) {
            dest = x_num(get_program(pc+i), dest);
            if(debug) printf("%c",get_program(pc+i));
        }
        if(debug) printf("\n");
        if(dest != 0) {
            pc = pc - dest;
            if(debug) printf("goto dest:%04x\n",pc+dest);
        }
        else {
            int c=0; int n=1;
            while(!(get_program(pc-n)=='[' && c==0)) {
                if(get_program(pc-n)=='[') c--;
                else if(get_program(pc-n)==']') c++;
                n++;
            }
            // Rücksprungadresse gefunden
            dest = n-5;
            sprintf(deststr,"%04x", dest);
            strncpy(&program[pc+1], deststr, 4);
            pc = pc - dest;
            if(debug) printf("dest:%04x goto:%04x\n", dest, pc);
        }
    }
    else pc+=5;

    pcadd = 0;
}

void init()  {
  cpu.reg_i=0;
  cpu.reg_j=0;
  cpu.reg_k=0;
}

char *run(char *prog, int addr, int patch, int debug) {
    int jump_pc[100];
    int jump_dest[100];
    int jumpstack[100];
    int jsp;

    cpu.S=5;
    cpu.SP=0;
    cpu.R=100;
    cpu.RP=0;

    char bc;

    char ch;

    int resp = 0;

    // init
    pcadd = 1;
    cpu.pc = addr;
    cpu.accu = 0;
    program = (char *)malloc(5000);
    for(int i=0;i<5000; i++) {
        program[i]=0;
    }
    result[0] = '\0';
    strcpy(program+addr, prog);

    if(patch) {
        // printf("%d %d\n",strlen(prog), addr);
        patchJumpAddresses(strlen(prog), addr, debug);
        if(patch == 2) { // patch only!
           sprintf(result, "%.*s\0", strlen(prog), program + addr);
           return result;
        }
    }
    while(*(program+pc)!='H') {
        bc = *(program+pc);

        if(debug) print_status(bc, result);

        switch(bc) {
            case '0': case '1': case '2': case '3': case '4': case '5':
            case '6': case '7': case '8': case '9': case 'a': case 'b':
            case 'c': case 'd': case 'e': case 'f':
                cpu.accu = x_num (bc, cpu.accu); break;

            case 'p': x_store_accu (cpu.pc); pcadd=0; break;
            case 'i': x_store_accu (cpu.reg_i); break;
            case 'j': x_store_accu (cpu.reg_j); break;
            case 'k': x_store_accu (cpu.reg_k); break;
            case 'l': x_store_accu (cpu.reg_l); break;
            case 'm': x_store_accu (cpu.reg_m); break;
            case 'n': x_store_accu (cpu.reg_n); break;

            case 'P': x_load_accu (cpu.pc); break;
            case 'I': x_load_accu (cpu.reg_i); break;
            case 'J': x_load_accu (cpu.reg_j); break;
            case 'K': x_load_accu (cpu.reg_k); break;
            case 'L': x_load_accu (cpu.reg_l); break;
            case 'M': x_load_accu (cpu.reg_m); break;
            case 'N': x_load_accu (cpu.reg_n); break;

            case 's': x_push (S); break;
            case 'r': x_store(cpu.accu, R+RP); RP+=4; break;

            case 'S': x_pop (S); break;
            case 'R': cpu.accu = RP-=4; x_load(R+RP); break;
            case 'D': x_drop (S); break;

            case '.': x_print (cpu.accu); break;

            case '+': x_add (S) ; break;
            case '-': x_sub (S) ; break;

            case 'A': x_and (S) ; break;
            case 'O': x_or  (S) ; break;
            case 'X': x_xor (S) ; break;
            case '~': x_not ; break;

            case '!': x_store(cpu.accu, cpu.reg_m +cpu.reg_i); break;
            case '@': cpu.accu = x_load(cpu.reg_m + cpu.reg_i); break;

            case '_': x_clear; break;

            case '[': x_while(cpu.accu==0); break;
            case ']': x_wend(cpu.accu!=0); break;

            case 'Z': x_while(cpu.accu==0); break; // break_on_zero
            case 'C': x_wend(cpu.accu==0); break;  // continue_on_zero

            case '>': x_rshift; break;
            case '<': x_lshift; break;

            case 'B': print_status(bc,result);
                      printf("%s", result);
                      scanf("%c",&ch);
            case ' ':
            case '\t':
            case '\n':
                    // nop, skip
                    break;
            default:
              printf("unknown bytecode: %04x\n", bc);
              exit(1);
              break;
        }
        if(pcadd) pc++;
        else      pcadd=1;
    }

    bc = *(program+pc);
    if(debug) print_status(bc, result);

    return result;
}
void intro() {

  printf("                                  :|   ++     \n");
  printf("                        :\\:| .::\\ ::'| :|     \n");
  printf("                        `::| `::| :::| :|     \n");
  printf("                        .,:'                  \n");
  printf(" \n");
  printf("        -----   yet another bytecode interpreter   -----\n");
}

void usage() {
    char *s =
    "-f <file>     : read input from file and execute\n" \
    "-x <bytecode> : execute bytecode\n" \
    "-c <string>   : check if result equals string\n" \
    "\n" \
    "bytecodes:\n" \
    "----------\n" \
    "          _ : clear accu with zero \n" \
    "    0-9,a-f : shift accu 4 bits left and set lower nibble of last significant byte to given number\n" \
    "          p : jump to address in accu (load register p with accu)\n" \
    "          P : load register p to accu \n" \

    "          s : push the accu on the S-stack \n" \
    "          S : pop from S-stack into accu \n" \

    "          D : drop S-stack \n" \
    "        r,R : push and pop for the R-stack, but store/fetch 4 bytes (address) \n" \
    "i,j,k,l,m,n : store accu in register (j,k,l,...) \n" \
    "I,J,K,L,M,N : load accu with register (J,K,L,...) \n" \

    "          A : bitwise 'and' of accu and top of stack, result stored in accu\n" \

    "          O : bitwise 'or' of accu and top of stack, result stored in accu\n" \
    "          X : bitwise 'xor' of accu and top of stack, result stored in accu\n" \
    "          ~ : bitwise 'not' of accu, stored in accu\n" \
    "      [0000 : while loop is entered if accu is not zero; \n" \
    "              relative jump address is patched automatically\n" \
    "      ]0000 : end of while loop, jumps to begin of while loop if accu is not zero; \n" \
    "              relative jump offset is patched automatically\n" \
    "      Z0000 : break on zero - jumps to end of while loop if accu is zero;\n" \
    "              relative jump offset is patched automatically\n" \
    "      C0000 : continue on zero - jumps to begin of while loop if accu is zero;\n" \
    "              relative jump offset is patched automatically\n" \
    "          B : breakpoint - stop execution and read char from keyboard before execution continues \n" \
    "          H : halt - stop execution and ends program\n" \
    "\n" \
    "          . : write the accu value as ascii character to stdout\n" \
    "          ? : read a char from stdin into accu\n" \
    "\n" \
    "          ! : store the accu byte value (4 bytes) to the memory address reg_m + reg_i (m and i register)\n" \
    "          @ : fetch the value from memory adress reg_m + reg_i into accu (4 bytes)\n" \
    "\n" \
    "\n" \
    "\n" \
    "";

    printf("%s",s);
}
