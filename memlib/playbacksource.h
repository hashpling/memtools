#ifndef PLAYBACKSOURCE_H
#define PLAYBACKSOURCE_H

// Copyright (c) 2008 Charles Bailey

#include "mmsource.h"
#include "memorymap.h"
#include <fstream>

namespace MemMon
{

class PlaybackSource : public MemMon::Source
{
public:
	PlaybackSource( const char* fname );
	~PlaybackSource();

	size_t Update( MemMon::MemoryMap& );
	bool Poll( double dtime, const MemMon::CPUPrefs& prefs );
	double GetPos() const;

private:
	std::filebuf _buf;
	MemMon::MemoryMap _mem;
};

}

#endif//PLAYBACKSOURCE_H
