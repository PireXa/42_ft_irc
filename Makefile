SRCS	= main.cpp Server.cpp  commands.cpp User.cpp\

OBJS	=	$(addprefix objs/,$(SRCS:.cpp=.o))

CPPFLAGS	=	c++ -Wall -Wextra -Werror -g -std=c++98

RM		=	rm -f

NAME	=	ircserv

all:	$(NAME)

objs/%.o: srcs/%.cpp
	@mkdir -p objs
	$(CPPFLAGS) -c $< -o $@

$(NAME):	$(OBJS)
	$(CPPFLAGS) $(OBJS) -o $(NAME)

clean:
	@$(RM) $(OBJS)

fclean:	clean
	@$(RM) $(NAME)

re:	fclean all