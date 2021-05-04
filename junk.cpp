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


size_t getBufferLength (const std::string &content,
			const std::string &bufferName,
			const size_t bc,
			const MauveBuffer *buffers)
{
	size_t len;
	size_t aepos_bufcon [2];
	getBufferContentBounds (content, bufferName, aepos_bufcon);
	len = getSongLength (content.substr (aepos_bufcon[0],
					     aepos_bufcon[1] - aepos_bufcon[0]),
			     bc, buffers);
	return len;
}




bool getBufferContentBounds (const std::string &content,
			     const std::string &bufferName, size_t *aepos)
{
	size_t pos_defbuffer, pos_name, poe_name, pos_content, poe_content;
	while ((pos_defbuffer = content.find ("defbuffer")) != std::string::npos)
	{
		pos_name = content.find ("\n", pos_defbuffer);
		while (content[pos_name] == '\n')
			pos_name++;
		poe_name = content.find ("\n", pos_name);
		if (content.substr(pos_name, poe_name - pos_name) != bufferName)
			continue;
		poe_content = content.find ("def", poe_name);
		return true;
	}
	std::cout << "ERROR: Couldn't find buffer " << bufferName << std::endl;
	return false;
}

bool doesBufferHaveDependency (const std::string &content,
			       const std::string &bufferName)
{
	size_t aepos_bufcon [2];
	size_t pos_usebuffer;
	// just assume buffer exists. if it doesn't, song won't work anyway
	getBufferContentBounds(content, bufferName, aepos_bufcon);
	pos_usebuffer = content.find ("usebuffer", aepos_bufcon[0]);
	// pos_use == npos == epos_bufcon -> buffer does not have dependency
	// pos_use == epos_bufcon -> false
	return pos_usebuffer < aepos_bufcon[1] || pos_usebuffer == std::string::npos;
}


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
