
#include <sys/cdefs.h>
#include "md_init.h"
#include "extern.h"

static init_f __CTOR_LIST__[1]
    __attribute__((section(".ctors"))) = { (void *)0 };		/* XXX */
static init_f __DTOR_LIST__[1]
    __attribute__((section(".dtors"))) = { (void *)0 };		/* XXX */

static const int __EH_FRAME_END__[]
__attribute__((unused, mode(SI), section(".eh_frame"), aligned(4))) = { 0 };

MD_SECTION_EPILOGUE(".init");
MD_SECTION_EPILOGUE(".fini");
