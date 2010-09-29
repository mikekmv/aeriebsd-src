#!/usr/bin/awk -f
#
# Copyright (c) 2010 Konrad Merz
# All rights reserved.
#
# Permission to use, copy, modify, and distribute this software for any
# purpose with or without fee is hereby granted, provided that the above
# copyright notice and this permission notice appear in all copies.
#
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
# WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
# ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
# WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
# ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
# OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
#

function quote_string(str) {
	gsub(/</, "\\&lt;", str);
	gsub(/>/, "\\&gt;", str);

	return str;
}

function punkt(str) {
	p = ""
	if (str ~ /^[\.,\!\?;:()\[\]]*$/) {
		p = str
		NF--
	}
	return p;
}

function close_arg() {
	if (DV == 1) {
		DV = 0;
		return "</b>"
	}
	if (SY == 1) {
		SY = 0;
		return "</b>"
	}
	if (LI == 1) {
		LI = 1;
		return "</i>";
	}
}

function search_arg(str) {

	gsub(/"/, "", str);

	if (str ~ /Dv /) {
		DV = 1;
		gsub(/Dv /, "<b> ", str);
	}
	if (str ~ /Sy /) {
		SY = 1;
		gsub(/Sy /, "<b> ", str);
	}
	if (str ~ /Li /) {
		LI = 1;
		gsub(/Li /, "<i> ", str);
	}
	if  (str ~ /No /) {
		cls_str = close_arg();
		sub(/No /, cls_str, str);
	}
	return str;
}

BEGIN {
	rev="$ABSD: manhtml.awk,v 1.3 2010/09/17 13:15:14 kmerz Exp $"
	path = "";

	# We need to track begin, end and order of lists.
	# We will keep track of it with two arrays. One array to keep the
	# order of the various lists and one array to keep the amount of the
	# individual lists.
	TAG_LIST=0
	COLUMN_LIST=0
	LIST_CNT=0

	print "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\""
	print "\"http://www.w3.org/TR/html4/loose.dtd\">"
	print "<html>"
}

END {
	print "<p><hr></p>";
	print "<table width=100% ><tr>";
	print "<td>" OSNAME " " OSREL " Reference Manual </td>";
	print "<td align=center>" date "</td>";
	print "<td align=right>" title "(" section ") </td>";
	print "</tr></table>";
	print "</body>";
	print "</html>";
}

# Formatting
/^\.Aq / {
	sub (/^\.Aq /, "");
	print "&#139;" $0 "&#155";
	next;
}

/^\.Dq / {
	sub (/^\.Dq /, "");
	print "&#147;" $0 "&#148;";
	next;
}

/^\.Sq / {
	sub (/^\.Sq /, "");
	print "&#145" $0 "&#146;";
	next;
}

/^\.Em / {
	sub (/^\.Em /, "");
	print "<em>" $0 "</em>";
	next;
}

# Parenthesis open quote
/^\.Po/ {
	print "(";
	next;
}

# Parenthesis close quote
/^\.Pc/ {
	print ")";
	next;
}

# Parenthesis
/^\.Pq / {
	sub(/^\.Pq /, "");
	str = search_arg($0);
	print "(" str ")";
	close_arg();
	next;
}

# Pathname or file
/^\.Pa / {
	sub(/^\.Pa /, "");
	print "<i>" $0 "</i>";
	next;
}

# Document Tilte
/^\.Dt / {
	sub(/^\.Dt /,"");
	title = $1;
	section = $2;
	next;
}

# Document Date
/\.Dd / {
	sub(/\.Dd \$Mdocdate\:/, "");
	sub(/ \$$/, "");
	date = $0;
	next;
}

# Ogranisation
/^\.Os/ {
	print "<head>";
	print "<style type=\"text/css\">";
	print "body { margin-left: 30px; margin-right: 30px; }";
	print "div.tag_list { margin-left: 30px; }";
	print "</style>";
	print "<title>" title "</title></head>";
	print "<body>";
	print "<table width=100% ><tr>";
	print "<td>" title "(" section ") </td>";
	print "<td align=center>" OSNAME " " OSREL " Refernce Manual </td>";
	print "<td align=right>" title "(" section ") </td>";
	print "</tr></table>";
	next;
}

# Section Header
/^\.Sh / {
	sub(/^\.Sh /,"");
	print "<h2>" $0 "</h2>";
	next;
}

# Comment
/^\.\\\"/ {
	sub(/^\.\\\"/,"");
	print "<!--" $0 " -->";
	next;
}

# Special Comment
/^\\\!\\\"/ {
	sub(/^\\\!\\\"/, "");
	gsub(/--/, "\\-\\-");
	print "<!--" $0 " -->";
	next;
}

# Name
/^\.Nm/ {
	post = punkt($NF);
	if (NF > 1 && name == "") {
		name = $2;
		print "<b>" $2 "</b> - ";
	} else
		print "<b>" name "</b>" post;
	next
}

# Name Description
/^\.Nd / {
	sub(/^\.Nd /, "");
	print $0;
	next;
}

# Subsection
/^\.Ss / {
	sub(/^\.Ss /, "");
	print "<h3>" $0 "</h3>";
	next;
}

# Paragraph Break
/^\.Pp/ {
	print "<p></p>";
	next;
}

# Display-one (literal)
/^\.Dl / || /^\.D1 / {
	sub(/^\.D. /, "");
	print "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<kbd>" $0 "</kbd>";
	next;
}

# Cross Reference
/^\.Xr / {
	post = punkt($NF)
	print "<a href=\"" path "/man/man" $3 "/" $2 ".html\">" $2 "(" $3 ")" "</a>" post;
	next;
}

# Function declaration
/^\.Fd / {
	sub(/^\.Fd /, "");
	str = quote_string($0);
	print "<b><kbd>" str "</kbd></b><br>";
	next;
}

# Function type
/^\.Ft / {
	sub(/^\.Ft /, "");
	str = quote_string($0);
	print "<p></p><i>" str "</i> ";
	next;
}

# Function name
/^\.Fn / {
	sub(/^\.Fn /, "");
	sub(/ \"/, "(");
	sub(/\"$/, ");");
	gsub(/\" \"/, " ,");
	str = quote_string($0);
	print "<i>" str "</i>";
	next;
}

# Function argument
/^\.Fa / {
	sub(/^\.Fa /, "");
	str = quote_string($0);
	print "<i>" str "</i>";
	next;
}

# Begin-display block
/^\.Bd / {
	if ($2 == "-literal") {
		print "<p>";
		print "<pre>";
		literal = 1;
	}
	next;
}

# End display-block
/^\.Ed/ {
	if (literal == 1) {
		literal = 0;
		print "</pre>";
		print "<p>";
	}
	next;
}

# Literal text
/^\.Li / {
	sub(/^\.Li/, "");
	print "<kbd>" $0 "</kbd>";
	next;
}

# Symbolic (traditional English)
/^\.Sy / {
	sub(/^\.Sy/, "");
	print "<b>" $0 "</b>";
	next;
}

# Error p
/^\.Er / {
	printf ("<b>%s</b>", $2);
	print $3;
	next;
}

/^\.Dv / {
	printf "<b>%s</b>", $2;
	print $3
	next;
}

# Begin list
/^\.Bl/ {
	if ($2 == "-tag") {
		TAG_LIST++;
		LISTS[++LIST_CNT] = "TAG_LIST";
		next;
	}
	if ($2 == "-column") {
		print "<p></p>";
		print "<table>";
		COLUMN_LIST++;
		LISTS[++LIST_CNT] = "COLUMN_LIST";
		next;
	}
}

# List item
/^\.It / {
	sub(/^\.It /, "");
	if (TAG_LIST != 0 && LISTS[LIST_CNT] == "TAG_LIST") {
		if ((TAG_LIST%2) == 1) {
			str = search_arg($0);
			print str;
			cls_str = close_arg();
			print cls_str;
			print "<div class=\"tag_list\">";
			TAG_LIST++;
			next;
		}
		if ((TAG_LIST%2) == 0) {
			print "</div>";
			str = search_arg($0);
			print str;
			cls_str = close_arg();
			print cls_str;
			print "<div class=\"tag_list\">";
			next;
		}
	}
	if (COLUMN_LIST != 0 && LISTS[LIST_CNT] == "COLUMN_LIST") {
		# XXX: I can't be serious
		# We will assume for now that if the first tokken is a
		# Sy we deal with the table header.
		if ($1 == "Sy") {
			sub(/Sy/, "");
			split($0, str_array, "\t");
			print "<tr>";
			for (i = 1; i <= length(str_array); i++) {
				print "<td><b>" str_array[i] "</b></td>";
			}
			print "</tr>";
			next;
		}
		printf "<tr>";
		split($0, str_array, "\t");
		for (i = 1; i <= length(str_array); i++) {
			str = search_arg(str_array[i]);
			print "<td>" str "</td>";
			str = close_arg();
		}
		print cls_str;
		print "</tr>";
		next;
	}
	next;
}

# End list
/^\.El/ {
	if (TAG_LIST != 0 && LISTS[LIST_CNT] == "TAG_LIST") {
		print "</div>";
		delete LISTS[LIST_CNT];
		LIST_CNT--;
		# This may look wired, but yes it is correct.
		TAG_LIST--;
		TAG_LIST--;
		next;
	}
	if (COLUMN_LIST != 0 && LISTS[LIST_CNT] == "COLUMN_LIST") {
		print "</table>";
		print "<p></p>";
		delete LISTS[LIST_CNT];
		LIST_CNT--;
		COLUMN_LIST--;
		next;
	}
}

# AT&T UNIX
/^\.At/ {
	v=""
	post = punkt($NF)
	if (NF == 2) {
		if ($NF == "32v")
			v = "Version 32V "
		else if ($NF == "v1")
			v = "Version 1 "
		else if ($NF == "v2")
			v = "Version 2 "
		else if ($NF == "v3")
			v = "Version 3 "
		else if ($NF == "v4")
			v = "Version 4 "
		else if ($NF == "v5")
			v = "Version 5 "
		else if ($NF == "v6")
			v = "Version 6 "
		else if ($NF == "v7")
			v = "Version 7 "
		else if ($NF == "V")
			v = "System V "
		else if ($NF == "V.1")
			v = "System V.1 "
		else if ($NF == "V.2")
			v = "System V.2 "
		else if ($NF == "V.3")
			v = "System V.3 "
		else if ($NF == "V.4")
			v = "System V.4 "
	}
	print "<em>" v "AT&T UNIX" "</em>" post
	next
}

# AerieBSD
/^\.Ax/ {
	v=""
	post = punkt($NF)
	if (NF == 2)
		v = " " $2
	print "<em>" "&#198;rieBSD" v "</em>" post
	next
}

# BSDI BSD/OS
/^\.Bsx/ {
	v=""
	post = punkt($NF)
	if (NF == 2)
		v = " " $2
	print "<em>" "BSDI BSD/OS" v "</em>" post
	next
}

# -BSD version
/^\.Bx/ {
	v=""
	rev=""
	post = punkt($NF)
	if (NF == 2)
		v = $2
	else if (NF == 3) {
		v = $2
		if ($3 ~ /[Rr]eno/)
			rev = "-Reno"
		else if ($3 ~ /[Tt]ahoe/)
			rev = "-Tahoe"
	}
	print "<em>" v "BSD" rev "</em>" post
	next
}

# FreeBSD
/^\.Fx/ {
	v=""
	post = punkt($NF)
	if (NF == 2)
		v = " " $2
	print "<em>" "FreeBSD" v "</em>" post
	next
}

# NetBSD
/^\.Nx/ {
	v=""
	post = punkt($NF)
	if (NF == 2)
		v = " " $2
	print "<em>" "NetBSD" v "</em>" post
	next
}

# OpenBSD
/^\.Ox/ {
	v=""
	post = punkt($NF)
	if (NF == 2)
		v = " " $2
	print "<em>" "OpenBSD" v "</em>" post
	next
}

# Text
{ print }
