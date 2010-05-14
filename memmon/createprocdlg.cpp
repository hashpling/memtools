// Copyright (c) 2008 Charles Bailey
#include "stdafx.h"
#include "createprocdlg.h"
#include "mmpainter.h"
#include "filedialog.h"
#include "resource.h"
#include <list>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <shlobj.h>
#include <iomanip>
#include <iterator>

using std::list;
using std::vector;
using std::basic_string;

namespace
{

class CPDlgContext
{
public:
	CPDlgContext( MMPainter* pPaint ) : _ppaint( pPaint ) {}

	typedef basic_string< TCHAR > tstring;
	typedef vector< TCHAR > tchvector;

	class Command
	{
	public:

		Command( tchvector::const_iterator
			   , tchvector::const_iterator
			   , tchvector::const_iterator
			   , tchvector::const_iterator);

		Command( const TCHAR *, const TCHAR *, const TCHAR * );

		bool operator==( const Command& other ) const
		{
			return _cmd == other._cmd && _arg == other._arg && _wd == other._wd;
		}

		bool operator!=( const Command& other ) const
		{
			return *this != other;
		}

		const tstring& GetCmd() const { return _cmd; }
		const tstring& GetArg() const { return _arg; }
		const tstring& GetWd() const { return _wd; }

		tstring MakeSummaryStr() const;

	private:
		tstring _cmd;
		tstring _arg;
		tstring _wd;
	};

	void Load();
	void Save( const Command& );

	static const TCHAR prefs_subkey[];
	static const TCHAR prefs_value[];

	const list< Command >& GetCommands() const { return _commands; }
	MMPainter* GetPainter() const { return _ppaint; }
	const tstring& GetSaveFile() const { return _savefile; }
	void SetSaveFile( const tstring& str ) { _savefile = str; }
private:
	list< Command > _commands;
	MMPainter* _ppaint;
	tstring _savefile;
};

const TCHAR CPDlgContext::prefs_subkey[] = _T("SOFTWARE\\hashpling.org\\memmon");
const TCHAR CPDlgContext::prefs_value[] = _T("mru_commands");

CPDlgContext::Command::Command( tchvector::const_iterator a
	   , tchvector::const_iterator b
	   , tchvector::const_iterator c
	   , tchvector::const_iterator d )
: _cmd( a, b )
, _arg( b + 1, c )
, _wd( c + 1, d )
{
}

CPDlgContext::Command::Command( const TCHAR *c, const TCHAR *a, const TCHAR *w )
: _cmd( c )
, _arg( a )
, _wd( w )
{
}

CPDlgContext::tstring CPDlgContext::Command::MakeSummaryStr() const
{
	const TCHAR dots[] = _T("...");
	const TCHAR* const dotend = dots + 3;

	tstring::size_type lastslash = _cmd.find_last_of( _T('\\') );
	tstring::size_type arglen = _arg.size();

	tstring::size_type totalsize =  arglen + _cmd.size();

	tstring::size_type totrunc = totalsize > 40 ? totalsize - 40 : 0;

	tstring::size_type trunccmd = lastslash == tstring::npos || lastslash < 6 ? 0 : lastslash;
	tstring::size_type truncarg = arglen < 6 ? 0 : arglen;

	if( trunccmd + truncarg > totrunc )
	{
		tstring::size_type tolose = trunccmd + truncarg - totrunc;
		if( trunccmd < truncarg )
		{
			tstring::size_type losecmd = std::min( trunccmd, tolose / 2 );
			trunccmd -= losecmd;
			truncarg -= tolose - losecmd;
		}
		else
		{
			tstring::size_type losearg = std::min( truncarg, tolose / 2 );
			truncarg -= losearg;
			trunccmd -= tolose - losearg;
		}
	}

	if( trunccmd < 5 )
		trunccmd = 0;

	if( truncarg < 5 )
		truncarg = 0;

	tstring::const_iterator cmd_trunc_begin = _cmd.begin() + (lastslash - trunccmd) / 2;
	tstring::const_iterator cmd_trunc_end = cmd_trunc_begin + trunccmd;

	tstring::const_iterator arg_trunc_begin = _arg.end() - truncarg;

	tstring out;

	std::back_insert_iterator< tstring > bins = std::back_inserter( out );

	tstring::size_type cmdretain = lastslash - trunccmd;

	std::copy( _cmd.begin(), cmd_trunc_begin, bins );

	if( cmd_trunc_begin != cmd_trunc_end )
		std::copy( dots, dotend, bins );

	std::copy( cmd_trunc_end, _cmd.end(), bins );
	*bins++ = _T( ' ' );
	std::copy( _arg.begin(), arg_trunc_begin, bins );

	if( arg_trunc_begin != _arg.end() )
		std::copy( dots, dotend, bins );

	return out;
}

void CPDlgContext::Load()
{
	HKEY hk;
	if (::RegOpenKeyEx(HKEY_CURRENT_USER, prefs_subkey, 0, KEY_QUERY_VALUE, &hk) == ERROR_SUCCESS)
	{
		tchvector buffer( 8192 );

		LONG ls;

		DWORD dwType;
		DWORD dwSize = static_cast< DWORD >( buffer.size() * sizeof buffer[0] );

		while( ls = ::RegQueryValueEx( hk, prefs_value, NULL, &dwType, reinterpret_cast< LPBYTE >( &buffer[0] ), &dwSize ) == ERROR_MORE_DATA )
		{
			buffer.resize( buffer.size() + buffer.size() / 2 );
			dwSize = static_cast< DWORD >( buffer.size() * sizeof buffer[0] );
		}

		::RegCloseKey( hk );

		if( ls == ERROR_SUCCESS && dwType == REG_BINARY )
		{
			buffer.resize( dwSize / sizeof buffer[0] );

			tchvector::const_iterator i = buffer.begin();
			const tchvector::const_iterator end = buffer.end();

			while( i != end )
			{
				tchvector::const_iterator j = std::find( i, end, 0 );

				if( j == end )
					break;

				tchvector::const_iterator k = std::find( j + 1, end, 0 );

				if( k == end )
					break;

				tchvector::const_iterator l = std::find( k + 1, end, 0 );

				_commands.push_back( Command( i, j, k, l ) );

				if( l == end )
					break;

				i = l + 1;
			}
		}
	}
}

void CPDlgContext::Save( const Command& cmd )
{
	list< Command >::iterator q = std::find( _commands.begin(), _commands.end(), cmd );

	if( q == _commands.end() )
	{
		_commands.push_front( cmd );
		if( _commands.size() > 10U )
			_commands.pop_back();
	}
	else
	{
		_commands.erase( q );
		_commands.push_front( cmd );
	}

	tchvector buffer;
	std::back_insert_iterator< tchvector > out = std::back_inserter( buffer );

	for( list< Command >::const_iterator i = _commands.begin(); i != _commands.end(); ++i )
	{
		if( i != _commands.begin() )
			*out++ = 0;

		std::copy( i->GetCmd().begin(), i->GetCmd().end(), out );
		*out++ = 0;
		std::copy( i->GetArg().begin(), i->GetArg().end(), out );
		*out++ = 0;
		std::copy( i->GetWd().begin(), i->GetWd().end(), out );
	}

	HKEY hk;
	DWORD dwDisposition;

	if( ::RegCreateKeyEx(HKEY_CURRENT_USER, prefs_subkey, 0, NULL, 0, KEY_SET_VALUE, NULL, &hk, &dwDisposition) == ERROR_SUCCESS )
	{
		::RegSetValueEx( hk, prefs_value, 0, REG_BINARY, reinterpret_cast< LPCBYTE >( &buffer[0] ), static_cast< DWORD >( buffer.size() * sizeof buffer[0] ) );
		::RegCloseKey( hk );
	}
}

void ShowRecButtons( HWND hwndDlg, bool bEnable )
{
	BOOL enable = bEnable ? TRUE : FALSE;
	::EnableWindow( ::GetDlgItem( hwndDlg, IDC_EDIT_RECORD ), enable );
	::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BROWSE_REC ), enable );
}

void UpdateRecFileGuess( HWND hwndDlg, CPDlgContext::Command* pCmd )
{
	TCHAR recfile[ MAX_PATH ];
	CPDlgContext* pcu = reinterpret_cast< CPDlgContext* >( ::GetWindowLongPtr( hwndDlg, GWLP_USERDATA ) );

	if( ::GetDlgItemText( hwndDlg, IDC_EDIT_RECORD, recfile, MAX_PATH ) == 0
		|| pcu->GetSaveFile().empty()
		|| pcu->GetSaveFile() == recfile )
	{
		TCHAR docsFolder[ MAX_PATH ];

		if( ::SHGetFolderPath( hwndDlg, CSIDL_PERSONAL, NULL, 0, docsFolder ) == S_OK )
		{
			typedef CPDlgContext::tstring tstring;

			const tstring& cmd = pCmd->GetCmd();
			tstring::size_type start = cmd.find_last_of( _T('\\') );
			if( start == tstring::npos )
				start = 0;

			tstring::size_type end = cmd.find_last_of( _T('.') );
			if( end == tstring::npos || end < start )
				end = cmd.size();

			tstring tmp( docsFolder );
			if( tmp.empty() || *(tmp.end() - 1) == _T('\\') )
				++start;

			tmp.append( cmd.c_str() + start, end - start );

			tstring file;

			for( int i = 0; i < 100; ++i )
			{
				std::basic_ostringstream< TCHAR > tstream;

				tstream << tmp;
				tstream << std::setfill( _T('0') ) << std::setw( 2 ) << i;
				tstream << _T(".rec");

				tstring tmpfile( tstream.str() );

				HANDLE hFile = ::CreateFile( tmpfile.c_str(), GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_TEMPORARY
 | FILE_FLAG_DELETE_ON_CLOSE, NULL);
				if( hFile != INVALID_HANDLE_VALUE )
				{
					CloseHandle( hFile );
					file = tmpfile;
					break;
				}
			}

			if( !file.empty() )
			{
				pcu->SetSaveFile( file );
				::SetWindowText( ::GetDlgItem( hwndDlg, IDC_EDIT_RECORD ), file.c_str() );
				::CheckDlgButton( hwndDlg, IDC_RECORD, BST_CHECKED );
				ShowRecButtons( hwndDlg, true );
			}
			else
			{
				::CheckDlgButton( hwndDlg, IDC_RECORD, BST_UNCHECKED );
				ShowRecButtons( hwndDlg, false );
			}
		}
	}
}

void ChangeHistSel( HWND hwndDlg, HWND hwndCombo )
{
	LRESULT lRes;
	if( (lRes = ::SendMessage(hwndCombo, CB_GETCURSEL, 0, 0)) >= 0
		&& (lRes = ::SendMessage(hwndCombo, CB_GETITEMDATA, lRes, 0)) != CB_ERR )
	{
		CPDlgContext::Command* pCmd = reinterpret_cast< CPDlgContext::Command* >( lRes );
		::SetWindowText( ::GetDlgItem( hwndDlg, IDC_EDIT_CMD ), pCmd->GetCmd().c_str() );
		::SetWindowText( ::GetDlgItem( hwndDlg, IDC_EDIT_ARGS ), pCmd->GetArg().c_str() );
		::SetWindowText( ::GetDlgItem( hwndDlg, IDC_EDIT_WD ), pCmd->GetWd().c_str() );

		UpdateRecFileGuess( hwndDlg, pCmd );
	}
}

void InitDialog( HWND hwndDlg, LPARAM lParam )
{
	::SetWindowLongPtr( hwndDlg, GWLP_USERDATA, lParam );
	::CheckDlgButton( hwndDlg, IDC_RECORD, BST_CHECKED );

	CPDlgContext* pcu = reinterpret_cast< CPDlgContext* >( lParam );

	pcu->Load();

	const list< CPDlgContext::Command >& cmds = pcu->GetCommands();

	if( !cmds.empty() )
	{
		HWND hwndHist = ::GetDlgItem( hwndDlg, IDC_CMD_HIST );
		for( list< CPDlgContext::Command >::const_iterator i = cmds.begin(); i != cmds.end(); ++i )
		{
			LRESULT lRes = ::SendMessage( hwndHist, CB_ADDSTRING, 0, reinterpret_cast< LPARAM >( i->MakeSummaryStr().c_str() ) );
			lRes = ::SendMessage( hwndHist, CB_SETITEMDATA, lRes, reinterpret_cast< LPARAM >( &*i ) );
		}
		ShowWindow( hwndHist, SW_SHOWNOACTIVATE );

		::SendMessage( hwndHist, CB_SETCURSEL, 0, 0 );
		ChangeHistSel( hwndDlg, hwndHist );
	}
}

void EscapePath( std::basic_string< TCHAR >& str, const TCHAR* path )
{
	bool needsescape = false;
	for( const TCHAR* p = path; *p; ++p )
	{
		if( *p == '\"' || *p == ' ' )
		{
			needsescape = true;
			break;
		}
	}

	if( !needsescape )
		str.assign( path );
	else
	{
		std::basic_stringbuf< TCHAR > buf;

		buf.sputc( _T('\"') );
		for( const TCHAR* p = path; *p; ++p )
		{
			if( *p == '"' )
				buf.sputc( _T('\\') );

			buf.sputc( *p );
		}
		buf.sputc( _T('\"') );

		str.assign( buf.str() );
	}
}

bool CallRun( HWND hwndDlg )
{
	TCHAR command[ MAX_PATH ];

	bool success = false;

	if( GetDlgItemText( hwndDlg, IDC_EDIT_CMD, command, sizeof command / sizeof( TCHAR ) ) )
	{
		TCHAR workingdir[ MAX_PATH ];
		const TCHAR *pwd = workingdir;
		TCHAR args[ 65536 ];
		const TCHAR *pag = args;
		std::basic_string< TCHAR > argstr;

		if( GetDlgItemText( hwndDlg, IDC_EDIT_ARGS, args, sizeof args / sizeof( TCHAR ) ) == 0 )
			pag = NULL;

		if( GetDlgItemText( hwndDlg, IDC_EDIT_WD, workingdir, sizeof workingdir / sizeof( TCHAR ) ) == 0)
			pwd = NULL;

		if( pag != NULL )
		{
			EscapePath( argstr, command );
			argstr += _T(' ');
			argstr += args;
			pag = argstr.c_str();
		}

		try
		{
			CPDlgContext* pcu = reinterpret_cast< CPDlgContext* >( ::GetWindowLongPtr( hwndDlg, GWLP_USERDATA ) );
			pcu->GetPainter()->Run( command, pag, pwd );

			success = true;

			pcu->Save( CPDlgContext::Command( command, args, workingdir ) );
			if( ::IsDlgButtonChecked( hwndDlg, IDC_RECORD ) )
			{
				char recfile[ MAX_PATH ];

				if( ::GetDlgItemTextA( hwndDlg, IDC_EDIT_RECORD, recfile, MAX_PATH ) != 0 )
				{
					pcu->GetPainter()->Update( true );
					pcu->GetPainter()->Record( recfile );
				}
			}
		}
		catch( const std::exception& ex )
		{
			::MessageBoxA( hwndDlg, ex.what(), "Exception", MB_ICONINFORMATION | MB_OK );
		}
	}

	return success;
}

void DoCmdBrowse( HWND hwnd )
{
	char fname[ MAX_PATH ];
	if( MemMon::Win::RunOpenFileDialog( hwnd, fname ) )
	{
		::SetWindowTextA( ::GetDlgItem( hwnd, IDC_EDIT_CMD ), fname );
	}
}

void DoRecBrowse( HWND hwnd )
{
	char fname[ MAX_PATH ];
	if( MemMon::Win::RunSaveFileDialog( hwnd, fname ) )
	{
		::SetWindowTextA( ::GetDlgItem( hwnd, IDC_EDIT_RECORD ), fname );
	}
}

void DoWdBrowse( HWND hwnd )
{
	char fname[ MAX_PATH ];
	if( MemMon::Win::RunSelectDirDialog( hwnd, fname ) )
	{
		::SetWindowTextA( ::GetDlgItem( hwnd, IDC_EDIT_WD ), fname );
	}
}

INT_PTR CALLBACK CreateProcProc( HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam )
{
	INT_PTR ret = (INT_PTR)FALSE;
	switch (message)
	{
	case WM_COMMAND:
		switch( LOWORD(wParam) )
		{
		case IDOK:
			if( !CallRun( hwndDlg ) )
				break;
			// Drop through
		case IDCANCEL:
			EndDialog( hwndDlg, wParam );
			ret = (INT_PTR)TRUE;
			break;
		case IDC_RECORD:
			ShowRecButtons( hwndDlg, ::IsDlgButtonChecked( hwndDlg, IDC_RECORD ) == BST_CHECKED );
			break;
		case IDC_CMD_HIST:
			if( HIWORD(wParam) == CBN_SELCHANGE )
				ChangeHistSel( hwndDlg, reinterpret_cast< HWND >( lParam ) );
			break;
		case IDC_BROWSE_CMD:
			DoCmdBrowse( hwndDlg );
			break;
		case IDC_BROWSE_REC:
			DoRecBrowse( hwndDlg );
			break;
		case IDC_BROWSE_WD:
			DoWdBrowse( hwndDlg );
			break;
		}
		break;

	case WM_INITDIALOG:
		InitDialog( hwndDlg, lParam );
		ret = (INT_PTR)TRUE;
		break;
	}
	return ret;
}

}

bool CreateProcDialog::Run( HINSTANCE hInst, HWND hWnd, MMPainter* pPaint )
{
	CPDlgContext ctx( pPaint );
	return DialogBoxParam( hInst, MAKEINTRESOURCE(IDD_CREATEPROC_DLOG), hWnd, CreateProcProc, reinterpret_cast< LPARAM >( &ctx ) ) == IDOK;
}
