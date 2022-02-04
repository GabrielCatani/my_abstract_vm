CC=g++
FLAGS=-Wall -Wextra -Wall
DEBUG= -g3 -fsanitize=address
TARGET=avm
SRC=./my_abstract_vm.cpp

all: $(TARGET)

$(TARGET): $(SRC)
	@$(CC) $(FLAGS) $(DEBUG) $< -o $@

fclean:
	@/bin/rm -rf $(TARGET) ./avm.dSYM

re: fclean $(TARGET)

.PHONY: all fclean re
