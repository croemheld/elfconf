#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <elf.h>

/*
 * Special macros
 */

#define ELFCONF_SECTION_SYMTAB  ".symtab"
#define ELFCONF_SECTION_STRTAB  ".strtab"

/*
 * Structures and typedefs
 */

struct elfconf_arguments {
	/*
	 * Arguments from command line
	 */
	char *elf;
	char *sym;
	unsigned long val;
	/*
	 * FILE pointer and ELF buffer
	 */
	FILE *efp;
	void *buf;
};

struct elfconf_ehdr {
	unsigned char e_ident[EI_NIDENT];
	Elf32_Half e_type;
	Elf32_Half e_machine;
	Elf32_Word e_version;
};

struct elfconf_elf32file {
	union {
		void *head;
		Elf32_Ehdr *ehdr;
	};
	Elf32_Shdr *shdr;
	Elf32_Sym *symtab;
	unsigned int numsyms;
	char *strtab;
	char *shstrtab;
};

struct elfconf_elf64file {
	union {
		void *head;
		Elf64_Ehdr *ehdr;
	};
	Elf64_Shdr *shdr;
	Elf64_Sym *symtab;
	unsigned int numsyms;
	char *strtab;
	char *shstrtab;
};

/*
 * ELF header information
 */

#ifdef ELFCONF_DEBUG
static const char *ehdr_class[] = {
	"(invalid)",
	"32-bit",
	"64-bit"
};

static const char *ehdr_etype[] = {
	"No file type",
	"Relocatable file",
	"Executable file",
	"Shared object file",
	"Core file",
};

static const char *ehdr_earch[] = {
	[EM_NONE] = "No machine",
	[EM_M32] = "AT&T WE 32100",
	[EM_SPARC] = "SUN SPARC",
	[EM_386] = "Intel 80386",
	[EM_68K] = "Motorola m68k family",
	[EM_88K] = "Motorola m88k family",
	[EM_IAMCU] = "Intel MCU",
	[EM_860] = "Intel 80860",
	[EM_MIPS] = "MIPS R3000 big-endian",
	[EM_S370] = "IBM System/370",
	[EM_MIPS_RS3_LE] = "MIPS R3000 little-endian",
	/* reserved 11-14 */
	[EM_PARISC] = "HPPA",
	/* reserved 16 */
	[EM_VPP500] = "Fujitsu VPP500",
	[EM_SPARC32PLUS] = "Sun's v8plus",
	[EM_960] = "Intel 80960",
	[EM_PPC] = "PowerPC",
	[EM_PPC64] = "PowerPC 64-bit",
	[EM_S390] = "IBM S390",
	[EM_SPU] = "IBM SPU/SPC",
	/* reserved 24-35 */
	[EM_V800] = "NEC V800 series",
	[EM_FR20] = "Fujitsu FR20",
	[EM_RH32] = "TRW RH-32",
	[EM_RCE] = "Motorola RCE",
	[EM_ARM] = "ARM",
	[EM_FAKE_ALPHA] = "Digital Alpha",
	[EM_SH] = "Hitachi SH",
	[EM_SPARCV9] = "SPARC v9 64-bit",
	[EM_TRICORE] = "Siemens Tricore",
	[EM_ARC] = "Argonaut RISC Core",
	[EM_H8_300] = "Hitachi H8/300",
	[EM_H8_300H] = "Hitachi H8/300H",
	[EM_H8S] = "Hitachi H8S",
	[EM_H8_500] = "Hitachi H8/500",
	[EM_IA_64] = "Intel Merced",
	[EM_MIPS_X] = "Stanford MIPS-X",
	[EM_COLDFIRE] = "Motorola Coldfire",
	[EM_68HC12] = "Motorola M68HC12",
	[EM_MMA] = "Fujitsu MMA Multimedia Accelerator",
	[EM_PCP] = "Siemens PCP",
	[EM_NCPU] = "Sony nCPU embeeded RISC",
	[EM_NDR1] = "Denso NDR1 microprocessor",
	[EM_STARCORE] = "Motorola Start*Core processor",
	[EM_ME16] = "Toyota ME16 processor",
	[EM_ST100] = "STMicroelectronic ST100 processor",
	[EM_TINYJ] = "Advanced Logic Corp. Tinyj emb.fam",
	[EM_X86_64] = "AMD x86-64 architecture",
	[EM_PDSP] = "Sony DSP Processor",
	[EM_PDP10] = "Digital PDP-10",
	[EM_PDP11] = "Digital PDP-11",
	[EM_FX66] = "Siemens FX66 microcontroller",
	[EM_ST9PLUS] = "STMicroelectronics ST9+ 8/16 mc",
	[EM_ST7] = "STmicroelectronics ST7 8 bit mc",
	[EM_68HC16] = "Motorola MC68HC16 microcontroller",
	[EM_68HC11] = "Motorola MC68HC11 microcontroller",
	[EM_68HC08] = "Motorola MC68HC08 microcontroller",
	[EM_68HC05] = "Motorola MC68HC05 microcontroller",
	[EM_SVX] = "Silicon Graphics SVx",
	[EM_ST19] = "STMicroelectronics ST19 8 bit mc",
	[EM_VAX] = "Digital VAX",
	[EM_CRIS] = "Axis Communications 32-bit emb.proc",
	[EM_JAVELIN] = "Infineon Technologies 32-bit emb.proc",
	[EM_FIREPATH] = "Element 14 64-bit DSP Processor",
	[EM_ZSP] = "LSI Logic 16-bit DSP Processor",
	[EM_MMIX] = "Donald Knuth's educational 64-bit proc",
	[EM_HUANY] = "Harvard University machine-independent object files",
	[EM_PRISM] = "SiTera Prism",
	[EM_AVR] = "Atmel AVR 8-bit microcontroller",
	[EM_FR30] = "Fujitsu FR30",
	[EM_D10V] = "Mitsubishi D10V",
	[EM_D30V] = "Mitsubishi D30V",
	[EM_V850] = "NEC v850",
	[EM_M32R] = "Mitsubishi M32R",
	[EM_MN10300] = "Matsushita MN10300",
	[EM_MN10200] = "Matsushita MN10200",
	[EM_PJ] = "picoJava",
	[EM_OPENRISC] = "OpenRISC 32-bit embedded processor",
	[EM_ARC_COMPACT] = "ARC International ARCompact",
	[EM_XTENSA] = "Tensilica Xtensa Architecture",
	[EM_VIDEOCORE] = "Alphamosaic VideoCore",
	[EM_TMM_GPP] = "Thompson Multimedia General Purpose Proc",
	[EM_NS32K] = "National Semi. 32000",
	[EM_TPC] = "Tenor Network TPC",
	[EM_SNP1K] = "Trebia SNP 1000",
	[EM_ST200] = "STMicroelectronics ST200",
	[EM_IP2K] = "Ubicom IP2xxx",
	[EM_MAX] = "MAX processor",
	[EM_CR] = "National Semi. CompactRISC",
	[EM_F2MC16] = "Fujitsu F2MC16",
	[EM_MSP430] = "Texas Instruments msp430",
	[EM_BLACKFIN] = "Analog Devices Blackfin DSP",
	[EM_SE_C33] = "Seiko Epson S1C33 family",
	[EM_SEP] = "Sharp embedded microprocessor",
	[EM_ARCA] = "Arca RISC",
	[EM_UNICORE] = "PKU-Unity & MPRC Peking Uni. mc series",
	[EM_EXCESS] = "eXcess configurable cpu",
	[EM_DXP] = "Icera Semi. Deep Execution Processor",
	[EM_ALTERA_NIOS2] = "Altera Nios II",
	[EM_CRX] = "National Semi. CompactRISC CRX",
	[EM_XGATE] = "Motorola XGATE",
	[EM_C166] = "Infineon C16x/XC16x",
	[EM_M16C] = "Renesas M16C",
	[EM_DSPIC30F] = "Microchip Technology dsPIC30F",
	[EM_CE] = "Freescale Communication Engine RISC",
	[EM_M32C] = "Renesas M32C",
	/* reserved 121-130 */
	[EM_TSK3000] = "Altium TSK3000",
	[EM_RS08] = "Freescale RS08",
	[EM_SHARC] = "Analog Devices SHARC family",
	[EM_ECOG2] = "Cyan Technology eCOG2",
	[EM_SCORE7] = "Sunplus S+core7 RISC",
	[EM_DSP24] = "New Japan Radio (NJR) 24-bit DSP",
	[EM_VIDEOCORE3] = "Broadcom VideoCore III",
	[EM_LATTICEMICO32] = "RISC for Lattice FPGA",
	[EM_SE_C17] = "Seiko Epson C17",
	[EM_TI_C6000] = "Texas Instruments TMS320C6000 DSP",
	[EM_TI_C2000] = "Texas Instruments TMS320C2000 DSP",
	[EM_TI_C5500] = "Texas Instruments TMS320C55x DSP",
	[EM_TI_ARP32] = "Texas Instruments App. Specific RISC",
	[EM_TI_PRU] = "Texas Instruments Prog. Realtime Unit",
	/* reserved 145-159 */
	[EM_MMDSP_PLUS] = "STMicroelectronics 64bit VLIW DSP",
	[EM_CYPRESS_M8C] = "Cypress M8C",
	[EM_R32C] = "Renesas R32C",
	[EM_TRIMEDIA] = "NXP Semi. TriMedia",
	[EM_QDSP6] = "QUALCOMM DSP6",
	[EM_8051] = "Intel 8051 and variants",
	[EM_STXP7X] = "STMicroelectronics STxP7x",
	[EM_NDS32] = "Andes Tech. compact code emb. RISC",
	[EM_ECOG1X] = "Cyan Technology eCOG1X",
	[EM_MAXQ30] = "Dallas Semi. MAXQ30 mc",
	[EM_XIMO16] = "New Japan Radio (NJR) 16-bit DSP",
	[EM_MANIK] = "M2000 Reconfigurable RISC",
	[EM_CRAYNV2] = "Cray NV2 vector architecture",
	[EM_RX] = "Renesas RX",
	[EM_METAG] = "Imagination Tech. META",
	[EM_MCST_ELBRUS] = "MCST Elbrus",
	[EM_ECOG16] = "Cyan Technology eCOG16",
	[EM_CR16] = "National Semi. CompactRISC CR16",
	[EM_ETPU] = "Freescale Extended Time Processing Unit",
	[EM_SLE9X] = "Infineon Tech. SLE9X",
	[EM_L10M] = "Intel L10M",
	[EM_K10M] = "Intel K10M",
	/* reserved 182 */
	[EM_AARCH64] = "ARM AARCH64",
	/* reserved 184 */
	[EM_AVR32] = "Amtel 32-bit microprocessor",
	[EM_STM8] = "STMicroelectronics STM8",
	[EM_TILE64] = "Tileta TILE64",
	[EM_TILEPRO] = "Tilera TILEPro",
	[EM_MICROBLAZE] = "Xilinx MicroBlaze",
	[EM_CUDA] = "NVIDIA CUDA",
	[EM_TILEGX] = "Tilera TILE-Gx",
	[EM_CLOUDSHIELD] = "CloudShield",
	[EM_COREA_1ST] = "KIPO-KAIST Core-A 1st gen.",
	[EM_COREA_2ND] = "KIPO-KAIST Core-A 2nd gen.",
	[EM_ARC_COMPACT2] = "Synopsys ARCompact V2",
	[EM_OPEN8] = "Open8 RISC",
	[EM_RL78] = "Renesas RL78",
	[EM_VIDEOCORE5] = "Broadcom VideoCore V",
	[EM_78KOR] = "Renesas 78KOR",
	[EM_56800EX] = "Freescale 56800EX DSC",
	[EM_BA1] = "Beyond BA1",
	[EM_BA2] = "Beyond BA2",
	[EM_XCORE] = "XMOS xCORE",
	[EM_MCHP_PIC] = "Microchip 8-bit PIC(r)",
	/* reserved 205-209 */
	[EM_KM32] = "KM211 KM32",
	[EM_KMX32] = "KM211 KMX32",
	[EM_EMX16] = "KM211 KMX16",
	[EM_EMX8] = "KM211 KMX8",
	[EM_KVARC] = "KM211 KVARC",
	[EM_CDP] = "Paneve CDP",
	[EM_COGE] = "Cognitive Smart Memory Processor",
	[EM_COOL] = "Bluechip CoolEngine",
	[EM_NORC] = "Nanoradio Optimized RISC",
	[EM_CSR_KALIMBA] = "CSR Kalimba",
	[EM_Z80] = "Zilog Z80",
	[EM_VISIUM] = "Controls and Data Services VISIUMcore",
	[EM_FT32] = "FTDI Chip FT32",
	[EM_MOXIE] = "Moxie processor",
	[EM_AMDGPU] = "AMD GPU",
	/* reserved 225-242 */
	[EM_RISCV] = "RISC-V",
	[EM_BPF] = "Linux BPF -- in-kernel virtual machine"
};
#endif

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
