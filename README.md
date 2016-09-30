# scam

scam is a toy/minimalistic Lisp interpreter written in C.


## Status

Lacking critical features:

- Garbage collection;
- String manipulation primitives;
- Tail recursion optimization;


## Compiling

```bash
$ make
```

## Examples

Code is read from stdin. The `examples.scam` file is interpretable. You can run it with `cat examples.scam | ./scam`. Here it is:

```lisp
;
; Types
;

; Numeric
42
; => NUMBER 42.000000

; String
"Hello, world"
; => STRING Hello, world

; Boolean
true
; => BOOLEAN 1

false
; => BOOLEAN 0

;
; Numeric primitive operators
;

(+ 1 2)
; => NUMBER 3.000000

(* 2 (- 1 2) (/ 2 2))
; => NUMBER -2.000000

;
; Numeric primitive predicates
;

(== 1 2)
; => BOOLEAN 0

(> 2 1)
; => BOOLEAN 1

(< 1 2)
; => BOOLEAN 0

;
; User defined procedures
;

(define factorial (lambda (n)
  (if (== n 1)
    1
    (* n (factorial (- n 1))))))
; => USER DEFINED PROCEDURE WITH ARGS n,

;
; I/O
;

(display "6! = " (factorial 6))
; 6! = 720.000000
; => BOOLEAN 1

(display "This is a number: " 1 " and this is another: " 2)
; This is a number: 1.000000 and this is another: 2.000000
; => BOOLEAN 1

```


## Debugging

You can add a `--debug` switch to add debug information to the output. E.g.:

- `fib.scam` - Evaluates the 8th Fibonacci number
```
$ cat fib.scam | ./scam
=> NUMBER 21.000000
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
=> NUMBER 21.000000
```
