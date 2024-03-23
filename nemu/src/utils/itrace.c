#include <common.h>
#include <fcntl.h>
#include <unistd.h>
#include <elf.h>

#define MAX_IRINGBUF 8

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

SymEntry *symbol_tbl = NULL;
int symbol_tbl_size = 0;
int call_depth = 0;

static void read_elf_header(int fd, Elf32_Ehdr * eh) {
    assert(lseek(fd, 0, SEEK_SET) == 0);
    assert(read(fd, (void *)eh, sizeof(Elf32_Ehdr)) == sizeof(Elf32_Ehdr));

    // check Magic Number
    if (strncmp((char*)eh->e_ident, "\177ELF", 4) != 0)
        panic("malformed ELF file");
}

static void read_section_header(int fd, Elf32_Ehdr eh, Elf32_Shdr *sh_tbl) {
    assert(lseek(fd, eh.e_shoff, SEEK_SET) == eh.e_shoff);
    int i;
    for (i = 0; i < eh.e_shnum; i++) {
        assert(read(fd, (void *)&sh_tbl[i], eh.e_shentsize) == eh.e_shentsize);
    }
}

static void read_symbols(int fd, Elf32_Ehdr eh, Elf32_Shdr *sh_tbl) {
    
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
}

void trace_func_call(paddr_t pc, paddr_t target) {
   if (!symbol_tbl) return; 
}

void trace_func_ret(paddr_t pc) {

}