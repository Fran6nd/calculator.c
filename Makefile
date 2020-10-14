calculator:
	#-rm calculator
	gcc -g -Wextra -Wall main.c token.c token_list.c -o calculator -lm