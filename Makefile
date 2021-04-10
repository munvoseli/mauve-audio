all:
	g++ -g mauve.cpp -o mauve
	./mauve example
play:
	ffplay -nodisp -f f32le -ar 44100 -ac 1 example.raw
clean:
ifneq (,$(wildcard ./example.raw))
	rm example.raw
endif
ifneq (,$(wildcard ./mauve))
	rm mauve
endif
debug:
	gdb --args ./mauve example
