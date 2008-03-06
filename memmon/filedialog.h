#ifndef FILEDIALOG_H
#define FILEDIALOG_H

namespace MemMon
{
namespace Win
{

bool RunSaveFileDialog( HWND hwnd, char* fnamebuf );
bool RunOpenFileDialog( HWND hwnd, char* fnamebuf );
bool RunSelectDirDialog( HWND hwnd, char* fnamebuf );

}
}

#endif//FILEDIALOG_H
