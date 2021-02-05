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

void handleNewStringWait (std::string *lastToken, std::string *token)
{
  
}

std::string getContent (std::ifstream &infile)
{
  std::string content( (std::istreambuf_iterator<char>(infile) ),
		       (std::istreambuf_iterator<char>(      ) ) );
  // replace spaces with newlines
  std::string newline = "\n";
  std::string space = " ";
  size_t pos = 0;
  while ((pos = content.find(space)) != std::string::npos)
    content.replace(pos, 1, newline);
  content.append("\n");
  // remove redundant newlines
  pos = 1;
  while (pos < content.size())
    {
      if (content.substr(pos - 1, 2) == newline + newline)
	{
	  content.erase(pos, 1);
	}
      else
	{
	  pos++;
	}
    }
  return content;
}

float getSongLength (std::string content)
{
  // search for waits and add
  // https://stackoverflow.com/questions/14265581/parse-split-a-string-in-c-using-string-delimiter-standard-c
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
	len += 44100 * std::stoi (token);
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
  infile.open(argv[1] + std::string(".ma"));
  outfile.open(argv[1] + std::string(".raw"), std::ios::out | std::ios::binary | std::ios::trunc);
  if (!infile)
    std::cout << "No input file :(\n";
  if (!outfile)
    std::cout << "Failed output file :(\n";
  content = getContent (infile);
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
      else if (lastToken == "w")
	{
	  int goal = datai + std::stoi (token) * (float) rate;
	  while (datai < goal)
	    {
	      data[datai] = data[datai - 1] + freq / (float) rate;
	      if (data[datai] > .5)
		data[datai] -= 1.0;
	      data[datai];
	      datai++;
	    }
	}
      posM = posL;
      posL = pos;
    }
  for (int i = 0; i < len; i++)
    {
      //outfile << data[i];
      outfile.write ( reinterpret_cast<char*>(&data[i]), sizeof(data[i]));
    }
  outfile.close();
  infile.close();
  std::cout << "Data:" << std::endl;
  for (int i = 0; i < 10; i++)
    std::cout << data[i] << std::endl;
  delete [] data;
  return 0;
}
