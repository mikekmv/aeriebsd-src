patsubst(`quote s in string', `(s)', `\\\1')
patsubst(`check whether subst
over several lines
works as expected', `^', `>>>')
patsubst(`# This is a line to zap
# and a second line
keep this one', `^ *#.*
')
dnl Special case: empty regexp
patsubst(`empty regexp',`',`a ')
