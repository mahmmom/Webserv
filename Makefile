NAME1 = webserv
NAME2 = testclient

SERVERDIR = Server
OBJDIR = obj

HTTPDIR = HTTPRequest
HTTPSRCS = $(addprefix $(HTTPDIR)/, HTTPRequest.cpp)
HTTPOBJS = $(addprefix $(OBJDIR)/, $(HTTPSRCS:$(HTTPDIR)/%.cpp=%.o))

SERVERSRCS1 = $(addprefix $(SERVERDIR)/, main.cpp NonBlockingServer.cpp Client.cpp Errors.cpp)
SERVEROBJS1 = $(addprefix $(OBJDIR)/, $(SERVERSRCS1:$(SERVERDIR)/%.cpp=%.o))

SERVERSRCS2 = $(addprefix $(SERVERDIR)/, TestClient.cpp)
SERVEROBJS2 = $(addprefix $(OBJDIR)/, $(SERVERSRCS2:$(SERVERDIR)/%.cpp=%.o))

CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98

all: $(NAME1) $(NAME2)

$(NAME1): $(SERVEROBJS1) $(HTTPOBJS)
	$(CXX) $(CXXFLAGS) -o $(NAME1) $(SERVEROBJS1) $(HTTPOBJS)

$(NAME2): $(SERVEROBJS2)
	$(CXX) $(CXXFLAGS) -o $(NAME2) $(SERVEROBJS2)

$(OBJDIR)/%.o: $(SERVERDIR)/%.cpp | $(OBJDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJDIR)/%.o: $(HTTPDIR)/%.cpp | $(OBJDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJDIR):
	mkdir -p $(OBJDIR)

clean:
	rm -f $(SERVEROBJS1) $(SERVEROBJS2) $(HTTPOBJS)

fclean: clean
	rm -f $(NAME1) $(NAME2)

re: fclean all

.PHONY: all clean fclean re
