SRC=main.c 

OBJSRC=$(SRC:.c=.o)
OBJDIR=obj

OBJ=$(addprefix $(OBJDIR)/, $(OBJSRC))
NAME=ft_traceroute
FLAG=-Wall -Wextra -Wunused -g

all: $(NAME)

$(NAME): $(OBJ)
	cc $(FLAG) -lm $(OBJ) -o $(NAME)

$(OBJDIR)/%.o: %.c
	mkdir -p obj
	cc $(FLAG) -c $< -o $@

clean:
	rm -rf $(OBJDIR)

fclean: clean
	rm -rf $(NAME)

re: fclean all


.PHONY: all clean fclean re