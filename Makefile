LDFLAGS = -lm -lSDL3 -lSDL3_image

game: main.c
	cc -std=c11 main.c -o a.out $(LDFLAGS)
	./a.out

clean:
	rm -f a.out

