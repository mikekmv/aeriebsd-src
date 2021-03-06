
.include <bsd.own.mk>			# for ELF_TOOLCHAIN definition

BINDIR=			/usr/mdec

# For descriptions of regions available to bootstrap programs, see
# section 3.4.1.2 (pp. III 3-14 - III 3-18) of the second edition of
# the Alpha AXP Architecture Reference Manual.

PRIMARY_LOAD_ADDRESS=	20000000	# "Region 1 start"
SECONDARY_LOAD_ADDRESS=	20004000	# "Region 1 start" + 32k
HEAP_LIMIT=		20040000	# "Region 1 start" + 256k

CPPFLAGS+=		-DPRIMARY_LOAD_ADDRESS="0x${PRIMARY_LOAD_ADDRESS}"
CPPFLAGS+=		-DSECONDARY_LOAD_ADDRESS="0x${SECONDARY_LOAD_ADDRESS}"

CPPFLAGS+=		-D_STANDALONE

XCPPFLAGS+=		-DHEAP_LIMIT="0x${HEAP_LIMIT}"

CFLAGS+=		-fno-stack-protector -fno-builtin-printf \
			-fno-builtin-vprintf -fno-builtin-putchar \
			-fno-builtin-puts
