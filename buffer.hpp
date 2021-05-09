
void loadTimestampCount (const size_t bc, const MauveBuffer *buffers, MauveBuffer &buffer)
{
	size_t nPhrase = 0;
	size_t cTimestamp = 0;
	while (nPhrase < buffer.phrasec)
	{
		if (buffer.phrases[nPhrase] == "time")
			++cTimestamp;
		else if (buffer.phrases[nPhrase] == "usebuffer")
			cTimestamp += buffers[getIndexByName(buffer.phrases[nPhrase + 1], bc, buffers)].cTimestamp;
		nPhrase += cCommandArgs (buffer.phrases[nPhrase]);
	}
	buffer.cTimestamp = cTimestamp;
	buffer.anTimestamp = new size_t [cTimestamp];
	buffer.asTimestamp = new std::string [cTimestamp];
}

float fGetFloatAssumeFraction (const std::string &str, const size_t &split)
{
	return std::stof (str.substr(0,split)) / std::stof (str.substr(split+1));
}

float fGetFloatMaybeFraction (const std::string &str)
{
	size_t split = str.find("/");
	if (split == std::string::npos)
		return std::stof (str);
	else
		return fGetFloatAssumeFraction (str, split);
}

size_t nDataDelta (const std::string &str, const int &rate, const float &fTempo)
{
	return rate * (fGetFloatMaybeFraction (str) / fTempo);
}

// assumes that timestamp arrays have already been counted and allocated, but not filled, with loadTimestampCount
void loadBufferLengthAndTimestamps (const size_t bc, const MauveBuffer *buffers, MauveBuffer &buffer, const int rate)
{
	size_t ngData = 0;
	size_t nData = 0;
	size_t nPhrase = 0;
	size_t nTimestamp = 0;
	float fTempo = 1;
	while (nPhrase < buffer.phrasec)
	{
		if (buffer.phrases[nPhrase] == "w")
			nData += nDataDelta (buffer.phrases[nPhrase + 1], rate, fTempo);
		else if (buffer.phrases[nPhrase] == "bps")
			fTempo = fGetFloatMaybeFraction (buffer.phrases[nPhrase + 1]);
		else if (buffer.phrases[nPhrase] == "usebuffer")
		{
			const MauveBuffer &buf = buffers[getIndexByName(buffer.phrases[nPhrase + 1], bc, buffers)];
			// load timestamps from remote buffer
			for (size_t n = 0; n < buf.cTimestamp; ++n)
			{
				buffer.asTimestamp[nTimestamp] = buf.asTimestamp[n];
				buffer.anTimestamp[nTimestamp] = buf.anTimestamp[n] + nData;
				++nTimestamp;
			}
			nData += buf.bufferLength;
		}
		else if (buffer.phrases[nPhrase] == "time")
		{
			buffer.asTimestamp[nTimestamp] = buffer.phrases[nPhrase + 1];
			buffer.anTimestamp[nTimestamp] = nData;
			++nTimestamp;
		}
		else if (buffer.phrases[nPhrase] == "timego")
		{
			nData = nDataAtTimestamp (buffer, buffer.phrases[nPhrase + 1], nTimestamp);
		}
		if (nData > ngData)
			ngData = nData;
		nPhrase += cCommandArgs (buffer.phrases[nPhrase]);
	}
	ngData += 1;
	buffer.bufferLength = ngData;
	buffer.data = new float [ngData];
	for (size_t i = 0; i < ngData; ++i)
		buffer.data[i] = 0;
}

void handleWait (MauveBuffer &buffer, size_t &datai, const NoteInfo noteInfo,
		 const int len, const std::string &token)
{
	int start = datai;
	int attackGoal = std::min(start + noteInfo.attackLength, len - 1);
	int goal = datai + nDataDelta (token, noteInfo.rate, noteInfo.tempo);
	int releaseGoal = std::max(start, goal - noteInfo.releaseLength);
	while (datai < goal)
	{
		buffer.data[datai] = buffer.data[datai - 1] + noteInfo.vol * noteInfo.freq / (float) noteInfo.rate;
		if (buffer.data[datai] > noteInfo.vol / 2.0)
			buffer.data[datai] -= noteInfo.vol;
		buffer.data[datai];
		++datai;
	}
	printf ("handleWait: freq = %f Hz\n", noteInfo.freq);
	float i = 0;
	// now, datai == goal.
	while (datai > releaseGoal)
	{
		buffer.data[datai] *= i / (float) noteInfo.releaseLength;
		--datai;
		++i;
	}
	datai = start;
	i = 0;
	while (datai < attackGoal)
	{
		buffer.data[datai] *= i / (float) noteInfo.attackLength;
		++datai;
		++i;
	}
	datai = goal;
}

void handleUsebuffer (const size_t bc, const MauveBuffer *buffers, MauveBuffer &buffer, NoteInfo &noteInfo)
{
	size_t index = getIndexByName (buffer.phrases[noteInfo.nPhrase + 1], bc, buffers);
	const MauveBuffer &buf = buffers[index];
	size_t i = 0;
	while (i < buf.bufferLength)
	{
		buffer.data[noteInfo.nData] += buf.data[i] * noteInfo.vol;
		++noteInfo.nData;
		++i;
	}
	noteInfo.nTimestamp += buf.cTimestamp;
}

void handleTimego (const size_t bc, const MauveBuffer *buffers, MauveBuffer &buffer, NoteInfo &noteInfo)
{
	noteInfo.nData = nDataAtTimestamp (buffer, buffer.phrases[noteInfo.nPhrase + 1], noteInfo.nTimestamp);
}

void handleTokens (MauveBuffer &buffer,
		   size_t &datai, NoteInfo &noteInfo,
		   const size_t bc, const MauveBuffer *buffers)
{
	const std::string &lastToken = buffer.phrases[noteInfo.nPhrase];
	const std::string &token = buffer.phrases[noteInfo.nPhrase + 1];
	const size_t len = buffer.bufferLength;
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
		noteInfo.attackLength = nDataDelta (token, noteInfo.rate, noteInfo.tempo);
	else if (lastToken == "rl")
		noteInfo.releaseLength = nDataDelta (token, noteInfo.rate, noteInfo.tempo);
	else if (lastToken == "v")
		noteInfo.vol = std::stof (token);
	else if (lastToken == "bps")
		noteInfo.tempo = fGetFloatMaybeFraction (token);
	else if (lastToken == "w")
		handleWait (buffer, noteInfo.nData, noteInfo, len, token);
	else if (lastToken == "usebuffer")
		handleUsebuffer (bc, buffers, buffer, noteInfo);
	else if (lastToken == "timego")
		handleTimego (bc, buffers, buffer, noteInfo);
	else if (lastToken == "time")
		++noteInfo.nTimestamp;
}

void evaluateMauveBuffer (MauveBuffer &buffer, const size_t bc, const MauveBuffer *buffers)
{
	int pitch = 0;
	size_t datai = 1;
	NoteInfo noteInfo;
	loadTimestampCount (bc, buffers, buffer);
	loadBufferLengthAndTimestamps (bc, buffers, buffer, 44100);
	std::string lastToken;
	std::string token;
	printf ("%ld %ld\n", noteInfo.nPhrase, buffer.phrasec);
	while (noteInfo.nPhrase < buffer.phrasec)
	{
		//printf ("%s\n", buffer.phrases[noteInfo.nPhrase].c_str());
		handleTokens (buffer, datai, noteInfo, bc, buffers);
		noteInfo.nPhrase += cCommandArgs (buffer.phrases[noteInfo.nPhrase]);
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

float* evaluateBuffers (const std::string &content, int rate, MauveBuffer *&buffers) // macros have been evaluated
{
	int bufferCount = getBufferCount (content);
	buffers = new MauveBuffer [bufferCount]; // collected in main
	// load the buffer names into the MauveBuffers
	loadBufferNamesAndContent (content, bufferCount, buffers);
	loadAllPhrases (bufferCount, buffers);
	size_t i, j;
	// try and get each buffer to load
	bool allBuffersLoaded = false;
	printf ("evaluateBuffers: bufferCount is %d\n", bufferCount);
	for (i = 0; i < bufferCount; ++i)
	{
		printf ("evaluateBuffers: buffer %s\n", buffers[i].name.c_str());
		for (j = 0; j < buffers[i].phrasec; ++j)
			printf ("%s ", buffers[i].phrases[j].c_str());
		printf ("\n");
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
	printf("Data: ");
	for (int i = 0; i < 10; i++)
		printf("%f ", buffers[0].data[i]);
	printf("\n");
	return buffers[0].data;
}
