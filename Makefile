################################################################################
### INIT
################################################################################

NAME			= Proxy

SRC_PATH_C		= src/

OBJ_PATH_C		= obj

INC_PATH		= inc/

CC			= gcc
AS			= nasm
CFLAGS			= -masm=intel -I $(INC_PATH) #-g3 -fsanitize=address -fno-sanitize-address-use-after-scope

################################################################################
### OBJECTS
################################################################################

OBJ_NAME_WAR		= proxy.o						\

OBJ			= $(addprefix $(OBJ_PATH_C)/,$(OBJ_NAME_WAR))	\

################################################################################
### RULES
################################################################################

.PHONY: all clean fclean re

all: obj $(NAME)

obj:
	@echo $(OBJ)
	@mkdir -p obj/

$(NAME): $(OBJ)
	$(CC) $(CFLAGS) $^ -o $@

$(OBJ_PATH_C)/%.o: $(SRC_PATH_C)/%.c Makefile
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	@rm -rf $(OBJ)

fclean: clean
	@rm -rf obj
	@rm -rf /tmp/logger
	@rm -rf $(NAME)

re: fclean all
