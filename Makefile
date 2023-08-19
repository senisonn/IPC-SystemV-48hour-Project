NAME_INITIAL	=	initial
NAME_WAITER		=	waiter
NAME_COOKER		=	cooker
NAME_CLIENT		=	client

NAME_KEY		=	ipc_key_file

INCS_DIR		=	incs
SRCS_DIR		=	srcs
COMPILED_DIR	=	.objs

INCS_FILES		=	$(wildcard $(INCS_DIR)/*.h) $(wildcard $(INCS_DIR)/*/*.h) $(wildcard $(INCS_DIR)/*/*/*.h) $(wildcard $(INCS_DIR)/*/*/*/*.h)
SRCS_FILES		=	$(wildcard $(SRCS_DIR)/*.c) $(wildcard $(SRCS_DIR)/*/*.c) $(wildcard $(SRCS_DIR)/*/*/*.c) $(wildcard $(SRCS_DIR)/*/*/*/*.c)

CC				=	gcc
FLAGS			=	-Wall

all: $(NAME_INITIAL) $(NAME_WAITER) $(NAME_COOKER) $(NAME_CLIENT)

$(COMPILED_DIR)/%.o:$(SRCS_DIR)/%.c	$(INCS_FILES)
	@mkdir -p $(@D)
	@$(CC) $(FLAGS) -I $(INCS_DIR) -o $@ -c $<

$(NAME_INITIAL):	$(SRCS_DIR)/initial.c $(SRCS_DIR)/common.c $(INCS_FILES) $(COMPILED_DIR)/initial.o $(COMPILED_DIR)/common.o
	@$(CC) $(FLAGS) -I $(INCS_DIR) -o $(NAME_INITIAL) $(COMPILED_DIR)/initial.o $(COMPILED_DIR)/common.o

$(NAME_WAITER):		$(SRCS_DIR)/waiter.c $(SRCS_DIR)/common.c $(INCS_FILES) $(COMPILED_DIR)/waiter.o $(COMPILED_DIR)/common.o
	@$(CC) $(FLAGS) -I $(INCS_DIR) -o $(NAME_WAITER) $(COMPILED_DIR)/waiter.o $(COMPILED_DIR)/common.o

$(NAME_COOKER):		$(SRCS_DIR)/cooker.c $(SRCS_DIR)/common.c $(INCS_FILES) $(COMPILED_DIR)/cooker.o $(COMPILED_DIR)/common.o
	@$(CC) $(FLAGS) -I $(INCS_DIR) -o $(NAME_COOKER) $(COMPILED_DIR)/cooker.o $(COMPILED_DIR)/common.o

$(NAME_CLIENT):		$(SRCS_DIR)/client.c $(SRCS_DIR)/common.c $(INCS_FILES) $(COMPILED_DIR)/client.o $(COMPILED_DIR)/common.o
	@$(CC) $(FLAGS) -I $(INCS_DIR) -o $(NAME_CLIENT) $(COMPILED_DIR)/client.o $(COMPILED_DIR)/common.o

clean:
	@rm -rf $(COMPILED_DIR)

fclean: clean
	@rm -f $(NAME_INITIAL) $(NAME_WAITER) $(NAME_COOKER) $(NAME_CLIENT) $(NAME_KEY)

re:	fclean all

.PHONY: all clean fclean re test