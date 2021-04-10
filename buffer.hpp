
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

void handleWait (float *&data, int &datai, const int rate,
		 const float vol, const float freq,
		 const int attackLength, const int releaseLength,
		 const int len, const std::string &token)
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
		++datai;
	}
	printf("handleWait: %f %f %d\n", data[5], vol, rate);
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
		   float *&data, int len,
		   int &pitch,
		   int &datai,
		   int &attackLength,
		   int &releaseLength,
		   float &vol,
		   float &freq,
		   int rate)
{
	int rem;
	if (false)
	{}
        else if (lastToken == "p")
	{
		rem = std::stoi (token, 0, 12);
		pitch = std::round( (float) (pitch - rem) / 12.0) * 12 + rem;
		std::cout << pitch << std::endl;
		freq = 440.0 * std::pow (2.0, ((float) pitch) / 12.0);
	}
        else if (lastToken == "po")
	{
		if (token == "+")
			pitch += 12;
		else if (token == "-")
			pitch -= 12;
		else
			std::cout << "Unrecognized po parameter" << std::endl;
		freq = 440.0 * std::pow (2.0, ((float) pitch) / 12.0);
	}
	else if (lastToken == "al")
		attackLength = rate * std::stof (token);
	else if (lastToken == "rl")
	{
		releaseLength = rate * std::stof (token);
	}
	else if (lastToken == "v")
	{
		vol = std::stof (token);
	}
	else if (lastToken == "w")
	{
		handleWait (data, datai, rate, vol, freq,
			    attackLength, releaseLength, len, token);
	}
}
void evaluateBuffer (std::string &content, float *data, int len)
{
	//float *data = new float[len];
	int pitch = 0;
	int datai = 1;
	int attackLength = 0;
	int releaseLength = 0;
	int rate = 44100;
	float vol = 1;
	float freq = 440;
	size_t pos = 0;
	size_t posL = 0;
	size_t posM = 0;
	std::string lastToken;
	std::string token;
	while ((pos = content.find("\n", posL + 1)) != std::string::npos)
	{
		pos++;
		lastToken = content.substr(posM, posL - posM - 1);
		token = content.substr(posL, pos - posL - 1);
		handleTokens (lastToken, token, data, len, pitch, datai,
			      attackLength, releaseLength, vol, freq, rate);
		posM = posL;
		posL = pos;
	}
};

void evaluateMauveBuffer (MauveBuffer &buffer)
{
	int pitch = 0;
	int datai = 1;
	int attackLength = 0;
	int releaseLength = 0;
	int rate = 44100;
	float vol = 1;
	float freq = 440;
	size_t pos = 0;
	size_t posL = 0;
	size_t posM = 0;
	size_t len = getSongLength (buffer.content) + 1;
	buffer.bufferLength = len;
	buffer.data = new float [len];
	std::string lastToken;
	std::string token;
	while ((pos = buffer.content.find("\n", posL + 1)) != std::string::npos)
	{
		pos++;
		lastToken = buffer.content.substr(posM, posL - posM - 1);
		token = buffer.content.substr(posL, pos - posL - 1);
		handleTokens (lastToken, token, buffer.data, len, pitch, datai,
			      attackLength, releaseLength, vol, freq, rate);
		posM = posL;
		posL = pos;
	}
}

size_t getIndexByName (const std::string &name, size_t bc, MauveBuffer *buffers)
{
	for (size_t i = 0; i < bc; i++)
		if (buffers[i].name == name)
			return i;
	return -1;
}

bool bufferDependenciesMet (size_t testi, size_t bc, MauveBuffer *buffers)
{
	bool met = true;
	size_t pos = 0, poe;
	std::string name;
	size_t ind;
	while ((pos = buffers[testi].content.find("usebuffer", pos + 1)) != std::string::npos)
	{
		pos = buffers[testi].content.find("\n", pos);
		while (buffers[testi].content[pos] == '\n')
			pos++;
		poe = buffers[testi].content.find("\n", pos);
		name = substring (buffers[testi].content, pos, poe);
		ind = getIndexByName (name, bc, buffers);
		if (!buffers[ind].calculated)
			return false;
	}
	return true;
}

// MauveBuffer *&buffers feels like it should be illegal but I want a reference to the pointer instead of a pointer to the pointer because personal preference?
float* evaluateBuffers (const std::string &content, int rate, MauveBuffer *&buffers) // macros have been evaluated
{
	int bufferCount = getBufferCount (content);
	buffers = new MauveBuffer [bufferCount]; // collected in main
	// load the buffer names into the MauveBuffers
	loadBufferNamesAndContent (content, bufferCount, buffers);
	// try and get each buffer to load
	bool allBuffersLoaded = false;
	size_t i, j;
	printf ("Hi%d\n", bufferCount);
	while (!allBuffersLoaded)
	{
		for (i = 0; i < bufferCount; ++i)
		{
			if (!buffers[i].calculated && bufferDependenciesMet (i, bufferCount, buffers))
			{
				// calculate buffer
				evaluateMauveBuffer (buffers[i]);
				buffers[i].calculated = 1;
			}
		}
		allBuffersLoaded = true;
		for (i = 0; i < bufferCount; ++i)
		{
			if (!buffers[i].calculated)
			{
				allBuffersLoaded = false;
				break;
			}
		}
	}
	printf ("Buffer address (evalBuf): %p\n", buffers);
	printf ("Data address for buffer 0: %p\n", buffers[0].data);
	for (int i = 0; i < 10; i++)
		printf("%f ", buffers[0].data[i]);
	printf("\n");
	return buffers[0].data;
}