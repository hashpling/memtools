#ifndef PROCESSSOURCE_H
#define PROCESSSOURCE_H

#include "mmsource.h"

namespace MemMon
{

class MemoryMap;
struct CPUPrefs;

namespace Win
{

class ProcessSource : public MemMon::Source
{
public:
	ProcessSource( int pid );
	ProcessSource( const TCHAR* cmd, const TCHAR* args, const TCHAR* wd );

	~ProcessSource();

	size_t Update( MemMon::MemoryMap& );
	double Poll( const MemMon::CPUPrefs& prefs );
	double GetPos() const;

private:
	HANDLE _proc;

	double actual_u;
	double actual_k;

	double ind_pos;
	double ind_vel;

	double last_poll;
};


}

}

#endif//PROCESSSOURCE_H
