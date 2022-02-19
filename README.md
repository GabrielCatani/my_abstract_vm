# my_abstract_vm
MyAbstractVM is a machine with a stack that is able to compute simple arithmetic
expressions. These arithmetic expressions are provided to the machine as basic
assembly programs.

## Installation

```make```

## Usage

A program file, with valid instructions, or instrunctions passed directly 
to standard input can be passed as program parameters
```./avm [valid_program.avm]
or

./avm [instructions]
```

# valid instructions
```push
pop    // pop tha last pushed element
dump   // output all values of the stack from the VM (without modifying it)
assert // check if the value, passed with the asserted instruction, is equal to the element on the top of the stack
add    // pop the last two elements pushed to the stack, adds them, and pushs the result back to the stack
sub    // pop the last two elements pushed to the stack, subs them, and pushs the result back to the stack
mul    // pop the last two elements pushed to the stack, multiplies them, and pushs the result back to the stack
div    // pop the last two elements pushed to the stack, divides them, and pushs the result back to the stack
mod    // pop the last two elements pushed to the stack, obtains the modulo them, and pushs the result back to the stack
print  // check if the top element of the stack has a int8 type, and prints it's ASCII character
exit   // terminates the program
```
