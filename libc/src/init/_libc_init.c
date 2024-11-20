#include <stdint.h>

extern uint64_t _stdin;
extern uint64_t _stdout;
extern uint64_t _stderr;

void initialize_standard_library()
{
    _stdin = 0;
    _stdout = 1;
    _stderr = 2;
}
