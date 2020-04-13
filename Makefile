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

VALUE			= ""
################################################################################
### OBJECTS
################################################################################

OBJ_NAME_PROXY		= proxy.o						\
			  split.o						\
			  exec_cmds.o						\

OBJ			= $(addprefix $(OBJ_PATH_C)/,$(OBJ_NAME_PROXY))	\
################################################################################
### RULES
################################################################################

.PHONY: all clean fclean re

all: obj $(NAME)

obj:
	echo $(OBJ)
	@mkdir obj/

$(NAME): $(OBJ)
	$(CC) $(CFLAGS) $^ -o $@


$(OBJ_PATH_C)/%.o: $(SRC_PATH_C)/%.c Makefile
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	@rm -rf $(OBJ)

fclean: clean
	make -C rootkit clean
	@rm -rf obj
	@rm -rf $(NAME)

re: fclean all
