#ifndef GDISWITCHER_H
#define GDISWITCHER_H

#include <windows.h>
#include "mminfo.h"

namespace GDI
{

class Switcher
{
public:
	Switcher(HDC hdc, HGDIOBJ hobj)
		: _hdc(hdc)
		, _hobj(::SelectObject(_hdc, hobj))
	{
		if (_hobj == NULL)
			throw MemMon::ConstructorFailure<Switcher>();
	}

	~Switcher()
	{
		::SelectObject(_hdc, _hobj);
	}

private:
	HDC _hdc;
	HGDIOBJ _hobj;
};

}

#endif//GDISWITCHER_H
