#ifndef MMSOURCE_H
#define MMSOURCE_H

// Copyright (c) 2008 Charles Bailey

class MMPrefs;

namespace MemMon
{

class MemoryMap;

struct CPUPrefs
{
	CPUPrefs()
	: k(2.0)
	, damper(4.0)
	, use_cpu_count(true)
	{
	}

	double k;
	double damper;
	bool use_cpu_count;
};

class Source
{
public:
	virtual ~Source() {}
	virtual size_t Update( MemoryMap& ) = 0;
	virtual bool Poll( double, const CPUPrefs& ) = 0;
	virtual double GetPos() const = 0;
	virtual bool IsPlaybackDevice() const = 0;
};

}

#endif//MMSOURCE_H
