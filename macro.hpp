
std::string substring (const std::string &str, size_t pos, size_t poe)
{
	if (poe == std::string::npos)
		return str.substr (pos, std::string::npos);
	else
		return str.substr (pos, poe - pos);
}





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
	printf ("ERROR: Couldn't find macro %s", bufferName.c_str());
	return false;
}

size_t findUsemacroPos (std::string &content, std::string &macroName, size_t &pos_name, size_t &poe_name)
{
	size_t pos_usemacro = 0;
	while ((pos_usemacro = content.find("usemacro", pos_usemacro)) != std::string::npos)
	{
		printf ("hi");
		pos_name = content.find('\n', pos_usemacro + 8);
		while (content[pos_name] == '\n')
			++pos_name;
		poe_name = content.find('\n', pos_name);
		if ( content.substr ( pos_name, poe_name - pos_name ) == macroName )
			return pos_usemacro;
		pos_usemacro = poe_name;
	}
	return std::string::npos;
}

void applyMacro (std::string& content, std::string& macroName, std::string& macroContent)
{
	size_t pos_usemacro, pos_name, poe_name;
	while ((pos_usemacro = findUsemacroPos(content, macroName, pos_name, poe_name)) != std::string::npos)
	{
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
