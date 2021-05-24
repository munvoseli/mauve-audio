#pragma once

// information used for describing buffer
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

// information used in filling in buffer
typedef struct {
	int rate = 44100;
	int attackLength = 0;
	int releaseLength = 0;
	float vol = 1;
	float freq = 440;
	int pitch = 0;
	float tempo = 1; // bps
	
	size_t nData = 0;
	size_t nPhrase = 0;
	size_t nTimestamp = 0;
	
	size_t cPitch = 0;
	int aPitch [16];
	float aFreq [16];
} NoteInfo;
