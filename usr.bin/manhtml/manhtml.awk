#!/usr/bin/awk -f
#
# Copyright (c) 2010 Konrad Merz
# Copyright (c) 2010 Michael Shalayeff
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

function punkt(str, p) {
	p = ""
	if (str ~ /^[\.,\!\?;:()\[\]]*$/) {
		p = str
		NF--
		sub(/ [^ ]*$/, "")
	}
	return p
}

function callable(str, pp) {
	pp = punkt($NF);
	fun = $1
	sub(/[^ ]* ?/,"")

	if (fun == "Ad") {
		return "<i>" callable($0) "</i>" pp
	} else if (fun == "Ac") {
		return "&#155;" callable($0) "" pp
	} else if (fun == "Ao") {
		return "&#139;" callable($0) "" pp
	} else if (fun == "Aq") {
		return "&#139;" callable($0) "&#155;" pp
	} else if (fun == "Ar") {
		# Command line argument modifier
		ar = quote_string($1);
		sub(/[^ ]* ?/,"")
		return "<u>" ar "</u>" callable($0) "" pp
	} else if (fun == "Bc") {
		return "]" callable($0) "" pp
	} else if (fun == "Bo") {
		return "[" callable($0) "" pp
	} else if (fun == "Bq") {
		return "[" callable($0) "]" pp
	} else if (fun == "Cm") {
		# Command line argument modifier
		ar = quote_string($1);
		sub(/[^ ]* ?/,"")
		return "<i>-" ar "</i>" callable($0) "" pp
	} else if (fun == "Dc") {
		return "&#148;" callable($0) "" pp
	} else if (fun == "Do") {
		return "&#147;" callable($0) "" pp
	} else if (fun == "Dq") {
		return "&#147;" callable($0) "&#148;" pp
	} else if (fun == "Dv") {
		# Defined variable (source code)
		dv = quote_string($1);
		sub(/[^ ]* ?/,"")
		return "<b>" dv "</b>" callable($0) "" pp
	} else if (fun == "Em") {
		return "<em>" callable($0) "</em>" pp
	} else if (fun == "Eo") {
		return "&#171;" callable($0) "" pp
	} else if (fun == "Ec") {
		return "&#187;" callable($0) "" pp
	} else if (fun == "Er") {
		# Error number (source code)
		er = quote_string($1);
		sub(/[^ ]* ?/,"")
		return "<b>" er "</b>" callable($0) "" pp
	} else if (fun == "Ev") {
		# Function argument
		ev = quote_string($1);
		sub(/[^ ]* ?/,"")
		return "<b>" ev "</b>" callable($0) "" pp
	} else if (fun == "Fa") {
		# Function argument
		fa = quote_string($1);
		sub(/[^ ]* ?/,"")
		return "<i>" fa "</i>" callable($0) "" pp
	} else if (fun == "Fl") {
		return "-<b>" callable($0) "</b>" "" pp
	} else if (fun == "Fc") {
	} else if (fun == "Fo") {
	} else if (fun == "Fn") {
		# Function name
		fn = quote_string($1)
		sub(/[^ ]* ?/,"")
		sub(/^\"/, "");
		sub(/\"$/, "");
		gsub(/\" \"/, ", ");
		fnarg = quote_string($0);
		return "<b>" fn "</b>(" fnarg ");" pp
	} else if (fun == "Ft") {
		# Function type
		ft = quote_string($1)
		sub(/[^ ]* ?/,"")
		return "<p></p><i>" ft "</i>" callable($0) "" pp
	} else if (fun == "Ic") {
		ic = quote_string($1)
		sub(/[^ ]* ?/,"")
		return "<i>" ic "</i>" callable($0) "" pp
	} else if (fun == "Li") {
		return "<u>" callable(0) "</u>" pp 
	} else if (fun == "Nm") {
		if (NF > 0 && name == "") {
			name = $1;
			sub(/[^ ]* ?/,"")
			return "<b>" name "</b> &#151; "
		} else if (NF > 0) {
			nam = quote_string($1)
			sub(/[^ ]* ?/,"")
			return "<b>" nam "</b>" callable($0) "" pp
		} else
			return "<b>" name "</b>" pp
	} else if (fun == "No") {
		NO = no
		no = ""
		return NO "" callable($0) "" pp
	} else if (fun == "Ns") {
		# no space
	} else if (fun == "Op") {
		return "[" callable($0) "]" pp
	} else if (fun == "Ot") {
		# Old style function type (Fortran only)
		ot = quote_string($1)
		sub(/[^ ]* ?/,"")
		return "<p></p><i>" ot "</i>" callable($0) "" pp
	} else if (fun == "Pa") {
		# path name or a file
		return "<i>" callable($0) "</i>" pp
	} else if (fun == "Pc") {
		return ")" callable($0) "" pp
	} else if (fun == "Po") {
		return "(" callable($0) "" pp
	} else if (fun == "Pq") {
		return "(" callable($0) ")" pp
	} else if (fun == "Ql") {
		ql = quote_string($1)
		sub(/[^ ]* ?/,"")
		return "&quot;" ql "&quot;" callable($0) "" pp
	} else if (fun == "Qc") {
		return "&quot;" callable($0) "" pp
	} else if (fun == "Qo") {
		return "&quot;" callable($0) "" pp
	} else if (fun == "Qq") {
		return "&quot;" callable($0) "&quot;" pp
	} else if (fun == "Sc") {
		return "&#146" callable($0) "" pp
	} else if (fun == "So") {
		return "&#145" callable($0) "" pp
	} else if (fun == "Sq") {
		return "&#145" callable($0) "&#146;" pp
	} else if (fun == "St") {
		# standards
	} else if (fun == "Sx") {
		# Section Cross Reference
	} else if (fun == "Sy") {
		# Symbolic (traditional English)
		sy = no
		no = "</i>"
		return sy "<i>" callable(0) "" pp
	} else if (fun == "Ta") {
		return "</td><td>" callable(0) "" pp
	} else if (fun == "Tn") {
		return "<b>" callable($0) "</b>" pp
	} else if (fun == "Va") {
		# Variable name
		va = quote_string($1)
		sub(/[^ ]* ?/,"")
		return "<u>" va "</u>" callable($0) "" pp
	} else if (fun == "Vt") {
		# Variable type
		vt = quote_string($1)
		sub(/[^ ]* ?/,"")
		return "<i>" vt "</i>" callable($0) "" pp
	} else if (fun == "Xc") {
	} else if (fun == "Xo") {
	} else if (fun == "Xr") {
		# Cross Reference (need to fix arch pages search)
		man = $1
		sect = $2
		arch = ""
		sub(/[^ ]* ?/,"")
		sub(/[^ ]* ?/,"")
		if (NF > 0) {
			arch = "/" $1
			sub(/[^ ]* ?/,"")
		}
		return "<a href=\"/man/man" sect arch "/" man ".html\">" \
		    man "(" sect arch ")" "</a>" pp
	} else if (NF > 0) {
		return fun " " callable($0) "" pp
	} else
		return fun "" pp
}

BEGIN {
	rev="$ABSD: manhtml.awk,v 1.8 2010/09/30 17:04:51 mickey Exp $"
	no = "";	# normal text
	name = ""	# man name from .Nm
	ibl = 0		# nested .bl/.el stack
	blit[ibl] = 0
	blbl[ibl] = ""

	print "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\""
	print "\"http://www.w3.org/TR/html4/loose.dtd\">"
	print "<html>"
}

END {
	print "<p><hr></p>";
	print "<table width=100% ><tr>";
	print "<td> " OSNAME " " OSREL " Reference Manual </td>";
	print "<td align=center> " date " </td>";
	print "<td align=right> " title "(" section ") </td>";
	print "</tr></table>";
	print "</body>";
	print "</html>";
}

# parsed macros
/^\.(Ad|Ac|Ao|Aq|Ar|Bc|Bo|Bq|Cm|Dc|Do|Dq|Dv|Em|Ec|Eo|Er|Ev|Fa|Fl|Fn|Ft|Ic|Li|Nm|No|Ns|Op|Ot|Pa|Pc|Po|Pq|Qc|Ql|Qo|Qq|Sc|So|Sq|St|Sx|Sy|Tn|Va|Vt|Xc|Xo|Xr) ?/ {
	sub(/^\./, "")
	print callable($0) "" no
	no = ""
	next
}

# Formatting
/^\.An / {
	post = punkt($NF)
	sub (/^\.An /, "")
	print "<i>" callable($0) no "</i>" post
	no = ""
	next
}

/^\.In / {
	str = quote_string($2);
	print "#include &lt;<i>" str "</i>&gt;<br>"
	next
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

# Function declaration
/^\.Fd / {
	sub(/^\.Fd /, "");
	str = quote_string($0);
	print "<b><kbd>" str "</kbd></b><br>";
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

# Begin list
/^\.Bl/ {
	if (ibl++) {
		blit[ibl] = it
		blbl[ibl] = bl
	}
	it = 0
	bl = $2
	if (bl == "-tag")
		print "<dl>"
	else if (bl == "-enum")
		print "<ol>"
	else if (bl == "-bullet")
		print "<ul style=\"list-style-type:disc\">"
	else if (bl == "-dash" || bl == "-hyphen")
		print "<ul style=\"list-style-type:circle\">"
	else if ($2 == "-column")
		print "<p></p><table>"
	next
}

# List item
/^\.It ?/ {
	it++
	sub(/^\.It /, "");
	if (bl == "-tag") {
		if (it > 1)
			print "</dd>"
		print "<dt>" callable($0) "" no "</dt><dd>"
		no = ""
	} else if (bl == "-bullet" || bl == "-dash" || bl == "-hyphen" ||
	    bl == "-enum") {
		print "<li>"
	} else if (bl == "-column") {
		bit = ""
		bot = ""
		if (it == 1) {
			print "<table>"
			bit = "<b>"
			bot = "</b>"
		}
		gsub(/"/,"")
		nit = split($0, sa, "\t");
		print "<tr>";
		for (i = 1; i <= nit; i++) {
			$0 = sa[i]
			print "<td>" bit "" callable($0) "" bot "</td>";
		}
		print no "</tr>";
		no = ""
	}
	next
}

# End list
/^\.El/ {
	if (bl == "-tag")
		print "</dl>"
	else if (bl == "-enum")
		print "</ol>"
	else if (bl == "-bullet" || bl == "-dash" || bl == "-hyphen")
		print "</ul>"
	else if (bl == "-column")
		print "</table><p></p>";
	if (ibl) {
		it = blit[ibl]
		bl = blbl[ibl]
		ibl--
	}
	next
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
