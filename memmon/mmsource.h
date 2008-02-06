#ifndef MMSOURCE_H
#define MMSOURCE_H

// Copyright (c) 2008 Charles Bailey

class MMPrefs;

namespace MemMon
{

class MemoryMap;

class Source
{
public:
	virtual ~Source() {}
	virtual size_t Update( MemoryMap& ) = 0;
	virtual double Poll( MMPrefs* pPrefs ) = 0;
	virtual double GetPos() const = 0;
};

}

#endif//MMSOURCE_H
