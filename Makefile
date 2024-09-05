SRCFILES=main.c utils.c option.c init.c packet.c

OBJSRC=$(SRCFILES:.c=.o)
OBJDIR=obj

SRCDIR=src
SRC=$(addprefix $(SRCDIR)/, $(SRCFILES))
OBJ=$(addprefix $(OBJDIR)/, $(OBJSRC))

NAME=ft_traceroute
FLAG=-Wall -Wextra -Werror

all: $(NAME)

$(NAME): $(OBJ)
	cc $(FLAG) -lm $(OBJ) -o $(NAME)

$(OBJDIR)/%.o: src/%.c
	mkdir -p obj
	cc $(FLAG) -c $< -o $@

clean:
	rm -rf $(OBJDIR)

fclean: clean
	rm -rf $(NAME)

re: fclean all


.PHONY: all clean fclean re