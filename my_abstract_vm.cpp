#include <iostream>
#include <sys/stat.h>
#include <fstream>
#include <queue>
#include <string.h>
#include <vector>
#define INVALID_TOKEN "<invalid>"

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

//TODO: upp only the first, in a std::string container
/*
void upper_first(std::string str) {
  str.c_str() -= 32; 
}
*/
int how_many_words(const char *line, char delim) {
  int nbr_words = 0;
  
  for (int index = 0; line[index]; index++) {
    if (line[index] == delim) {
      nbr_words++;
    }
  }

  return (nbr_words + 1);
}

std::string get_word(const std::string &line, int pos, char delim) {
  std::string word;
  int end = line.find_first_of(delim, pos);
  word.assign(line, pos, (end - pos));

  return word;
}

std::vector<std::string> split_string(const std::string& line, char delim) {
  std::vector<std::string> split;
  std::string word;
  int index = 0;

  while (index < (int)line.size()) {
    index = line.find_first_not_of(delim, index);
    word = get_word(line, index, delim);
    split.push_back(word);
    index += word.size();
  }
  
  return split;
}

std::string check_command(std::string str) {
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
    if (!strcmp(str.c_str(), instructions[index])) {
      return str;
    }
  }

  return INVALID_TOKEN;
}

std::string check_value(const std::vector<std::string> split) {
  std::string value_type;
  std::string value;
  std::string result;

  const char *values[5] = {"int8",
                          "int16",
                          "int32",
                          "float",
                          "double"};

  value_type = get_word(split[1], 0, '(');
  value = get_word(split[1], value_type.size(), ' ');
  
  for (int index = 0; index < 5; index++) {
    if (!strcmp(value_type.c_str(), values[index])) {
      value_type[0] = toupper(value_type[0]);
      result = value_type;
      if (value.c_str()[0] == '(' && value.c_str()[value.size() - 1] == ')') {
        result += "-" + value.substr(1, value.size() - 2);
        return result;
      }
    }
  }

  return INVALID_TOKEN;
}

std::string is_valid_instruction(const std::vector<std::string> split) {
  std::string result;
  std::string instr;
  std::string value;

  if (split.empty()) {
    return "<blank>";  
  }
  else if(!strcmp(split[0].c_str(), ";") || split[0].c_str()[0] == ';') {
    return "<comment>";
  }
  
  instr = check_command(split[0]);
  if (!strcmp(instr.c_str(), INVALID_TOKEN)) {
    return INVALID_TOKEN;
  }
  else if (!strcmp(instr.c_str(), "push") ||
           !strcmp(instr.c_str(), "assert")) {
    value = check_value(split);
    if (strcmp(value.c_str(), INVALID_TOKEN)) {
      result += instr + "-";
    }
    result += value;
  }
  else {
    result += instr;
  }

  if (split.size() > 2) {
    if (strcmp(split[2].c_str(), ";") && split[2].c_str()[0] != ';') {
      return INVALID_TOKEN;
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
    std::queue<std::string> LexedQueue;

    std::queue<std::string> lex_it(std::ifstream& p_file) {
      std::string token;
      std::string line;
      int line_nbr = 0;

      while (p_file.good()) {
        line_nbr++;
        std::getline(p_file, line);
        try {
          token = tokenize(line);
          if (!strcmp(token.c_str(), INVALID_TOKEN)) {
            throw token;
          } 
          LexedQueue.push(token);
        }
        catch(std::string token) {
          std::cout << "Line " << line_nbr << ": Error : " << token << ": " << line << std::endl;
          break;
        }
      }

      return this->LexedQueue; 
    }
    
    std::string tokenize(const std::string line) {
      std::vector<std::string> split = split_string(line, ' ');
      std::string token;
      
      token = is_valid_instruction(split);

      return token;    
    }

    /*
    *
    * lex_it for when program is on stdin
    *
    std::queue<std::string> lex_it(char *cli_input) {
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
  //TODO: lexer when from stdin
  if (arg_type == PROGRAM_FILE) {
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

  //TODO: Parser
  //PARSER
  /*
   * Parser ps = new Parser(lexed);
   * queue<IOperand> parsed = ps.parse_it();
   */

  //TODO: Executor
  //EXECUTOR
  /*
   * executor(queue<IOperand> parsed);
   * create stack container
   * loop through Parsed Queue:
   *   -> until Queue empty
   *   -> until queue element != exit
   * Executor class methods == 'Assembler' instructions
   */

  //TODO: Destructor
  //DEL PARSER & LEXER
  
  

  return 0;
}
