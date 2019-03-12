CFLAGS= -Wall -Werror --std=gnu++03 -pedantic -ggdb3


test: myShell.cpp parse.cpp variable.cpp
	g++ $(CFLAGS) -o myShell parse.cpp variable.cpp myShell.cpp
