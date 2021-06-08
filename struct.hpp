#pragma once

// units for measuring things for the intranote dynamics
enum LengthUnit
{
	luBeats = 'b',   // b
	luFree = 'f',    // f
	luNotes = 'n',   // n
	luSeconds = 's'  // s
};

// information used for describing buffer as a whole, mostly static when evaluating buffer
typedef struct {
	std::string name;
	std::string content;
	size_t phrasec = 0;
	std::string *phrases;
	
	bool calculated = false;
	
	size_t bufferLength;
	float *data;
	
	size_t cTimestamp;
	size_t *anTimestamp;
	std::string *asTimestamp;
	bool *bTimestampCalced;
} MauveBuffer;

// information used for constructing notes, mostly dynamic when evaluating buffer
typedef struct {
	int rate = 44100; // move to MauveBuffer someday? used for constructing notes
	int attackLength = 0;
	int releaseLength = 0;
	float vol = 1;
	float freq = 440;
	int pitch = 0;
	float tempo = 1; // beats per second
	// n beats = tempo * m seconds
	// m seconds = n beats / tempo
	// m samples = n beats * rate / tempo
	
	size_t nData = 0;
	size_t nPhrase = 0;
	size_t nTimestamp = 0;
	
	size_t cPitch = 0;
	int aPitch [16];
	float aFreq [16];

	size_t cDynamicLength = 1; // min should be 1 in practice
	char      aluDynamic [16] = {'f'}; //  units for intranote dynamic interstop lengths
	float aDynamicLength [16] = {'1'}; // values for intranote dynamic interstop lengths
	float aDynamicVolume [16] = {1,1}; // values for intranote dynamic stop volumes
} NoteInfo;
