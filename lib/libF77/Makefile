
LIB=F77
MISC =	main.c s_rnge.c abort_.c getarg_.c iargc_.c signal_.c s_stop.c s_paus.c
POW =	pow_ci.c pow_dd.c pow_di.c pow_hh.c pow_ii.c  pow_ri.c pow_zi.c pow_zz.c
CX =	c_abs.c c_cos.c c_div.c c_exp.c c_log.c c_sin.c c_sqrt.c
DCX =	z_abs.c z_cos.c z_div.c z_exp.c z_log.c z_sin.c z_sqrt.c
REAL =	r_abs.c r_acos.c r_asin.c r_atan.c r_atn2.c r_cnjg.c r_cos.c \
	r_cosh.c r_dim.c r_exp.c r_imag.c r_int.c \
	r_lg10.c r_log.c r_mod.c r_nint.c r_sign.c \
	r_sin.c r_sinh.c r_sqrt.c r_tan.c r_tanh.c
DBL =	d_abs.c d_acos.c d_asin.c d_atan.c d_atn2.c \
	d_cnjg.c d_cos.c d_cosh.c d_dim.c d_exp.c \
	d_imag.c d_int.c d_lg10.c d_log.c d_mod.c \
	d_nint.c d_prod.c d_sign.c d_sin.c d_sinh.c \
	d_sqrt.c d_tan.c d_tanh.c 
INT =	i_abs.c i_dim.c i_dnnt.c i_indx.c i_len.c i_mod.c i_nint.c i_sign.c
HALF =	h_abs.c h_dim.c h_dnnt.c h_indx.c h_len.c h_mod.c  h_nint.c h_sign.c
CMP =	l_ge.c l_gt.c l_le.c l_lt.c hl_ge.c hl_gt.c hl_le.c hl_lt.c
CHAR =	s_cat.c s_cmp.c s_copy.c

SRCS=$(MISC) $(POW) $(CX) $(DCX) $(REAL) $(DBL) $(INT) $(HALF) $(CMP) $(CHAR) cabs.c
CPPFLAGS+=-I${.CURDIR}
NOMAN=

.include <bsd.lib.mk>
