Driver
======

General `lbc` command line format follows simplified g++.

Options:

    `-c`         emit object/bitcode files
    `-o`         output file name
    `-S`         emit assembly/llvm-ir
    `-emit-llvm` emit llvm. Must be combined with `-S` or `-c` flags

1. Input files and options can me mingled: \
    `lbc foo.bas -o foo other.bas`
2. `-o` can only be used with a single generated target \
    cannot specify -o when generating multiple output files
3. Inputs can be converted from one type to another \
   `.ll` > `.bc`, `.o`, `.s` \
   `.bc` > `.ll`, `.o`, `.s` \
   `.o`  >  \
   `.s`  > `.o` 

Examples:
---------
1. Source to executable: \
    `lbc foo.bas` \
    `lbc foo.bar bar.bas` \
    will, if successful, produce executable `a.out`
2. Named output \
    `lbc foo.bas -o hello` \
    `lbc foo.bas bar.bas -o hello` \
    will output executable `hello`
3. Sourced to object files: \
    `lbc foo.bas bar.bas -c` \
    `lbc -c foo.bas bar.bas` \
    will output `foo.o` and `bar.o`
4. Link object files together into an executable \
    `lbc foo.o bar.o` \
    will output `a.out`
5. Convert between different types \
    `lbc -c asm.s` asm > object \
    `lbc -S -emit-llvm bitcode.bc` bitcode > llvm ir
    `lbc -c -emit-llvm ir.ll` llvm ir > bitcode
