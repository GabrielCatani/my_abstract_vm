#include <iostream>
#include <sys/stat.h>
#include <fstream>
#include <stack>
#include <queue>
#include <string.h>
#include <vector>
#include <cstdint>
#include <typeinfo>
#define INVALID_TOKEN "<invalid>"

//*************************************** 
/*
*  ENUMS
*    1. eOperandType
*    2. args_type
*
****************************************/

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

//*************************************** 
/*
*  HELPER FUNCTIONS
*    1. pretty_print_type
*    2. get_word
*    3. split_string
*    4. check_command
*    5. check_value
*    6. is_valid_instruction
*    7. check_if_program_file
*    8. mapType
*
****************************************/

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
    word  = get_word(line, index, delim);

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
  value      = get_word(split[1], value_type.size(), ' ');
  
  for (int index = 0; index < 5; index++) {
    if (!strcmp(value_type.c_str(), values[index])) {
      
      value_type = std::to_string(index);
      result        = value_type;
      
      if (value.c_str()[0] == '(' && value.c_str()[value.size() - 1] == ')') {

        result      += "-" + value.substr(1, value.size() - 2);
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
    result   += value;
  }
  else {
    result   += instr;
  }

  if (split.size() > 2) {
    if (strcmp(split[2].c_str(), ";") && split[2].c_str()[0] != ';') {
      return INVALID_TOKEN;
    }
  }

  return result;
}

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

eOperandType mapType(const std::string s_type) {
  eOperandType type;  

  switch(std::stoi(s_type)) {
    case 0:
      type = Int8;
      break;
    case 1:
      type = Int16;
      break;
    case 2:
      type = Int32;
      break;
    case 3:
      type = Float;
      break;
    case 4:
      type = Double;
      break;
  }
  return type;
}

//*************************************** 
/*
*  CLASSES
*    1. IOperand
*    2. Lexer
*    3. Parser
*    4. Operand
*    5. IOperandFactory
*    6. Executor
*
****************************************/

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
    int line_nbr;
  public:
    /*
    * lex_it for when program is from a file
    * (converted to a stream)
    */
    void lex_it(std::ifstream& p_file) {
      std::string token;
      std::string line;
      this->line_nbr = 0;

      LexedQueue.push("<file>");

      while (p_file.good()) {
        line_nbr++;
        std::getline(p_file, line);
        token = tokenize(line);
        if (!strcmp(token.c_str(), INVALID_TOKEN)) {
          throw std::string("Invalid instruction: " + line);
        } 
        LexedQueue.push(token);
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
      this->line_nbr = 0;

      LexedQueue.push("<stdin>");

      line = get_word(user_input, index, '\n');
      while (!line.empty()) {
        token = tokenize(line);
        if (!strcmp(token.c_str(), INVALID_TOKEN)) {
          throw std::string("Invalid instruction: " + line);
        } 
        LexedQueue.push(token);

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

    void print_lexed() {
      std::queue<std::string> LexedQueue = this->getLexedQueue();
      while(!LexedQueue.empty()) {
        std::cout<< LexedQueue.front() << std::endl;
        LexedQueue.pop();
      }
    }

    int getLineNbr() {
      return this->line_nbr;
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
          throw std::string("Missing ;; at end of program.");
        }
      }
      else if (!strcmp(input_type.c_str(), "<file>")) {
        if (strcmp(LexedQueue.back().c_str(), "exit")) {
          throw std::string("Missing 'exit' instruction at the end of program.");
        }
      }

      line_nbr = -1;
      while(!LexedQueue.empty()) {
        line_nbr++;
        result_status = simulate_instruction(LexedQueue.front()); 
        if (strcmp(result_status.c_str(), "OK")) {
          throw result_status;
        }

        this->ParsedQueue.push(LexedQueue.front());
        LexedQueue.pop();
      }
    }

    std::queue<std::string> getParsedQueue() {
      return this->ParsedQueue;
    }

    std::string simulate_instruction(const std::string& instr) {
      std::string str;
      std::string result = "OK";
      std::string type   = split_string(instr, '-')[1];
      std::string value  = split_string(instr, '-')[2];      

      str = get_word(instr, 0, '-');
      if (!strcmp(str.c_str(), "pop")    ||
          !strcmp(str.c_str(), "assert") ||
          !strcmp(str.c_str(), "print")) {
        if (nbr_elements_simulated_stack == 0) {
          str[0] = toupper(str[0]);
          result = str + " on empty stack";
        }
        else {
          nbr_elements_simulated_stack--;
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
        try {
          switch(std::stoi(type)) {
            case 0:
              std::stoi(value);
              break;
            case 1:
              std::stoi(value);
              break;
            case 2:
              std::stoul(value);
              break;
            case 3:
              std::stof(value);
              break;
            case 4:
              std::stoll(value);
              break;
            }
           }
            catch(std::out_of_range e) {
              if (value[0] == '-') {
                throw std::string("Underflow");
              }
              else {
                throw std::string("Overflow");
              }
            }
          this->nbr_elements_simulated_stack++;
        }


      return result;
    }

    void print_parsed() {
      std::queue<std::string> ParsedQueue = this->getParsedQueue();
      while(!ParsedQueue.empty()) {
        std::cout<< ParsedQueue.front() << std::endl;
        ParsedQueue.pop();
      }
    }

    int getLineNbr() {
      return this->line_nbr;
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

class Executor {
  private:
    std::stack<IOperand *> stack_container;
    IOperandFactory factory;
    int line_nbr;
  public:

     void execute_it (std::queue<std::string> ParsedQueue) {
       std::vector<std::string> instruction;
       this->line_nbr = 0;

       while(!ParsedQueue.empty()) {
         this->line_nbr++;
         instruction = split_string(ParsedQueue.front(), '-');
         if (!strcmp(instruction[0].c_str(), "push")) {
           Executor::push_it(instruction[1], instruction[2]);
         }
         else if (!strcmp(instruction[0].c_str(), "pop")) {
           Executor::pop_it();
         }
         else if (!strcmp(instruction[0].c_str(), "dump")) {
           Executor::dump_it();
         }
         else if (!strcmp(instruction[0].c_str(), "assert")) {
           Executor::assert_it(instruction[1], instruction[2]);
         }
         else if (!strcmp(instruction[0].c_str(), "add")) {
           IOperand *v1 = this->stack_container.top();
           this->stack_container.pop();
           IOperand *v2 = this->stack_container.top();
           this->stack_container.pop();

           this->stack_container.push(*v2 + *v1);
         }
         else if (!strcmp(instruction[0].c_str(), "sub")) {
           IOperand *v1 = this->stack_container.top();
           this->stack_container.pop();
           IOperand *v2 = this->stack_container.top();
           this->stack_container.pop();

           this->stack_container.push(*v2 - *v1);
         }
         else if (!strcmp(instruction[0].c_str(), "mul")) {
           IOperand *v1 = this->stack_container.top();
           this->stack_container.pop();
           IOperand *v2 = this->stack_container.top();
           this->stack_container.pop();

           this->stack_container.push(*v2 * *v1);
         }
         else if (!strcmp(instruction[0].c_str(), "div")) {
           IOperand *v1 = this->stack_container.top();
           if (std::stoull(v1->toString()) == 0 ||
               std::stof(v1->toString()) == 0 || 
               std::stod(v1->toString()) == 0) {
             throw std::string("Division by zero."); 
           }
           this->stack_container.pop();
           IOperand *v2 = this->stack_container.top();
           this->stack_container.pop();

           this->stack_container.push(*v2 / *v1);
         }
         else if (!strcmp(instruction[0].c_str(), "mod")) {
           IOperand *v1 = this->stack_container.top();
           if (std::stoull(v1->toString()) == 0 ||
               std::stof(v1->toString()) == 0 || 
               std::stod(v1->toString()) == 0) {
             throw std::string("Mod division by zero.");
           }
           this->stack_container.pop();
           IOperand *v2 = this->stack_container.top();
           this->stack_container.pop();

           this->stack_container.push(*v2 % *v1);
         }
         else if (!strcmp(instruction[0].c_str(), "print")) {
           Executor::print_it();
         }
         ParsedQueue.pop();
       }
     }
     
     void push_it (const std::string s_type, 
                   const std::string s_value) {
       IOperand *element = NULL;
       eOperandType type = mapType(s_type);

       element = factory.createOperand(type, s_value);
       this->stack_container.push(element);
     }
     
     void pop_it() {
       this->stack_container.pop();
     }

     void dump_it() {
       std::stack<IOperand *> copy_of_stack = this->stack_container;
       while (!copy_of_stack.empty()) {
         std::cout << copy_of_stack.top()->toString() << std::endl;
         copy_of_stack.pop();
       }
     }

     void assert_it (const std::string s_type, 
                   const std::string s_value) {
       eOperandType type = mapType(s_type);
       IOperand *assert_element = factory.createOperand(type, s_value);
       try {
         if (type != this->stack_container.top()->getType()) {
           throw "Not same type!";
         }
         if (type < 3) {
           if (std::stoull(assert_element->toString()) != 
               std::stoull(this->stack_container.top()->toString())) {
             throw "Not same value!";
           }
         }
         else if (type == Float) {
           if (std::stof(assert_element->toString()) != 
               std::stof(this->stack_container.top()->toString())) {
             throw "Not same value!";
           }
         }
         else if (type == Double) {
           if (std::stod(assert_element->toString()) != 
               std::stod(this->stack_container.top()->toString())) {
             throw "Not same value!";
           }
         }
       }
       catch(const char *e) {
         std::cout << e << std::endl;
       }
     }

     void print_it () {
       if (this->stack_container.top()->getType() == Int8) {
         std::cout << (char)std::stoi(this->stack_container.top()->toString()) << std::endl;
       }  
     }

    ~Executor() {}

     int getLineNbr() {
       return this->line_nbr;
     }
};

//*************************************** 
/*
*  MAIN
*
****************************************/

int main(int ac, char **av) {
  int *arg_types = check_if_program_file(ac, av);

  if (!arg_types || arg_types[0] == NO_PARAMS) {
    std::cout << "No instructions passed" << std::endl;
    return -1;
  }
  
  for (int index = 0; index < (ac - 1); index++) {
    //Instantiate objects;
    Lexer lx;
    Parser ps;
    //LEXER
    try {
      if (arg_types[index] == PROGRAM_FILE) {
        std::ifstream p_file;
        p_file.open(av[index + 1]);
        lx.lex_it(p_file);
        p_file.close();
      }
      else if (arg_types[index] == FROM_STDIN) {
        std::string str(av[index + 1]);
        lx.lex_it(str);
      }
    }
    catch(std::string e) {
      std::cout << "Line " << lx.getLineNbr() << ": Error : " << e << std::endl;
      return 1;
    }
    //PARSER
    try {
      ps.parse_it(lx.getLexedQueue());
    }
    catch(std::string e) {
      std::cout << "Line " << ps.getLineNbr() << ": Error : " << e << std::endl;
      return 1;
    }

    Executor ex;
    try {
      ex.execute_it(ps.getParsedQueue());
    }
    catch(std::string e) {
      std::cout << "Line " << ex.getLineNbr() << ": Error : " << e << std::endl;
    }
   }

  return 0;
}
