#!/bin/sh
# $FreeBSD: src/tools/regression/fstest/tests/link/14.t,v 1.1 2007/01/17 01:42:09 pjd Exp $

desc="link returns EXDEV if the source and the destination files are on different file systems"

n0=`namegen`
n1=`namegen`
n2=`namegen`

expect 0 mkdir ${n0} 0755
dd if=/dev/zero of=tmpdisk bs=1k count=1024 2>/dev/null
vnconfig svnd1 tmpdisk
newfs /dev/rsvnd1c >/dev/null
mount /dev/svnd1c ${n0}
expect 0 create ${n0}/${n1} 0644
expect EXDEV link ${n0}/${n1} ${n2}
expect 0 unlink ${n0}/${n1}
expect 0 create ${n1} 0644
expect EXDEV link ${n1} ${n0}/${n2}
expect 0 unlink ${n1}
umount /dev/svnd1c
vnconfig -u svnd1
rm tmpdisk
expect 0 rmdir ${n0}
