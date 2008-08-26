dnl Checking the way changequote() is supposed to work
define(`string',`STRING')dnl
1: normal
`quoted string'
[quoted string]
normal string
`half quoted string
going up to that string'

2: kill quotes
changequote()dnl
`quoted string'
[quoted string]
normal string
`half quoted string
going up to that string'

3: normal changed quote
changequote([,])dnl
`quoted string'
[quoted string]
normal string
`half quoted string
going up to that string'

4: empty quotes, kill them too
changequote(,)dnl
`quoted string'
[quoted string]
normal string
`half quoted string
going up to that string'

5: start quote only
changequote(`)dnl
`quoted string'
[quoted string]
normal string
`half quoted string
going up to that string'

6: normal quotes are back
changequote
`quoted string'
[quoted string]
normal string
`half quoted string
going up to that string'

7: start quote+empty end quote
changequote([,)dnl
`quoted string'
[quoted string]
normal string
`half quoted string
going up to that string'
