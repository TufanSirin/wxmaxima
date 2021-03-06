// -*- mode: c++; c-file-style: "linux"; c-basic-offset: 2; indent-tabs-mode: nil -*-
//
//  Copyright (C) 2007-2015 Andrej Vodopivec <andrej.vodopivec@gmail.com>
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

#include "SlideShowCell.h"
#include "ImgCell.h"

#include <wx/file.h>
#include <wx/filename.h>
#include <wx/filesys.h>
#include <wx/fs_mem.h>
#include <wx/utils.h>
#include <wx/clipbrd.h>
#include <wx/config.h>
#include "Config.h"

SlideShow::SlideShow(wxFileSystem *filesystem,int framerate) : MathCell()
{
  m_size = m_displayed = 0;
  m_type = MC_TYPE_SLIDE;
  m_fileSystem = filesystem; // NULL when not loading from wxmx
  m_framerate = framerate;
  m_imageBorderWidth = 1;
}

SlideShow::~SlideShow()
{
  for (int i=0; i<m_size; i++)
    delete m_bitmaps[i];
  if (m_next != NULL)
    delete m_next;
}


int SlideShow::GetFrameRate()
{
  int framerate=2;

  if(m_framerate>-1)
    framerate=m_framerate;
  else
    {
      wxConfigBase *config = wxConfig::Get();
      
      config->Read(wxT("DefaultFramerate"),&framerate);
    }
  return(framerate);
}

int SlideShow::SetFrameRate(int Freq)
{

  m_framerate=Freq;
  
  if(Freq<0)
     m_framerate=-1;
  else{
    if(Freq<1)
      m_framerate=1;
    if(Freq>200)
      m_framerate=200;
    }

  return m_framerate;
}

void SlideShow::LoadImages(wxArrayString images)
{
  m_size = images.GetCount();

  if (m_fileSystem) {
    for (int i=0; i<m_size; i++)
    {
      bool loadedImage = false;

      wxFSFile *fsfile = m_fileSystem->OpenFile(images[i]);
      if (fsfile) { // open successful

        wxInputStream *istream = fsfile->GetStream();
        wxImage pngImage(*istream, wxBITMAP_TYPE_PNG);
        if (pngImage.Ok())
        {
          loadedImage = true;
          m_bitmaps.push_back(new wxBitmap(pngImage));
        }
        delete fsfile;
      }

      if (!loadedImage)
      {
        wxBitmap *bitmap = new wxBitmap;
        bitmap->Create(400, 250);

        wxString error = wxString::Format(_("Error %d"), i);

        wxMemoryDC dc;
        dc.SelectObject(*bitmap);

        int width = 0, height = 0;
        dc.GetTextExtent(error, &width, &height);
        dc.DrawRectangle(0, 0, 400, 250);
        dc.DrawLine(0, 0,   400, 250);
        dc.DrawLine(0, 250, 400, 0);
        dc.DrawText(error, 200 - width/2, 125 - height/2);

        m_bitmaps.push_back(bitmap);
      }
    }

    m_fileSystem = NULL;
  }
  else
    for (int i=0; i<m_size; i++)
    {
      bool loadedImage = false;

      if (wxFileExists(images[i]))
      {
        wxBitmap *bitmap = new wxBitmap;
        if (bitmap->LoadFile(images[i], wxBITMAP_TYPE_PNG))
        {
          loadedImage = true;
          m_bitmaps.push_back(bitmap);
        }
        else
          delete bitmap;

        wxRemoveFile(images[i]);
      }

      if (!loadedImage)
      {
        wxBitmap *bitmap = new wxBitmap;
        bitmap->Create(400, 250);

        wxString error = wxString::Format(_("Error %d"), i);

        wxMemoryDC dc;
        dc.SelectObject(*bitmap);

        int width = 0, height = 0;
        dc.GetTextExtent(error, &width, &height);
        
        dc.DrawRectangle(0, 0, 400, 250);
        dc.DrawLine(0, 0,   400, 250);
        dc.DrawLine(0, 250, 400, 0);
        dc.DrawText(error, 200 - width/2, 125 - height/2);

        m_bitmaps.push_back(bitmap);
      }
    }

  m_displayed = 0;
}

MathCell* SlideShow::Copy()
{
  SlideShow* tmp = new SlideShow;
  CopyData(this, tmp);

  for(int i=0;i<m_bitmaps.size();i++)
  {
    wxBitmap *image = new wxBitmap(*m_bitmaps[i]);
    tmp->m_bitmaps.push_back(image);
  }

  tmp->m_size = m_size;
  
  return tmp;
}

void SlideShow::Destroy()
{
  for (int i=0; i<m_size; i++)
    if (m_bitmaps[i] != NULL)
    {
      delete m_bitmaps[i];
      m_bitmaps[i] = NULL;
    }
  m_next = NULL;
}

void SlideShow::SetDisplayedIndex(int ind)
{
  if (ind >= 0 && ind < m_size)
    m_displayed = ind;
  else
    m_displayed = m_size - 1;
}

void SlideShow::RecalculateWidths(CellParser& parser, int fontsize)
{
  int height;
  if (m_bitmaps[m_displayed] != NULL)
  {
    height = m_bitmaps[m_displayed]->GetHeight();
    m_width  = m_bitmaps[m_displayed]->GetWidth();
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

void SlideShow::RecalculateSize(CellParser& parser, int fontsize)
{
  int width;
  if (m_bitmaps[m_displayed] != NULL)
  {
    m_height = m_bitmaps[m_displayed]->GetHeight();
    width  = m_bitmaps[m_displayed]->GetWidth();
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
  
  MathCell::RecalculateSize(parser, fontsize);
}

void SlideShow::Draw(CellParser& parser, wxPoint point, int fontsize)
{
  if (DrawThisCell(parser, point) && m_bitmaps[m_displayed] != NULL)
  {
    wxDC& dc = parser.GetDC();
    wxMemoryDC bitmapDC;

    dc.SetPen(*wxRED_PEN);

    dc.DrawRectangle(wxRect(point.x, point.y - m_center, m_width, m_height));

    bool rescale=false;
    if (m_bitmaps[m_displayed] != NULL)
    {
      if(m_bitmaps[m_displayed]->GetHeight() + 2 * m_imageBorderWidth != m_height)
        rescale=true;;
    }               
    
    if (rescale)
    {
      wxImage img = m_bitmaps[m_displayed]->ConvertToImage();
      img.Rescale(m_width - 2 * m_imageBorderWidth,m_height - 2 * m_imageBorderWidth,wxIMAGE_QUALITY_BICUBIC);
      
      wxBitmap bmp = img;
      bitmapDC.SelectObject(bmp);
    }
    else
      bitmapDC.SelectObject(*m_bitmaps[m_displayed]);

    dc.Blit(point.x + m_imageBorderWidth, point.y - m_center + m_imageBorderWidth,m_width - 2 * m_imageBorderWidth,m_height - 2 * m_imageBorderWidth, &bitmapDC, 0, 0);
  }
  MathCell::Draw(parser, point, fontsize);
}

wxString SlideShow::ToString()
{
  return wxT(" << Graphics >> ");
}

wxString SlideShow::ToTeX()
{
  return wxT(" << Graphics >> ");
}

wxString SlideShow::ToXML()
{
  wxString images;

  for (int i=0; i<m_size; i++) {
    wxImage image = m_bitmaps[i]->ConvertToImage();
    wxString basename = ImgCell::WXMXGetNewFileName();

    // add to memory
    wxMemoryFSHandler::AddFile(basename, image, wxBITMAP_TYPE_PNG);

    images += basename + wxT(";");
  }

  if(m_framerate<0)
    return wxT("\n<slide>") + images + wxT("</slide>");
  else
    return wxT("\n<slide fr=\"")+ wxString::Format(wxT("%i\">"),GetFrameRate()) + images + wxT("</slide>");
}

wxSize SlideShow::ToImageFile(wxString file)
{
  wxImage image = m_bitmaps[m_displayed]->ConvertToImage();

  if(image.SaveFile(file, wxBITMAP_TYPE_PNG))
    return image.GetSize();
  else
  {
    wxSize retval;
    retval.x = retval.y = -1;
    return retval;
  }
}

wxSize SlideShow::ToGif(wxString file)
{
  wxArrayString which;
  bool success = true;

  wxString convert(wxT("convert -delay "+wxString::Format(wxT("%i"),100/GetFrameRate())));
  wxString convertArgs;

  wxString tmpdir = wxFileName::GetTempDir();

  for (int i=0; i<m_size; i++)
  {
    wxFileName imgname(tmpdir, wxString::Format(wxT("wxm_anim%d.png"), i));

    wxImage image = m_bitmaps[i]->ConvertToImage();
    image.SaveFile(imgname.GetFullPath(), wxBITMAP_TYPE_PNG);

    // Saving an animation might need loads of time. Since we use this time
    // in the foreground and many operation systems assume that an application
    // that is busy with other things and therefore isn't reacting is stuck
    // and therefore offer to kill the application we should now listen to
    // requests from the OS before continuing the save.
    wxYield();

    convert << wxT(" \"") << imgname.GetFullPath() << wxT("\"");
  }

  convert << wxT(" \"") << file << wxT("\"");

#if defined __WXMSW__
  if (!wxShell(convert))
#else
  if (wxExecute(convert, wxEXEC_SYNC) != 0)
#endif
  {
    success = false;
    wxMessageBox(_("There was an error during GIF export!\n\nMake sure ImageMagick is installed and wxMaxima can find the convert program."),
        wxT("Error"), wxICON_ERROR);
  }

  for (int i=0; i<m_size; i++)
  {
    wxFileName imgname(tmpdir, wxString::Format(wxT("wxm_anim%d.png"), i));
    wxRemoveFile(imgname.GetFullPath());
  }

  // Converting images to an animation might need loads of time. Since we do this
  // in the foreground and many operation systems assume that an application
  // that is busy with other things and therefore isn't reacting is stuck
  // and therefore offer to kill the application we should now listen to
  // requests from the OS before continuing the save.
  wxYield();

  if(success)
  {
    if(m_size>0)
      return m_bitmaps[1]->GetSize();
    else
    {
      wxSize retval;
      retval.x=retval.y=0;
      return retval;
    }
  }
  else
  {
    wxSize retval;
    retval.x=retval.y=-1;
    return retval;
  }
}

bool SlideShow::CopyToClipboard()
{
  if (wxTheClipboard->Open())
  {
    bool res = wxTheClipboard->SetData(new wxBitmapDataObject(*m_bitmaps[m_displayed]));
    wxTheClipboard->Close();
    return res;
  }
  return false;
}

