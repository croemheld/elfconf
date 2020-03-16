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
 * Printing functions
 */

static void print_elfconf_info(char *name) {
	printf("Usage: %s {-h | -f <filename> -s <symbol> -v <value>}\n", name);
}

static void print_elfconf_ehdr(char *name, struct elfconf_ehdr *ehdr) {
#ifdef ELFCONF_DEBUG
	printf("elfconf: Information about ELF %s\n", name);

	/* ELF class: 32-bit or 64-bit */
	printf("%-20s %s\n", "ELF object class:", ehdr_class[ehdr->e_ident[EI_CLASS]]);

	/* ELF object type (ET_REL, ET_EXEC, ET_DYN, ET_CORE) */
	if (ehdr->e_type >= ET_LOPROC && ehdr->e_type <= ET_HIPROC)
		printf("%-20s %s\n", "ELF object type:", "Processor-specific file");
	else
		printf("%-20s %s\n", "ELF object type:", ehdr_etype[ehdr->e_type]);

	/* ELF architecture */
	printf("%-20s %s\n", "ELF architecture:", ehdr_earch[ehdr->e_machine]);
#endif
}

/*
 * General ELF helper functions and macros (for both 32-bit and 64-bit)
 */

/* Section relevant macros */
#define elf_section_header(elf, shndx)	(elf)->shdr + shndx
#define elf_section(elf, shndx)			(elf)->head + ((elf)->shdr + shndx)->sh_offset
#define elf_section_name(elf, name)		(elf)->shstrtab + name

/* Symbol relevant macros */
#define elf_symbol(elf, symndx)			(elf)->symtab + symndx
#define elf_symbol_name(elf, sym)		(elf)->strtab + (sym)->st_name

/* Get the offset of a symbol in the ELF binary */
#define elf_symbol_offset(elf, sym)	 ({		\
	typeof((elf)->shdr) __section = elf_section_header(elf, (sym)->st_shndx);	\
	(sym)->st_value - (__section)->sh_addr + (__section)->sh_offset;			\
})

/*
 * Parsing ELF file
 */

static inline void *elf_offset(struct elfconf_arguments *args, unsigned long offset) {
	return args->buf + offset;
}

static void clear_elfconf_file(struct elfconf_arguments *args) {
	if (args->efp)
		fclose(args->efp);

	if (args->buf)
		free(args->buf);
}

/*
 * 32-bit ELF functions
 */

static void *find_elf32_section(struct elfconf_elf32file *elf, char *name, unsigned int *num) {
	Elf32_Shdr *section;
	unsigned int shndx;

	for (shndx = 0; shndx < elf->ehdr->e_shnum; shndx++) {
		section = elf_section_header(elf, shndx);

		if (strcmp(name, elf_section_name(elf, section->sh_name)))
			continue;

		if (num)
			*num = section->sh_size / section->sh_entsize;

		return elf_section(elf, shndx);
	}

	return NULL;
}

static int parse_elf32_file(struct elfconf_arguments *args, struct elfconf_elf32file *elf) {
	/* Initialize pointers to section headers */
	elf->ehdr = elf_offset(args, 0);
	elf->shdr = elf_offset(args, elf->ehdr->e_shoff);

	/* Assign .shstrtab section first */
	elf->shstrtab = elf_section(elf, elf->ehdr->e_shstrndx);

	/* Search for .symtab and .strtab section */
	elf->symtab = find_elf32_section(elf, ELFCONF_SECTION_SYMTAB, &elf->numsyms);
	if (!elf->symtab)
		return -ENAVAIL;

	elf->strtab = find_elf32_section(elf, ELFCONF_SECTION_STRTAB, NULL);
	if (!elf->strtab)
		return -ENAVAIL;

	return 0;
}

static Elf32_Sym *find_elf32_symbol(struct elfconf_elf32file *elf, char *name) {
	Elf32_Sym *symbol;
	unsigned int index;

	for(index = 0; index < elf->numsyms; index++) {
		symbol = elf_symbol(elf, index);
		if (strcmp(name, elf_symbol_name(elf, symbol)))
			continue;

		return symbol;
	}

	return NULL;
}

static int configure_elf32_symbol(struct elfconf_arguments *args, struct elfconf_elf32file *elf) {
	Elf32_Sym *symbol;
	size_t offset, write;

	symbol = find_elf32_symbol(elf, args->sym);
	if (!symbol)
		return -ENAVAIL;

	/* Move pointer to the symbol in the ELF file */
	offset = elf_symbol_offset(elf, symbol);
	fseek(args->efp, offset, SEEK_SET);

	/* Write new value at the specified symbol */
	write = fwrite(&args->val, 1, symbol->st_size, args->efp);
	if (write != symbol->st_size)
		return -EBADFD;

	return 0;
}

static int apply_elf32_args(struct elfconf_arguments *args) {
	struct elfconf_elf32file elf;

	/* Fill up data structure */
	if (parse_elf32_file(args, &elf))
		return -EFAULT;

	/* Search and modify symbol */
	if (configure_elf32_symbol(args, &elf))
		return -EFAULT;

	return 0;
}

/*
 * 64-bit ELF functions
 */

static void *find_elf64_section(struct elfconf_elf64file *elf, char *name, unsigned int *num) {
	Elf64_Shdr *section;
	unsigned int shndx;

	for (shndx = 0; shndx < elf->ehdr->e_shnum; shndx++) {
		section = elf_section_header(elf, shndx);

		if (strcmp(name, elf_section_name(elf, section->sh_name)))
			continue;

		if (num)
			*num = section->sh_size / section->sh_entsize;

		return elf_section(elf, shndx);
	}

	return NULL;
}

static int parse_elf64_file(struct elfconf_arguments *args, struct elfconf_elf64file *elf) {
	/* Initialize pointers to section headers */
	elf->ehdr = elf_offset(args, 0);
	elf->shdr = elf_offset(args, elf->ehdr->e_shoff);

	/* Assign .shstrtab section first */
	elf->shstrtab = elf_section(elf, elf->ehdr->e_shstrndx);

	/* Search for .symtab and .strtab section */
	elf->symtab = find_elf64_section(elf, ELFCONF_SECTION_SYMTAB, &elf->numsyms);
	if (!elf->symtab)
		return -ENAVAIL;

	elf->strtab = find_elf64_section(elf, ELFCONF_SECTION_STRTAB, NULL);
	if (!elf->strtab)
		return -ENAVAIL;

	return 0;
}

static Elf64_Sym *find_elf64_symbol(struct elfconf_elf64file *elf, char *name) {
	Elf64_Sym *symbol;
	unsigned int index;

	for(index = 0; index < elf->numsyms; index++) {
		symbol = elf_symbol(elf, index);
		if (strcmp(name, elf_symbol_name(elf, symbol)))
			continue;

		return symbol;
	}

	return NULL;
}

static int configure_elf64_symbol(struct elfconf_arguments *args, struct elfconf_elf64file *elf) {
	Elf64_Sym *symbol;
	size_t offset, write;

	symbol = find_elf64_symbol(elf, args->sym);
	if (!symbol)
		return -ENAVAIL;

	/* Move pointer to the symbol in the ELF file */
	offset = elf_symbol_offset(elf, symbol);
	fseek(args->efp, offset, SEEK_SET);

	/* Write new value at the specified symbol */
	write = fwrite(&args->val, 1, symbol->st_size, args->efp);
	if (write != symbol->st_size)
		return -EBADFD;

	return 0;
}

static int apply_elf64_args(struct elfconf_arguments *args) {
	struct elfconf_elf64file elf;

	/* Fill up data structure */
	if (parse_elf64_file(args, &elf))
		return -EFAULT;

	/* Search and modify symbol */
	if (configure_elf64_symbol(args, &elf))
		return -EFAULT;

	return 0;
}

static int parse_elfconf_file(struct elfconf_arguments *args) {
	struct elfconf_ehdr *ehdr = args->buf;

	/* Validate ELF signature at beginning of file */
	if (memcmp(ehdr->e_ident, ELFMAG, SELFMAG))
		return -ENOTSUP;

	print_elfconf_ehdr(args->elf, ehdr);

	/*
	 * Determine ELF class: 32-bit or 64-bit
	 */

	if (ehdr->e_ident[EI_CLASS] == ELFCLASS32)
		return apply_elf32_args(args);

	if (ehdr->e_ident[EI_CLASS] == ELFCLASS64)
		return apply_elf64_args(args);

	return -ENOTSUP;
}

static int apply_elfconf_args(struct elfconf_arguments *args) {
	size_t size, read;

	/* Open ELF file */
	args->efp = fopen(args->elf, "rb+");
	if (!args->efp)
		return -EBADFD;

	/* Get ELF size */
	fseek(args->efp, 0L, SEEK_END);
	size = ftell(args->efp);

	/* Alloc buffer and read ELF */
	args->buf = malloc(size);
	if (!args->buf) {
		clear_elfconf_file(args);
		return -ENOMEM;
	}

	fseek(args->efp, 0L, SEEK_SET);
	read = fread(args->buf, 1, size, args->efp);
	if (read != size) {
		clear_elfconf_file(args);
		return -EBADFD;
	}

	if (parse_elfconf_file(args)) {
		clear_elfconf_file(args);
		return -EFAULT;
	}

	return 0;
}

/*
 * Parsing arguments
 */

static int parse_elfconf_args(int argc, char *argv[], struct elfconf_arguments *args)
{
	int option;

	/*
	 * Options supported by elfconf:
	 *
	 * -h: Show usage of this program and quit.
	 *
	 * The following options need to be specified together:
	 *
	 * -f: ELF input file to be manipulated.
	 * -s: Symbol name in ELF which we want to modify.
	 * -v: The value that should be written to the symbol.
	 */

	while ((option = getopt(argc, argv, "hf:s:v:")) != -1) {
		switch (option) {
			case 'h':
				print_elfconf_info(argv[0]);
				return 1;
			case 'f':
				args->elf = optarg;
				break;
			case 's':
				args->sym = optarg;
				break;
			case 'v':
				args->val = strtoull(optarg, NULL, 0);
				break;
			case '?':
				return -EFAULT;
			default:
				abort();
		}
	}

	return 0;
}

int main(int argc, char *argv[]) {
	struct elfconf_arguments args;

	if (parse_elfconf_args(argc, argv, &args))
		return -EFAULT;

	if (apply_elfconf_args(&args))
		return -EFAULT;

	clear_elfconf_file(&args);

	return 0;
}
