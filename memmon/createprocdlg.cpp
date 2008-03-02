#include "stdafx.h"
#include "createprocdlg.h"
#include "mmpainter.h"
#include "filedialog.h"
#include "resource.h"
#include <list>
#include <vector>
#include <string>
#include <sstream>

using std::list;
using std::vector;
using std::wstring;

namespace
{

class CPDlgContext
{
public:
	CPDlgContext( MMPainter* pPaint ) : _ppaint( pPaint ) {}

	struct Command
	{
		Command( vector< wchar_t >::const_iterator
			   , vector< wchar_t >::const_iterator
			   , vector< wchar_t >::const_iterator
			   , vector< wchar_t >::const_iterator);

		Command( const wchar_t *, const wchar_t *, const wchar_t * );

		bool operator==( const Command& other ) const
		{
			return cmd == other.cmd && arg == other.arg && wd == other.wd;
		}

		bool operator!=( const Command& other ) const
		{
			return *this != other;
		}

		wstring cmd;
		wstring arg;
		wstring wd;
	};

	void Load();
	void Save( const Command& );

	static const TCHAR prefs_subkey[];
	static const TCHAR prefs_value[];

	const list< Command >& GetCommands() const { return _commands; }
	MMPainter* GetPainter() const { return _ppaint; }

private:
	list< Command > _commands;
	MMPainter* _ppaint;
};

const TCHAR CPDlgContext::prefs_subkey[] = _T("SOFTWARE\\hashpling.org\\memmon");
const TCHAR CPDlgContext::prefs_value[] = _T("mru_commands");

CPDlgContext::Command::Command( vector< wchar_t >::const_iterator a
	   , vector< wchar_t >::const_iterator b
	   , vector< wchar_t >::const_iterator c
	   , vector< wchar_t >::const_iterator d )
: cmd( a, b )
, arg( b + 1, c )
, wd( c + 1, d )
{
}

CPDlgContext::Command::Command( const wchar_t *c, const wchar_t *a, const wchar_t *w )
: cmd( c )
, arg( a )
, wd( w )
{
}

void CPDlgContext::Load()
{
	HKEY hk;
	if (::RegOpenKeyEx(HKEY_CURRENT_USER, prefs_subkey, 0, KEY_QUERY_VALUE, &hk) == ERROR_SUCCESS)
	{
		vector< wchar_t > buffer( 8192 );

		LSTATUS ls;

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

			vector< wchar_t >::const_iterator i = buffer.begin();
			const vector< wchar_t >::const_iterator end = buffer.end();

			while( i != end )
			{
				vector< wchar_t >::const_iterator j = std::find( i, end, 0 );

				if( j == end )
					break;

				vector< wchar_t >::const_iterator k = std::find( j + 1, end, 0 );

				if( k == end )
					break;

				vector< wchar_t >::const_iterator l = std::find( k + 1, end, 0 );

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

	vector< wchar_t > buffer;
	std::back_insert_iterator< vector< wchar_t > > out = std::back_inserter( buffer );

	for( list< Command >::const_iterator i = _commands.begin(); i != _commands.end(); ++i )
	{
		if( i != _commands.begin() )
			*out++ = 0;

		std::copy( i->cmd.begin(), i->cmd.end(), out );
		*out++ = 0;
		std::copy( i->arg.begin(), i->arg.end(), out );
		*out++ = 0;
		std::copy( i->wd.begin(), i->wd.end(), out );
	}

	HKEY hk;
	DWORD dwDisposition;

	if( ::RegCreateKeyEx(HKEY_CURRENT_USER, prefs_subkey, 0, NULL, 0, KEY_SET_VALUE, NULL, &hk, &dwDisposition) == ERROR_SUCCESS )
	{
		::RegSetValueEx( hk, prefs_value, 0, REG_BINARY, reinterpret_cast< LPCBYTE >( &buffer[0] ), static_cast< DWORD >( buffer.size() * sizeof buffer[0] ) );
		::RegCloseKey( hk );
	}
}

void ChangeHistSel( HWND hwndDlg, HWND hwndCombo )
{
	LRESULT lRes;
	if( (lRes = ::SendMessage(hwndCombo, CB_GETCURSEL, 0, 0)) >= 0
		&& (lRes = ::SendMessage(hwndCombo, CB_GETITEMDATA, lRes, 0)) != CB_ERR )
	{
		CPDlgContext::Command* pCmd = reinterpret_cast< CPDlgContext::Command* >( lRes );
		::SetWindowText( ::GetDlgItem( hwndDlg, IDC_EDIT_CMD ), pCmd->cmd.c_str() );
		::SetWindowText( ::GetDlgItem( hwndDlg, IDC_EDIT_ARGS ), pCmd->arg.c_str() );
		::SetWindowText( ::GetDlgItem( hwndDlg, IDC_EDIT_WD ), pCmd->wd.c_str() );
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
			LRESULT lRes = ::SendMessage( hwndHist, CB_ADDSTRING, 0, reinterpret_cast< LPARAM >( i->cmd.c_str() ) );
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

void ShowRecButtons( HWND hwndDlg, bool bEnable )
{
	BOOL enable = bEnable ? TRUE : FALSE;
	::EnableWindow( ::GetDlgItem( hwndDlg, IDC_EDIT_RECORD ), enable );
	::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BROWSE_REC ), enable );
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

void CreateProcDialog::Run( HINSTANCE hInst, HWND hWnd, MMPainter* pPaint, UINT_PTR& timerid )
{
	CPDlgContext ctx( pPaint );
	if( DialogBoxParam( hInst, MAKEINTRESOURCE(IDD_CREATEPROC_DLOG), hWnd, CreateProcProc, reinterpret_cast< LPARAM >( &ctx ) ) )
	{
		timerid = SetTimer(hWnd, 1, 100, NULL);
	}
}
