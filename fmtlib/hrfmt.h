#ifndef HRFMT_H
#define HRFMT_H

namespace HRFormat
{

size_t hr_format(char* buffer, size_t bufsize, int t);
size_t hr_format(char* buffer, size_t bufsize, size_t t);
size_t hr_format(wchar_t* buffer, size_t bufsize, int t);
size_t hr_format(wchar_t* buffer, size_t bufsize, size_t t);

}

#endif/*HRFMT_H*/
