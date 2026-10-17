BINDIR= ./build/bin

install:
	cmake -S . -B build
	cmake --build build --config Release

clean:
	cmake --build build --target clean

distclean:
	rm -rf build
