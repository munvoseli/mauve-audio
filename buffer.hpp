
size_t getSongLength (const std::string& content)
{
  // search for waits and add
  size_t pos = 0;
  size_t posL = 0;
  size_t posM = 0;
  size_t len = 0; // number of samples
  std::string lastToken;
  std::string token;
  std::string newline = "\n";
  while ((pos = content.find(newline, posL + 1)) != std::string::npos)
    {
      pos++;
      lastToken = content.substr(posM, posL - posM - 1);
      token = content.substr(posL, pos - posL - 1);
      if (lastToken == "w")
	len += 44100 * std::stof (token);
      posM = posL;
      posL = pos;
    }
  std::cout << std::to_string(len) << std::endl;
  return len;
}

size_t getBufferLength (const std::string &content,
			const std::string &bufferName)
{
  size_t len;
  size_t aepos_bufcon [2];
  getBufferContentBounds (content, bufferName, aepos_bufcon);
  len = getSongLength (content.substr (aepos_bufcon[0],
				       aepos_bufcon[1] - aepos_bufcon[0]));
  return len;
}

void handleWait (float *data, int &datai, const int rate,
		 const float vol, const float freq
		 const int attackLength, const int releaseLength)
{
  int start = datai;
  int attackGoal = std::min(start + attackLength, len - 1);
  int goal = datai + std::stof (token) * (float) rate;
  int releaseGoal = std::max(start, goal - releaseLength);
  while (datai < goal)
    {
      data[datai] = data[datai - 1] + vol * freq / (float) rate;
      if (data[datai] > vol / 2.0)
	data[datai] -= vol;
      data[datai];
      datai++;
    }
  float i = 0;
  // now, datai == goal.
  // if releaseLength == 0
  // then releaseGoal == goal (unless start > goal),
  // then no loop occurs
  while (datai > releaseGoal)
    {
      data[datai] *= i / (float) releaseLength;
      datai--;
      i++;
    }
  datai = start;
  i = 0;
  while (datai < attackGoal)
    {
      data[datai] *= i / (float) attackLength;
      datai++;
      i++;
    }
  datai = goal;
}

void handleTokens (const std::string &lastToken, const std::string &token,
		   float *data, int len)
{
  switch (lastToken)
    {
    case "p":
      int rem = std::stoi (token, 0, 12);
      pitch = std::round( (float) (pitch - rem) / 12.0) * 12 + rem;
      std::cout << pitch << std::endl;
      freq = 440.0 * std::pow (2.0, ((float) pitch) / 12.0);
      break;
    case "po":
      if (token == "+")
	pitch += 12;
      else if (token == "-")
	pitch -= 12;
      else
	std::cout << "Unrecognized po parameter" << std::endl;
      freq = 440.0 * std::pow (2.0, ((float) pitch) / 12.0);
      break;
    case "al":
      attackLength = rate * std::stof (token);
      break;
    case "rl":
      releaseLength = rate * std::stof (token);
      break;
    case "v":
      vol = std::stof (token);
      break;
    case "w":
      handleWait (data, datai);
    }
}
void evaluateBuffer (std::string &content, float *data, int len)
{
  //float *data = new float[len];
  int pitch = 0;
  int datai = 1;
  int attackLength = 0;
  int releaseLength = 0;
  float vol = 1;
  float freq = 440;
  size_t pos = 0;
  size_t posL = 0;
  size_t posM = 0;
  std::string lastToken;
  std::string token;
  while ((pos = content.find(newline, posL + 1)) != std::string::npos)
    {
      pos++;
      lastToken = content.substr(posM, posL - posM - 1);
      token = content.substr(posL, pos - posL - 1);
      
      posM = posL;
      posL = pos;
    }
};

bool bufferLengthDependenciesMet (size_t testi, MauveBuffer *buffers)
{
  size_t pos = 0, poe;
  while ((pos = buffers[testi].content.find("usebuffer", pos + 1)) != std::string::npos)
    {
      pos = buffers[testi].content.find("\n", pos);
      while (buffers[testi].content[pos] == '\n')
	pos++;
      poe = buffers[testi].find("\n", pos);
    }
}

size_t getIndexByName (const std::string &name, size_t bc, MauveBuffer *buffers)
{
  for (size_t i = 0; i < bc; i++)
    if (buffers[i].name == name)
      return i;
}

void evaluateBuffers (const std::string &content, int rate) // macros have been evaluated
{
  int bufferCount = getBufferCount (content);
  MauveBuffer *buffers = new MauveBuffer [bufferCount];
  // load the buffer names into the MauveBuffers
  loadBufferNamesAndContent (content, bufferCount, buffers);
  // try and get the buffer lengths to load
  delete [] buffers; // may cause memory leak. if memory leak occurs, look at this
}
