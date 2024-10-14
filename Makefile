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

EVENTDIR = EVENTS
EVENTSRCS = $(addprefix $(EVENTDIR)/, EventManager.cpp KqueueManager.cpp)
EVENTOBJS = $(addprefix $(OBJDIR)/, $(EVENTSRCS:$(EVENTDIR)/%.cpp=%.o))

CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98 -g3 -fsanitize=address

all: $(NAME1) $(NAME2)

$(NAME1): $(SERVEROBJS1) $(HTTPOBJS) $(EVENTOBJS)
	$(CXX) $(CXXFLAGS) -o $(NAME1) $(SERVEROBJS1) $(HTTPOBJS) $(EVENTOBJS)

$(NAME2): $(SERVEROBJS2)
	$(CXX) $(CXXFLAGS) -o $(NAME2) $(SERVEROBJS2)

$(OBJDIR)/%.o: $(SERVERDIR)/%.cpp | $(OBJDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJDIR)/%.o: $(HTTPDIR)/%.cpp | $(OBJDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJDIR)/%.o: $(EVENTDIR)/%.cpp | $(OBJDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJDIR):
	mkdir -p $(OBJDIR)

clean:
	rm -f $(SERVEROBJS1) $(SERVEROBJS2) $(HTTPOBJS) $(EVENTOBJS)

fclean: clean
	rm -f $(NAME1) $(NAME2)

re: fclean all

.PHONY: all clean fclean re
