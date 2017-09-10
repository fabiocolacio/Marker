build: clean
	gcc src/*.c -c -o /build/objects
	gcc src/hoedown/*.c -c -o /build/objects
	

clean:
	rm -rf build
	mkdir -p build/objects
