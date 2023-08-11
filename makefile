
CLI_src=src/uxn.c src/devices/system.c src/devices/console.c src/devices/file.c src/devices/datetime.c 
EMU_src=${CLI_src} src/devices/screen.c src/devices/controller.c src/devices/mouse.c

RELEASE_flags=-DNDEBUG -O2 -g0 -s
DEBUG_flags=-std=c89 -D_POSIX_C_SOURCE=199309L -DDEBUG -Wall -Wno-unknown-pragmas -Wpedantic -Wshadow -Wextra -Werror=implicit-int -Werror=incompatible-pointer-types -Werror=int-conversion -Wvla -g -Og -fsanitize=address -fsanitize=undefined

.PHONY: all debug dest rom run test install uninstall format clean

all: dest uxnasm uxncli uxn11
debug: dest uxnasm-debug uxncli-debug uxn11-debug

dest:
	mkdir -p bin
rom:
	./bin/uxnasm etc/polycat.tal bin/polycat.rom

uxnasm: src/uxnasm.c
	cc ${RELEASE_flags} src/uxnasm.c -o bin/uxnasm 
uxncli: ${CLI_src} src/uxncli.c
	gcc ${RELEASE_flags} ${CLI_src} src/uxncli.c -o bin/uxncli
uxn11: ${EMU_src} src/uxn11.c
	gcc ${RELEASE_flags} ${EMU_src} src/uxn11.c -lX11 -o bin/uxn11 

uxnasm-debug: src/uxnasm.c
	cc ${DEBUG_flags} src/uxnasm.c -o bin/uxnasm 
uxncli-debug: ${CLI_src} src/uxncli.c
	gcc ${DEBUG_flags} ${CLI_src} src/uxncli.c -o bin/uxncli
uxn11-debug: ${EMU_src} src/uxn11.c
	gcc ${DEBUG_flags} ${EMU_src} src/uxn11.c -lX11 -o bin/uxn11 

run: uxnasm uxncli uxn11 rom
	./bin/uxn11 bin/polycat.rom
test: uxnasm uxncli uxn11
	./bin/uxnasm && ./bin/uxncli && ./bin/uxn11 && ./bin/uxnasm -v && ./bin/uxncli -v && ./bin/uxn11 -v
install: uxnasm uxncli uxn11
	cp bin/uxn11 bin/uxnasm bin/uxncli ~/bin/
uninstall:
	rm -f ~/bin/uxn11 ~/bin/uxnasm ~/bin/uxncli
format:
	clang-format -i src/uxnasm.c src/uxncli.c src/uxn11.c src/devices/*
clean:
	rm -f bin/uxnasm bin/uxncli bin/uxn11 bin/polycat.rom bin/polycat.rom.sym