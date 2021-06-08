#pragma once

#include <cmath>

void loadTimestampCount (const size_t bc, const MauveBuffer *buffers, MauveBuffer &buffer)
{
	size_t nPhrase = 0;
	size_t cTimestamp = 0;
	while (nPhrase < buffer.phrasec)
	{
		if (buffer.phrases[nPhrase] == "time")
			++cTimestamp;
		else if (buffer.phrases[nPhrase] == "usebuffer")
			cTimestamp += buffers[getIndexByName(buffer.phrases[nPhrase + 1], bc, buffers)].cTimestamp;
		nPhrase += cCommandArgs (buffer.phrases[nPhrase]);
	}
	buffer.cTimestamp = cTimestamp;
	buffer.anTimestamp = new size_t [cTimestamp];
	buffer.asTimestamp = new std::string [cTimestamp];
}

// assumes that timestamp arrays have already been counted and allocated, but not filled, with loadTimestampCount
void loadBufferLengthAndTimestamps (const size_t bc, const MauveBuffer *buffers, MauveBuffer &buffer, const int rate)
{
	size_t ngData = 0;
	size_t nData = 0;
	size_t nPhrase = 0;
	size_t nTimestamp = 0;
	float fTempo = 1;
	while (nPhrase < buffer.phrasec)
	{
		if (buffer.phrases[nPhrase] == "w")
			nData += nDataDelta (buffer.phrases[nPhrase + 1], rate, fTempo);
		else if (buffer.phrases[nPhrase] == "bps")
			fTempo = fGetFloatMaybeFraction (buffer.phrases[nPhrase + 1]);
		else if (buffer.phrases[nPhrase] == "usebuffer")
		{
			const MauveBuffer &buf = buffers[getIndexByName(buffer.phrases[nPhrase + 1], bc, buffers)];
			// load timestamps from remote buffer
			for (size_t n = 0; n < buf.cTimestamp; ++n)
			{
				buffer.asTimestamp[nTimestamp] = buf.asTimestamp[n];
				buffer.anTimestamp[nTimestamp] = buf.anTimestamp[n] + nData;
				++nTimestamp;
			}
			nData += buf.bufferLength;
		}
		else if (buffer.phrases[nPhrase] == "time")
		{
			buffer.asTimestamp[nTimestamp] = buffer.phrases[nPhrase + 1];
			buffer.anTimestamp[nTimestamp] = nData;
			++nTimestamp;
		}
		else if (buffer.phrases[nPhrase] == "timego")
		{
			nData = nDataAtTimestamp (buffer, buffer.phrases[nPhrase + 1], nTimestamp);
		}
		if (nData > ngData)
			ngData = nData;
		nPhrase += cCommandArgs (buffer.phrases[nPhrase]);
	}
	ngData += 1;
	buffer.bufferLength = ngData;
	buffer.data = new float [ngData];
	for (size_t i = 0; i < ngData; ++i)
		buffer.data[i] = 0;
}

// works like the unit fr in CSS
// should be positive
// could be negative, with rounding error, but should only be rounding error
// if it's too negative, something is wrong
size_t cSamplePerFreeVV (const NoteInfo &noteInfo, const int &cNoteSampleTotal)
{
	int len = 0;
	float cFree = 0;
	float fSpanValue = 0;
	for (size_t nSpan = 0; nSpan < noteInfo.cDynamicLength; ++nSpan)
	{
		fSpanValue = noteInfo.aDynamicLength [nSpan];
		switch (noteInfo.aluDynamic [nSpan])
		{
		case 'b': // beats
			len += noteInfo.rate * (fSpanValue / noteInfo.tempo);
			break;
		case 'n': // fraction of this note
			len += cNoteSampleTotal * fSpanValue;
			break;
		case 's': // seconds
			len += noteInfo.rate * fSpanValue;
			break;
		case 'f': // free
			cFree += fSpanValue;
		}
	}
	// just return 0 if it would be negative
	if (len > cNoteSampleTotal)
		return 0;
	else
		return ((cNoteSampleTotal - len) / cFree);
}

void vApplyVV (MauveBuffer &buffer, const NoteInfo &noteInfo, const int start, const int goal, const size_t cSample)
{
	size_t cSampleFree = cSamplePerFreeVV (noteInfo, cSample);
	size_t nSample = start;
	size_t nSpanGoal;
	size_t nSpanStart;
	size_t dnSample;
	size_t cSampleSpan;
	float fSpanValue;
	bool bBreakAfter;
	// printf ("vApplyVV: %d %d %ld %ld\n", start, goal, noteInfo.cDynamicLength, cSampleFree);
	for (size_t nSpan = 0; nSpan < noteInfo.cDynamicLength; ++nSpan)
	{
		bBreakAfter = false;
		fSpanValue = noteInfo.aDynamicLength [nSpan];
		if (fSpanValue == 0)
			continue;
		switch (noteInfo.aluDynamic [nSpan])
		{
		case 'b': cSampleSpan = noteInfo.rate * (fSpanValue / noteInfo.tempo); break;
		case 'n': cSampleSpan = cSample * fSpanValue; break;
		case 's': cSampleSpan = noteInfo.rate * fSpanValue; break;
		case 'f': cSampleSpan = cSampleFree * fSpanValue;
		}
		nSpanGoal = cSampleSpan + nSample;
		if (nSpanGoal > goal)
		{
			bBreakAfter = true;
			nSpanGoal = goal;
		}
		nSpanStart = nSample;
		float voldif = noteInfo.aDynamicVolume [nSpan + 1] - noteInfo.aDynamicVolume [nSpan];
		while (nSample < nSpanGoal)
		{
			// use linear interpolation
			float fLerpFactor = ((float) (nSample - nSpanStart)) / (float) cSampleSpan;
			buffer.data [nSample] *= noteInfo.aDynamicVolume [nSpan] + fLerpFactor * voldif;
			++nSample;
		}
		if (bBreakAfter)
			break;
	}
	while (nSample < goal)
	{
		buffer.data [nSample] *= noteInfo.aDynamicVolume [noteInfo.cDynamicLength];
		++nSample;
	}
}

// to do: break into smaller functions
void handleWait (MauveBuffer &buffer, size_t &datai, const NoteInfo &noteInfo,
		 const int len, const std::string &token)
{
	//float cfSample = ((float) noteInfo.rate) * (fGetFloatMaybeFraction (token) / noteInfo.tempo);
	// length of note in samples
	size_t cSample = nDataDelta (token, noteInfo.rate, noteInfo.tempo);
	int start = datai;
	int goal = datai + cSample;
	size_t nSample;
	float tempSample;
	buffer.data[0] = 0;
	// for (size_t np = 0; np < noteInfo.cPitch; ++np)
	// {
	// 	tempSample = 0;
	// 	for (nSample = datai; nSample < goal; ++nSample)
	// 	{
	// 		tempSample += noteInfo.vol * noteInfo.aFreq[np] / (float) noteInfo.rate;
	// 		if (tempSample * 2 > noteInfo.vol)
	// 			tempSample -= noteInfo.vol;
	// 		buffer.data[nSample] += tempSample / 16;
	// 	}
	// }
	//float cSampleWavelength;
	float fProgressPerSample;
	float fracWavelengthProgress;
	for (size_t np = 0; np < noteInfo.cPitch; ++np)
	{
		// samples/sec  /  cycles/sec  = samples/cycle
		//cSampleWavelength = noteInfo.rate / noteInfo.aFreq [np];
		fProgressPerSample = noteInfo.aFreq [np] / noteInfo.rate;
		fracWavelengthProgress = 0;
		for (nSample = datai; nSample < goal; ++nSample)
		{
			if (fracWavelengthProgress < .45)
				tempSample = -noteInfo.vol;
			else if (fracWavelengthProgress < .55)
			{
				tempSample = (float) std::lerp ((float) -noteInfo.vol, noteInfo.vol, (fracWavelengthProgress - (float) .45) / ((float) .1));
			}
			else
				tempSample = noteInfo.vol;
			buffer.data [nSample] += tempSample / 16;
			fracWavelengthProgress += fProgressPerSample;
			fracWavelengthProgress = fracWavelengthProgress - (char) fracWavelengthProgress;
		}
	}
	// printf ("handleWait: doing %ld pitches\n", noteInfo.cPitch);
	// printf ("handleWait: frequency is %f\n", noteInfo.aFreq[0]);
	// printf ("handleWait: pitch is %d\n", noteInfo.aPitch[0]);
	vApplyVV (buffer, noteInfo, start, goal, cSample);
	datai = goal;
}

void handleUsebuffer (const size_t bc, const MauveBuffer *buffers, MauveBuffer &buffer, NoteInfo &noteInfo)
{
	size_t index = getIndexByName (buffer.phrases[noteInfo.nPhrase + 1], bc, buffers);
	const MauveBuffer &buf = buffers[index];
	size_t i = 0;
	while (i < buf.bufferLength)
	{
		buffer.data[noteInfo.nData] += buf.data[i] * noteInfo.vol;
		++noteInfo.nData;
		++i;
	}
	noteInfo.nTimestamp += buf.cTimestamp;
}

void handleTimego (const size_t bc, const MauveBuffer *buffers, MauveBuffer &buffer, NoteInfo &noteInfo)
{
	noteInfo.nData = nDataAtTimestamp (buffer, buffer.phrases[noteInfo.nPhrase + 1], noteInfo.nTimestamp);
}

void handleTokens (MauveBuffer &buffer,
		   size_t &datai, NoteInfo &noteInfo,
		   const size_t bc, const MauveBuffer *buffers)
{
	const std::string &lastToken = buffer.phrases[noteInfo.nPhrase];
	const std::string &token = buffer.phrases[noteInfo.nPhrase + 1];
	const size_t len = buffer.bufferLength;
	if (false)
	{}
        else if (lastToken == "p")
	{
		//noteInfo.pitch = iPitchFromString (token, noteInfo);
		vLoadPitches (token, noteInfo);
	}
        else if (lastToken == "po")
	{
		if (token == "+")
			noteInfo.pitch += 12;
		else if (token == "-")
			noteInfo.pitch -= 12;
		else
			printf ("Unrecognized po parameter");
		noteInfo.freq = 440.0 * std::pow (2.0, ((float) noteInfo.pitch) / 12.0);
	}
	else if (lastToken == "al")
		noteInfo.attackLength = nDataDelta (token, noteInfo.rate, noteInfo.tempo);
	else if (lastToken == "rl")
		noteInfo.releaseLength = nDataDelta (token, noteInfo.rate, noteInfo.tempo);
	else if (lastToken == "v")
		noteInfo.vol = std::stof (token);
	else if (lastToken == "vv")
		vLoadDynamicsInfo (token, noteInfo);
	else if (lastToken == "bps")
		noteInfo.tempo = fGetFloatMaybeFraction (token);
	else if (lastToken == "w")
		handleWait (buffer, noteInfo.nData, noteInfo, len, token);
	else if (lastToken == "usebuffer")
		handleUsebuffer (bc, buffers, buffer, noteInfo);
	else if (lastToken == "timego")
		handleTimego (bc, buffers, buffer, noteInfo);
	else if (lastToken == "time")
		++noteInfo.nTimestamp;
}

void evaluateMauveBuffer (MauveBuffer &buffer, const size_t bc, const MauveBuffer *buffers)
{
	int pitch = 0;
	size_t datai = 1;
	NoteInfo noteInfo;
	loadTimestampCount (bc, buffers, buffer);
	loadBufferLengthAndTimestamps (bc, buffers, buffer, 44100);
	std::string lastToken;
	std::string token;
	printf ("%ld %ld\n", noteInfo.nPhrase, buffer.phrasec);
	while (noteInfo.nPhrase < buffer.phrasec)
	{
		//printf ("%s\n", buffer.phrases[noteInfo.nPhrase].c_str());
		handleTokens (buffer, datai, noteInfo, bc, buffers);
		noteInfo.nPhrase += cCommandArgs (buffer.phrases[noteInfo.nPhrase]);
	}
}

bool bufferDependenciesMet (size_t testi, size_t bc, MauveBuffer *buffers)
{
	bool met = true;
	size_t pos = 0, poe;
	std::string name;
	size_t ind;
	while ((pos = buffers[testi].content.find("usebuffer", pos)) != std::string::npos)
	{
		pos = buffers[testi].content.find("\n", pos);
		while (buffers[testi].content[pos] == '\n')
			pos++;
		poe = buffers[testi].content.find("\n", pos);
		name = substring (buffers[testi].content, pos, poe);
		ind = getIndexByName (name, bc, buffers);
		if (!buffers[ind].calculated)
			return false;
	}
	return true;
}

float* evaluateBuffers (const std::string &content, int rate, MauveBuffer *&buffers) // macros have been evaluated
{
	int bufferCount = getBufferCount (content);
	buffers = new MauveBuffer [bufferCount]; // collected in main
	// load the buffer names into the MauveBuffers
	loadBufferNamesAndContent (content, bufferCount, buffers);
	loadAllPhrases (bufferCount, buffers);
	size_t i, j;
	// try and get each buffer to load
	bool allBuffersLoaded = false;
	printf ("evaluateBuffers: bufferCount is %d\n", bufferCount);
	for (i = 0; i < bufferCount; ++i)
	{
		printf ("evaluateBuffers: buffer %s\n", buffers[i].name.c_str());
		for (j = 0; j < buffers[i].phrasec; ++j)
			printf ("%s ", buffers[i].phrases[j].c_str());
		printf ("\n");
	}
	while (!allBuffersLoaded)
	{
		for (i = 0; i < bufferCount; ++i)
		{
			if (!buffers[i].calculated && bufferDependenciesMet (i, bufferCount, buffers))
			{
				// calculate buffer
				printf ("Evaluating buffer %s\n", buffers[i].name.c_str());
				evaluateMauveBuffer (buffers[i], bufferCount, buffers);
				buffers[i].calculated = 1;
			}
		}
		allBuffersLoaded = true;
		for (i = 0; i < bufferCount; ++i)
		{
			if (!buffers[i].calculated)
			{
				allBuffersLoaded = false;
				break;
			}
		}
	}
	printf ("evaluateBuffers: Buffer address (evalBuf): %p\n", buffers);
	printf ("Data address for buffer 0: %p\n", buffers[0].data);
	printf("Data: ");
	for (int i = 0; i < 10; i++)
		printf("%f ", buffers[0].data[i]);
	printf("\n");
	return buffers[0].data;
}
