file := sprayz.exe

src_files := ./src/main.c ./src/drawing.c ./src/movement.c ./src/images.c ./src/log.c ./src/animations.c

FLAGS := -lgdi32 -lz -lmsimg32

sprayz:
	gcc -o $(file) $(src_files) $(FLAGS)

run: sprayz
	$(file)