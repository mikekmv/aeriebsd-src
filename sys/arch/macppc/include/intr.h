
#include <powerpc/intr.h>

#ifndef _LOCORE
void softtty(void);

void openpic_send_ipi(int);
void openpic_set_priority(int, int);
#endif
