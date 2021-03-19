typedef struct {
  std::string name;
  std::string content;
  bool lengthLoaded = false;
  size_t bufferLength;
  bool dataLoaded = false;
  float *data;
} MauveBuffer;
