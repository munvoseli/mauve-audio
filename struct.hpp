typedef struct {
	std::string name;
	std::string content;
	bool calculated = false;
	//bool lengthLoaded = false;
	size_t bufferLength;
	//bool dataLoaded = false;
	float *data;
} MauveBuffer;
