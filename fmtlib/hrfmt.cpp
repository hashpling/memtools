// Copyright (c) 2007 Charles Bailey
#include "stdafx.h"
#include "hrfmt.h"
#include <cstdio>
#include <cstdarg>

namespace
{

template <typename T>
inline int get_range(T t)
{
	int r = 0;
	while (t >= 1000)
	{
		++r;
		t /= 1024;
	}
	return r;
}

template <typename T>
inline int get_digits(T t)
{
	int r = 1;
	while (t >= 10)
	{
		++r;
		t /= 10;
	}
	return r;
}

template <typename T>
inline void split(T t, T& t1, T& t2, int range)
{
	if (range > 0)
	{
		range *= 10;
		t1 = t >> range;
		if (range > 10)
		{
			t2 = (((t - (t1 << range)) >> (range - 10)) * 1000) >> 10;
		}
		else
		{
			t2 = ((t - (t1 << range)) * 1000) >> range;
		}
	}
	else
	{
		t1 = t;
		t2 = 0;
	}
}

char symbols[] = { ' ', 'k', 'M', 'G', 'T' };

void mysprintf(char* buffer, size_t bufsize, const char* format, ...)
{
	va_list ap;
	va_start(ap, format);

	vsprintf(buffer, format, ap);

	va_end(ap);
}

void mysprintf(wchar_t* buffer, size_t bufsize, const wchar_t* format, ...)
{
	va_list ap;
	va_start(ap, format);

	vswprintf(buffer, bufsize, format, ap);

	va_end(ap);
}

#define FMTSTRING1 "%u bytes"
#define FMTSTRING2 "%u.%02u%c"
#define FMTSTRING3 "%u.%01u%c"
#define FMTSTRING4 "%u%c"

#define WIDE(x) L##x
#define WIDEN(x) WIDE(x)

const char* fmtstrings[] =
{
	FMTSTRING1,
	FMTSTRING2,
	FMTSTRING3,
	FMTSTRING4
};

const wchar_t* wfmtstrings[] =
{
	WIDEN(FMTSTRING1),
	WIDEN(FMTSTRING2),
	WIDEN(FMTSTRING3),
	WIDEN(FMTSTRING4)
};

template<typename C>
const C* GetFormatString(int n);

template<>
const char* GetFormatString<char>(int n)
{
	return fmtstrings[n];
}

template<>
const wchar_t* GetFormatString<wchar_t>(int n)
{
	return wfmtstrings[n];
}

template <typename T, typename C>
size_t format(C* buffer, size_t bufsize, T t)
{
	size_t required = 0;
	T t1, t2;
	int r = get_range(t);
	if (r > 4) r = 4;

	switch (r)
	{
	case 0:
		required = get_digits(t) + 7;
		if (bufsize >= required)
		{
			mysprintf(buffer, bufsize, GetFormatString<C>(0), t);
		}
		break;

	default:
		split(t, t1, t2, r);
		if (t1 < 10)
		{
			T tmp = (t2 + 5) / 10;
			while (tmp >= 100) { tmp -= 100; ++t1; }
			required = get_digits(t1) + 5;
			if (bufsize >= required)
			{
				mysprintf(buffer, bufsize, GetFormatString<C>(1), t1, tmp, symbols[r]);
			}
		}
		else if (t1 < 100)
		{
			T tmp = (t2 + 50) / 100;
			while (tmp >= 10) { tmp -= 10; ++t1; }
			required = get_digits(t1) + 4;
			if (bufsize >= required)
			{
				mysprintf(buffer, bufsize, GetFormatString<C>(2), t1, tmp, symbols[r]);
			}
		}
		else
		{
			T tmp = (t2 + 500) / 1000;
			while (tmp >= 1) { tmp -= 1; ++t1; }
			required = get_digits(t1) + 2;
			if (bufsize >= required)
			{
				mysprintf(buffer, bufsize, GetFormatString<C>(3), t1, symbols[r]);
			}
		}
		break;
	}

	return required;
}

}

size_t HRFormat::hr_format(char* buffer, size_t bufsize, int t)
{
	return format(buffer, bufsize, t);
}

size_t HRFormat::hr_format(char* buffer, size_t bufsize, size_t t)
{
	return format(buffer, bufsize, t);
}

size_t HRFormat::hr_format(wchar_t* buffer, size_t bufsize, int t)
{
	return format(buffer, bufsize, t);
}

size_t HRFormat::hr_format(wchar_t* buffer, size_t bufsize, size_t t)
{
	return format(buffer, bufsize, t);
}
