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

#include "DiffCell.h"

DiffCell::DiffCell() : MathCell()
{
  m_baseCell = NULL;
  m_diffCell = NULL;
}

DiffCell::~DiffCell()
{
  if (m_baseCell != NULL)
    delete m_baseCell;
  if (m_diffCell != NULL)
    delete m_diffCell;
  if (m_next != NULL)
    delete m_next;
}

void DiffCell::SetParent(MathCell *parent)
{
  m_group = parent;
  if (m_baseCell != NULL)
    m_baseCell->SetParentList(parent);
  if (m_diffCell != NULL)
    m_diffCell->SetParentList(parent);
}

MathCell* DiffCell::Copy()
{
  DiffCell* tmp = new DiffCell;
  CopyData(this, tmp);
  tmp->SetDiff(m_diffCell->CopyList());
  tmp->SetBase(m_baseCell->CopyList());

  return tmp;
}

void DiffCell::Destroy()
{
  if (m_baseCell != NULL)
    delete m_baseCell;
  if (m_diffCell != NULL)
    delete m_diffCell;
  m_baseCell = NULL;
  m_diffCell = NULL;
  m_next = NULL;
}

void DiffCell::SetDiff(MathCell *diff)
{
  if (diff == NULL)
    return;
  if (m_diffCell != NULL)
    delete m_diffCell;
  m_diffCell = diff;

  m_diffCell->m_SuppressMultiplicationDot=true;
}

void DiffCell::SetBase(MathCell *base)
{
  if (base == NULL)
    return;
  if (m_baseCell != NULL)
    delete m_baseCell;
  m_baseCell = base;
}

void DiffCell::RecalculateWidths(CellParser& parser, int fontsize)
{
  double scale = parser.GetScale();
  m_baseCell->RecalculateWidthsList(parser, fontsize);
  m_diffCell->RecalculateWidthsList(parser, fontsize);
  m_width = m_baseCell->GetFullWidth(scale) + m_diffCell->GetFullWidth(scale) + 2*MC_CELL_SKIP;
  ResetData();
}

void DiffCell::RecalculateSize(CellParser& parser, int fontsize)
{
  m_baseCell->RecalculateSizeList(parser, fontsize);
  m_diffCell->RecalculateSizeList(parser, fontsize);
  m_center = MAX(m_diffCell->GetMaxCenter(), m_baseCell->GetMaxCenter());
  m_height = m_center + MAX(m_diffCell->GetMaxDrop(), m_baseCell->GetMaxDrop());
}

void DiffCell::Draw(CellParser& parser, wxPoint point, int fontsize)
{
  if (DrawThisCell(parser, point)) {
    wxPoint bs, df;
    df.x = point.x;
    df.y = point.y;
    m_diffCell->DrawList(parser, df, fontsize);

    bs.x = point.x + m_diffCell->GetFullWidth(parser.GetScale()) + 2*MC_CELL_SKIP;
    bs.y = point.y;
    m_baseCell->DrawList(parser, bs, fontsize);
  }

  MathCell::Draw(parser, point, fontsize);
}

wxString DiffCell::ToString()
{
  MathCell* tmp = m_baseCell->m_next;
  wxString s = wxT("'diff(");
  if (tmp != NULL)
    s += tmp->ListToString();
  s += m_diffCell->ListToString();
  s += wxT(")");
  return s;
}

wxString DiffCell::ToTeX()
{
  wxString s = m_diffCell->ListToTeX() + m_baseCell->ListToTeX();
  return s;
}

wxString DiffCell::ToXML()
{
  return _T("<d>") + m_baseCell->ListToXML() +
    m_diffCell->ListToXML() + _T("</d>");
}

void DiffCell::SelectInner(wxRect& rect, MathCell** first, MathCell** last)
{
  *first = NULL;
  *last = NULL;
  if (m_baseCell->ContainsRect(rect))
    m_baseCell->SelectRect(rect, first, last);
  if (*first == NULL || *last == NULL) {
    *first = this;
    *last = this;
  }
}
