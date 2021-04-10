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

size_t getIndexByName (const std::string &name, size_t bc, const MauveBuffer *buffers)
{
	for (size_t i = 0; i < bc; i++)
		if (buffers[i].name == name)
			return i;
	return -1;
}
