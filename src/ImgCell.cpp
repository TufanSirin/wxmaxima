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

#include "ImgCell.h"

#include <wx/file.h>
#include <wx/filename.h>
#include <wx/filesys.h>
#include <wx/fs_mem.h>
#include <wx/clipbrd.h>

ImgCell::ImgCell() : MathCell()
{
  m_bitmap = NULL;
  m_type = MC_TYPE_IMAGE;
  m_fileSystem = NULL;
  m_drawRectangle = true;
  m_imageBorderWidth = 1;
}

int ImgCell::s_counter = 0;

// constructor which load image
ImgCell::ImgCell(wxString image, bool remove, wxFileSystem *filesystem) : MathCell()
{
  m_bitmap = NULL;
  m_type = MC_TYPE_IMAGE;
  m_fileSystem = filesystem; // != NULL when loading from wxmx
  m_drawRectangle = true;
  if (image != wxEmptyString)
    LoadImage(image, remove);
}

ImgCell::~ImgCell()
{
  if (m_bitmap != NULL)
    delete m_bitmap;
  if (m_next != NULL)
    delete m_next;
}

void ImgCell::LoadImage(wxString image, bool remove)
{
  if (m_bitmap != NULL)
    delete m_bitmap;

  bool loadedImage = false;

  if (m_fileSystem) {
    wxFSFile *fsfile = m_fileSystem->OpenFile(image);
    if (fsfile) { // open successful

      wxInputStream *istream = fsfile->GetStream();
      wxImage pngImage(*istream, wxBITMAP_TYPE_PNG);
      if (pngImage.Ok())
      {
        loadedImage = true;
        m_bitmap = new wxBitmap(pngImage);
      }
      delete fsfile;
    }
    m_fileSystem = NULL;
  }
  else {
    if (wxFileExists(image))
    {
      wxImage pngImage(image);

      if (pngImage.Ok())
      {
        loadedImage = true;
        m_bitmap = new wxBitmap(pngImage);
      }

      if (remove)
        wxRemoveFile(image);
    }
  }

  if (!loadedImage)
  {
    m_bitmap = new wxBitmap;

    m_bitmap->Create(400, 250);

    wxString error(_("Error"));

    wxMemoryDC dc;
    dc.SelectObject(*m_bitmap);

    int width = 0, height = 0;
    dc.GetTextExtent(error, &width, &height);

    dc.DrawRectangle(0, 0, 400, 250);
    dc.DrawLine(0, 0,   400, 250);
    dc.DrawLine(0, 250, 400, 0);
    dc.DrawText(error, 200 - width/2, 125 - height/2);

    dc.GetTextExtent(image, &width, &height);
    dc.DrawText(image, 200 - width/2, 150 - height/2);

  }
}

void ImgCell::SetBitmap(wxBitmap bitmap)
{
  if (m_bitmap != NULL)
    delete m_bitmap;

  m_width = m_height = -1;
  m_bitmap = new wxBitmap(bitmap);
}

MathCell* ImgCell::Copy()
{
  ImgCell* tmp = new ImgCell;
  CopyData(this, tmp);
  tmp->m_drawRectangle = m_drawRectangle;

  tmp->m_bitmap = new wxBitmap(*m_bitmap);

  return tmp;
}

void ImgCell::Destroy()
{
  if (m_bitmap != NULL)
    delete m_bitmap;
  m_bitmap = NULL;
  m_next = NULL;
}

void ImgCell::RecalculateWidths(CellParser& parser, int fontsize)
{
  int height;
  if (m_bitmap != NULL)
  {
    height = m_bitmap->GetHeight();
    m_width  = m_bitmap->GetWidth();
  }
  else
  {
    height = 0;
    m_width  = 0;
    }

  double scale = parser.GetScale();
  scale = MAX(scale, 1.0);

  // Shrink to .9* the canvas size
  if(scale * m_width > .9 * m_canvasSize.x)
    scale = .9 * m_canvasSize.x / m_width;
  if(scale * height > .9 * m_canvasSize.y)
    scale = .9 * m_canvasSize.y / height;

  m_width = (int) (scale * m_width) + 2 * m_imageBorderWidth;
  ResetData();
}

void ImgCell::RecalculateSize(CellParser& parser, int fontsize)
{
  int width;
  if (m_bitmap != NULL)
  {
    m_height = m_bitmap->GetHeight();
    width  = m_bitmap->GetWidth();
  }
  else
  {
    m_height = 0;
    width  = 0;
  }
  
  double scale = parser.GetScale();
  scale = MAX(scale, 1.0);
  
  // Shrink to .9* the canvas size
  if(scale * width > .9 * m_canvasSize.x)
    scale = .9 * m_canvasSize.x / width;
  if(scale * m_height > .9 * m_canvasSize.y)
    scale = .9 * m_canvasSize.y / m_height;

  m_height= (int) (scale * m_height) + 2 * m_imageBorderWidth;

  m_center = m_height / 2;
}

void ImgCell::Draw(CellParser& parser, wxPoint point, int fontsize)
{
  wxDC& dc = parser.GetDC();

  if (DrawThisCell(parser, point) && m_bitmap != NULL)
  {
    wxMemoryDC bitmapDC;
    double scale = parser.GetScale();

    SetPen(parser);
    if (m_drawRectangle)
      
      dc.DrawRectangle(wxRect(point.x, point.y - m_center, m_width, m_height));  

    bool rescale=false;
    if(m_bitmap->GetHeight() + 2 * m_imageBorderWidth != m_height)
      rescale=true;
    
    if (rescale)
    {
      wxImage img = m_bitmap->ConvertToImage();
      img.Rescale(m_width - 2 * m_imageBorderWidth, m_height - 2 * m_imageBorderWidth,wxIMAGE_QUALITY_BICUBIC);

      wxBitmap bmp = img;
      bitmapDC.SelectObject(bmp);
    }
    else
    {
      bitmapDC.SelectObject(*m_bitmap);
    }

    dc.Blit(point.x + m_imageBorderWidth, point.y - m_center + m_imageBorderWidth, m_width - 2 * m_imageBorderWidth, m_height - 2 * m_imageBorderWidth, &bitmapDC, 0, 0);
  }

  MathCell::Draw(parser, point, fontsize);
}

wxString ImgCell::ToString()
{
  return _(" (Graphics) ");
}

wxString ImgCell::ToTeX()
{
  return _(" (Graphics) ");
}

wxSize ImgCell::ToImageFile(wxString file)
{
  wxImage image = m_bitmap->ConvertToImage();

  SizeInMillimeters retval;
  if(image.SaveFile(file, wxBITMAP_TYPE_PNG))
  {
    // Saving an animation might need loads of time. Since we use this time
    // in the foreground and many operation systems assume that an application
    // that is busy with other things and therefore isn't reacting is stuck
    // and therefore offer to kill the application we should now listen to
    // requests from the OS before continuing the save.
    wxYield();

    return image.GetSize();
  }
  else
  {
    wxSize retval;
    retval.x = retval.y = -1;
    return retval;
  }
}

wxString ImgCell::ToXML()
{
  wxImage image = m_bitmap->ConvertToImage();
  wxString basename = ImgCell::WXMXGetNewFileName();

	// add to memory
  wxMemoryFSHandler::AddFile(basename, image, wxBITMAP_TYPE_PNG);

  return (m_drawRectangle ? wxT("<img>") : wxT("<img rect=\"false\">")) +
         basename + wxT("</img>");
}

wxString ImgCell::WXMXGetNewFileName()
{
   wxString file(wxT("image"));
   file << (++s_counter) << wxT(".png");
   return file;
}

bool ImgCell::CopyToClipboard()
{
  if (wxTheClipboard->Open())
  {
    bool res = wxTheClipboard->SetData(new wxBitmapDataObject(*m_bitmap));
    wxTheClipboard->Close();
    return res;
  }
  return false;
}
