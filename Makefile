NAME = ircserv
CPP = c++
CPPFLAGS = -Wall -Wextra -Werror -std=c++98 #-pedantic-errors
SRCS = main.cpp Server.cpp User.cpp Commands.cpp Channel.cpp Utils.cpp

OBJS = $(SRCS:.cpp=.o)

all: $(NAME)

$(NAME): $(OBJS)
		$(CPP) $(CPPFLAGS) -o $(NAME) $(OBJS)

%.o: %.cpp
		$(CPP) $(CPPFLAGS) -c -o $@ $<

clean:
		rm -rf $(OBJS)

fclean: clean
		rm -rf $(NAME)

re: fclean all

.PHONY: all clean fclean re
