file := main

src_files := ./src/main.c ./src/drawing.c ./src/movement.c

FLAGS := -lgdi32

revised:
	gcc -o $(file) $(src_files) $(FLAGS)

run: revised
	$(file)