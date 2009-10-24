#ifndef GDIPEN_H
#define GDIPEN_H

#include <windows.h>
#include "mminfo.h"

namespace GDI
{

class Pen
{
public:
	explicit Pen(int style, int width, COLORREF colour)
		: _pen(::CreatePen(style, width, colour))
	{
		if (_pen == NULL)
			throw MemMon::ConstructorFailure<Pen>();
	}

	~Pen()
	{
		::DeleteObject(_pen);
	}

	operator HPEN() const { return _pen; }

private:
	HPEN _pen;
};

}

#endif
