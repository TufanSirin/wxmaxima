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

#include "MathCell.h"

MathCell::MathCell()
{
  m_next = NULL;
  m_previous = NULL;
  m_nextToDraw = NULL;
  m_previousToDraw = NULL;
  m_group = NULL;
  m_fullWidth = -1;
  m_lineWidth = -1;
  m_maxCenter = -1;
  m_maxDrop = -1;
  m_width = -1;
  m_height = -1;
  m_center = -1;
  m_breakLine = false;
  m_breakPage = false;
  m_forceBreakLine = false;
  m_bigSkip = true;
  m_isHidden = false;
  m_isBroken = false;
  m_highlight = false;
  m_type = MC_TYPE_DEFAULT;
  m_textStyle = TS_VARIABLE;
  m_SuppressMultiplicationDot = false;
  m_imageBorderWidth = 0;
}

/***
 * Derived classes must test if m_next equals NULL if it doesn't delete it!!!
 */
MathCell::~MathCell()
{}

void MathCell::SetType(int type)
{
  m_type = type;

  switch (m_type)
  {
  case MC_TYPE_MAIN_PROMPT:
    m_textStyle = TS_MAIN_PROMPT;
    break;
  case MC_TYPE_PROMPT:
    m_textStyle = TS_OTHER_PROMPT;
    break;
  case MC_TYPE_LABEL:
    m_textStyle = TS_LABEL;
    break;
  case MC_TYPE_INPUT:
    m_textStyle = TS_INPUT;
    break;
  case MC_TYPE_ERROR:
    m_textStyle = TS_ERROR;
    break;
  case MC_TYPE_TEXT:
    m_textStyle = TS_TEXT;
    break;
  case MC_TYPE_SUBSUBSECTION:
    m_textStyle = TS_SUBSUBSECTION;
    break;
  case MC_TYPE_SUBSECTION:
    m_textStyle = TS_SUBSECTION;
    break;
  case MC_TYPE_SECTION:
    m_textStyle = TS_SECTION;
    break;
  case MC_TYPE_TITLE:
    m_textStyle = TS_TITLE;
    break;
  default:
    m_textStyle = TS_DEFAULT;
    break;
  }
}

MathCell *MathCell::CopyList()
{
  MathCell *dest = Copy();
  MathCell *src = this->m_next;
  MathCell *ret = dest;
  
  while(src != NULL)
  {
    dest->AppendCell(src->Copy());
    src = src->m_next;
    dest = dest->m_next;
  }

  return ret;
}


void MathCell::SetParentList(MathCell *parent)
{
  MathCell *tmp=this;
  while(tmp != NULL)
  {
    tmp->SetParent(parent);
    tmp=tmp->m_next;
  }
}

/***
 * Append new cell to the end of this list.
 */
void MathCell::AppendCell(MathCell *p_next)
{
  if (p_next == NULL)
    return ;
  m_maxDrop = -1;
  m_maxCenter = -1;

  // Search the last cell in the list
  MathCell *LastInList=this;
  while(LastInList->m_next!=NULL)
    LastInList=LastInList->m_next;

  // Append this p_next to the list
  LastInList->m_next = p_next;
  LastInList->m_next->m_previous = LastInList;

  // Search the last cell in the list that is sorted by the drawing order
  MathCell *LastToDraw = LastInList;
  while (LastToDraw->m_nextToDraw != NULL)
    LastToDraw = LastToDraw->m_nextToDraw;

  // Append p_next to this list.
  LastToDraw->m_nextToDraw = p_next;
  p_next->m_previousToDraw = LastToDraw;
};


/***
 * Get the pointer to the parent group cell
 */

MathCell* MathCell::GetParent()
{
  wxASSERT_MSG(m_group != NULL,_("Bug: Math Cell that claims to have no group Cell it belongs to"));
  return m_group;
}

/***
 * Get the maximum drop of the center.

 */
int MathCell::GetMaxCenter()
{
  if (m_maxCenter < 0)
  {
    MathCell *tmp = this;
    while(tmp != NULL)
    {
      m_maxCenter = MAX(m_maxCenter, tmp->m_center);
      if(tmp->m_nextToDraw == NULL)
        break;
      if(tmp->m_nextToDraw->m_breakLine)
        break;
      tmp = tmp-> m_nextToDraw;
    }
  }
  return m_maxCenter;
}

/***
 * Get the maximum drop of cell.

\todo Convert this function to not using recursive function calls any more.
 */
int MathCell::GetMaxDrop()
{

  if (m_maxDrop < 0)
  {
    MathCell *tmp = this;
    while(tmp != NULL)
    {
      m_maxDrop = MAX(m_maxDrop, tmp->m_isBroken ? 0 : (tmp->m_height - tmp->m_center));
      if(tmp->m_nextToDraw == NULL)
        break;
      if(tmp->m_nextToDraw->m_breakLine && !tmp->m_nextToDraw->m_isBroken)
        break;
      tmp = tmp-> m_nextToDraw;
    }
  }
  return m_maxDrop;
}

//!  Get the maximum hight of cells in line.
int MathCell::GetMaxHeight()
{
  return GetMaxCenter() + GetMaxDrop();
}

/*! Get full width of this group.

  \todo Change not to use recursive function calls any more.
 */
int MathCell::GetFullWidth(double scale)
{
  // Recalculate the with of this list of cells only if this has been marked as necessary.
  if (m_fullWidth < 0)
  {
    MathCell *tmp = this;

    // We begin this calculation with a negative offset since the full width of only a single
    // cell doesn't contain the space that separates two cells - that is automatically added
    // to every cell in the next step.
    m_fullWidth = -SCALE_PX(MC_CELL_SKIP, scale);
    while(tmp != NULL)
    {
      m_fullWidth += tmp->m_width;
      tmp = tmp->m_next + SCALE_PX(MC_CELL_SKIP, scale);
    }
  }
  return m_fullWidth;
}

/*! Get the width of this line.

\todo Convert this function to not using recursive function calls any more.
 */
int MathCell::GetLineWidth(double scale)
{
  int width = m_isBroken ? 0 : m_width;
  if (m_lineWidth == -1)
  {
    if (m_nextToDraw == NULL || m_nextToDraw->m_breakLine ||
        m_nextToDraw->m_type == MC_TYPE_MAIN_PROMPT)
      m_lineWidth = width;
    else
      m_lineWidth = width + m_nextToDraw->GetLineWidth(scale) +
                    SCALE_PX(MC_CELL_SKIP, scale);
  }
  return m_lineWidth;
}

/*! Draw this cell to dc

 To make this work each derived class must draw the content of the cell
 and then call MathCall::Draw(...).
 */
void MathCell::Draw(CellParser& parser, wxPoint point, int fontsize)
{
  m_currentPoint.x = point.x;
  m_currentPoint.y = point.y;
}

void MathCell::DrawList(CellParser& parser, wxPoint point, int fontsize)
{
  MathCell *tmp=this;
  while(tmp!=NULL)
  {
    tmp->Draw(parser,point,fontsize);
    double scale = parser.GetScale();
    point.x += tmp->m_width + SCALE_PX(MC_CELL_SKIP, scale);
    tmp=tmp->m_nextToDraw;
  }
}

void MathCell::RecalculateSizeList(CellParser& parser, int fontsize)
{
  MathCell *tmp=this;

  while(tmp!=NULL)
    {
      tmp->RecalculateSize(parser, fontsize);
      tmp=tmp->m_next;
    }  
}

/*! Recalculate widths of cells. 

  (Used for changing font size since in this case all size information has to be 
  recalculated).
  
  Should set: set m_width.
*/
void MathCell::RecalculateWidthsList(CellParser& parser, int fontsize)
{
  MathCell *tmp=this;

  while(tmp!=NULL)
    {
      tmp->RecalculateWidths(parser, fontsize);
      tmp=tmp->m_next;
    }
}

void MathCell::RecalculateWidths(CellParser& parser, int fontsize)
{
  ResetData();
}

/*! Is this cell currently visible in the window?.
 */
bool MathCell::DrawThisCell(CellParser& parser, wxPoint point)
{
  int top = parser.GetTop();
  int bottom = parser.GetBottom();
  if (top == -1 || bottom == -1)
    return true;
  if (point.y - GetMaxCenter() > bottom || point.y + GetMaxDrop() < top)
    return false;
  return true;
}

/*! Get the rectangle around this cell

  \param all
   - true  return the rectangle around the whole line.
   - false return the rectangle around this cell. 
 */
wxRect MathCell::GetRect(bool all)
{
  if (m_isBroken)
    return wxRect( -1, -1, 0, 0);
  if (all)
    return wxRect(m_currentPoint.x, m_currentPoint.y - GetMaxCenter(),
                  GetLineWidth(1.0), GetMaxHeight());
  return wxRect(m_currentPoint.x, m_currentPoint.y - m_center,
                m_width, m_height);
}

/***
 * Draws a box around this cell - if all is true draws a box around the whole
 * line.
 */
void MathCell::DrawBoundingBox(wxDC& dc, bool all, int border)
{
  wxRect rect = GetRect(all);
  int x = rect.GetX(), y = rect.GetY();
  int width = rect.GetWidth(), height = rect.GetHeight();
  dc.DrawRectangle(x - border, y - border, width + 2*border, height + 2*border);
}

/***
 * Do we have an operator in this line - draw () in frac...
 */
bool MathCell::IsCompound()
{
  if (IsOperator())
    return true;
  if (m_next == NULL)
    return false;
  return m_next->IsCompound();
}

/***
 * Is operator - draw () in frac...
 */
bool MathCell::IsOperator()
{
  return false;
}

/***
 * Return the string representation of cell.
 */
wxString MathCell::ToString()
{
  return wxEmptyString;
}

wxString MathCell::ListToString()
{
  wxString retval;
  MathCell *tmp = this;
  bool firstline = true;
  
  while(tmp!=NULL)
    {
      retval+=tmp->ToString();
      if((!firstline)&&(tmp->m_forceBreakLine))
        retval+=wxT("\n");

      firstline = false;
      tmp=tmp->m_next;
    }
  
  return retval;
}

wxString MathCell::ToTeX()
{
  return wxEmptyString;
}

wxString MathCell::ListToTeX()
{
  wxString retval;
  MathCell *tmp=this;
  
  while(tmp!=NULL)
  {
    if ((tmp->m_textStyle == TS_LABEL && retval != wxEmptyString) ||
        (tmp->m_breakLine && retval != wxEmptyString))
      retval += wxT("\\]\\[");
    retval += tmp->ToTeX();
    tmp = tmp->m_next;
  }
  
  return retval;
}

wxString MathCell::ToXML()
{
  return wxEmptyString;
}

wxString MathCell::ListToXML()
{
  bool highlight=false;
  
  wxString retval;
  MathCell *tmp=this;

  while(tmp!=NULL)
  {
    if((tmp->GetHighlight())&&(!highlight))
    {
      retval+=wxT("<hl>\n");
      highlight=true;
    }
    
    if((!tmp->GetHighlight())&&(highlight))
    {
      retval+=wxT("</hl>\n");
      highlight=false;
    }
    
    retval+=tmp->ToXML();
    tmp=tmp->m_next;
  }
  
  if(highlight)
  {
    retval+=wxT("</hl>\n");
  }
  
  return retval;
}

/***
 * Get the part for diff tag support - only ExpTag overvrides this.
 */
wxString MathCell::GetDiffPart()
{
  return wxEmptyString;
}

/***
 * Find the first and last cell in rectangle rect in this line.
 */
void MathCell::SelectRect(wxRect& rect, MathCell** first, MathCell** last)
{
  SelectFirst(rect, first);
  if (*first != NULL)
  {
    *last = *first;
    (*first)->SelectLast(rect, last);
    if (*last == *first)
      (*first)->SelectInner(rect, first, last);
  }
  else
    *last = NULL;
}

/***
 * Find the first cell in rectangle rect in this line.
 */
void MathCell::SelectFirst(wxRect& rect, MathCell** first)
{
  if (rect.Intersects(GetRect(false)))
    *first = this;
  else if (m_nextToDraw != NULL)
    m_nextToDraw->SelectFirst(rect, first);
  else
    *first = NULL;
}

/***
 * Find the last cell in rectangle rect in this line.
 */
void MathCell::SelectLast(wxRect& rect, MathCell** last)
{
  if (rect.Intersects(GetRect(false)))
    *last = this;
  if (m_nextToDraw != NULL)
    m_nextToDraw->SelectLast(rect, last);
}

/***
 * Select rectangle in deeper cell - derived classes should override this
 */
void MathCell::SelectInner(wxRect& rect, MathCell** first, MathCell** last)
{
  *first = this;
  *last = this;
}

bool MathCell::BreakLineHere()
{
  return (!m_isBroken && (m_breakLine || m_forceBreakLine));
}

bool MathCell::ContainsRect(wxRect& sm, bool all)
{
  wxRect big = GetRect(all);
  if (big.x <= sm.x &&
          big.y <= sm.y &&
          big.x + big.width >= sm.x + sm.width &&
          big.y + big.height >= sm.y + sm.height)
    return true;
  return false;
}

/*!
 Resets remembered data.

 Resets cached data like width and the height of the current cell
 as well as the vertical position of the center. Temporarily unbreaks all
 lines until the widths are recalculated if there aren't any hard line 
 breaks.
 */
void MathCell::ResetData()
{
  m_fullWidth = -1;
  m_lineWidth = -1;
  m_maxCenter = -1;
  m_maxDrop = -1;
//  m_currentPoint.x = -1;
//  m_currentPoint.y = -1;
  m_breakLine = m_forceBreakLine;
}


MathCell *MathCell::first()
{
  MathCell *tmp=this;
  while(tmp->m_previous)
    tmp = tmp->m_previous;

  return tmp;
}

MathCell *MathCell::last()
{
  MathCell *tmp=this;
  while(tmp->m_next)
    tmp = tmp->m_next;

  return tmp;
}

void MathCell::Unbreak()
{
  ResetData();
  m_isBroken = false;
  m_nextToDraw = m_next;
  if (m_nextToDraw != NULL)
    m_nextToDraw->m_previousToDraw = this;
}

void MathCell::UnbreakList()
{
  MathCell *tmp=this;
  while(tmp != NULL)
  {
    tmp->Unbreak();
    tmp=tmp->m_next;
  }
}

void MathCell::DestroyList()
{
  MathCell *tmp, *next;
  tmp = this;
  while(tmp != NULL)
  {
    next = tmp->m_next;
    tmp->Destroy();
    tmp = next;
  }
}
/***
 * Set the pen in device context accordint to the style of the cell.
 */
void MathCell::SetPen(CellParser& parser)
{
  wxDC& dc = parser.GetDC();
  if (m_highlight)
    dc.SetPen(*(wxThePenList->FindOrCreatePen(parser.GetColor(TS_HIGHLIGHT),
                1, wxPENSTYLE_SOLID)));
  else if (m_type == MC_TYPE_PROMPT)
    dc.SetPen(*(wxThePenList->FindOrCreatePen(parser.GetColor(TS_OTHER_PROMPT),
                1, wxPENSTYLE_SOLID)));
  else if (m_type == MC_TYPE_INPUT)
    dc.SetPen(*(wxThePenList->FindOrCreatePen(parser.GetColor(TS_INPUT),
                1, wxPENSTYLE_SOLID)));
  else
    dc.SetPen(*(wxThePenList->FindOrCreatePen(parser.GetColor(TS_DEFAULT),
                    1, wxPENSTYLE_SOLID)));
}

/***
 * Reset the pen in the device context.
 */
void MathCell::UnsetPen(CellParser& parser)
{
  wxDC& dc = parser.GetDC();
  if (m_type == MC_TYPE_PROMPT || m_type == MC_TYPE_INPUT || m_highlight)
    dc.SetPen(*(wxThePenList->FindOrCreatePen(parser.GetColor(TS_DEFAULT),
                1, wxPENSTYLE_SOLID)));
}

/***
 * Copy all important data from s to t
 */
void MathCell::CopyData(MathCell* s, MathCell* t)
{
  t->m_altCopyText = s->m_altCopyText;
  t->m_forceBreakLine = s->m_forceBreakLine;
  t->m_type = s->m_type;
  t->m_textStyle = s->m_textStyle;
}

void MathCell::SetForeground(CellParser& parser)
{
  wxColour color;
  wxDC& dc = parser.GetDC();
  if (m_highlight)
  {
    color = parser.GetColor(TS_HIGHLIGHT);
  }
  else {
    switch (m_type)
    {
    case MC_TYPE_PROMPT:
      color = parser.GetColor(TS_OTHER_PROMPT);
      break;
    case MC_TYPE_MAIN_PROMPT:
      color = parser.GetColor(TS_MAIN_PROMPT);
      break;
    case MC_TYPE_ERROR:
      color = wxColour(wxT("red"));
      break;
    case MC_TYPE_LABEL:
      color = parser.GetColor(TS_LABEL);
      break;
    default:
      color = parser.GetColor(m_textStyle);
      break;
    }
  }

  dc.SetTextForeground(color);
}

bool MathCell::IsMath()
{
  return !(m_textStyle == TS_DEFAULT ||
           m_textStyle == TS_LABEL ||
           m_textStyle == TS_INPUT);
}

wxSize MathCell::m_canvasSize;
