#ifndef MMPAINTER_H
#define MMPAINTER_H

#include <windows.h>

struct Mem;

class MMPainter
{
public:
	MMPainter();
	~MMPainter();

	void Paint(HDC hdc) const;

	void DisplayGauge(HDC hdc) const;

	void SetProcessId(int p) { procid = p; }
	void Update();

	class Mem
	{
	public:
		Mem();
		~Mem();
		void Populate(int pid);

		size_t min;
		size_t max;

		struct Region
		{
			size_t base;
			size_t size;
			int type;
		};

		Region* head;
	};

	COLORREF GetColour(Mem::Region** preg, size_t base, size_t end) const;

private:
	Mem mem;
	HBRUSH hBrush;
	HBRUSH hWBrush;
	HPEN hPen;
	int procid;
};

#endif//MMPAINTER_H
