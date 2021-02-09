play:
	ffplay -f f32le -ar 44100 -ac 1 p.raw
all:
	g++ caudio.cpp -o caudio
	./caudio p
