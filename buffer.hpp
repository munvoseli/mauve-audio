
size_t getSongLength (const std::string& content, const size_t bc, const MauveBuffer *buffers, const int rate)
{
	// search for waits and add
	size_t pos = 0;
	size_t posL = 0;
	size_t posM = 0;
	size_t len = 0; // number of samples
	float tempo = 1;
	std::string lastToken;
	std::string token;
	std::string newline = "\n";
	while ((pos = content.find(newline, posL)) != std::string::npos)
	{
		pos++;
		lastToken = content.substr(posM, posL - posM - 1);
		token = content.substr(posL, pos - posL - 1);
		if (lastToken == "w")
			len += rate * (std::stof (token) / tempo);
		else if (lastToken == "usebuffer")
		{
			len += buffers[getIndexByName(token, bc, buffers)].bufferLength;
		}
		else if (lastToken == "bps")
		{
			tempo = std::stof (token);
		}
		posM = posL;
		posL = pos;
	}
	return len;
}


void handleWait (float *&data, size_t &datai, const NoteInfo noteInfo,
		 const int len, const std::string &token)
{
	int start = datai;
	int attackGoal = std::min(start + noteInfo.attackLength, len - 1);
	int goal = datai + std::stof (token) / noteInfo.tempo * (float) noteInfo.rate;
	int releaseGoal = std::max(start, goal - noteInfo.releaseLength);
	while (datai < goal)
	{
		data[datai] = data[datai - 1] + noteInfo.vol * noteInfo.freq / (float) noteInfo.rate;
		if (data[datai] > noteInfo.vol / 2.0)
			data[datai] -= noteInfo.vol;
		data[datai];
		++datai;
	}
	printf ("handleWait: freq = %f Hz\n", noteInfo.freq);
	float i = 0;
	// now, datai == goal.
	while (datai > releaseGoal)
	{
		data[datai] *= i / (float) noteInfo.releaseLength;
		--datai;
		++i;
	}
	datai = start;
	i = 0;
	while (datai < attackGoal)
	{
		data[datai] *= i / (float) noteInfo.attackLength;
		++datai;
		++i;
	}
	datai = goal;
}

void handleUsebuffer (float *&data, size_t &datai, const size_t bc, const MauveBuffer *buffers, const std::string &token, const float vol)
{
	size_t index = getIndexByName (token, bc, buffers);
	size_t i = 0;
	while (i < buffers[index].bufferLength)
	{
		data[datai] = buffers[index].data[i] * vol;
		++datai;
		++i;
	}
}

void handleTokens (const std::string &lastToken, const std::string &token,
		   float *&data, int len,
		   size_t &datai,
		   NoteInfo &noteInfo,
		   const size_t bc, const MauveBuffer *buffers)
{
	int rem;
	if (false)
	{}
        else if (lastToken == "p")
	{
		rem = std::stoi (token, 0, 12);
		noteInfo.pitch = std::round( (float) (noteInfo.pitch - rem) / 12.0) * 12 + rem;
		noteInfo.freq = 440.0 * std::pow (2.0, ((float) noteInfo.pitch) / 12.0);
	}
        else if (lastToken == "po")
	{
		if (token == "+")
			noteInfo.pitch += 12;
		else if (token == "-")
			noteInfo.pitch -= 12;
		else
			printf ("Unrecognized po parameter");
		noteInfo.freq = 440.0 * std::pow (2.0, ((float) noteInfo.pitch) / 12.0);
	}
	else if (lastToken == "al")
		noteInfo.attackLength = noteInfo.rate * std::stof (token) / noteInfo.tempo;
	else if (lastToken == "rl")
	{
		noteInfo.releaseLength = noteInfo.rate * std::stof (token);
	}
	else if (lastToken == "v")
	{
		noteInfo.vol = std::stof (token);
	}
	else if (lastToken == "w")
	{
		handleWait (data, datai, noteInfo, len, token);
	}
	else if (lastToken == "bps")
	{
		noteInfo.tempo = std::stof (token);
	}
	else if (lastToken == "usebuffer")
	{
		handleUsebuffer (data, datai, bc, buffers, token, noteInfo.vol);
	}
}

void evaluateMauveBuffer (MauveBuffer &buffer, const size_t bc, const MauveBuffer *buffers)
{
	int pitch = 0;
	size_t datai = 1;
	NoteInfo noteInfo;
	size_t pos = 0;
	size_t posL = 0;
	size_t posM = 0;
	size_t len = getSongLength (buffer.content, bc, buffers, noteInfo.rate) + 1;
	buffer.bufferLength = len;
	buffer.data = new float [len];
	std::string lastToken;
	std::string token;
	while ((pos = buffer.content.find("\n", posL)) != std::string::npos)
	{
		++pos;
		lastToken = buffer.content.substr(posM, posL - posM - 1);
		token = buffer.content.substr(posL, pos - posL - 1);
		handleTokens (lastToken, token, buffer.data, len, datai, noteInfo, bc, buffers);
		posM = posL;
		posL = pos;
	}
}

bool bufferDependenciesMet (size_t testi, size_t bc, MauveBuffer *buffers)
{
	bool met = true;
	size_t pos = 0, poe;
	std::string name;
	size_t ind;
	while ((pos = buffers[testi].content.find("usebuffer", pos)) != std::string::npos)
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
	printf ("evaluateBuffers: bufferCount is %d\n", bufferCount);
	for (i = 0; i < bufferCount; ++i)
	{
		printf ("evaluateBuffers: buffer %s\n%s\n", buffers[i].name.c_str(), buffers[i].content.c_str());
	}
	while (!allBuffersLoaded)
	{
		for (i = 0; i < bufferCount; ++i)
		{
			if (!buffers[i].calculated && bufferDependenciesMet (i, bufferCount, buffers))
			{
				// calculate buffer
				printf ("Evaluating buffer %s\n", buffers[i].name.c_str());
				evaluateMauveBuffer (buffers[i], bufferCount, buffers);
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
	printf ("evaluateBuffers: Buffer address (evalBuf): %p\n", buffers);
	printf ("Data address for buffer 0: %p\n", buffers[0].data);
	return buffers[0].data;
}
