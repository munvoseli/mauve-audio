all:
	g++ mauve.cpp -o mauve
	./mauve example
play:
	ffplay -f f32le -ar 44100 -ac 1 example.raw
