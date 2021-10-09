
                                  :|   ++     
                        :\:| .::\ ::'| :|     
                        `::| `::| :::| :|     
                        .,:'                  
 
        -----   yet another bytecode interpreter   -----


-f <file>     : read input from file and execute
-x <bytecode> : execute bytecode
-p            : patch jump addresses before execution
-P            : patch-only - patch and return patched program as result
-c <string>   : check if result equals string

bytecodes:
----------
          _ : clear accu with zero 
    0-9,a-f : shift accu 4 bits left and set lower nibble of last significant byte to given number
          p : jump to address in accu (load register p with accu)
          P : load register p to accu 
          s : push the accu on the S-stack 
          S : pop from S-stack into accu 
          D : drop S-stack 
        r,R : push and pop for the R-stack, but store/fetch 4 bytes (address) 
i,j,k,l,m,n : store accu in register (j,k,l,...) 
I,J,K,L,M,N : load accu with register (J,K,L,...) 
          A : bitwise 'and' of accu and top of stack, result stored in accu
          O : bitwise 'or' of accu and top of stack, result stored in accu
          X : bitwise 'xor' of accu and top of stack, result stored in accu
          ~ : bitwise 'not' of accu, stored in accu
      [0000 : while loop is entered if accu is not zero; 
              relative jump address is patched automatically
      ]0000 : end of while loop, jumps to begin of while loop if accu is not zero; 
              relative jump offset is patched automatically
      Z0000 : break on zero - jumps to end of while loop if accu is zero;
              relative jump offset is patched automatically
      C0000 : continue on zero - jumps to begin of while loop if accu is zero;
              relative jump offset is patched automatically
          B : breakpoint - stop execution and read char from keyboard before execution continues 
          H : halt - stop execution and ends program

          . : write the accu value as ascii character to stdout
          ? : read a char from stdin into accu

          ! : store the accu byte value (4 bytes) to the memory address reg_m + reg_i (m and i register)
          @ : fetch the value from memory adress reg_m + reg_i into accu (4 bytes)




