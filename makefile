file := main.exe

src_files := ./src/main.c ./src/drawing.c ./src/movement.c ./src/images.c

FLAGS := -lgdi32 -lz -lmsimg32

revised:
	gcc -o $(file) $(src_files) $(FLAGS)

run: revised
	$(file)

exam: revised
	drmemory -suppress drmemory.supp -- $(file)

images:
	gcc -o images.exe ./src/images.c $(FLAGS)
	images.exe