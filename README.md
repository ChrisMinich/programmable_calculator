programmable_calculator
=======================

Command line rpn programmable calculator for Linux

INTERACTIVE MODE

When you start the program without any command-line arguments, you are presented with 
a "?" prompt at which you will enter calculations that you want to compute in rpn (postfix)
notation. This means that you enter, usually, 2 operands and then an operator, separated 
by spaces. You may put the entire equation on one line, or you can enter each operand/operator 
on separate lines. You can even enter multiple equations on just one line.

Every operand that is entered goes onto a stack. When you enter an operator, two numbers
are popped from the stack, and the result is pushed back in their place. The number at the 
top of the stack is x. The number just beneath x is y.

Each memory (0-9) is also a stack.

The calculator is programmable, which means that you can write a text file with calculator
equations which will be executable when you are in interactive mode. There is a default user
program loaded automatically. It resides in ./user/default.txt

You may put your own programs in the ./user directory by storing them with a "txt" extension.
To see the available calculator commands, type "keys" while in interactive mode. Any line
that starts with "#" is a comment. There is a "load" command which is used to replace the
default program with your own one.

Definitions:
"User program" - a file which includes calculator commands. It may contain one or more "routines".
"Routine" - One of the subroutines within the user program file. Each routine is usually
a function of some sort.

To see what user routines are loaded, you can type "labels" which will output the name of 
every label in the loaded program file. Labels with cryptic names, like "1" or "one", are
probably not routines. Instead, they are labels which are used by the routines. However, 
following the example of the default program, you should name the actual routines (that are 
meant to be run) intuitive names like "fact" or "binary". To run the routine, just type the 
name of a label from the "?" prompt.

(The default routine "fact" computes the factorial of the number in the x-register. The
routine "binary" shows you the binary form of the number in the x-register, although you
cannot use that number for anything because it is really, still, a decimal number that just
looks like a binary value. 101 is actually one-hundred-one, not five.)

You can also type "help" <<command name>> to find out what each commands does. Once again,
the possible commands can be viewed by typing "keys".

COMMAND-LINE MODE

If you just want to make a quick calculation and then exit the program, just include your
calculation on the command line.

For example: calc 5 3 "*" <<Enter>>
This would print the answer of 15 and automatically exit.

In this example, the multiplication operator was enclosed within double quotes because the
asterisk has a special meaning to the bash shell when used as an argument to a program.
The operators "+", "-", and "/" can be used without the quotes.

Example: calc 55 45 + <<Enter>>
This would output the result of 100.

If you enter a calculation like: calc 100 3 / <<Enter>>
you will obtain a result of 33. To get the floating point answer, enter it 
this way: calc 100 3 / float 9
This will yield 33.333333333

This trick is not needed in interactive mode, because numbers with a decimal
are automatically printed with 2 decimal places. However, to increase the number
of decimal places shown, type: float n <<Enter>> (0 <= n <= 9).

---

Well, now that you've read the introduction, please explore the program by using the 
"keys" command and the "help" command.

Bon appetit.
