programmable_calculator
=======================

Command line rpn programmable calculator for Linux

When you start the program without any command-line arguments, you are presented with 
a "?" prompt. You may enter calculations that you want to compute in rpn (or postfix)
notation. This means that you enter, usually, 2 operands and then an operator, seperated 
by spaces. You may put the entire equation on one line, or you can enter each operand/operator 
on seperate lines. You can even enter multiple equations on just one line.

The calculator is programmable, meaning that you can write a text file with calculator
commands which will be executable when you are in interactive mode. There is a default user
program loaded automatically. The file can be found in ./user/default.txt

You may put your own programs in the ./user directory by storing them with a "txt" extension.
To see the available calculator commands, type "keys" while in interactive mode. Any line
that starts with "#" is a comment.

To run the actual program, you will need to type "labels". This will output the names of 
every label in your loaded program. Labels with meaningless names, like "1" or "one", are
probably not programs. However, following the example of the default program, you should
give your actual programs that are meant to be run, self-explanatory names like "fact" or 
"binary". To run the program, just type the name of a label from the "?" prompt.

You can also type "help" <command name> to find out what each of the commands do. Once again,
the possible commands can be viewed by typing "keys".
