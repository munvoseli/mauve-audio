// This is for getting metadata-like information about buffers

int getBufferCount (const std::string &content)
{
	int c = 0;
	size_t pos = 0;
	while ((pos = content.find("defbuffer", pos)) != std::string::npos)
	{
		++c; // do not roll credits and do not collect 200
		pos += 9;
	}
	return c;
}

void loadBufferNamesAndContent (const std::string &content, int bufferCount, MauveBuffer *buffers)
{
	int i;
	size_t pos = 0;
	size_t pos_name, poe_name;
	size_t pos_next;
	// store buffer names
	for (i = 0; i < bufferCount; ++i)
	{
		pos = content.find ("defbuffer", pos);
		pos_name = content.find ("\n", pos);
		while (content[pos_name] == '\n')
			++pos_name;
		poe_name = content.find ("\n", pos_name);
		buffers[i].name = content.substr (pos_name, poe_name - pos_name);
		pos_next = content.find ("defbuffer", poe_name);
		if (pos_next == std::string::npos)
			buffers[i].content = content.substr (poe_name);
		else
			buffers[i].content = content.substr (poe_name, pos_next - poe_name);
		pos = pos_next;
	}
}

void loadPhraseIndices (MauveBuffer &buffer)
{
	size_t pos = 0;
	size_t i = 0;
	size_t *ai_phrase;
	while (true)
	{
		pos = buffer.content.find("\n", pos) + 1;
		while (buffer.content[pos] == '\n')
			++pos;
		if (pos == buffer.content.length())
			break;
		else if (pos > buffer.content.length())
			printf ("loadPhraseIndices: idk anymore");
		++buffer.phrasec;
	}
	ai_phrase = new size_t [buffer.phrasec + 1];
	ai_phrase[buffer.phrasec] = buffer.content.length();
	pos = 0;
	while (true)
	{
		pos = buffer.content.find("\n", pos) + 1;
		while (buffer.content[pos] == '\n')
			++pos;
		if (pos == buffer.content.length())
			break;
		else if (pos > buffer.content.length())
			printf ("loadPhraseIndices: idk anymore");
		ai_phrase[i] = pos;
		++i;
	}
	buffer.phrases = new std::string [buffer.phrasec];
	for (i = 0; i < buffer.phrasec; ++i)
	{
		buffer.phrases[i] = buffer.content.substr(ai_phrase[i], ai_phrase[i + 1] - ai_phrase[i] - 1);
	}
}

void loadAllPhrases (size_t bc, MauveBuffer *&buffers)
{
	for (size_t i = 0; i < bc; ++i)
		loadPhraseIndices (buffers[i]);
}

size_t getIndexByName (const std::string &name, const size_t bc, const MauveBuffer *buffers)
{
	for (size_t i = 0; i < bc; i++)
		if (buffers[i].name == name)
			return i;
	return -1;
}

size_t cCommandArgs (const std::string &command) // includes command; (w 1) => "w"->2
{
	if (command == "w" ||
	    command == "timego" ||
	    command == "time" ||
	    command == "usebuffer" ||
	    command == "al" ||
	    command == "rl" ||
	    command == "po" ||
	    command == "p" ||
	    command == "v" ||
	    command == "bps")
		return 2;
	printf ("commandToNargs: Command %s not found\n", command.c_str());
	return 1;
}

size_t nDataAtTimestamp (MauveBuffer &buffer, const std::string &sName, const size_t nSearchLength)
{
	// going backwards; want to make sure that most recent timestamp is used
	for (size_t nTimestamp = nSearchLength - 1; nTimestamp >= 0; --nTimestamp)
	{
		if (sName == buffer.asTimestamp[nTimestamp])
			return buffer.anTimestamp[nTimestamp];
	}
	//printf ("nDataAtNominalTime: AAAAH timestamp %s not found", sName.c_str());
	return 0;
}

int iPitchFromNumberRound (const std::string &str, const NoteInfo &noteInfo)
{
	int rem = std::stoi (str, 0, 12);
	return std::round( (float) (noteInfo.pitch - rem) / 12.0) * 12 + rem;
}

int iPitchFromString (const std::string &str, const NoteInfo &noteInfo)
{
	size_t split = str.find (":");
	if (split == std::string::npos || split == 0)
		return iPitchFromNumberRound (str, noteInfo);
	else
		return std::stoi (str.substr (0, split), nullptr, 12) * 12
		     + std::stoi (str.substr (split + 1), nullptr, 12);
}
