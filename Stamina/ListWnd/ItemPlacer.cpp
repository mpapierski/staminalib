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


/* Model statyczny */
#include "stdafx.h"

#include "ListView.h"
#include "ItemPlacer.h"
namespace Stamina
{
namespace ListWnd
{
	bool ItemPerRow::place(ListView* lv, const oItemCollection& coll, const oItemCollection& parent, RefreshFlags refresh) {
		/*
		Warunki:
		- Ustawiana kolekcja POSIADA prawid�ow� pozycj�.
		- Ustawiana kolekcja JEST widoczna
		- Ustawiamy TYLKO podelementy...

		W ItemPerRow nie musimy sprawdzac flagRepos, mozemy zobaczyc
		czy aktualna pozycja odpowiada spodziewanej

		*/
		ObjLocker lock(coll, lockWrite);
		// pt - punkt w kt�rym aktualnie si� znajdujemy...
		Rect rect = coll->getEntry()->getICollection()->getItemsRect(lv, coll);
		rect.left += marginLeft;
		rect.top += marginTop;
		rect.right -= marginRight;
		rect.bottom -= marginBottom;

		Point start = coll->getPos();
		Point pt = rect.getLT();
		int width = rect.width();
		bool paint = false;
		int wholeWidth = width;
		ItemList& items = coll->getItemList();
		bool firstVisible = true;
		for (ItemList::iterator it = items.begin(); it != items.end(); ++it) {
			Item* item = (*it).get();
			bool paintItem = false;
			if (!item->isVisible()) { // nie zajmujemy si� niewidocznymi
				if (item->getFlag(flagChanged)) {
					paint = true;
					item->setFlag(flagChanged, false);
				}
				continue;
			}
			pt.y += firstVisible ? this->spaceTop : this->spaceBetween;
			firstVisible = false;
			bool positioned = item->isPositioned();
			Rect oldRect = positioned ? item->getRect() : Rect();
			if (setPos(item, pt)) {
				paintItem = true;
			}
			if ((refresh & refreshSize) || item->getFlag(flagResize) || !positioned) {
				// ustalamy rozmiar...
				if (setSize(item, 
					item->getEntry()->getEntrySize(lv, *it, coll, Size(width, 0 /*dowolna wysoko��*/))
					)) {
					paintItem = true;
				}
				if (!positioned) {
					item->setState(stateNormal); // zosta� spozycjonowany...
					paintItem = true;
				}
			}

			if (item->getFlag(flagRepaint)) {
				paintItem = true;
			}
			if (paintItem) {
				Rect r = item->getRect();
				lv->repaintItemRect(item->getFlag(flagRepaintWhole) ? item->getWholeRect() : r);
				if (positioned && r != oldRect)
					lv->repaintItemRect(oldRect);
				paint = true;
			}
			if (item->getItemCollection()) {
				if (refreshItems(item->getItemCollection(), lv, refresh, coll))
					paint = true;

			}
			if (item->getWholeSize().w > wholeWidth)
				wholeWidth = item->getWholeSize().w;
			// zdejmujemy flagi...
			item->setRefreshFlag(refreshAll, false);
			pt.y += item->getWholeSize().h+1;
		} // for
		if (!firstVisible)
			pt.y += this->spaceBottom;
		if (paint || refresh == refreshSize || coll->getFlag(flagSubitemsChanged)) { // co� si� pozmienia�o...
			setWholeSize(coll.get(), Size(wholeWidth, pt.y - start.y));
			coll->setFlag(flagSubitemsChanged, false);
			paint = true;
		}
		return paint;
	}

} /* ListWnd */

} /* Stamina */

