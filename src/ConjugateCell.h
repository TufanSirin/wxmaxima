// -*- mode: c++; c-file-style: "linux"; c-basic-offset: 2; indent-tabs-mode: nil -*-
//
//  Copyright (C) 2004-2015 Andrej Vodopivec <andrej.vodopivec@gmail.com>
//            (C) 2014-2015 Gunter Königsmann <wxMaxima@physikbuch.de>
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

#ifndef CONJUGATECELL_H
#define CONJUGATECELL_H

#include "MathCell.h"

/*! \file

  This file defines the class for the cell type that represents an conjugate(x) block.
 */

/*! A cell that represents an conjugate(x) block

 */
class ConjugateCell : public MathCell
{
public:
  ConjugateCell();
  ~ConjugateCell();
  void Destroy();
  void SetInner(MathCell *inner);
  MathCell* Copy();
  void SelectInner(wxRect& rect, MathCell** first, MathCell** last);
  bool BreakUp();
  void Unbreak();
  void SetParent(MathCell *parent);
protected:
  MathCell *m_innerCell;
  MathCell *m_open, *m_close, *m_last;
  void RecalculateSize(CellParser& parser, int fontsize);
  void RecalculateWidths(CellParser& parser, int fontsize);
  void Draw(CellParser& parser, wxPoint point, int fontsize);
  wxString ToString();
  wxString ToTeX();
  wxString ToXML();
};

#endif // CONJUGATECELL_H
