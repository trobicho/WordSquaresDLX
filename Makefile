CC = gcc
CFLAGS = -Wall -Wextra -g
SRCS_NAME = main.c dlx.c dlxAllocator.c
HDRS_NAME = dlx.c dlxAllocator.c

SRCS_DIR = srcs
OBJS_DIR = objs

SRCS = $(addprefix $(SRCS_DIR)/, $(SRCS_NAME))
HDRS = $(addprefix $(SRCS_DIR)/, $(HDRS_NAME))
OBJS = $(addprefix $(OBJS_DIR)/, $(SRCS_NAME:%.c=%.o))

NAME = wordSquaresDLX

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(OBJS) $(CFLAGS) $(LFLAGS) -o $(NAME)

$(OBJS_DIR)/%.o: $(SRCS_DIR)/%.c
	$(CC) $(IFLAGS) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re 
