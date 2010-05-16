#ifndef GDIBRUSH_H
#define GDIBRUSH_H

// Copyright (c) 2009 Charles Bailey

#include <windows.h>
#include "mminfo.h"

namespace GDI
{

class Brush
{
public:
	explicit Brush(COLORREF colour)
		: _brush(::CreateSolidBrush(colour))
	{
		if (_brush == NULL)
			throw MemMon::ConstructorFailure<Brush>();
	}

	~Brush()
	{
		::DeleteObject (_brush);
	}

	operator HBRUSH() const { return _brush; }

private:
	HBRUSH _brush;
};

}

#endif
