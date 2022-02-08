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

void remove_leading_whitespace(std::string *line) {
  if (!line->empty()) {
    std::size_t pos = line->find_first_not_of(' ');
    line->assign(line->substr(pos));
  }
}

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

std::string is_valid_instruction(const char *str) {
  std::string result;

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
      result = instructions[index];
      return instructions[index];
    }
  } 
  
  return result;
}

std::string is_valid_value(const char *str) {
  std::string result;

  const char *values[5] = {"int8",
                          "int16",
                          "int32",
                          "float",
                          "double"};

  if (how_many_words(str, '(') < 2) {
    return result;
  }

  char **full_value = split_string(str, '(');
  char *value_type = full_value[0];
  char *nbr = NULL;
  
  for (int index = 0; index < 5; index++) {
    if (str && !strcmp(value_type, values[index]) &&
        strstr(full_value[1], ")")) {
      upper_first(&value_type);
      result = value_type;
      result.append("-");
      nbr = strtok(full_value[1], ")");
      result.append(nbr);
      return result;
    }
  }

  return result;
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

    std::queue<IOperand>* lex_it(std::ifstream& p_file) {
      //char line[100];
      std::string token;
      std::string line;       

      while (p_file.good()) {
        //p_file.getline(line, 100);
        std::getline(p_file, line);
        token = tokenize(line);
        //TODO: push Token to Queue
        //std::cout << token << std::endl;
      }

      return this->LexedQueue; 
    }

    const char *tokenize(std::string line) {
      
      remove_leading_whitespace(&line);
      std::cout << line.c_str() << std::endl;
      char **split = split_string(line.c_str(), ' ');
      int nbr_words = how_many_words(line.c_str(), ' ');
      std::string instr;
      std::string value;
      std::string token;
      
      if (!(instr = is_valid_instruction(split[0])).empty()) {
        token = instr;
        if (instr == "push" || instr == "assert") {
          value = is_valid_value(split[1]);
          if (!value.empty()) {
            token += "-" + value;
          }
          else {
            token = "<invalid>";
          }
        }
      }

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
