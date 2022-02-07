#include <iostream>
#include <sys/stat.h>
#include <fstream>
#include <queue>
#include <string.h>

enum eOperandType {
  Int8,
  Int16,
  Int32,
  Float,
  Double
};

enum args_type {
  NO_PARAMS,
  PROGRAM_FILE,
  FROM_STDIN
};

void upper_first(char **str) {
  (*str)[0] -= 32; 
}

int how_many_words(const char *line, char delim) {
  int nbr_words = 0;

  for (int index = 0; line[index]; index++) {
    if (line[index] == delim) {
      nbr_words++;
    }
  }

  return (nbr_words + 1);
}

char *get_word(const char *line, int index, char delim) {
  char *word = NULL;
  int end = index;

  while (line[end]) {
    if (line[end] == delim) {
      break;
    }
    end++;
  }

  word = strndup(&line[index], end - index);
  return word;
}

char **split_string(const char *line, char delim) {
  char **split;
  int nbr_words = how_many_words(line, delim);
  char *tmp = NULL;
  int index = 0;

  split = (char **)malloc(sizeof(char *) * nbr_words);
  for (int i = 0; i < nbr_words; i++) {
    tmp = get_word(line, index, delim);
    split[i] = strndup(tmp, strlen(tmp));
    index += strlen(tmp) + 1;
  }

  return split;
}

const char *is_valid_instructions(const char *str) {
  const char *instructions [11] = {"push",
    "pop",
    "dump",
    "assert",
    "add",
    "sub",
    "mul",
    "div",
    "mod",
    "print",
    "exit"};

  for (int index = 0; index < 11; index++) {
    if (str && !strcmp(str, instructions[index])) {
      return instructions[index];
    }
  } 
  
  return NULL;
}

//TODO: return full toke in the format: "type_value-nbr_value"
const char *is_valid_value(const char *str) {
  const char *values[5] = {"int8",
                          "int16",
                          "int32",
                          "float",
                          "double"};
  if (how_many_words(str, '(') < 2) {
    return NULL;
  }

  char **full_value = split_string(str, '(');
  char *value_type = full_value[0];
  char *nbr = strtok(full_value[1], ")"); 

  for (int index = 0; index < 5; index++) {
    if (str && !strcmp(value_type, values[index])) {
      upper_first(&value_type);
      return value_type;
    }
  }

  return NULL;
} 

class IOperand
{
  public:

    virtual std::string const & toString() const = 0;

    virtual int           getPrecision() const = 0;
    virtual eOperandType  getType() const = 0;

    virtual IOperand *  operator+(const IOperand &rhs) const = 0;
    virtual IOperand *  operator-(const IOperand &rhs) const = 0;
    virtual IOperand *  operator*(const IOperand &rhs) const = 0;
    virtual IOperand *  operator/(const IOperand &rhs) const = 0;
    virtual IOperand *  operator%(const IOperand &rhs) const = 0;

    virtual ~IOperand() {}
};

class Lexer {
  public:
    std::queue<IOperand> *LexedQueue;
    //TODO: include const token, to receive token from check_line_gramar. Change the name of the functon to "generate token"
    std::queue<IOperand>* lex_it(std::ifstream& p_file) {
      char line[100];

      while (p_file.good()) {
        p_file.getline(line, 100);
        check_line_grammar(line);
/*
        if (token) {
          std::cout << token << std::endl;
        }
*/
      }

      return this->LexedQueue; 
    }

    const char *check_line_grammar(const char *line) {

      char **split = split_string(line, ' ');
      int nbr_words = how_many_words(line, ' ');
      const char *instr = NULL;
      const char *value = NULL;
      std::string token;

      for (int i = 0; i < nbr_words; i++) {
        if (i == 0) {
          instr = is_valid_instructions(split[i]);
        }
        else if (i == 1) {
          value = is_valid_value(split[i]);
        }
      }
   
      if (instr) {
        token = strdup(instr);
        //std::cout << "INSTR -> " << instr << std::endl;
      }
      if (value) {
        token = token + "-" + value;
        //std::cout <<"VALUE -> " << value << std::endl;
      }
      
      std::cout << token.c_str() << std::endl; 
      return token.c_str();    
    }

    /*
    *
    * lex_it for when program is on stdin
    *
    std::queue<IOperand>* lex_it(char *cli_input) {
      return this->LexedQueue; 
    }
    */
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
    * close(p_file);
    */
    Lexer ls;
    std::ifstream p_file;
    p_file.open(av[1]);
    ls.lex_it(p_file);
    p_file.close();
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
