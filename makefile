file := sprayz.exe

src_files := ./src/main.c ./src/drawing.c ./src/movement.c ./src/images.c ./src/log.c \
./src/animations.c ./src/level_loader.c ./src/npc.c ./src/generalVars.c ./src/console.c\
./src/console_cmd.c ./src/handler.c ./src/bicycle.c ./src/particles.c ./src/throwables.c

FLAGS := -lgdi32 -lz -lmsimg32

sprayz:
	gcc -o $(file) $(src_files) $(FLAGS)

run: sprayz
	$(file)