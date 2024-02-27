#include <common.h>

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