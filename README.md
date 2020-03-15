# elfconf

### Tool for manipulating ELF binaries

The elfconf tool was originally designed to patch ELF files from the [elfboot](https://github.com/croemheld/elfboot) project. The goal was to write the sectors from each stage into the ELF to avoid having to search for each stage in assembly.

**elfconf** is a simple CLI tool which can be used as follows:

```
Usage: elfconf {-h | -f <filename> -s <symbol> -v <value>}
```
The value written at the given symbol depends on the symbol size. Symbols in assembly have to specifically use the `.size` directive (GNU AS) in order to write the correct amount of bytes.

### Examples

For the program **global.c**:

```
#include <stdio.h>

static int var = 5;

int main(void) {
        printf("Value of var: %d\n", var);
        return 0;
}

```
the executable **global.o** will return:
```
 $ ./global.o
Value of var: 5
```
By using **elfconf**, we want to set the value of the variable `var` to 1337:
```
$ elfconf -f global.o -s var -v 1337
```
Subsequently, the program will now return:
```
 $ ./global.o
Value of var: 1337
```
> Note: It is not possible to set variables in the `.bss` section of an ELF object, as this section is zeroed out when it is loaded into memory.

### Future work

The following features might be added in the near future:

* Specify size to write in arguments
* Modify values via binary operators, e.g. `OR`, `AND`, `XOR`, ...
* ... TBD