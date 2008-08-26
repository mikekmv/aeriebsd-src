#
# csh login file

set tterm='?'$TERM
set noglob
onintr finish
eval `tset -s -Q $tterm`
finish:
unset noglob
unset tterm
onintr

if ( `logname` == `whoami` ) then
	echo "Read the afterboot(8) man page for administration advice."
endif
