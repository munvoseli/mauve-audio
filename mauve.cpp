#include <iostream>
#include <fstream>
#include <vector>
#include <iterator>
#include <regex>
#include <cmath>
#include <cstdint>
// ffplay -f s16le -ar 44100 -ac 1 p.raw
// ffplay -formats
// ffplay -f f32le -ar 44100 -ac 1 p.raw

//     g++ caudio.cpp -o caudio; ./caudio p; ffplay -f f32le -ar 44100 -ac 1 p.raw

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
  size_t pos_defmacro,  pos_name, poe_name,  pos_content, poe_content;
  std::string macroName;
  std::string macroContent;
  while ((pos_defmacro = content.find ("defmacro")) != std::string::npos)
    {
      pos_name = content.find('\n', pos_defmacro);
      while (content[pos_name] == '\n')
	++pos_name;
      poe_name = content.find ('\n', pos_name);
      pos_content = content.find ("{", pos_name) + 1;
      poe_content = content.find ('}', pos_content);
      macroName = content.substr (pos_name, poe_name - pos_name);
      macroContent = content.substr (pos_content, poe_content - pos_content);
      content.erase (pos_defmacro, poe_content + 1 - pos_defmacro);
      applyMacro (content, macroName, macroContent);
    }
}

std::string getContent (std::ifstream& infile)
{
  std::string content( (std::istreambuf_iterator<char>(infile) ),
		       (std::istreambuf_iterator<char>(      ) ) );
  size_t pos = 0;
  std::string newline = "\n";
  std::string space = " ";
  // replace spaces with newlines
  while ((pos = content.find(space)) != std::string::npos)
    content.replace(pos, 1, newline);
  content.append("\n");
  // preprocess macros
  applyMacros (content);
  if (content.find("macro") != std::string::npos)
    std::cout << "ERROR: Recursive macro, probably, or something else. Program will not function." << std::endl;
  // remove redundant newlines
  pos = 1;
  while (pos < content.size())
    {
      if (content.substr(pos - 1, 2) == newline + newline)
	  content.erase(pos, 1);
      else
	  pos++;
    }
  std::cout << content << std::endl;
  return content;
}

float getSongLength (const std::string& content)
{
  // search for waits and add
  size_t pos = 0;
  size_t posL = 0;
  size_t posM = 0;
  int len = 0; // number of samples
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

int main (int argc, char **argv)
{
  std::ifstream infile;
  std::ofstream outfile;
  std::string line;
  std::string content;
  infile.open(argv[1] + std::string(".maud"));
  if (!infile)
    std::cout << "No input file :(\n";
  content = getContent (infile);
  infile.close();
  std::string newline = "\n";
  int a = 400;
  int rate = 44100;
  int freqs [16];
  int louds [16];
  int notes = 0;
  int len = getSongLength(content) + 1;
  float *data = new float[len];
  std::cout << sizeof(float) << std::endl;
  // while (getline (infile, line) )
  // 	{
  // 	  if (std::string(line).substr(0, 4) == " ")
  // 	    {
  // 	      std::string wtimestr = std::string(line).substr (4);
  // 	    }
  // 	}
  int pitch = 0;
  int datai = 1;
  int attackLength = 0;
  int releaseLength = 0;
  float vol = 1;
  float freq = 440;
  size_t pos = 0;
  size_t posL = 0;
  size_t posM = 0;
  std::string lastToken;
  std::string token;
  while ((pos = content.find(newline, posL + 1)) != std::string::npos)
    {
      pos++;
      lastToken = content.substr(posM, posL - posM - 1);
      token = content.substr(posL, pos - posL - 1);
      if (lastToken == "p")
	{
	  int rem = std::stoi (token, 0, 12);
	  pitch = std::round( (float) (pitch - rem) / 12.0) * 12 + rem;
	  std::cout << pitch << std::endl;
	  freq = 440.0 * std::pow (2.0, ((float) pitch) / 12.0);
	}
      if (lastToken == "po")
	{
	  if (token == "+")
	    pitch += 12;
	  else if (token == "-")
	    pitch -= 12;
	  else
	    std::cout << "Unrecognized po parameter" << std::endl;
	  freq = 440.0 * std::pow (2.0, ((float) pitch) / 12.0);
	}
      else if (lastToken == "al") // attack length
	{
	  attackLength = rate * std::stof (token);
	}
      else if (lastToken == "rl") // release length
	{
	  releaseLength = rate * std::stof (token);
	}
      else if (lastToken == "v") // volume
	{
	  vol = std::stof (token);
	}
      else if (lastToken == "w")
	{
	  int start = datai;
	  int attackGoal = std::min(start + attackLength, len - 1);
	  int goal = datai + std::stof (token) * (float) rate;
	  int releaseGoal = std::max(start, goal - releaseLength);
	  while (datai < goal)
	    {
	      data[datai] = data[datai - 1] + vol * freq / (float) rate;
	      if (data[datai] > vol / 2.0)
		data[datai] -= vol;
	      data[datai];
	      datai++;
	    }
	  float i = 0;
	  // now, datai == goal. If releaseLength == 0, then releaseGoal == goal (unless start > goal), then no loop occurs
	  while (datai > releaseGoal)
	    {
	      data[datai] *= i / (float) releaseLength;
	      datai--;
	      i++;
	    }
	  datai = start;
	  i = 0;
	  while (datai < attackGoal)
	    {
	      data[datai] *= i / (float) attackLength;
	      datai++;
	      i++;
	    }
	  datai = goal;
	}
      posM = posL;
      posL = pos;
    }
  // write data
  outfile.open(argv[1] + std::string(".raw"), std::ios::out | std::ios::binary | std::ios::trunc);
  if (!outfile)
    std::cout << "Failed output file :(\n";
  for (int i = 0; i < len; i++)
    {
      outfile.write ( reinterpret_cast<char*>(&data[i]), sizeof(data[i]));
    }
  outfile.close();
  std::cout << "Data:" << std::endl;
  for (int i = 0; i < 10; i++)
    std::cout << data[i] << std::endl;
  delete [] data;
  return 0;
}
