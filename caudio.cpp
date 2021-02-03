#include <iostream>
#include <fstream>
#include <vector>
#include <iterator>
#include <regex>

// ffplay -f s16le -ar 44100 -ac 1 p.raw
// ffplay -formats

void handleNewStringWait (std::string *lastToken, std::string *token)
{
  
}

float getSongLength (std::string filename)
{
  std::ifstream infile;
  infile.open(filename + std::string(".ma"));
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

  // search for waits and add
  // https://stackoverflow.com/questions/14265581/parse-split-a-string-in-c-using-string-delimiter-standard-c
  pos = 0;
  size_t posL = 0;
  size_t posM = 0;
  int len = 0; // number of samples
  std::string lastToken;
  std::string token;
  while ((pos = content.find(newline, posL + 1)) != std::string::npos)
    {
      pos++;
      lastToken = content.substr(posM, posL - posM - 1);
      token = content.substr(posL, pos - posL - 1);
      if (lastToken == "w")
	len += 44100 * std::stoi (token);
      //std::cout << token << std::endl;
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
  //std::cout << "hello" << std::endl;
  getSongLength(argv[1]);
  infile.open(argv[1] + std::string(".ma"));
  outfile.open(argv[1] + std::string(".raw"), std::ios::out | std::ios::binary);
  //int data [44100] = { };
  int a = 400;
  int ar = 44100;
  int freqs [16];
  int louds [16];
  int notes = 0;
  int len = getSongLength(argv[1]);
  int *data = new int[len];
  if (!infile)
      std::cout << ":(\n";
  else
    {
      // while (getline (infile, line) )
      // 	{
      // 	  //std::cout << line << "\n";
      // 	  outfile << line << "\n";
      // 	}
      while (getline (infile, line) )
	{
	  if (std::string(line).substr(0, 4) == " ")
	    {
	      std::string wtimestr = std::string(line).substr (4);
	    }
	}
      for (int i = 0; i < len; i++)
	{
	  data[i] = ((i % 100) * a / 100);
	  outfile << data[i];
	}
      outfile.close();
      infile.close();
    }
  /*for (int i = 1; i < argc; i++)
    std::cout << argv[i] << "\n";*/
  delete [] data;
  return 0;
}
