dnl
dnl $KTH: try-cpp-kernel.m4,v 1.1 2000/03/16 10:20:34 assar Exp $
dnl

AC_DEFUN(AC_TRY_CPP_KERNEL,[
save_CPPFLAGS="$CPPFLAGS"
CPPFLAGS="$CPPFLAGS $KERNEL_CPPFLAGS"
AC_TRY_CPP([$1], [$2], [$3])
CPPFLAGS="$save_CPPFLAGS"
])
