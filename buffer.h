int getBufferCount (const std::string &content)
{
  int c = 0;
  size_t pos;
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

void evaluateBuffers (const std::string &content) // macros have been evaluated
{
  int bufferCount = getBufferCount (content);
  std::string *bufferNames = new std::string [bufferCount];
  int *bufferLengths = new int [bufferCount];
  int *bufferIndices = new int [bufferCount];
  int i;
  size_t pos = -1;
  size_t pos_name, poe_name;
  for (i = 0; i < bufferCount; ++i)
    {
      pos = content.find ("defbuffer", pos + 1);
      pos_name = content.find ("\n", pos);
      while (content[pos_name] == '\n')
	pos_name++;
      poe_name = content.find ("\n", pos_name);
    }
  delete [] bufferNames;
  delete [] bufferLengths;
}
