#include <iostream>
#include <fstream>
#include <vector>
#include <iterator>
#include <cmath>
#include <cstdint>
#include "struct.hpp"
#include "macro.hpp"
#include "contentbufferutils.hpp"
#include "buffer.hpp"



// ffplay -f s16le -ar 44100 -ac 1 p.raw
// ffplay -formats
// ffplay -f f32le -ar 44100 -ac 1 p.raw

std::string getContent (std::ifstream& infile)
{
	std::string content( (std::istreambuf_iterator<char>(infile) ),
			     (std::istreambuf_iterator<char>(      ) ) );
	size_t pos = 0;
	std::string newline = "\n";
	std::string space = " ";
	// replace spaces with newlines
	while ((pos = content.find(space)) != std::string::npos)
	{
		content.replace(pos, 1, newline);
		++pos;
	}
	content.append("\n");
	// preprocess macros
	applyMacros (content);
	if (content.find("macro") != std::string::npos)
		printf("ERROR: Recursive macro, probably, or something else. Program will not function.\n");
	// remove redundant newlines
	pos = 1;
	while (pos < content.size())
	{
		if (content.substr(pos - 1, 2) == newline + newline)
			content.erase(pos, 1);
		else
			pos++;
	}
	return content;
}

int main (int argc, char **argv)
{
	printf ("main: entering main\n");
	std::ifstream infile;
	std::ofstream outfile;
	std::string line;
	std::string content;
	infile.open(argv[1] + std::string(".maud"));
	if (!infile)
		printf ("No input file :(\n");
	content = getContent (infile);
	infile.close();
	std::string newline = "\n";
	int a = 400;
	int rate = 44100;
	int freqs [16];
	int louds [16];
	int notes = 0;
	MauveBuffer *buffers;
	evaluateBuffers (content, rate, buffers);
	printf ("Buffer address (main) %p\n", buffers);
	outfile.open("out.raw", std::ios::out | std::ios::binary | std::ios::trunc);
	if (!outfile)
		printf ("Failed output file :(\n");
	for (int i = 0; i < buffers[0].bufferLength; i++)
	{
		outfile.write ( reinterpret_cast<char*>(&buffers[0].data[i]), sizeof(buffers[0].data[i]));
	}
	outfile.close();
	delete [] buffers;
	return 0;
}
