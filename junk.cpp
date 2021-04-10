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
