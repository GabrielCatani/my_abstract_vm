#include <iostream>
#include <sys/stat.h>
#include <fstream>
#include <queue>
#include <string.h>
#include <vector>
#include <cstdint>
#include <typeinfo>
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

void pretty_print_type(int type_index) {
  std::string types[] = {"Int8",
                         "Int16",
                         "Int32",
                         "Float",
                         "Double"};

  std::cout << types[type_index] << std::endl;
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

std::string check_command(const std::string& str) {
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

std::string check_value(const std::vector<std::string>& split) {
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

std::string is_valid_instruction(const std::vector<std::string>& split) {
  std::string result;
  std::string instr;
  std::string value;

  if (split.empty()) {
    return "<blank>";  
  }
  else if (!strcmp(split[0].c_str(), ";;")) {
    return "<EOP>";
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
  private:
    std::queue<std::string> LexedQueue;

  public:
    /*
    * lex_it for when program is from a file
    * (converted to a stream)
    */
    void lex_it(std::ifstream& p_file) {
      std::string token;
      std::string line;
      int line_nbr = 0;

      LexedQueue.push("<file>");

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
    }
    
    /*
    *
    * lex_it for when program is from stdin
    */
    void lex_it(std::string& user_input) {
      std::string line;
      std::string token;
      int index = 0;
      int line_nbr = 0;

      LexedQueue.push("<stdin>");

      line = get_word(user_input, index, '\n');
      while (!line.empty()) {
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
        index += line.size() + 1;
        if (index > (int)user_input.size()) {
          break;
        }
        line = get_word(user_input, index, '\n');
      }
    }

    std::string tokenize(const std::string& line) {
      std::vector<std::string> split = split_string(line, ' ');
      std::string token;
      
      token = is_valid_instruction(split);

      return token;    
    }

    std::queue<std::string> getLexedQueue() {
      return this->LexedQueue;
    }
};

class Parser {
  private:
    std::queue<std::string> ParsedQueue;
    int line_nbr;
    int nbr_elements_simulated_stack;
  
  public:
    void parse_it(std::queue<std::string> LexedQueue) {
      std::string input_type = LexedQueue.front();
      std::string result_status;      

      if (!strcmp(input_type.c_str(), "<stdin>")) {
        if (strcmp(LexedQueue.back().c_str(), "<EOP>")) {
          std::cout << "invalid program" << std::endl;
        }
      }
      else if (!strcmp(input_type.c_str(), "<file>")) {
        if (strcmp(LexedQueue.back().c_str(), "exit")) {
          std::cout << "invalid program" << std::endl;
        }
      }

      line_nbr = -1;
      while(!LexedQueue.empty()) {
        line_nbr++;
        try {
          result_status = simulate_instruction(LexedQueue.front());       if (strcmp(result_status.c_str(), "OK")) {
            throw result_status;
          }
        }
        catch(std::string e) {
          std::cout << "Line " << line_nbr << ": Error : " << e << std::endl;
          break;
        }
        LexedQueue.pop();
      }
       
    }

    std::queue<std::string> getParsedQueue() {
      return this->ParsedQueue;
    }

    std::string simulate_instruction(const std::string& instr) {
      std::string str;
      std::string result = "OK";      

      str = get_word(instr, 0, '-');
      if (!strcmp(str.c_str(), "pop")    ||
          !strcmp(str.c_str(), "assert") ||
          !strcmp(str.c_str(), "print")) {
        if (nbr_elements_simulated_stack == 0) {
          str[0] = toupper(str[0]);
          result = str + " on empty stack";
        }
      }
      else if (!strcmp(str.c_str(), "add") ||
               !strcmp(str.c_str(), "sub") ||
               !strcmp(str.c_str(), "mul") ||
               !strcmp(str.c_str(), "div") ||
               !strcmp(str.c_str(), "mod")) {
        if (nbr_elements_simulated_stack < 2) {
          str[0] = toupper(str[0]);
          result = str + " on stack with less then 2 operands";
        }
        else {
          this->nbr_elements_simulated_stack--;
        }
      }
      else if (!strcmp(str.c_str(), "push")) {
        this->nbr_elements_simulated_stack++;
      }

      return result;
    }

};

template<typename T>
class Operand : public IOperand {
  private:
    eOperandType _type;
    T _value;
    std::string _value_str;
    
  public:
   
    Operand() {}

    Operand(eOperandType type, T value, std::string value_str) {
      this->_type = type;
      this->_value = value;
      this->_value_str = value_str;
    }    

    std::string const & toString() const {
      return this->_value_str;
    }


    int           getPrecision() const {
      return static_cast<eOperandType>(this->_type);
    }

    eOperandType  getType() const {
      return this->_type;
    }
    
    T getValue() const {
      return this->_value;
    }

    IOperand *  operator+(const IOperand &rhs) const {
      eOperandType highest_type = (this->_type >= rhs.getType()) ? this->_type : rhs.getType();
      float f_value = 0.0;
      double d_value = 0.0;
      int8_t i8_value = 0;
      int16_t i16_value = 0;
      int32_t i32_value = 0;

      switch(highest_type) {
        case(Int8):
          i8_value = static_cast<int8_t>(this->_value) + 
                    static_cast<int8_t>(std::stoull(rhs.toString()));
          return new Operand<int8_t>(Int8, i8_value, std::to_string(i8_value));
        case(Int16):
          i16_value = static_cast<int16_t>(this->_value) + 
                    static_cast<int16_t>(std::stoull(rhs.toString()));
          return new Operand<int16_t>(Int16, i16_value, std::to_string(i16_value));
        case(Int32):
          i32_value = static_cast<int32_t>(this->_value) + 
                    static_cast<int32_t>(std::stoull(rhs.toString()));
          return new Operand<int32_t>(Int32, i32_value, std::to_string(i32_value));
        case(Float):
          f_value = static_cast<float>(this->_value) + 
                    static_cast<float>(std::stof(rhs.toString()));
          return new Operand<float>(Float, f_value, std::to_string(f_value));
        case(Double):
          d_value = static_cast<double>(this->_value) + 
                    static_cast<double>(std::stod(rhs.toString()));
          return new Operand<double>(Double, d_value, std::to_string(d_value));
      }

      return nullptr; 
    }

    IOperand *  operator-(const IOperand &rhs) const {
      eOperandType highest_type = (this->_type >= rhs.getType()) ? this->_type : rhs.getType();
      float f_value = 0.0;
      double d_value = 0.0;
      int8_t i8_value = 0;
      int16_t i16_value = 0;
      int32_t i32_value = 0;

      switch(highest_type) {
        case(Int8):
          i8_value = static_cast<int8_t>(this->_value) - 
                    static_cast<int8_t>(std::stoull(rhs.toString()));
          return new Operand<int8_t>(Int8, i8_value, std::to_string(i8_value));
        case(Int16):
          i16_value = static_cast<int16_t>(this->_value) - 
                    static_cast<int16_t>(std::stoull(rhs.toString()));
          return new Operand<int16_t>(Int16, i16_value, std::to_string(i16_value));
        case(Int32):
          i32_value = static_cast<int32_t>(this->_value) - 
                    static_cast<int32_t>(std::stoull(rhs.toString()));
          return new Operand<int32_t>(Int32, i32_value, std::to_string(i32_value));
        case(Float):
          f_value = static_cast<float>(this->_value) -
                    static_cast<float>(std::stof(rhs.toString()));
          return new Operand<float>(Float, f_value, std::to_string(f_value));
        case(Double):
          d_value = static_cast<double>(this->_value) - 
                    static_cast<double>(std::stod(rhs.toString()));
          return new Operand<double>(Double, d_value, std::to_string(d_value));
      }

      return nullptr; 
    }


    IOperand *  operator*(const IOperand &rhs) const {
      eOperandType highest_type = (this->_type >= rhs.getType()) ? this->_type : rhs.getType();
      float f_value = 0.0;
      double d_value = 0.0;
      int8_t i8_value = 0;
      int16_t i16_value = 0;
      int32_t i32_value = 0;

      switch(highest_type) {
        case(Int8):
          i8_value = static_cast<int8_t>(this->_value) * 
                    static_cast<int8_t>(std::stoull(rhs.toString()));
          return new Operand<int8_t>(Int8, i8_value, std::to_string(i8_value));
        case(Int16):
          i16_value = static_cast<int16_t>(this->_value) * 
                    static_cast<int16_t>(std::stoull(rhs.toString()));
          return new Operand<int16_t>(Int16, i16_value, std::to_string(i16_value));
        case(Int32):
          i32_value = static_cast<int32_t>(this->_value) * 
                    static_cast<int32_t>(std::stoull(rhs.toString()));
          return new Operand<int32_t>(Int32, i32_value, std::to_string(i32_value));
        case(Float):
          f_value = static_cast<float>(this->_value) *
                    static_cast<float>(std::stof(rhs.toString()));
          return new Operand<float>(Float, f_value, std::to_string(f_value));
        case(Double):
          d_value = static_cast<double>(this->_value) * 
                    static_cast<double>(std::stod(rhs.toString()));
          return new Operand<double>(Double, d_value, std::to_string(d_value));
      }

      return nullptr; 
    }


    IOperand *  operator/(const IOperand &rhs) const {
      eOperandType highest_type = (this->_type >= rhs.getType()) ? this->_type : rhs.getType();
      float f_value = 0.0;
      double d_value = 0.0;
      int8_t i8_value = 0;
      int16_t i16_value = 0;
      int32_t i32_value = 0;

      if (std::stoull(rhs.toString()) == 0) {
        throw "Division by Zero";
        return nullptr;
      }

      switch(highest_type) {
        case(Int8):
          i8_value = static_cast<int8_t>(this->_value) / 
                    static_cast<int8_t>(std::stoull(rhs.toString()));
          return new Operand<int8_t>(Int8, i8_value, std::to_string(i8_value));
        case(Int16):
          i16_value = static_cast<int16_t>(this->_value) / 
                    static_cast<int16_t>(std::stoull(rhs.toString()));
          return new Operand<int16_t>(Int16, i16_value, std::to_string(i16_value));
        case(Int32):
          i32_value = static_cast<int32_t>(this->_value) / 
                    static_cast<int32_t>(std::stoull(rhs.toString()));
          return new Operand<int32_t>(Int32, i32_value, std::to_string(i32_value));
        case(Float):
          f_value = static_cast<float>(this->_value) /
                    static_cast<float>(std::stof(rhs.toString()));
          return new Operand<float>(Float, f_value, std::to_string(f_value));
        case(Double):
          d_value = static_cast<double>(this->_value) / 
                    static_cast<double>(std::stod(rhs.toString()));
          return new Operand<double>(Double, d_value, std::to_string(d_value));
      }

      return nullptr;       
    }

    IOperand *  operator%(const IOperand &rhs) const {
      eOperandType highest_type = (this->_type >= rhs.getType()) ? this->_type : rhs.getType();
      float f_value = 0.0;
      double d_value = 0.0;
      int8_t i8_value = 0;
      int16_t i16_value = 0;
      int32_t i32_value = 0;

      if (std::stoull(rhs.toString()) == 0) {
        throw "Division by Zero";
        return nullptr;
      }

      switch(highest_type) {
        case(Int8):
          i8_value = static_cast<int8_t>(this->_value) %
                    static_cast<int8_t>(std::stoull(rhs.toString()));
          return new Operand<int8_t>(Int8, i8_value, std::to_string(i8_value));
        case(Int16):
          i16_value = static_cast<int16_t>(this->_value) % 
                    static_cast<int16_t>(std::stoull(rhs.toString()));
          return new Operand<int16_t>(Int16, i16_value, std::to_string(i16_value));
        case(Int32):
          i32_value = static_cast<int32_t>(this->_value) % 
                    static_cast<int32_t>(std::stoull(rhs.toString()));
          return new Operand<int32_t>(Int32, i32_value, std::to_string(i32_value));
        case(Float):
          f_value = static_cast<float>(static_cast<int32_t>(this->_value) % 
                    static_cast<int32_t>(std::stoull(rhs.toString())));
          return new Operand<float>(Float, f_value, std::to_string(f_value));
        case(Double):
          d_value = static_cast<double>(static_cast<int32_t>(this->_value) % 
                    static_cast<int32_t>(std::stoull(rhs.toString())));
          return new Operand<double>(Double, d_value, std::to_string(d_value));
      }

      return nullptr;
    }

    ~Operand() {};
};

//TODO: Implement executor 
//TODO: make code clearer for the operator overloads
/*
* Check if CLI parameters are reference to a programa file
* or the program itself is been passed on stdin
* @param1 - int
* @param2 - char **
* 
* return ENUM args_type
*/
int *check_if_program_file(int ac, char **av) {
  int *array = NULL;  
  
  if (ac < 2) {
    array = (int *)malloc(sizeof(int) * 1);
    if (!array) {
      return NULL;
    }
    array[0] = NO_PARAMS;
    return array;
  }
  
  array = (int *)malloc(sizeof(int) * (ac - 1));
  if (!array) {
    return NULL;
  }  

  for (int index = 1; index < ac; index++) {
    if (ac >= 2) {
      struct stat buf;
      if (!stat(av[index], &buf)) {
        std::cout << "It's a file!" << std::endl;
        array[index - 1] = PROGRAM_FILE;
      }
      else {
        std::cout << "Not a file!" << std::endl;
        array[index - 1] = FROM_STDIN;
      }
    }
}
  return array;
}

void print_lexed(Lexer& lx) {
  std::queue<std::string> LexedQueue = lx.getLexedQueue();
  while(!LexedQueue.empty()) {
    std::cout<< LexedQueue.front() << std::endl;
    LexedQueue.pop();
  }
}

class IOperandFactory {
  public:
    IOperand * createInt8(const std::string & value) {
      return new Operand<int8_t>(Int8, std::stoull(value), value);
    }
    IOperand * createInt16(const std::string & value) {
      return new Operand<int16_t>(Int16, std::stoull(value), value);
    }
    IOperand * createInt32(const std::string & value) {
      return new Operand<int32_t>(Int32, std::stoull(value), value);
    }
    IOperand * createFloat(const std::string & value) {
      return new Operand<float>(Float, std::stof(value), value);
    }
    IOperand * createDouble(const std::string & value) {
      return new Operand<double>(Double, std::stod(value), value);
    }

    IOperand * createOperand(eOperandType type, const std::string & value) {

      IOperand *(IOperandFactory::*functions[5])(const std::string & value) = {&IOperandFactory::createInt8,
              &IOperandFactory::createInt16,
              &IOperandFactory::createInt32,
              &IOperandFactory::createFloat,
              &IOperandFactory::createDouble
              };

      return (this->*functions[type])(value);
    }

    ~IOperandFactory(){}
};

int main(int ac, char **av) {
  int *arg_types = check_if_program_file(ac, av);

  if (!arg_types || arg_types[0] == NO_PARAMS) {
    std::cout << "No instructions passed" << std::endl;
    return -1;
  }
  
  for (int index = 0; index < (ac - 1); index++) {
    //Instantiate objects;
    
    IOperandFactory factory;
    IOperand *integer8 = factory.createOperand(Int8, std::to_string(34));

    IOperand *floater = factory.createOperand(Float, std::to_string(7.0));

    std::cout << (*integer8 / *floater)->toString() << std::endl;
    Lexer lx;
    Parser ps;
    //LEXER
    if (arg_types[index] == PROGRAM_FILE) {
      std::ifstream p_file;
      p_file.open(av[index + 1]);
      lx.lex_it(p_file);
      print_lexed(lx);
      p_file.close();
    }
    else if (arg_types[index] == FROM_STDIN) {
      std::string str(av[index + 1]);
      lx.lex_it(str);
      print_lexed(lx);
    }

    //PARSER
    ps.parse_it(lx.getLexedQueue());
   }

  free(arg_types);
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


  return 0;
}
