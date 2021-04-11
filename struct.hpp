typedef struct {
	std::string name;
	std::string content;
	bool calculated = false;
	//bool lengthLoaded = false;
	size_t bufferLength;
	//bool dataLoaded = false;
	float *data;
} MauveBuffer;

typedef struct {
	int rate = 44100;
	int attackLength = 0;
	int releaseLength = 0;
	float vol = 1;
	float freq = 440;
	int pitch = 0;
	float tempo = 1; // bps
} NoteInfo;
