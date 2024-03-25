#include <common.h>
#include <fcntl.h>
#include <unistd.h>
#include <elf.h>

#define MAX_IRINGBUF 8

#ifdef CONFIG_ITRACE
typedef struct 
{
    word_t pc;
    uint32_t inst;
} ItraceNode;

ItraceNode iringbuf[MAX_IRINGBUF];
int position = 0;
bool full = false;

void trace_inst(word_t pc, uint32_t inst) {
    iringbuf[position].pc = pc;
    iringbuf[position].inst = inst;
    position = (position + 1) % MAX_IRINGBUF;
    full = full || position == 0;
}

/* display insts in iringbuf */
void display_inst() {
    if (!full && !position) return ;

    int end = position;
    int i = full?position:0;

    void disassemble(char *str, int size, uint64_t pc, uint8_t *code, int nbyte);
    char buf[128];
    char *p;
    do {
        p = buf;
        p += snprintf(buf, sizeof(buf),"%s" FMT_WORD ": %08x ", (i+1)%MAX_IRINGBUF==end?" --> ":"     ", iringbuf[i].pc, iringbuf[i].inst);
        disassemble(p, buf+sizeof(buf)-p, iringbuf[i].pc, (uint8_t *)&iringbuf[i].inst, 4);

        if ((i+1)%MAX_IRINGBUF==end) printf(ANSI_FG_RED);
        puts(buf);
    } while (((i = (i+1)%MAX_IRINGBUF) != end));
    puts(ANSI_NONE);
}

/* parse the ELF file */
typedef struct {
    char name[32];
    paddr_t addr;
    unsigned char info;
    Elf32_Sword size;
} SymEntry;

typedef struct tail_rec_node {
    paddr_t pc;
    int depth;
    struct tail_rec_node *next;
} TailRecNode;

SymEntry *symbol_tbl = NULL;
int symbol_tbl_size = 0;
int call_depth = 0;
TailRecNode *tail_rec_head = NULL;

static void read_elf_header(int fd, Elf32_Ehdr * eh) {
    assert(lseek(fd, 0, SEEK_SET) == 0);
    assert(read(fd, (void *)eh, sizeof(Elf32_Ehdr)) == sizeof(Elf32_Ehdr));

    // check Magic Number
    if (strncmp((char*)eh->e_ident, ELFMAG, 4) != 0)
        panic("malformed ELF file");
}

static void read_section_header(int fd, Elf32_Ehdr eh, Elf32_Shdr *sh_tbl) {
    assert(lseek(fd, eh.e_shoff, SEEK_SET) == eh.e_shoff);
    int i;
    for (i = 0; i < eh.e_shnum; i++) {
        assert(read(fd, (void *)&sh_tbl[i], eh.e_shentsize) == eh.e_shentsize);
    }
}

static void read_section(int fd, Elf32_Shdr sh, void *dst) {
    assert(dst != NULL);
    assert(lseek(fd, (off_t)sh.sh_offset, SEEK_SET) == (off_t)sh.sh_offset);
    assert(read(fd, dst, sh.sh_size) == sh.sh_size);
}

static void read_symbol_table(int fd, Elf32_Ehdr eh, Elf32_Shdr *sh_tbl, int sym_idx) {
    Elf32_Sym sym_tbl[sh_tbl[sym_idx].sh_size];
    read_section(fd, sh_tbl[sym_idx], sym_tbl);

    int str_idx = sh_tbl[sym_idx].sh_link;
    char str_tbl[sh_tbl[str_idx].sh_size];
    read_section(fd, sh_tbl[str_idx], str_tbl);

    int sym_tbl_size = (sh_tbl[sym_idx].sh_size / sizeof(Elf32_Sym));
    symbol_tbl = malloc(sizeof(SymEntry) * sym_tbl_size);

    int i;
    for (i = 0; i < symbol_tbl_size; i++) {
        symbol_tbl[i].addr = sym_tbl[i].st_value;
        symbol_tbl[i].info = sym_tbl[i].st_info;
        symbol_tbl[i].size = sym_tbl[i].st_size;
        memset(symbol_tbl[i].name, 0, 32);
        strncpy(symbol_tbl[i].name, str_tbl + sym_tbl[i].st_name, 31);
    }
}

static void read_symbols(int fd, Elf32_Ehdr eh, Elf32_Shdr sh_tbl[]) {
    int i;
    for (i = 0; i < eh.e_shnum; i++) {
        switch (sh_tbl[i].sh_type)
        {
        case SHT_SYMTAB: case SHT_DYNSYM:
            read_symbol_table(fd, eh, sh_tbl, i);
            break;
        }
    }
}

static void init_tail_rec_list() {
	tail_rec_head = (TailRecNode *)malloc(sizeof(TailRecNode));
	tail_rec_head->pc = 0;
	tail_rec_head->next = NULL;
}

void parse_elf(const char *elf_file) {
    if (elf_file == NULL) return;
    log_write("specified ELF file: %s", elf_file);
    int fd = open(elf_file, O_RDONLY|O_SYNC);
    Assert(fd >= 0, "Error %d: unable to open %s\n", fd, elf_file);

    Elf32_Ehdr eh;
    read_elf_header(fd, &eh);

    Elf32_Shdr sh_tbl[eh.e_shnum];
    read_section_header(fd, eh, sh_tbl);

    read_symbols(fd, eh, sh_tbl);

    init_tail_rec_list();

    close(fd);
}

static int find_symbol_func(paddr_t target, bool is_call) {
    int i;
    for (i = 0; i < symbol_tbl_size; i++) {
        if (ELF32_ST_TYPE(symbol_tbl[i].info) == STT_FUNC) {
            if (is_call) {
                if (symbol_tbl[i].addr == target) break;
            } else {
                if (symbol_tbl[i].addr <= target && target < symbol_tbl[i].addr + symbol_tbl[i].size) break;
            }
        }
    }
    return i<symbol_tbl_size?i:-1;
}

static void insert_tail_rec(paddr_t pc, int depth) {
	TailRecNode *node = (TailRecNode *)malloc(sizeof(TailRecNode));
	node->pc = pc;
	node->depth = depth;
	node->next = tail_rec_head->next;
	tail_rec_head->next = node;
}

static void remove_tail_rec() {
	TailRecNode *node = tail_rec_head->next;
	tail_rec_head->next = node->next;
	free(node);
}

void trace_func_call(paddr_t pc, paddr_t target, bool is_tail) {
	if (symbol_tbl == NULL) return;

	++call_depth;

	if (call_depth <= 2) return; // ignore _trm_init & main

	int i = find_symbol_func(target, true);
	log_write(FMT_PADDR ": %*scall [%s@" FMT_PADDR "]\n",
		pc,
		(call_depth-3)*2, "",
		i>=0?symbol_tbl[i].name:"???",
		target
	);

	if (is_tail) {
		insert_tail_rec(pc, call_depth-1);
	}
}

void trace_func_ret(paddr_t pc) {
	if (symbol_tbl == NULL) return;
	
	if (call_depth <= 2) return; // ignore _trm_init & main

	int i = find_symbol_func(pc, false);
	log_write(FMT_PADDR ": %*sret [%s]\n",
		pc,
		(call_depth-3)*2, "",
		i>=0?symbol_tbl[i].name:"???"
	);
	
	--call_depth;

	TailRecNode *node = tail_rec_head->next;
	if (node != NULL) {
		if (node->depth == call_depth) {
			paddr_t ret_target = node->pc;
			remove_tail_rec();
			trace_func_ret(ret_target);
		}
	}
}
#endif