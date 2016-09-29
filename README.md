# scam

scam is a toy/minimalistic Lisp interpreter written in C.


## Status

Simple programs, such as `fib.scam` and `factorial.scam` -- which are included in the repo as examples -- work.

Lacking critical features:

- String manipulation primitives
- Garbage collection


## Compiling

```bash
$ make
```

## Running

Code is read from stdin. You can add a `--debug` switch to add debug information to the output.

- `fib.scam` - Evaluates the 8th Fibonacci number
```
$ cat fib.scam | ./scam
RESULT:
=======
NUMBER 21.000000
```

```
$ cat fib.scam | ./scam --debug
TOKENS (45)
==========
(, begin, (, define, fib, (, lambda, (, n, ), (, if, (, <, n, 3, ), 1, (, +, (, fib, (, -, n, 1, ), ), (, fib, (, -, n, 2, ), ), ), ), ), ), (, fib, 8, ), ),

AST:
====
COMPOUND
    SYMBOL begin
    COMPOUND
        SYMBOL define
        SYMBOL fib
        COMPOUND
            SYMBOL lambda
            COMPOUND
                SYMBOL n
            COMPOUND
                SYMBOL if
                COMPOUND
                    SYMBOL <
                    SYMBOL n
                    NUMBER 3.000000
                NUMBER 1.000000
                COMPOUND
                    SYMBOL +
                    COMPOUND
                        SYMBOL fib
                        COMPOUND
                            SYMBOL -
                            SYMBOL n
                            NUMBER 1.000000
                    COMPOUND
                        SYMBOL fib
                        COMPOUND
                            SYMBOL -
                            SYMBOL n
                            NUMBER 2.000000
    COMPOUND
        SYMBOL fib
        NUMBER 8.000000

ENV:
====
fib => USER DEFINED PROCEDURE WITH ARGS n,
< => PRIMITIVE PROCEDURE
> => PRIMITIVE PROCEDURE
== => PRIMITIVE PROCEDURE
/ => PRIMITIVE PROCEDURE
* => PRIMITIVE PROCEDURE
- => PRIMITIVE PROCEDURE
+ => PRIMITIVE PROCEDURE
true => BOOLEAN 1
false => BOOLEAN 0

RESULT:
=======
NUMBER 21.000000
```
