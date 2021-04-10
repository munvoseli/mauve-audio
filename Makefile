all:
	g++ -g mauve.cpp -o mauve
	./mauve example
play:
	ffplay -nodisp -f f32le -ar 44100 -ac 1 example.raw
clean:
	rm example.raw
	rm mauve
debug:
	gdb --args ./mauve example
