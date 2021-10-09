#!/c/Users/ninjakuza/tcc/tcc.exe -run

/* yet another bytecode interpreter */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void intro();
void usage();

typedef struct {
  int pc;
  int pcadd; // flag: if 0 then the pc is not incremented for one turn
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
  int debug;
  int patch;
  int patchadd;
  char *memory;
  int progstart;
  int proglen;
  int memsize;
  char *result;
} CPU;

typedef struct {
  char *bc;
  char *check;
  char *memory;
  int   memsize;
  int   debug;
  int   patch;
  CPU  *cpu;
} BCM; // ByteCodeMachine

int x_num(char char_digit, int val);
int get_memory(CPU *cpu, int index);

BCM *run(char *prog, int patch, int debug, int mem_size, int s_stack_address, int r_stack_address, int prog_start);

void patchJumpAddresses(CPU *cpu) {
    // patch jump addresses
    char addrstr[10];
    int dest = 0;
    int n=5;

    if(cpu->debug) printf("patchJumpAdresses");
    for(int i=cpu->progstart; i<cpu->progstart+cpu->proglen; i++) {
        if(cpu->debug) printf("%c",cpu->memory[i]);
        if(get_memory(cpu, i)=='[') {
            dest=0;
            for(int k=1;k<5;k++) {
              dest = x_num(get_memory(cpu, i+k), dest);
              if(cpu->debug) printf("%c",get_memory(cpu, i+k));
            }
            if(dest == 0) { //not patched yet
              int c=0; n=5;
              while(!(get_memory(cpu, i+n)==']' && c==0)) {
                if(get_memory(cpu,i+n)==']') c--;
                else if(get_memory(cpu,i+n)=='[') c++;
                n++;
              }
              // Rücksprungadresse gefunden
              dest=n+5;
              sprintf(addrstr,"%04x", dest);
              if(cpu->debug) printf("\n%d destiny:%d\n", i+1, dest);
              strncpy(cpu->memory+i+1, addrstr, 4);
              sprintf(addrstr,"%04x", n-5);
              if(cpu->debug) printf("\n%d start:%d\n", n+i, dest);
              strncpy(cpu->memory+i+n+1, addrstr, 4);
            }
        }
    }
}

int main(int argc, char **argv) {
  int retval = 0;
  int skip=0;

  int patch = 0;
  int debug = 0;

  char *bc;
  char *check = NULL;

  // default values
  int s_stack_addr = 5;
  int r_stack_addr = 100;
  int mem_size = 5000;
  int prog_start = 256;
/*
  printf("argc: %03d\n", argc);

  for(int i=0; i<argc; i++) {
      printf("%03d: %s\n",i,argv[i]);
  }
*/

  for(int i=1; i<argc; i++) {
      if(skip>0) {
          skip-=1;
          continue;
      }

      if(strcmp(argv[i],"-u") == 0 || strcmp(argv[i],"--usage") == 0 || strcmp(argv[i],"--help") == 0) {
        usage();
        bc = "";
        break;
      }

      else if(strcmp(argv[i],"-x")==0 || strcmp(argv[i],"--execute") == 0) {
        bc = argv[i+1];
        skip=1;
      }

      else if(strcmp(argv[i],"-c") == 0 || strcmp(argv[i],"--check") == 0) {
        check = argv[i+1];
        skip=1;
      }

      else if(strcmp(argv[i],"-s") == 0 || strcmp(argv[i],"--s-stackaddress") == 0) {
        s_stack_addr = atoi(argv[i+1]);
        skip=1;
      }

      else if(strcmp(argv[i],"-r") ==0  || strcmp(argv[i],"--r-stackaddress") == 0) {
        r_stack_addr = atoi(argv[i+1]);
        skip=1;
      }

      else if(strcmp(argv[i],"-m" ) == 0 || strcmp(argv[i],"--memsize") == 0) {
        mem_size = atoi(argv[i+1]);
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
    BCM *bcm = run(bc, patch, debug, mem_size, s_stack_addr, r_stack_addr, prog_start);
    // char *run(char *prog, int patch, int debug, int mem_size, int s_stack_address, int r_stack_address, int prog_start) {

    CPU *cpu = bcm->cpu;
    printf("%s\n", cpu->result);

    if(check!=NULL) {
      if(strcmp(cpu->result, check)==0) {
        printf("  -- check Ok\n");
      }
      else {
        printf("  -- check ERROR, expected: %s\n", check);
        printf("                   program: %.*s\n", cpu->proglen, cpu->memory + cpu->progstart);
      }
      retval=1;
    }
  }
  return retval;
}

void print_memory(CPU *cpu, int addr, int n, char delim) {
  for(int i=0; i<n; i++) {
    if(i>0 && delim != 0) printf("%c", delim);
     // printf("(%02x/%c)", get_memory(cpu, addr+i), get_memory(cpu, addr+i));
     printf("%c", get_memory(cpu, addr+i));
  }
}

void print_r_stack(CPU *cpu) {
  int i=0;
  while(i<cpu->RP) {
      if(i>0) printf(" ");
      print_memory(cpu, cpu->R+i, 4, ':');
      i+=4;
  }
}

void print_s_stack(CPU *cpu) {
      print_memory(cpu, cpu->S, cpu->SP, ' ');
}

void print_status(CPU *cpu, char byte) {
      printf("pc:%04x bc:%c ac:%04x i:%04x j:%04x k:%04x result:%s S-Stack:",
      cpu->pc, byte, cpu->accu, cpu->reg_i, cpu->reg_j, cpu->reg_k, cpu->result);
      print_s_stack(cpu);
      printf(" R-Stack(%x):",cpu->R);
      print_r_stack(cpu);
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
  if(num >= 0x0a) c = 'a' + num - 10;
  return c;
}

int x_num(char char_digit, int val) {
   int v = ((val << 4) & 0xffff); v |= get_num(char_digit);
   return v;
}

#define x_store_accu(X) { X = cpu->accu; }
#define x_load_accu(X) { cpu->accu = X; }

#define x_print(X){ cpu->result[resp] = (X & 0xff); resp++; cpu->result[resp] = '\0';}

#define x_push(STACK) { cpu->memory[STACK + STACK ## P ++] = cpu->accu; }
#define x_pop(STACK) { cpu->accu = cpu->memory[STACK + -- STACK ## P]; }
#define x_add(STACK) { cpu->accu += cpu->memory[STACK + STACK ## P - 1]; }
#define x_sub(STACK) { cpu->accu -= cpu->memory[STACK + STACK ## P - 1]; }
#define x_clear { cpu->accu = 0; }
#define x_drop(STACK) { -- STACK ## P; }

#define x_and(STACK) {cpu->accu &= cpu->memory[STACK + STACK ## P - 1]; }
#define x_or(STACK)  {cpu->accu |= cpu->memory[STACK + STACK ## P - 1]; }
#define x_xor(STACK) {cpu->accu ^= cpu->memory[STACK + STACK ## P - 1]; }
#define x_not {cpu->accu = (~cpu->accu) & 0xffff; }
#define x_lshift { cpu->accu = cpu->accu << 1; }
#define x_rshift { cpu->accu = cpu->accu >> 1; }

int x_load(CPU *cpu, int addr) {
  int a = 0;
  for(int i=0; i<4;i++) {
    a = x_num(get_memory(cpu, addr + i), a);
  }
  if(cpu->debug) printf("\nx_load:%x\n",a);
  return a;
}

void x_store(CPU *cpu, int val, int addr) {
  char v;
  for(int i=3; i>=0; i--) {
    v = get_char(val & 0x0f);
    if(cpu->debug) printf("x_store %x to %x\n", v, addr+i);
    cpu->memory[addr+i] = v;
    val = val >> 4;
  }
  return;
}

int get_memory(CPU *cpu, int index) {
    if(index < 0 || index >= cpu->memsize) {
        printf("access violation : program index: %d", index);
        exit(1);
    }
    return cpu->memory[index];
}

void x_while(CPU *cpu, int condition) {
    char deststr[10];
    int dest = 0;
    if(condition) {
        dest = 0;
        if(cpu->debug) printf("nibble:");
        for(int i=1;i<5;i++) {
            dest = x_num(get_memory(cpu, cpu->pc+i), dest);
            if(cpu->debug) printf("%c",get_memory(cpu, cpu->pc+i));
        }
        printf("\n");
        if(dest != 0) {
            cpu->pc =cpu->pc + dest;
            if(cpu->debug) printf("goto dest:%02ws",cpu->pc + dest);
        }
        else {
            int c=0; int n=5;
            while(!(get_memory(cpu, cpu->pc+n)==']' && c==0)) {
                if(get_memory(cpu, cpu->pc+n)==']') c--;
                else if(get_memory(cpu, cpu->pc+n)=='[') c++;
                n++;
            }
            // Rücksprungadresse gefunden
            dest=n+5;
            sprintf(deststr,"%04x", dest);
            strncpy(&cpu->memory[cpu->pc+1], deststr, 4);
            cpu->pc = cpu->pc + dest;
        }
    }
    else cpu->pc+=5;
    cpu->pcadd = 0;
}

void x_wend(CPU *cpu, int condition) {
    char deststr[10];
    int dest = 0;
    if(condition) {
        dest = 0;
        if(cpu->debug) printf("nibble:");
        for(int i=1;i<5;i++) {
            dest = x_num(get_memory(cpu, cpu->pc+i), dest);
            if(cpu->debug) printf("%c",get_memory(cpu, cpu->pc+i));
        }
        if(cpu->debug) printf("\n");
        if(dest != 0) {
            cpu->pc = cpu->pc - dest;
            if(cpu->debug) printf("goto dest:%04x\n",cpu->pc+dest);
        }
        else {
            int c=0; int n=1;
            while(!(get_memory(cpu, cpu->pc-n)=='[' && c==0)) {
                if(get_memory(cpu, cpu->pc-n)=='[') c--;
                else if(get_memory(cpu, cpu->pc-n)==']') c++;
                n++;
            }
            // Rücksprungadresse gefunden
            dest = n-5;
            sprintf(deststr,"%04x", dest);
            strncpy(&cpu->memory[cpu->pc+1], deststr, 4);
            cpu->pc = cpu->pc - dest;
            if(cpu->debug) printf("dest:%04x goto:%04x\n", dest, cpu->pc);
        }
    }
    else cpu->pc+=5;

    cpu->pcadd = 0;
}

void init_bcm(BCM *bcm, int prog_size, int mem_size, int s_stack_addr, int r_stack_addr, int prog_start_addr, int debug)  {
  CPU *cpu = bcm->cpu;
  cpu->accu = 0;
  cpu->reg_i = cpu->reg_j = cpu->reg_k = cpu->reg_l = cpu->reg_m = 0;

  cpu->S  = s_stack_addr>=0 ? s_stack_addr : 5;
  cpu->SP = 0;
  cpu->R  = r_stack_addr>=0 ? r_stack_addr : 100;
  cpu->RP = 0;

  bcm->memory = (char *)malloc(mem_size);
  bcm->memsize = mem_size;
  for(int i=0; i<5000; i++) {
      bcm->memory[i] = 0;
  }
  cpu->memory = bcm->memory;
  cpu->memsize = bcm->memsize;
  cpu->proglen = prog_size;
  cpu->progstart = prog_start_addr;

  cpu->result = malloc(sizeof(char)*500);
  cpu->result[0] = '\0';

  cpu->pcadd = 1;
  cpu->pc = cpu->progstart;
  cpu->accu = 0;

  cpu->debug = debug;
}

BCM *run(char *prog, int patch, int debug, int mem_size, int s_stack_address, int r_stack_address, int prog_start) {

    BCM *bcm = (BCM *) malloc(sizeof(BCM));
    CPU *cpu = (CPU *) malloc(sizeof(CPU));
    bcm->cpu = cpu;

    init_bcm(bcm, strlen(prog), mem_size, s_stack_address, r_stack_address, prog_start, debug);

    char bc;
    char ch;
    int resp = 0; // result pointer

    strcpy(cpu->memory+prog_start, prog);

    if(patch) {
        // printf("%d %d\n",strlen(prog), addr);
        patchJumpAddresses(cpu);
        if(patch == 2) { // patch only!
           cpu->result = realloc(cpu->result, strlen(prog)+1);
           sprintf(cpu->result, "%.*s\0", strlen(prog), cpu->memory + prog_start);
           return bcm;
        }
    }
    while( (bc = *(cpu->memory+cpu->pc)) !='H' ) {

        if(cpu->debug) print_status(cpu, bc);

        switch(bc) {
            case '0': case '1': case '2': case '3': case '4': case '5':
            case '6': case '7': case '8': case '9': case 'a': case 'b':
            case 'c': case 'd': case 'e': case 'f':
                cpu->accu = x_num (bc, cpu->accu); break;

            case 'p': x_store_accu (cpu->pc); cpu->pcadd=0; break;
            case 'i': x_store_accu (cpu->reg_i); break;
            case 'j': x_store_accu (cpu->reg_j); break;
            case 'k': x_store_accu (cpu->reg_k); break;
            case 'l': x_store_accu (cpu->reg_l); break;
            case 'm': x_store_accu (cpu->reg_m); break;
            case 'n': x_store_accu (cpu->reg_n); break;

            case 'P': x_load_accu (cpu->pc); break;
            case 'I': x_load_accu (cpu->reg_i); break;
            case 'J': x_load_accu (cpu->reg_j); break;
            case 'K': x_load_accu (cpu->reg_k); break;
            case 'L': x_load_accu (cpu->reg_l); break;
            case 'M': x_load_accu (cpu->reg_m); break;
            case 'N': x_load_accu (cpu->reg_n); break;

            case 's': x_push (cpu->S); break;
            case 'r': x_store(cpu, cpu->accu, cpu->R+cpu->RP); cpu->RP+=4; break;

            case 'S': x_pop (cpu->S); break;
            case 'R': cpu->RP-=4; cpu->accu = x_load(cpu, cpu->R+cpu->RP); break;
            case 'D': x_drop (cpu->S); break;

            case '.': x_print (cpu->accu); break;

            case '+': x_add (cpu->S) ; break;
            case '-': x_sub (cpu->S) ; break;

            case 'A': x_and (cpu->S) ; break;
            case 'O': x_or  (cpu->S) ; break;
            case 'X': x_xor (cpu->S) ; break;
            case '~': x_not ; break;

            case '!': x_store(cpu, cpu->accu, cpu->reg_m +cpu->reg_i); break;
            case '@': cpu->accu = x_load(cpu, cpu->reg_m + cpu->reg_i); break;

            case '_': x_clear; break;

            case '[': x_while(cpu, cpu->accu==0); break;
            case ']': x_wend(cpu, cpu->accu!=0); break;

            case 'Z': x_while(cpu, cpu->accu==0); break; // break_on_zero
            case 'C': x_wend(cpu, cpu->accu==0); break;  // continue_on_zero

            case '>': x_rshift; break;
            case '<': x_lshift; break;

            case 'B': print_status(cpu, bc);
                      printf("%s", cpu->result);
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
        if(cpu->pcadd) cpu->pc++;
        else cpu->pcadd=1;
    }
    bc = *(cpu->memory + cpu->pc);
    if(cpu->debug) print_status(cpu, bc);

    return bcm;
}
void intro() {
  printf("\n");
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
