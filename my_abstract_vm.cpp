#include <iostream>
#include <sys/stat.h>
#include <fstream>
#include <queue>

enum args_type {
  NO_PARAMS,
  PROGRAM_FILE,
  FROM_STDIN
};

/*
* Check if CLI parameters are reference to a programa file
* or the program itself is been passed on stdin
* @param1 - int
* @param2 - char **
* 
* return ENUM args_type
*/
int check_if_program_file(int ac, char **av) {
  if (ac >= 2) {
    struct stat buf;
    if (!stat(av[1], &buf)) {
      std::cout << "It's a file!" << std::endl;
      return PROGRAM_FILE;
    }
    std::cout << "Not a file!" << std::endl;
    return FROM_STDIN;
  }
  return NO_PARAMS;
}

int main(int ac, char **av) {
  int arg_type = check_if_program_file(ac, av);
  if (arg_type == NO_PARAMS) {
    std::cout << "No instructions passed" << std::endl;
    return -1;
  }

  //LEXER
  if (arg_type == PROGRAM_FILE) {
    /*
    * ifstream p_file;
    * p_file.open(av[1]);
    * Lexer *lx = new Lexer(p_file);
    * queue<IOperand> lexed =  lx.lex_it();
    */
  }
  else if (arg_type == FROM_STDIN) {
    /*
    * Lexer *lx = new Lexer(ac, av);
    * queue<IOperand> lexed = lx.lex_it();
    */
  }

  //PARSER
  /*
   * Parser ps = new Parser(lexed);
   * queue<IOperand> parsed = ps.parse_it();
   */

  //EXECUTOR
  /*
   * executor(queue<IOperand> parsed);
   * create stack container
   * loop through Parsed Queue:
   *   -> until Queue empty
   *   -> until queue element != exit
   * Executor class methods == 'Assembler' instructions
   */

  //DEL PARSER & LEXER
  
  

  return 0;
}
