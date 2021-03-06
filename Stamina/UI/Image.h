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

#pragma once


#include <memory.h>
#include "../Rect.h"
#include "Region.h"
#include "iImage.h"
#include "../ObjectImpl.h"
#include "../Mutex.h"

namespace Stamina { namespace UI {

	typedef SharedPtr<class Image> oImage;

	/** Draws simple gradient.
	@warning Now angle can be only 0 or 90.
	*/
	void drawSimpleGradient(HDC dc, int color1, int color2, const Rect& rect
		, short angle = 0 /// 0 - 90 
		, unsigned short midpoint = 0x80 /// 0 - 0xFF
		);

	/** Draws simple gradient.
	@warning Now angle can be only 0 or 90.
	*/
	void drawSimpleGradient(HBITMAP bmp, int color1, int color2, const Rect& rect,  short angle = 0, short midpoint = 0x80);
	HBRUSH createSimpleGradientBrush(int color1, int color2, const Size& size, short angle = 0, short midpoint = 0x80);

	oImage createSimpleGradient(int color1, int color2, const Size& size, short angle = 0, short midpoint = 0x80);


	/** Blends two colors using alpha (0-255) */
	inline int blendRGB(int color1 ,int  color2 ,unsigned char alpha) {
		return RGB(
			(GetRValue(color1)*alpha+GetRValue(color2)*(0xFF-alpha))/0xFF 
			, (GetGValue(color1)*alpha+GetGValue(color2)*(0xFF-alpha))/0xFF
			, (GetBValue(color1)*alpha+GetBValue(color2)*(0xFF-alpha))/0xFF
			);
	}


	inline COLORREF getUniqueColor(int i, int stepping, int range, bool bright, bool reversed = false) {
		int loss = range / stepping;
		int r = (i % stepping) * loss;
		i = i / stepping;
		int g = (i % stepping) * loss;
		i = i / stepping;
		int b = (i % stepping) * loss;
		if (bright) {
			r = 0xff - r;
			g = 0xff - g;
			b = 0xff - b;
		}
		return reversed ? RGB(b,g,r) : RGB(r,g,b);
	}


	/** Represents an Image.
	*/
	class Image: public SharedObject<iImage, LockableObject<iImage, Stamina::FastMutex> > {
	public:

		STAMINA_OBJECT_CLASS(Image, iImage);

		enum DrawMask {
			dmNone = 0,
			dmAlpha = 1,
			dmState = 2,
			dmSize = 4,
			dmSource = 8,
		};

		enum DrawState {
			stateNormal,
			stateHot,
			statePressed,
			stateDisabled,
			stateHidden,
		};

		/** Drawing Options.
		*/
		class DrawOpts {
		public:

			DrawOpts(DrawMask mask = dmNone):mask(mask) {}

			DrawOpts(const DrawOpts* opts) {
				if (opts)
					memcpy(this, opts, sizeof(*this));
				else
					this->mask = dmNone;
			}

			inline bool contains(DrawMask mask) const {
				return (this->mask & mask) == mask;
			}
			inline void set(DrawMask mask) {
				this->mask = (DrawMask)(this->mask | mask);
			}
			inline void unset(DrawMask mask) {
				this->mask = (DrawMask)(mask & ~mask);
			}

			DrawMask mask;
			unsigned char alpha;
			DrawState state;
			Size size;
			Rect source;

		};

		inline static DrawOpts optAlpha(unsigned char alpha) {
			DrawOpts opt(dmAlpha);
			opt.alpha = alpha;
			return opt;
		}
		inline static DrawOpts optState(DrawState state) {
			DrawOpts opt(dmState);
			opt.state = state;
			return opt;
		}
		inline static DrawOpts optSource(const Rect& source) {
			DrawOpts opt(dmSource);
			opt.source = source;
			return opt;
		}



		inline static unsigned char getOptAlpha(const DrawOpts* opt, unsigned char def = 255) {
			return (opt && opt->contains(dmAlpha)) ? opt->alpha : def;
		}
		inline static DrawState getOptState(const DrawOpts* opt, DrawState def = stateNormal) {
			return (opt && opt->contains(dmState)) ? opt->state : def;
		}
		inline static Size getOptSize(const DrawOpts* opt, const Size& def = Size()) {
			return (opt && opt->contains(dmSize)) ? opt->size : def;
		}
		inline static Rect getOptSource(const DrawOpts* opt, const Rect& def = Rect()) {
			return (opt && opt->contains(dmSource)) ? opt->source : def;
		}





		virtual ~Image() {}

		virtual Size getSize() {
			return this->getImageSize();
		}
		virtual void setSize(const Size& size) {
		}

		virtual Size getImageSize()=0;
		virtual void draw(HDC dc, const Point& pt, const DrawOpts* opts = 0)=0;
		void drawStretched(HDC dc, const Rect& rc, const DrawOpts* opts = 0);
		void drawCentered(HDC dc, const Point& pt, const DrawOpts* opts = 0);
		void drawCentered(HDC dc, const Rect& rc, const DrawOpts* opts = 0) {
			this->drawCentered(dc, rc.getCenter(), opts);
		}
		
		void drawPattern(HDC dc, const Region& rgn, const Point& origin = Point(), const DrawOpts* opts = 0);
		void drawPattern(HDC dc, const Rect& rc, const Point& origin = Point(), const DrawOpts* _opt = 0);



		static oImage loadIcon32(const char* filename, int size = 32, int bits = 32) {
			return loadIcon32(0, filename, size, bits);
		}
		static oImage loadIcon32(HINSTANCE inst, const char* id, int size = 32, int bits = 32);



	};

	class ImageSized: public Image {
	public:
		ImageSized(const Size& size=Size()):_defSize(size) {
		}

		Size getSize() {
			return _defSize ? _defSize : this->getImageSize();
		}
		void setSize(const Size& size) {
			_defSize = size;
		}

	protected:
		Size _defSize;
	};


	class Icon: public ImageSized {
	public:

		STAMINA_OBJECT_CLASS(Icon, ImageSized);

		static SharedPtr<Icon> OEMIcon(LPTSTR id);
		Icon(const char* filename, int size = 32, int bits = 32);
		Icon(HINSTANCE hinst, const char* resId, int size = 32, int bits = 32);
		Icon(HICON icon, bool shared);
		~Icon();

		Size getImageSize();
		void draw(HDC dc, const Point& pt, const Image::DrawOpts* opts = 0);



		HICON getHIcon() {
			return _icon; 
		}

	protected:
		HICON _icon;
		Size _size;
		bool _shared;
	};

	class Bitmap: public ImageSized {
	public:

		STAMINA_OBJECT_CLASS(Bitmap, ImageSized);

		Bitmap(HBITMAP bmp, bool shared);
		Bitmap(const char* filename, bool trans=false);
		Bitmap(HINSTANCE hinst, const char* resId, bool trans=false);
		~Bitmap();

		Size getImageSize();
		void draw(HDC dc, const Point& pt, const Image::DrawOpts* opts = 0);
		HBITMAP getHBitmap() {
			return _bitmap; 
		}

	protected:
		void setBitmap(HBITMAP bmp, bool shared);
		Bitmap() {}
		HBITMAP _bitmap;
		Size _size;
		bool _shared;
	};


	class Bitmap32: public Bitmap {
	public:

		STAMINA_OBJECT_CLASS(Bitmap32, Bitmap);

		//Bitmap32(const char* iconfile, int size);
		Bitmap32(HBITMAP bmp, bool transparent, bool shared);

		void draw(HDC dc, const Point& pt, const Image::DrawOpts* opts = 0);

	protected:
		Bitmap32() {}
		void setBitmap(HBITMAP bmp, bool transparent, bool shared);
		void prepareBitmap();
		bool _transparency;
	};


	class Brush: public Image {
	public:

		STAMINA_OBJECT_CLASS(Brush, Image);

		Brush(HBRUSH brush, bool shared = false) {
			_brush = brush;
			_shared = shared;
		}
		~Brush() {
			if (!_shared) {
				DeleteObject(_brush);
			}
		}
		void draw(HDC dc, const Point& pt, const Image::DrawOpts* opts = 0) {
			Rect rc (pt, Image::getOptSize(opts, this->getSize()));
			FillRect(dc, rc.ref(), _brush);

		}
		Size getImageSize() {
			return Size(8, 8);
		}
		HBRUSH getHBrush() {
			return _brush;
		}

	protected:
		HBRUSH _brush;
		bool _shared;

	};

	class Pattern: public Image {
	public:

		STAMINA_OBJECT_CLASS(Pattern, Image);

		Pattern(const oImage& pattern):_pattern(pattern) {
		}

		Size getImageSize() {
			return _pattern->getSize();
		}

		void draw(HDC dc, const Point& pt, const Image::DrawOpts* opts = 0) {
			DrawOpts subopts(opts);
			subopts.unset(Image::dmSize);
			_pattern->drawPattern(dc, Rect(pt, Image::getOptSize(opts, this->getSize())), Point(), &subopts);
		}

		oImage getPattern() {
			return _pattern;
		}

	protected:
		oImage _pattern;
	};


#ifdef _INC_COMMCTRL

	class ImageListIndex: public ImageSized {
	public:
		ImageListIndex(HIMAGELIST iml, int idx):_iml(iml), _idx(idx) {
		}
		Size getImageSize();
		void draw(HDC dc, const Point& pt, const Image::DrawOpts* opts = 0);

	protected:
		HIMAGELIST _iml;
		int _idx;
	};

#endif

} };