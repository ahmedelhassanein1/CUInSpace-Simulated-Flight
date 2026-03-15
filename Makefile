TARGETS= proj2 main.o manager.o event.o display.o resource.o system.o

all: ${TARGETS}

main.o: main.c defs.h
		gcc -g -c -Wall -Wextra -pthread main.c

manager.o: manager.c defs.h
		gcc -g -c -Wall -Wextra -pthread manager.c

event.o: event.c defs.h
		gcc -g -c -Wall -Wextra -pthread event.c

display.o: display.c defs.h
		gcc -g -c -Wall -Wextra -pthread display.c

resource.o: resource.c defs.h
		gcc -g -c -Wall -Wextra -pthread resource.c

system.o: system.c defs.h
		gcc -g -c -Wall -Wextra -pthread system.c

proj2: main.o manager.o display.o event.o resource.o system.o
		gcc -g -Wall -Wextra -pthread main.o display.o event.o manager.o resource.o system.o -o proj2

clean:
	rm -f ${TARGETS}
