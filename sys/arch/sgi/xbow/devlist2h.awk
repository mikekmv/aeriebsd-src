#! /usr/bin/awk -f
#
# Copyright (c) 1995, 1996 Christopher G. Demetriou
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
# 3. All advertising materials mentioning features or use of this software
#    must display the following acknowledgement:
#      This product includes software developed by Christopher G. Demetriou.
# 4. The name of the author may not be used to endorse or promote products
#    derived from this software without specific prior written permission
#
# THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
# IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
# OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
# IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
# NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
# THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
BEGIN {
	nproducts = nvendor_dup = nvendors = 0
	dfile="xbowdevs_data.h"
	hfile="xbowdevs.h"
	version="$ABSD: devlist2h.awk,v 1.1.1.1 2008/08/26 14:41:29 root Exp $"
}
NR == 1 {
	VERSION = $0
	gsub("^version[	 ]", "", VERSION)
	gsub("\\$", "", VERSION)
	gsub("\\$", "", version)

	printf("/*\n") > dfile
	printf(" * THIS FILE AUTOMATICALLY GENERATED.  DO NOT EDIT.\n") \
	    > dfile
	printf(" *\n") > dfile
	printf(" * generated from:\n") > dfile
	printf(" *\t%s\n", VERSION) > dfile
	printf(" * generated by:\n") > dfile
	printf(" *\t%s\n", version) > dfile
	printf(" */\n\n") > dfile

	printf("/*\n") > hfile
	printf(" * THIS FILE AUTOMATICALLY GENERATED.  DO NOT EDIT.\n") \
	    > hfile
	printf(" *\n") > hfile
	printf(" * generated from:\n") > hfile
	printf(" *\t%s\n", VERSION) > hfile
	printf(" * generated by:\n") > hfile
	printf(" *\t%s\n", version) > hfile
	printf(" */\n") > hfile

	next
}
$1 == "vendor" {
	nvendors++

	if ($2 in vendorindex) {
		printf("duplicate vendor name %s\n", $2);
		nvendor_dup++;
	}

	vendorindex[$2] = nvendors;		# record index for this name, for later.
	vendors[nvendors, 1] = $2;		# name
	vendors[nvendors, 2] = $3;		# id
	printf("#define\tXBOW_VENDOR_%s\t%s\n", vendors[nvendors, 1],
	    vendors[nvendors, 2]) > hfile

	next
}
$1 == "product" {
	nproducts++

	products[nproducts, 1] = $2;		# vendor name
	products[nproducts, 2] = $3;		# product id
	products[nproducts, 3] = $4;		# id
	printf("#define\tXBOW_PRODUCT_%s_%s\t%s\t", products[nproducts, 1],
	    products[nproducts, 2], products[nproducts, 3]) > hfile

	i=4; f = 5;

	# comments
	ocomment = oparen = 0
	if (f <= NF) {
		printf("\t/* ") > hfile
		ocomment = 1;
	}
	while (f <= NF) {
		if ($f == "#") {
			printf("(") > hfile
			oparen = 1
			f++
			continue
		}
		if (oparen) {
			printf("%s", $f) > hfile
			if (f < NF)
				printf(" ") > hfile
			f++
			continue
		}
		products[nproducts, i] = $f
		printf("%s", products[nproducts, i]) > hfile
		if (f < NF)
			printf(" ") > hfile
		i++; f++;
	}
	if (oparen)
		printf(")") > hfile
	if (ocomment)
		printf(" */") > hfile
	printf("\n") > hfile

	next
}
{
	if ($0 == "")
		blanklines++
	print $0 > hfile
	if (blanklines < 2)
		print $0 > dfile
}
END {
	# print out the match tables

	printf("\n") > dfile

	if (nvendor_dup > 0)
		exit(1);

	printf("/* Descriptions of known devices. */\n") > dfile

	printf("struct xbow_product {\n") > dfile
	printf("\tuint32_t vendor;\n") > dfile
	printf("\tuint32_t product;\n") > dfile
	printf("\tconst char *productname;\n") > dfile
	printf("};\n\n") > dfile


	printf("static const struct xbow_product xbow_products[] = {\n") \
	    > dfile
	for (i = 1; i <= nproducts; i++) {
		printf("\t{\n") > dfile
		printf("\t    XBOW_VENDOR_%s, XBOW_PRODUCT_%s_%s,\n",
		    products[i, 1], products[i, 1], products[i, 2]) \
		    > dfile

		printf("\t    \"") > dfile
		j = 4;
		needspace = 0;
		while (products[i, j] != "") {
			if (needspace)
				printf(" ") > dfile
			printf("%s", products[i, j]) > dfile
			needspace = 1
			j++
		}
		printf("\",\n") > dfile
		printf("\t},\n") > dfile
	}
	printf("\t{ 0, 0, NULL, }\n") > dfile
	printf("};\n\n") > dfile
}
