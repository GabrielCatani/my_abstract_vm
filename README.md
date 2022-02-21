# my_abstract_vm
MyAbstractVM is a machine with a stack that is able to compute simple arithmetic
expressions. These arithmetic expressions are provided to the machine as basic
assembly programs.

## Installation

```make```

## Usage

A program file, with valid instructions, or instrunctions passed directly 
to standard input can be passed as program parameters
```
./avm [valid_program.avm]

or

./avm [instructions]
```

## Valid instructions
```
push   // push value on the stack
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

# Architecture

## Classes used on the main flow of the program
```

   _____________
  |             | # Parses each instruction passed by the user, checks syntax and
  |    Lexer    | # converts each valid instruction into tokens.
  |_____________| # Tokens were organized into a queue, and passed to the parser.
  |lex_it()     |
  |tokenize()   |
  |getLQueue()  |
  |print_lexed()|
  | getLineNbr()|
  |_____________| 
         | 
         |
   ______|______
  |             | # The Parser iterates over the LexedQueue of instructions, and simulates each instruction
  |    Parser   | # checking for valid combinations and order of instructions, and operations.
  |_____________| # The simulate_instruction method, simulates que number of elements on the stack container of the VM
  |parse_it()   | # checking for illegal operations (ex: pop instructions, when the stack container is empty).
  |getPQueue()  | # Once validated, the parse_it() method generates a new queue, with the token given by the Lexer.
  |sim_instr()  |
  |print_par()  |
  |getLineNbr() |
  |_____________|
         |
         |
   ______|______
  |             | # The Executor class, is where all comes togeteher. The stack container (used a proper stack here),
  |   Executor  | # is created here, which will be used to receive and remove pushed and pop elements, and store operations
  |_____________| # results. The main method of this class, execute_it, iterates over the tokens on the ParsedQueue, 
  |execute_it() | # and starting executing the instructions. This class establishes a dependency with the IoperandFactory - 
  |push_it()    | # responsible for creating operands, of different types (int8, int16, int32, Float and Double).
  |pop_it()     | # In Execute, we check for invalid operations (ex: division by zero), and variables Over/Under flows
  |assert_it()  | # (ex: int8 x > 2147483647).
  |dump_it()    |
  |print_it()   |
  |_____________|

```

## Classes to support Operand types and Operand creation
```

                            _______________
                           | *interface*   | # Interface with basic methods for operands passed by the instruction push and assert. 
                           |   IOperand    | # It makes it easier to implement basic operations between operands 
 _______________           |_______________| # (with same or different types).
|    *enums*    |          |toString()     |
|  eOperandType |--------->|getPrecision() |
|_______________|          |getType()      |
                           |operator+      |
                           |operator-      |                       _________________
                           |operator*      |--------------------->|                 | # Class for creating new operands, based on
                           |operator/      |                      | IOperandFactory | # eOperandTypes passed
                           |operator%      |                      |_________________|
                           |_______________|                      |  createInt8()   |
                                  ^                               |  createInt16()  |
                                 / \                              |  createInt32()  |
                                  |                               |  createFloat()  |
                                  |                               |  createDouble() |
                                  |                               |_________________|
                            ______|______                         
                           | *template*  | # Template for implementing IOperand, with different value types
                           |   Operand   | # of eOperandType (enums).
                           |_____________| 
                           |getValue()   | 
                           |_____________|

```