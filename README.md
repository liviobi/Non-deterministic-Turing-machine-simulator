# Non-deterministic Turing machine simulator
This project has been developed as part of the "Algoritmi e Principi dell'Informatica" course at Politecnico di Milano.

## Aim
The aim of the project was to develop a simulator of non-deterministic Turing machines in the single tape variant. 

Given as input the transition function of the machine to be simulated and a string to be recognized, the simulator must return '1' if the string is accepted, '0' if rejected or 'U' if it is not recognized.

Only standard C libraries are allowed in the project.

## Specs

The input file is divided into 4 parts:
- A first part, starting with `tr`, contains the transitions, one per line - each character can be separated from the others by spaces.  
For example, `0 a c R 1` means that with the transition you go from state _0_ to state _1_, reading _a_ and writing _c_. The head is moved to the right (_R_).
- The next part, starting with `acc`, lists the acceptance states, one per line.
- To avoid infinite computation problems, the next section, which starts with `max`, lists the maximum number of moves you can make to accept a string.
- The final section, which starts with `run`, is a list of strings to be provided to the machine, one per line.

At the output a file is expected containing:
- `0` for the non-accepted strings;
- `1` for the accepted ones; 
- `U` if you have not reached acceptance, since there is also a limit on the number of steps.

> The input file is supplied via the standard input, while the output file is on the standard output.

*Conventions*:  
For simplicity the tape symbols are `char`, while the states are `int`. The `_` character indicates the blank symbol.  
The machine always starts from state `0` and the first character of the input string. The characters `L, R, S` are used for head movement.

### Example
Input file:
```
tr
0 a a R 0
0 b b R 0
0 a c R 1
0 b c R 2
1 a c L 3
2 b c L 3
3 c c L 3
3 a c R 4
3 b c R 5
4 c c R 4
4 a c L 3
5 c c R 5
5 b c L 3
3 _ _ R 6
6 c c R 6
6 _ _ S 7
acc
7
max
800
run
aababbabaa
aababbabaaaababbabaa
aababbabaaaababbabaab
aababbabaaaababbabaabbaababbabaaaababbabaa
aababbabbaaababbabaabbaababbabaaaababbabaa
```
Relative output:
```
1
1
0
U
0
```
