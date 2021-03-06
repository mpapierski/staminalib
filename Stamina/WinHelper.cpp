/*

The contents of this file are subject to the Mozilla Public License
Version 1.1 (the "License"); you may not use this file except in
compliance with the License. You may obtain a copy of the License from
/LICENSE.HTML in this package or at http://www.mozilla.org/MPL/

Software distributed under the License is distributed on an "AS IS"
basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
License for the specific language governing rights and limitations
under the License.

The Original Code is "Stamina.lib" library code, released Feb 1, 2006.

The Initial Developer of the Original Code is "STAMINA" - Rafa� Lindemann.
Portions created by STAMINA are 
Copyright (C) 2003-2006 "STAMINA" - Rafa� Lindemann. All Rights Reserved.

Contributor(s): 

--

$Id$

*/


#include "stdafx.h"

#include <windows.h>
#include <commctrl.h>
#include <stdstring.h>
#include "WinHelper.h"
#include "Helpers.h"
#include <io.h>
#include <direct.h>
#include <shlwapi.h>
#include <math.h>
namespace Stamina {

	HINSTANCE _instance = 0;

	void setInstance(HINSTANCE inst) {
		_instance = inst;
	}
	HINSTANCE getInstance() {
		return _instance;
	}


	int parseScrollRequest(HWND hwnd, short bar, short trackPos, short request, short line) {
		SCROLLINFO si;
		si.cbSize = sizeof(si);
		si.fMask = SIF_RANGE | SIF_PAGE | SIF_POS | SIF_TRACKPOS;
		GetScrollInfo(hwnd, bar, &si);
		switch (request) {
            case SB_THUMBTRACK:
				si.nPos = si.nTrackPos;
                break;
            case SB_LINEDOWN:
                si.nPos += line;
                break;
            case SB_LINEUP:
                si.nPos -= line;
                break;
            case SB_PAGEDOWN:
				si.nPos += 2*si.nPage/3;
                break;
            case SB_PAGEUP:
				si.nPos -= 2*si.nPage/3;
                break;
            case SB_TOP:
				si.nPos = si.nMin;
                break;
            case SB_BOTTOM:
				si.nPos = si.nMax - si.nPage;
                break;
		}
        return si.nPos;
	}


	bool getKeyState(int vkey) {
		return ( GetKeyState(vkey) & 0x80 ) != 0;
	}

	const char* vkeyToAscii(int vkey, int scan) {
		static char buff [3];
		// nie musimy obawia� si� wielu w�tk�w bo tej funkcji po prostu nie da si� u�ywa� z kilku w�tk�w na raz...
		buff[0] = 0;
		unsigned char keys [256];
		GetKeyboardState(keys);
		if (scan == -1) {
			/*TODO: scan code??*/
			scan = 0;
		}
		ToAscii(vkey, scan, keys, (LPWORD)buff, false);
		return buff;
	}


	void getChildRect (HWND hwnd , Rect& r) {
		GetWindowRect(hwnd , r.ref());
		Point pt (r.getLT());
		ScreenToClient(GetParent(hwnd) , pt.ref());
		r.setPos(pt);
	}


	void setParentsFont(HWND hwnd) {
		copyWindowFont(GetParent(hwnd), hwnd);
	}
	void copyWindowFont(HWND from, HWND to) {
		SendMessage(to, WM_SETFONT, SendMessage(from, WM_GETFONT, 0, 0), 0);
	}


	int removeDirTree(const char * path) {
		char * ch = (char*)malloc(strlen(path) + 255);
		strcpy(ch , path);
		strcat(ch , "\\*.*");
		WIN32_FIND_DATA fd;
		HANDLE hFile;
		BOOL found;
		found = ((hFile = FindFirstFile(ch,&fd))!=INVALID_HANDLE_VALUE);

		//   int i = 0;
		while (found)
		{
			if (*fd.cFileName != '.') {
				ch[strlen(path)+1] = 0;
				strcat(ch , fd.cFileName);
				if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
					removeDirTree(ch);
				} else {
					_unlink(ch);
				}
			}
			if (!FindNextFile(hFile , &fd)) break; 
		}
		FindClose(hFile);
		ch[strlen(path)] = 0;
		/*
		char * ch2 = strrchr(ch , '\\');
		if (!ch2) ch2 = path;
		else {*ch2 = 0;
		ch2++;
		}
		*/
		_rmdir(path);
		//MessageBox(0 , _sprintf("%s %s %d %d [%d]" , ch , ch2 , rmdir(path) , errno , ENOENT) , "" , 0);
		free(ch);
		return 0;
	}



	void setRectSizeByEdge(int edge , RECT * rc , int w, int h){
		if (edge == 0) edge = WMSZ_BOTTOMRIGHT;
		if (edge>=WMSZ_BOTTOM) 
			rc->bottom=rc->top+h;
		else 
			rc->top=rc->bottom-h;
		if (edge==WMSZ_RIGHT || edge==WMSZ_TOPRIGHT || edge==WMSZ_BOTTOMRIGHT)
			rc->right=rc->left+w;
		else 
			rc->left=rc->right-w;
	}
	void clipSize(int edge , RECT * rc , int minW , int minH, int maxW, int maxH){
		int w = rc->right-rc->left;
		int h = rc->bottom-rc->top;
		w = max(w, minW);
		h = max(h, minH);
		w = min(w, maxW);
		h = min(h, maxH);
		setRectSizeByEdge(edge, rc, w, h);
	}        




	std::string regQueryString(HKEY hKey , const char * name, const char * def) {
		unsigned long type=0,count=0;
		if (RegQueryValueEx(hKey,(LPCTSTR)name,0,&type,0,&count) == ERROR_SUCCESS && type == REG_SZ) {
			CStdString buff;
			if (!RegQueryValueEx(hKey,(LPCTSTR)name,0,&type,(LPBYTE)buff.GetBuffer(count-1),&count)) {
				buff.ReleaseBuffer();
				return buff;
			}
		}
		return def ? def : "";
	}

	int regQueryDWord(HKEY hKey , const char * name , int def) {
		unsigned long type,count=sizeof(DWORD);
		DWORD dw;
		if (!RegQueryValueEx(hKey,name,0,&type,(LPBYTE)&dw,&count))
			if (type==REG_DWORD)
				return dw;
		return def;
	}


	long regSetString(HKEY hKey , const char * name , const std::string& val) {
		return RegSetValueEx(hKey,name,0,REG_SZ,(LPBYTE)val.c_str(),val.length()+1);

	}

	long regSetDWord(HKEY hKey , const char * name , int val) {
		return RegSetValueEx(hKey,name,0,REG_DWORD,(LPBYTE)&val,sizeof(DWORD));
	}

#define PACKVERSION(major,minor) MAKELONG(minor,major)

	int getDllVersion(LPCTSTR lpszDllName)
	{

		HINSTANCE hinstDll;
		DWORD dwVersion = 0;

		hinstDll = LoadLibrary(lpszDllName);

		if(hinstDll)
		{
			DLLGETVERSIONPROC pDllGetVersion;

			pDllGetVersion = (DLLGETVERSIONPROC) GetProcAddress(hinstDll, "DllGetVersion");

			/*Because some DLLs might not implement this function, you
			must test for it explicitly. Depending on the particular
			DLL, the lack of a DllGetVersion function can be a useful
			indicator of the version.
			*/
			if(pDllGetVersion)
			{
				DLLVERSIONINFO dvi;
				HRESULT hr;

				ZeroMemory(&dvi, sizeof(dvi));
				dvi.cbSize = sizeof(dvi);

				hr = (*pDllGetVersion)(&dvi);

				if(SUCCEEDED(hr))
				{
					dwVersion = PACKVERSION(dvi.dwMajorVersion, dvi.dwMinorVersion);
				}
			}

			FreeLibrary(hinstDll);
		}
		return dwVersion;
	}


	bool isComctl(unsigned int maj , unsigned int min) {
		static unsigned int ver = 0;
		if (!ver) ver = getDllVersion(TEXT("comctl32.dll"));
		return ver >= (unsigned int)PACKVERSION(maj,min);
	}



	// -------------------------
#pragma pack(push, 1)
	struct NEWHEADER {
		WORD Reserved;
		WORD ResType;
		WORD ResCount;
	};
#define SIZEOF_NEWHEADER 6
	struct ICONRESDIR {
		BYTE Width;
		BYTE Height;
		BYTE ColorCount;
		BYTE reserved;
	};
	struct CURSORDIR {
		WORD Width;
		WORD Height;
	};
	typedef struct tagRESDIR {
		union {
			ICONRESDIR   Icon;
			CURSORDIR    Cursor;
		} ResInfo;
		WORD    Planes;
		WORD    BitCount;
		DWORD   BytesInRes;
		DWORD  IconCursorId; // Jedyna r�nica pomi�dzy czytaniem z DLLa a z ICO
	} RESDIR;
#define SIZEOF_RESDIR 14
#pragma pack(pop)
	HICON loadIconEx(HINSTANCE hInst , LPCTSTR id , int size , int bits) {
		HDC hDc = GetDC(GetDesktopWindow());
		int screenBits = GetDeviceCaps(hDc , BITSPIXEL);
		if (!bits) bits = screenBits;
		if (bits>=32 && !isComctl(6,0)) bits = 24;
		ReleaseDC(GetDesktopWindow() , hDc);
		HGDIOBJ icon;
		if (loadIcon32(icon, hInst, id, size, bits, true) == IMAGE_ICON) {
			return (HICON)icon;
		} else {
			DeleteObject(icon);
			return 0;
		}
	}

	int loadIcon32(HGDIOBJ& image, HINSTANCE hInst, LPCTSTR id , int size , int bits, bool preferIcon) {
		image = 0;
		HRSRC hr;
		HGLOBAL hg;
		int imageType=-1;
		void * p = 0;
		if (!hInst) {
			FILE * file = fopen(id , "rb");
			if (!file) goto end;
			fseek(file , 0 , SEEK_END);
			int size = ftell(file);
			rewind(file);
			if (size < SIZEOF_NEWHEADER) {fclose(file); return 0;}
			p = malloc(size);
			fread(p , 1 , size , file);
			fclose(file);
		} else {
			hr = FindResource(hInst , id , RT_GROUP_ICON);
			if (!hr) goto end;
			hg = LoadResource(hInst , hr);
			if (!hg) goto end;
			p = LockResource(hg);
			if (!p) goto end;
		}
		NEWHEADER * nh = (NEWHEADER *)p;
		RESDIR * rd;
		rd = (RESDIR *)((long)p+SIZEOF_NEWHEADER);
		RESDIR* found = 0;
		int rsize = 0;
		int sizev = size; // rozmiar pionowy
		for (int i = 0; i < nh->ResCount; i++ , rd++) {
			if (rd->ResInfo.Icon.Width == size || !found) {
				if (!size) size = rd->ResInfo.Icon.Width;
				if (rd->BitCount <= bits || !found) {
					//found = hInst?rd->IconCursorId & 0xFFFF : rd->IconCursorId;
					found = rd;
					rsize = rd->BytesInRes;

					sizev = rd->ResInfo.Icon.Width == rd->ResInfo.Icon.Height 
						?size : ceilf(float(size / rd->ResInfo.Icon.Width) * rd->ResInfo.Icon.Height);
				}
				if (rd->ResInfo.Icon.Width == size && rd->BitCount == bits) break;
			}
			if (hInst) rd = (RESDIR*)((char*)rd - 2); // Korygujemy pozycj�...
		}
		if (!found) goto end;
		void * p2 = 0;
		if (!hInst) {
			p2 = (char*)p + found->IconCursorId;
		} else {
			hr = FindResource(hInst , MAKEINTRESOURCE(found->IconCursorId & 0xFFFF) , RT_ICON);
			if (!hr) goto end;
			hg = LoadResource(hInst , hr);
			//    = SizeofResource(hInst , hr);
			if (!hg || !rsize) goto end;
			p2 = LockResource(hg);
			if (!p2) goto end;
		}
		if (found->BitCount == 32 && (!isComctl(6,0) || !preferIcon)) {
			
			BITMAPV5HEADER bi;
			BITMAPV5HEADER* _bi = (BITMAPV5HEADER*)p2;

			ZeroMemory(&bi,sizeof(BITMAPV5HEADER));
			bi.bV5Size = sizeof(BITMAPV5HEADER);
			bi.bV5Width = _bi->bV5Width;
			bi.bV5Height = _bi->bV5Height / 2;
			bi.bV5Planes = _bi->bV5Planes;
			bi.bV5BitCount = 32;
			bi.bV5Compression = BI_BITFIELDS;
			bi.bV5RedMask   =  0x00FF0000;
			bi.bV5GreenMask =  0x0000FF00;
			bi.bV5BlueMask  =  0x000000FF;
			bi.bV5AlphaMask =  0xFF000000; 

			void * bmpData;
			char* _bmpData = (char*)p2 + _bi->bV5Size;
				
			HDC hdc;
			hdc = GetDC(NULL);
			// Create the DIB section with an alpha channel.
			image = (HGDIOBJ)CreateDIBSection(hdc, (BITMAPINFO *)&bi, DIB_RGB_COLORS, (void **)&bmpData, NULL, (DWORD)0);
			memcpy(bmpData, _bmpData, bi.bV5Width * bi.bV5Height * 4);
			ReleaseDC(0, hdc);

			if (found->ResInfo.Icon.Width != size || found->ResInfo.Icon.Height != sizev) {
				// zmiana rozmiaru


			}
			
			imageType = IMAGE_BITMAP;
		} else {
			image = (HGDIOBJ)CreateIconFromResourceEx((PBYTE)p2 , rsize , true , 0x00030000 , size , sizev , 0);
			imageType = IMAGE_ICON;
		}
end:
		if (!hInst && p) 
			free(p);
		return imageType;
	}


	void bitmapPremultiply(void * pixels, unsigned int pixelsCount)
	{
		BYTE * ptrByte = (BYTE *) pixels;
		unsigned long i;
		for ( i = 0; i < pixelsCount; i++ )
		{
			ptrByte[0] = ( ptrByte[0] * ptrByte[3] ) / 0xFF;
			ptrByte[1] = ( ptrByte[1] * ptrByte[3] ) / 0xFF;
			ptrByte[2] = ( ptrByte[2] * ptrByte[3] ) / 0xFF;
			ptrByte += 4;
		}
	}

/*
	void drawBitmapBlended (HDC hDC , HBITMAP hBmp , int x , int y, int alpha) {
		BITMAP bmp;
		GetObject(hBmp , sizeof(BITMAP),&bmp);
		bool hasAlpha = (bmp.bmBitsPixel == 32);

		HDC hdcmem = CreateCompatibleDC(hDC);
		SelectObject(hdcmem , hBmp);
		BLENDFUNCTION bf;
		bf.BlendOp = AC_SRC_OVER;
		bf.BlendFlags = 0;
		bf.SourceConstantAlpha = alpha;
		bf.AlphaFormat = hasAlpha ? AC_SRC_ALPHA : 0;
		void * buff;
		int buffSize;
		if (hasAlpha) { // premultiply...
			buffSize = bmp.bmWidth * bmp.bmHeight * 4;
			buff = malloc(buffSize);
			memcpy(buff, bmp.bmBits, buffSize);
			bitmapPremultiply(bmp.bmBits, buffSize / 4);
		}
		AlphaBlend(hDC, x,
			y,
			bmp.bmWidth, bmp.bmHeight,
			hdcmem, 0, 0,bmp.bmWidth, bmp.bmHeight,
			bf);
		if (hasAlpha) {
			memcpy(bmp.bmBits, buff, buffSize);
			free(buff);
		}
		DeleteDC (hdcmem);
	}

*/

	int getFontHeight(HDC hdc) {
		return getTextSize(hdc, "0Ajg^_|").cy;
	}

	int getFontHeight(HWND hwnd, HFONT font) {
		return getTextSize(hwnd, font, "0Ajg^_|").cy;
	}
	Size getTextSize(HDC hdc, const char* text, int width) {
		if (!text || !*text) return Size(0,0);
		Rect rt (0,0,width,0);
		DrawText(hdc, text, -1, rt.ref(), DT_LEFT | DT_NOPREFIX | DT_CALCRECT | (!width ? 0 : DT_WORDBREAK));
		return Size(rt.right, rt.bottom);

	}
	Size getTextSize(HWND hwnd, HFONT font, const char* text, int width) {
		HDC hdc = GetDC(hwnd);
		if (font) 
			font = (HFONT)SelectObject(hdc , font);
		Size sz = getTextSize(hdc, text, width);
		if (font) 
			SelectObject(hdc , font);
		ReleaseDC(hwnd, hdc);
		return sz;
	}



	Point getScreenPoint() {
		Point pt;
		GetCursorPos(pt.ref());
		return pt;
	}
	Point getLocalPoint(HWND window, const Point& screen, int scrollX, int scrollY) {
		Point local = screen;
		ScreenToClient(window, local.ref());
		if (scrollX == -1 || scrollY == -1) {
			SCROLLINFO si;
			si.cbSize = sizeof(si);
			si.fMask = SIF_POS;
			if (scrollX == -1)
				scrollX = GetScrollInfo(window , SB_HORZ , &si) ? si.nPos : 0;
			if (scrollY == -1)
				scrollY = GetScrollInfo(window , SB_VERT , &si) ? si.nPos : 0;
		}

		local.x += scrollX;
		local.y += scrollY;
		return local;
	}

	bool windowBelongsTo(HWND window, HWND parent) {
		while (window) {
			if (window == parent) return true;
			window = GetParent(window);
		}
		return false;
	}



	bool _SetDllDirectory(const char * dir) {
#ifdef UNICODE
		static FARPROC proc = GetProcAddress(GetModuleHandle("kernel32.dll") ,  "SetDllDirectoryW");
#else
		static FARPROC proc = GetProcAddress(GetModuleHandle("kernel32.dll") ,  "SetDllDirectoryA");
#endif
		if (proc) {		 
			return ((bool (__stdcall*)(IN LPCSTR lpPathName))proc)(dir);
		} else {return false;}  
	}

	std::string getErrorMsg(int err) {
		std::string s;
		FormatMessage(
			FORMAT_MESSAGE_FROM_SYSTEM,
			NULL,
			err ? err:GetLastError(),
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
			stringBuffer(s, 1024),
			1024,
			NULL
		);
		return s;
	}

	// -------------------- LIST VIEW -----------------------------

	LPARAM ListView_GetItemData(HWND hwnd , int pos) {
		if (pos<0) return 0;
		LVITEM lvi;
		lvi.mask = LVIF_PARAM;
		lvi.iItem = pos;
		lvi.iSubItem = 0;
		if (ListView_GetItem(hwnd , &lvi))
			return lvi.lParam;
		else
			return 0;
	}


	int ListView_ItemFromPoint(HWND hwnd , POINT pt) {
		LVHITTESTINFO vhti;
		vhti.pt = pt;
		ListView_HitTest(hwnd , &vhti);
		return vhti.iItem;
	}

	int ListView_Deselect(HWND hwnd) {
		int pos;
		int c = ListView_GetItemCount(hwnd);
		//   while ((pos = ListView_GetSelectionMark(hwnd))>=0)
		for (pos = 0; pos < c; pos++)
			ListView_SetItemState(hwnd , pos , 0 , LVIS_SELECTED);
		return 1;
	}


	int ListView_GetSelItems(HWND hwnd , int count , int * buff) {
		int i = 0;
		int sc = ListView_GetSelectedCount(hwnd);
		int c = ListView_GetItemCount(hwnd);
		for (int j = 0; j < c; j++) {
			if (ListView_GetItemState(hwnd , j , LVIS_SELECTED)) {
				buff[i]=j;
				i++;
				if (i>=count || i>=sc) return i;
			}
		}
		return i;
	}

	int ListView_SetCurSel (HWND hwnd , int pos) {
		ListView_Deselect(hwnd);
		ListView_SetItemState(hwnd , pos , LVIS_SELECTED , LVIS_SELECTED);
		ListView_SetSelectionMark(hwnd , pos);
		ListView_SetHotItem(hwnd , pos);
		ListView_EnsureVisible(hwnd , pos , 0);
		return 1;
	}


	int ListView_AddString(HWND hwnd , const char * txt , int image , LPARAM param ) {
		LVITEM lvi;
		lvi.mask = LVIF_PARAM | LVIF_TEXT | LVIF_IMAGE;
		lvi.iItem = 0xFFFF;
		lvi.iSubItem = 0;
		lvi.lParam = param;
		lvi.iImage = image;
		lvi.pszText = (char*)txt;
		return ListView_InsertItem(hwnd , &lvi);
	}

	int ListView_SetString(HWND hwnd , int item , int subitem , char * txt) {
		LVITEM lvi;
		lvi.mask = LVIF_TEXT;
		lvi.iItem = item;
		lvi.iSubItem = subitem;
		lvi.pszText = txt;
		ListView_SetItem(hwnd , &lvi);
		return 1;
	}


	int ListView_AddColumn(HWND hwnd , const char * txt , int cx) {
		LVCOLUMN lvc;
		lvc.mask = LVCF_TEXT | LVCF_WIDTH;
		lvc.iSubItem = 0;
		lvc.pszText = (char*)txt;
		lvc.cx = cx;
		return ListView_InsertColumn(hwnd, 0x7FFF, &lvc);
		//    ListView_SetColumnWidth(hwnd , 0 , LVSCW_AUTOSIZE_USEHEADER);
	}

	int ListView_MoveItem(HWND hwnd , int pos , int newPos) {
		char buff [2000];
		int cols = Header_GetItemCount(ListView_GetHeader(hwnd));
		LVITEM lvi;
		lvi.mask = LVIF_IMAGE|LVIF_INDENT|LVIF_PARAM|LVIF_TEXT|LVIF_STATE;
		lvi.iItem = pos;
		lvi.iSubItem = 0;
		lvi.stateMask= (UINT)-1;
		lvi.pszText = buff;
		lvi.cchTextMax = 2000;
		ListView_GetItem(hwnd , &lvi);
		int check = ListView_GetCheckState(hwnd , pos);

		if (newPos>pos) newPos++; else pos++;
		lvi.iItem = newPos;
		newPos = ListView_InsertItem(hwnd , &lvi);
		ListView_SetCheckState(hwnd , newPos , check);

		lvi.mask = LVIF_TEXT;
		for (int i=1; i<cols; i++) {
			lvi.iSubItem=i;
			lvi.iItem=pos;
			ListView_GetItem(hwnd , &lvi);
			lvi.iItem=newPos;
			ListView_SetItem(hwnd , &lvi);
		}
		//    ListView_SetSelectionMark(hwnd , newPos);
		ListView_EnsureVisible(hwnd , newPos , 0);
		ListView_DeleteItem(hwnd , pos);
		return 1;
	}


	Size toolBar_getSize(HWND hwnd) {
		int c = SendMessage(hwnd , TB_BUTTONCOUNT , 0 , 0);
		Rect rc;
		Rect tbrc;
		for (int i=0; i < c; i++) {
			if (SendMessage(hwnd , TB_GETITEMRECT , i , (LPARAM)rc.ref())) {
				tbrc.include(rc);
			}
		}
		return tbrc.getSize();
	}


};