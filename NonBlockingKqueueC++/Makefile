NAME1 = webserv
NAME2 = testclient

SRCS1 = main.cpp NonBlockingServer.cpp Client.cpp Errors.cpp
OBJS1 = $(SRCS1:%.cpp=obj/%.o)

SRCS2 = TestClient.cpp
OBJS2 = $(SRCS2:%.cpp=obj/%.o)

CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98

all: $(NAME1) $(NAME2)

$(NAME1): $(OBJS1)
	$(CXX) $(CXXFLAGS) -o $(NAME1) $(OBJS1)

$(NAME2): $(OBJS2)
	$(CXX) $(CXXFLAGS) -o $(NAME2) $(OBJS2)

obj/%.o: %.cpp | obj
	$(CXX) $(CXXFLAGS) -c $< -o $@

obj:
	mkdir -p obj

clean:
	rm -f $(OBJS1) $(OBJS2)

fclean: clean
	rm -f $(NAME1) $(NAME2)
	rm -rf obj

re: fclean all

.PHONY: all clean fclean re