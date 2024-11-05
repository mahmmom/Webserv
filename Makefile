NAME1 = webserv
NAME2 = testclient

SERVERDIR = Server
OBJDIR = obj

HTTPDIR = HTTP
HTTPSRCS = $(addprefix $(HTTPDIR)/, HTTPRequest.cpp HTTPResponse.cpp ResponseGenerator.cpp ResponseManager.cpp)
HTTPOBJS = $(addprefix $(OBJDIR)/, $(HTTPSRCS:$(HTTPDIR)/%.cpp=%.o))

# NonBlockingServer.cpp
SERVERSRCS1 = $(addprefix $(SERVERDIR)/, main.cpp Client.cpp Errors.cpp Server.cpp ServerArena.cpp)
SERVEROBJS1 = $(addprefix $(OBJDIR)/, $(SERVERSRCS1:$(SERVERDIR)/%.cpp=%.o))

SERVERSRCS2 = $(addprefix $(SERVERDIR)/, TestClient.cpp)
SERVEROBJS2 = $(addprefix $(OBJDIR)/, $(SERVERSRCS2:$(SERVERDIR)/%.cpp=%.o))

EVENTDIR = Events
EVENTSRCS = $(addprefix $(EVENTDIR)/, EpollManager.cpp EventManager.cpp KqueueManager.cpp)
EVENTOBJS = $(addprefix $(OBJDIR)/, $(EVENTSRCS:$(EVENTDIR)/%.cpp=%.o))

PARSERDIR = Parser
PARSERSRCS = $(addprefix $(PARSERDIR)/, ConfigNode.cpp ConfigParser.cpp ConfigTokenizer.cpp ContextNode.cpp \
					DirectiveNode.cpp LoadSettings.cpp SyntaxAuditor.cpp TreeAuditor.cpp TreeGenerator.cpp \
					MimeTypesSettings.cpp)
PARSEROBJS = $(addprefix $(OBJDIR)/, $(PARSERSRCS:$(PARSERDIR)/%.cpp=%.o))

CONFIGDIR = Settings
CONFIGSRCS = $(addprefix $(CONFIGDIR)/, BaseSettings.cpp LocationSettings.cpp ReturnDirective.cpp ServerSettings.cpp \
					CGIDirective.cpp)
CONFIGOBJS = $(addprefix $(OBJDIR)/, $(CONFIGSRCS:$(CONFIGDIR)/%.cpp=%.o))

LOGGERDIR = Logger
LOGGERSRCS = $(addprefix $(LOGGERDIR)/,Logger.cpp)
LOGGEROBJS = $(addprefix $(OBJDIR)/, $(LOGGERSRCS:$(LOGGERDIR)/%.cpp=%.o))

CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98 -g3 -fsanitize=address

all: $(NAME1) $(NAME2)

$(NAME1): $(SERVEROBJS1) $(HTTPOBJS) $(EVENTOBJS) $(PARSEROBJS) $(CONFIGOBJS) $(LOGGEROBJS)
	$(CXX) $(CXXFLAGS) -o $(NAME1) $(SERVEROBJS1) $(HTTPOBJS) $(EVENTOBJS) $(PARSEROBJS) $(CONFIGOBJS) $(LOGGEROBJS)

$(NAME2): $(SERVEROBJS2)
	$(CXX) $(CXXFLAGS) -o $(NAME2) $(SERVEROBJS2)

$(OBJDIR)/%.o: $(SERVERDIR)/%.cpp | $(OBJDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJDIR)/%.o: $(HTTPDIR)/%.cpp | $(OBJDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJDIR)/%.o: $(EVENTDIR)/%.cpp | $(OBJDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJDIR)/%.o: $(PARSERDIR)/%.cpp | $(OBJDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJDIR)/%.o: $(CONFIGDIR)/%.cpp | $(OBJDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJDIR)/%.o: $(LOGGERDIR)/%.cpp | $(OBJDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJDIR):
	mkdir -p $(OBJDIR)

clean:
	rm -f $(SERVEROBJS1) $(SERVEROBJS2) $(HTTPOBJS) $(EVENTOBJS) $(PARSEROBJS) $(CONFIGOBJS) $(LOGGEROBJS)

fclean: clean
	rm -f $(NAME1) $(NAME2)

re: fclean all

.PHONY: all clean fclean re
