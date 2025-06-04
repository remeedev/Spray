file := main.exe

src_files := ./src/main.c ./src/drawing.c ./src/movement.c ./src/images.c ./src/log.c

FLAGS := -lgdi32 -lz -lmsimg32

revised:
	gcc -o $(file) $(src_files) $(FLAGS)

run: revised
	$(file)