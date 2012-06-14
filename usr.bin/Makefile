
.include <bsd.own.mk>

SUBDIR= apply apropos ar arch asa asn1_compile at aucat audioctl awk \
	banner basename bc bdes bgplg biff \
	cal calendar cap_mkdb cdio checknr chpass cmp col colcrt colrm \
	column comm compile_et compress cpp crontab csplit ctags cut \
	dc deroff diff diff3 dirname du encrypt env eqn expand \
	false file file2c find fgen finger fmt fold from fsplit fstat ftp \
	gencat getcap getconf getent getopt gprof graph grep gzsig \
	head hexdump id indent infocmp ipcrm ipcs join jot \
	kdump keynote ktrace lam last lastcomm ld leave lex lndir \
	locate lock logger login logname look lorder \
	m4 mail make man manhtml mesg mg midiplay mixerctl mkdep mklocale \
	mktemp modstat more msgs \
	nc netstat newsyslog nfsstat nice nm nohup nroff oldrdist \
	pagesize passwd paste patch pcc pctr pkill plot pmdb pr \
	printenv printf \
	quota radioctl rcs rdist rdistd readelf readlink renice rev \
	rpcgen rpcinfo rs rsh rup ruptime rusers rwall rwho \
	sdiff script sed sendbug shar showmount skey \
	skeyaudit skeyinfo skeyinit soelim sort spell spline split \
	ssh stat strings strip su systat sudo \
	tail talk tbl tcopy tcpbench tee telnet tftp tic time tip \
	top touch tput tr troff true tset tsort tty \
	usbhidaction usbhidctl ul uname unexpand unifdef uniq units \
	unvis users uudecode uuencode vacation vgrind vi vis vmstat \
	w wall wc what whatis which who whois write \
	x99token xargs xinstall xlint xstr yacc yes

.if (${YP:L} == "yes")
SUBDIR+=ypcat ypmatch ypwhich
.endif

.include <bsd.subdir.mk>
