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

BEGIN {
	osname = "AerieBSD";
	osrelease = "1.0";
	path = "/home/kmerz/";
	print "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\""
	print "\"http://www.w3.org/TR/html4/loose.dtd\">"
	print "<html>"
}

END {
	print "<p><hr><p>";
	print "<table width=100% ><tr>";
	print "<td>" osname " " osrelease " Refernce Manual </td>";
	print "<td align=center>" date "</td>";
	print "<td align=right>" title "(" section ") </td>";
	print "</tr></table>";
	print "</body>";
	print "</html>";
}

# Document Tilte
/^\.Dt / {
	sub(/^\.Dt /,"");
	title = $1;
	section = $2;
	next;
}

/^\.Dq / {
	sub (/^\.Dq /, "");
	print "\"" $0 "\"";
	next;
}

/^\.Sq / {
	sub (/^\.Sq /, "");
	print "\'" $0 "\'";
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
	print "<head><title>" title "</title></head>";
	print "<body>";
	print "<table width=100% ><tr>";
	print "<td>" title "(" section ") </td>";
	print "<td align=center>" osname " " osrelease " Refernce Manual </td>";
	print "<td align=right>" title "(" section ") </td>";
	print "</tr></table>";
	next;
}

# Section Header
/^\.Sh / {
	sub(/^\.Sh /,"");
	print "<h2>" $0 "</h1>";
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
/^\.Nm / {
	sub(/^\.Nm /,"");
	print "<b>"$0"</b> - ";
	next;
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
	print "<p>";
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
	sub(/^\.Xr /, "");
	print "<a href=\"" path "/" $1 "." $2 ".html\">" $1 "(" $2 ")" "</a>";
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
	print "<p><i>" str "</i> ";
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

# Error p
/^\.Er / {
	printf ("<b>%s</b>", $2);
	print $3;
	next;
}

/^\.Dv / {
	printf "<i>%s</i>", $2;
	print $3
	next;
}

# Begin list
/^\.Bl/ {
	print "<table>"
	next;
}

# List item
/^\.It / {
	sub(/^\.It /, "");
	print "<tr>";
	print "<td>";
	print $0;
	print "</td>";
	print "</tr>";
	next;
}

# End list
/^\.El/ {
	print "</table>"
	next;
}

# Text
// {
	print $0;
	next;
}
