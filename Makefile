PROJ=main

${PROJ}: main.c
	gcc -o ${PROJ} ${PROJ}.c

windows: main.c
	gcc -o ${PROJ}.exe ${PROJ}.c
