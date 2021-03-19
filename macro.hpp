bool getMacroContentBounds (const std::string &content, const std::string &bufferName, size_t *aepos)
{
  size_t pos_defbuffer, pos_name, poe_name, pos_content, poe_content;
  while ((pos_defbuffer = content.find ("defmacro")) != std::string::npos)
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
  std::cout << "ERROR: Couldn't find macro " << bufferName << std::endl;
  return false;
}

void applyMacro (std::string& content, std::string& macroName, std::string& macroContent)
{
  size_t pos_usemacro, pos_name, poe_name;
  while ((pos_usemacro = content.find("usemacro")) != std::string::npos)
    {
      pos_name = content.find('\n', pos_usemacro);
      while (content[pos_name] == '\n')
	++pos_name;
      poe_name = content.find('\n', pos_name);
      if ( content.substr ( pos_name, poe_name - pos_name ) != macroName )
	continue;
      content.erase (pos_usemacro, poe_name - pos_usemacro);
      content.insert (pos_usemacro, macroContent.c_str());
    }
}

void applyMacros (std::string& content)
{
  size_t pos_defmacro,  pos_name, poe_name, poe_content;
  std::string macroName;
  std::string macroContent;
  while ((pos_defmacro = content.find ("defmacro")) != std::string::npos)
    {
      pos_name = content.find('\n', pos_defmacro);
      while (content[pos_name] == '\n')
	++pos_name;
      poe_name = content.find ('\n', pos_name);
      poe_content = content.find ("def", poe_name);
      macroName = content.substr (pos_name, poe_name - pos_name);
      macroContent = content.substr (poe_name, poe_content - poe_name);
      content.erase (pos_defmacro, poe_content - pos_defmacro);
      applyMacro (content, macroName, macroContent);
    }
}
