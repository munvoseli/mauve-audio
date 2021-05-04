all:
	g++ -g mauve.cpp
	./a.out timego
play:
	ffplay -nodisp -f f32le -ar 44100 -ac 1 out.raw
clean:
ifneq (,$(wildcard ./out.raw))
	rm out.raw
endif
ifneq (,$(wildcard ./a.out))
	rm a.out
endif
debug:
	gdb --args ./a.out torturetime
# ./mauve taupe; ffplay -nodisp -f f32le -ar 44100 -ac 1 taupe.raw
