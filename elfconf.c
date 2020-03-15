#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <elf.h>

struct elfargs {
    const char *filename;
    const char *symbol;
    uint64_t value;
};

struct elffile {
    FILE *fp;
    union {
        /*
         * The elf pointer contains the address of the buffer where the entire
         * ELF file is placed in. When freeing all allocated data in this program,
         * this address is the only one that needs to be freed.
         */
        void *elf;
        Elf32_Ehdr *elf32Ehdr;
        Elf64_Ehdr *elf64Ehdr;
    };
    /*
     * The following section header pointers point to the first section header.
     * The pointer can be used to iterate over all section headers like an array.
     */
    union {
        Elf32_Shdr *elf32Shdr;
        Elf64_Shdr *elf64Shdr;
    };
    /*
     * The following symbol table pointers also point to the beginning of the ELFs
     * symbol table, which can be iteratet through like an array.
     */
    union {
        Elf32_Sym *elf32Syms;
        Elf64_Sym *elf64Syms;
    };
    unsigned int numsyms;
    char *strtab;
    char *shstrtab;
};

struct elfhead {
    char *symname;
    union {
        void *sym;
        Elf32_Sym *elf32Sym;
        Elf64_Sym *elf64Sym;
    };
    struct elfhead *next;
};

static struct elffile elffile;

static struct elfargs elfargs;

static struct elfhead *symbols;

/*
 * struct elfhead functions: alloc, search, insert, delete
 */

static struct elfhead *alloc_elfhead(char *symname, void *sym) {
    struct elfhead *elfhead = malloc(sizeof(struct elfhead));

    if (!elfhead)
        return NULL;

    elfhead->symname = symname;
    elfhead->sym = sym;

    /* Initialize list pointers */
    elfhead->next = NULL;

    return elfhead;
}

static struct elfhead *search_elfhead(const char *symbol) {
    struct elfhead *head;

    for (head = symbols; head != NULL; head = head->next) {
        if (strcmp(symbol, head->symname))
            continue;

        return head;
    }

    return NULL;
}

static void insert_elfhead(struct elfhead *elfhead) {
    if (symbols != NULL)
        elfhead->next = symbols;

    symbols = elfhead;
}

static void delete_symbols(void) {
    struct elfhead *head, *next;

    head = symbols;

    while (head) {
        next = head->next;
        free(head);
        head = next;
    }
}

/*
 * Utilities: Sections, symbols
 */

/* Only available after ELF has been read into buffer */

static void *elf_offset(long offset) {
    return elffile.elf + offset;
}

/* Only available after elffile fully initialized */

static Elf32_Shdr *elf32_section(int shndx) {
    if (shndx >= elffile.elf32Ehdr->e_shnum)
        return NULL;

    return &elffile.elf32Shdr[shndx];
}

static Elf64_Shdr *elf64_section(int shndx) {
    if (shndx >= elffile.elf64Ehdr->e_shnum)
        return NULL;

    return &elffile.elf64Shdr[shndx];
}

static char *section_name(int name) {
    return elffile.shstrtab + name;
}

static Elf32_Sym *elf32_symbol(int symndx) {
    if (symndx >= elffile.numsyms)
        return NULL;

    return &elffile.elf32Syms[symndx];
}

static Elf64_Sym *elf64_symbol(int symndx) {
    if (symndx >= elffile.numsyms)
        return NULL;

    return &elffile.elf64Syms[symndx];
}

static char *symbol_name(int name) {
    return elffile.strtab + name;
}

/*
 * Configuring struct elffile
 */

static void setup32_elffile(void) {
    Elf32_Shdr *section;
    void *secaddr;
    int index, numsyms;
    char *symbol;
    struct elfhead *elfsym;

    /* Ignore all other ELF classes. Handle them separately */
    if (elffile.elf32Ehdr->e_ident[EI_CLASS] != ELFCLASS32)
        return;

    /* Assign adress of section headers to member variable */
    elffile.elf32Shdr = elf_offset(elffile.elf32Ehdr->e_shoff);

    /* Assign symbol and string tables sections to elffile */
    for (index = 0; index < elffile.elf32Ehdr->e_shnum; index++) {
        section = elf32_section(index);
        secaddr = elf_offset(section->sh_offset);

        if (section->sh_type == SHT_SYMTAB) {
            elffile.elf32Syms = secaddr;
            elffile.numsyms = section->sh_size / section->sh_entsize;
        } else if (section->sh_type == SHT_STRTAB) {
            if (index == elffile.elf32Ehdr->e_shstrndx)
                elffile.shstrtab = secaddr;
            else
                elffile.strtab = secaddr;
        }
    }

    /* Setup symbol table for lookup */
    for (index = 0; index < elffile.numsyms; index++) {
        symbol = elffile.strtab + elffile.elf32Syms[index].st_name;
        elfsym = alloc_elfhead(symbol, &elffile.elf32Syms[index]);
        if (!elfsym) {
            delete_symbols();
            free(elffile.elf);
            return;
        }

        insert_elfhead(elfsym);
    }
}

static void setup64_elffile(void) {
    Elf64_Shdr *section;
    void *secaddr;
    int index, numsyms;
    char *symbol;
    struct elfhead *elfsym;

    /* Ignore all other ELF classes. Handle them separately */
    if (elffile.elf64Ehdr->e_ident[EI_CLASS] != ELFCLASS64)
        return;

    /* Assign adress of section headers to member variable */
    elffile.elf64Shdr = elf_offset(elffile.elf64Ehdr->e_shoff);

    /* Assign symbol and string tables sections to elffile */
    for (index = 0; index < elffile.elf64Ehdr->e_shnum; index++) {
        section = &elffile.elf64Shdr[index];
        secaddr = elffile.elf + section->sh_offset;

        if (section->sh_type == SHT_SYMTAB) {
            elffile.elf64Syms = secaddr;
            elffile.numsyms = section->sh_size / section->sh_entsize;
        } else if (section->sh_type == SHT_STRTAB) {
            if (index == elffile.elf64Ehdr->e_shstrndx)
                elffile.shstrtab = secaddr;
            else
                elffile.strtab = secaddr;
        }
    }

    /* Setup symbol table for lookup */
    for (index = 0; index < elffile.numsyms; index++) {
        symbol = elffile.strtab + elffile.elf32Syms[index].st_name;
        elfsym = alloc_elfhead(symbol, &elffile.elf32Syms[index]);
        if (!elfsym) {
            delete_symbols();
            free(elffile.elf);
            return;
        }

        insert_elfhead(elfsym);
    }
}

static int read_elffile(const char *filename) {
    FILE *fp;
    size_t size;

    fp = fopen(filename, "rb+");

    if (!fp)
        return 1;

    /* Get size of file */
    fseek(fp, 0L, SEEK_END);
    size = ftell(fp);

    elffile.elf = malloc(size);
    if (!elffile.elf) {
        fclose(fp);
        return 1;
    }

    /* Read entire file into buffer */
    fseek(fp, 0L, SEEK_SET);
    if (fread(elffile.elf, 1, size, fp) != size) {
        free(elffile.elf);
        fclose(fp);
        return 1;
    }

    setup32_elffile();
    setup64_elffile();

    elffile.fp = fp;

    return 0;
}

/*
 * Manipulating ELF file
 */

static size_t elf32_symbol_offset(Elf32_Sym *symbol) {
    Elf32_Shdr *section = &elffile.elf32Shdr[symbol->st_shndx];

    return symbol->st_value - section->sh_addr + section->sh_offset;
}

static size_t elf64_symbol_offset(Elf64_Sym *symbol) {
    Elf64_Shdr *section = &elffile.elf64Shdr[symbol->st_shndx];

    return symbol->st_value - section->sh_addr + section->sh_offset;
}

static int manipulate_elf32(struct elfargs *args, struct elfhead *symbol) {
    size_t offset = elf32_symbol_offset(symbol->elf32Sym);

    /* Symbol has to be in this ELF to be manipulated */
    if (symbol->elf32Sym->st_shndx == SHN_UNDEF)
        return 1;

    printf("Symbol located at %lx in ELF (Size: %lu Bytes)\n", offset, symbol->elf32Sym->st_size);

    fseek(elffile.fp, offset, SEEK_SET);

    if (fwrite(&elfargs.value, 1, symbol->elf32Sym->st_size, elffile.fp) != symbol->elf32Sym->st_size)
        return 1;

    return 0;
}

static int manipulate_elf64(struct elfargs *args, struct elfhead *symbol) {
    size_t offset = elf64_symbol_offset(symbol->elf64Sym);

    /* Symbol has to be in this ELF to be manipulated */
    if (symbol->elf64Sym->st_shndx == SHN_UNDEF)
        return 1;

    printf("Symbol located at %lx in ELF (Size: %lu Bytes)\n", offset, symbol->elf64Sym->st_size);

    fseek(elffile.fp, offset, SEEK_SET);

    if (fwrite(&elfargs.value, 1, symbol->elf64Sym->st_size, elffile.fp) != symbol->elf64Sym->st_size)
        return 1;

    return 0;
}

static int manipulate_elf(struct elfargs *args) {
    struct elfhead *symbol = search_elfhead(args->symbol);

    if (!symbol)
        return 1;

    if (elffile.elf32Ehdr->e_ident[EI_CLASS] == ELFCLASS32)
        return manipulate_elf32(args, symbol);
    else
        return manipulate_elf64(args, symbol);
}

/*
 * Argument parser
 */

static void show_elfconf_options(char * const program) {
    printf("Usage: %s {-h | -f <filename> -s <symbol> -v <value>}\n", program);
}

static int read_elfargs(int argc, char * const argv[]) {
    int c, i;
    char *end;

    while((c = getopt(argc, argv, "hf:s:v:")) != -1) {
        switch (c) {
            case 'h':
                show_elfconf_options(argv[0]);
                return 0;
            case 'f':
                elfargs.filename = optarg;
                break;
            case 's':
                elfargs.symbol = optarg;
                break;
            case 'v':
                elfargs.value = strtoull(optarg, &end, 0);
                break;
            case '?':
                if (optopt == 'f' || optopt == 's' || optopt == 'v')
                    printf("Option -%c requires an argument!\n", optopt);
                else if (isprint(optopt))
                    printf("Unknown option '-%c'\n", optopt);
                else
                    printf("Unknown option '\\x%x'\n", optopt);

                return 1;
            default:
                abort();
        }
    }

    for (i = optind; i < argc; i++)
        printf("Non-option argument %s, ignore...\n", argv[i]);

    return 0;
}

int main(int argc, char * const argv[]) {
    int ret;

    /* Parse arguments */
    ret = read_elfargs(argc, argv);
    if (ret)
        return 1;

    /* Load ELF */
    ret = read_elffile(elfargs.filename);
    if (ret)
        return 1;

    ret = manipulate_elf(&elfargs);

    fclose(elffile.fp);
    free(elffile.elf);
    delete_symbols();

    return ret;
}
