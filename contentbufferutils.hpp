// This is for getting metadata-like information about buffers

int getBufferCount (const std::string &content)
{
	printf ("%s", content.c_str());
	int c = 0;
	size_t pos = -1;
	while ((pos = content.find("defbuffer", pos + 1)) != std::string::npos)
		++c; // do not roll credits and do not collect 200
	return c;
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

void loadBufferNamesAndContent (const std::string &content, int bufferCount, MauveBuffer *buffers)
{
	int i;
	size_t pos = -1;
	size_t pos_name, poe_name;
	size_t pos_next;
	// store buffer names
	for (i = 0; i < bufferCount; ++i)
	{
		pos = content.find ("defbuffer", pos + 1);
		pos_name = content.find ("\n", pos);
		while (content[pos_name] == '\n')
			pos_name++;
		poe_name = content.find ("\n", pos_name);
		buffers[i].name = content.substr (pos_name, poe_name - pos_name);
		pos_next = content.find ("defbuffer", poe_name);
		if (pos_next == std::string::npos)
			buffers[i].content = content.substr (poe_name);
		else
			buffers[i].content = content.substr (poe_name, pos_next - poe_name);
	}
}

size_t getIndexByName (const std::string &name, size_t bc, const MauveBuffer *buffers)
{
	for (size_t i = 0; i < bc; i++)
		if (buffers[i].name == name)
			return i;
	return -1;
}
