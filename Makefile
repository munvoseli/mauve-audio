all:
	g++ caudio.cpp -o caudio
	./caudio example
play:
	ffplay -f f32le -ar 44100 -ac 1 example.raw
