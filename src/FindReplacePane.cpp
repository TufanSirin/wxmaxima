// -*- mode: c++; c-file-style: "linux"; c-basic-offset: 2; indent-tabs-mode: nil -*-
//
//  Copyright (C)      2016 Gunter Königsmann <wxMaxima@physikbuch.de>
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

/*! \file
  This file defines the class FindReplacePane

  FindReplacePane is the contents of the find/replace dialog/sidebar
 */


#include "FindReplacePane.h"
#include "EditorCell.h"
#include <wx/stattext.h>
#include <wx/button.h>

FindReplacePane::FindReplacePane(wxWindow *parent,wxFindReplaceData *data):
  wxPanel(parent,-1)
{
  m_active = true;
  m_findReplaceData = data;
  wxFlexGridSizer* grid_sizer = new wxFlexGridSizer(3,1,1);
  grid_sizer->SetFlexibleDirection(wxHORIZONTAL);
  grid_sizer->AddGrowableCol(0,0);
  grid_sizer->AddGrowableCol(1,1);
  grid_sizer->AddGrowableCol(2,0);

  std::cerr<<data->GetFindString()<<"\n";
  grid_sizer->Add(new wxStaticText(this,-1,_("Find:")),wxSizerFlags().Expand());
  m_searchText = new wxTextCtrl(this,-1,data->GetFindString());
  m_searchText->Connect(
    wxEVT_TEXT,
    wxCommandEventHandler(FindReplacePane::OnFindStringChange),
    NULL, this
    );
  grid_sizer->Add(m_searchText,wxSizerFlags().Expand());

  m_searchButton = new wxButton (this,wxID_FIND);
  grid_sizer->Add(m_searchButton,wxSizerFlags().Expand());
  m_searchButton->Connect(
    wxEVT_BUTTON,
    wxCommandEventHandler(FindReplacePane::OnSearch),
    NULL, this
    );
  grid_sizer->Add(new wxStaticText(this,-1,_("Replacement:")),wxSizerFlags().Expand());
  m_replaceText = new wxTextCtrl(this,-1,data->GetReplaceString());
  grid_sizer->Add(m_replaceText,wxSizerFlags().Expand());
  m_replaceButton = new wxButton (this,wxID_REPLACE);
  m_replaceButton->Connect(
    wxEVT_BUTTON,
    wxCommandEventHandler(FindReplacePane::OnReplace),
    NULL, this
    );
  grid_sizer->Add(m_replaceButton,wxSizerFlags().Expand());

  grid_sizer->Add(new wxStaticText(this,-1,_("Direction:")),wxSizerFlags().Expand());
  wxBoxSizer *fbbox=new wxBoxSizer(wxHORIZONTAL);
  m_forward = new wxRadioButton(this,-1,_("Up"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
  fbbox->Add(m_forward,wxSizerFlags().Expand());
  m_backwards = new wxRadioButton(this,-1,_("Down"));
  m_forward->SetValue(!(data->GetFlags() & wxFR_DOWN));
  m_backwards->SetValue(!!(data->GetFlags() & wxFR_DOWN));
  m_forward->Connect(
    wxEVT_RADIOBUTTON,
    wxCommandEventHandler(FindReplacePane::OnDirectionChange),
    NULL, this
    );
  m_backwards->Connect(
    wxEVT_RADIOBUTTON,
    wxCommandEventHandler(FindReplacePane::OnDirectionChange),
    NULL, this
    );

  fbbox->Add(m_backwards,wxSizerFlags().Expand());
  grid_sizer->Add(fbbox,wxSizerFlags().Expand());
  m_replaceAllButton = new wxButton (this,1,_("Replace All"));
  grid_sizer->Add(m_replaceAllButton,wxSizerFlags().Expand());
  m_replaceAllButton->Connect(
    wxEVT_BUTTON,
    wxCommandEventHandler(FindReplacePane::OnReplaceAll),
    NULL, this
    );

  wxBoxSizer *vbox=new wxBoxSizer(wxVERTICAL);
  vbox->Add(grid_sizer,wxSizerFlags().Expand());
  m_matchCase=new wxCheckBox(this,-1,wxT("Match Case"));
  m_matchCase->SetValue(!!(data->GetFlags() & wxFR_MATCHCASE));
  vbox->Add(m_matchCase,wxSizerFlags().Expand());
  m_matchCase->Connect(
    wxEVT_CHECKBOX,
    wxCommandEventHandler(FindReplacePane::OnMatchCase),
    NULL, this
    );
  this->SetSizerAndFit(vbox);
//  vbox->SetSizeHints(this);
//  Layout();
}
void FindReplacePane::SetFindString(wxString string)
{
  m_findReplaceData->SetFindString(string);
  m_searchText->SetValue(string);
}

void FindReplacePane::OnSearch(wxCommandEvent& event)
{
  wxFindDialogEvent *findEvent = new wxFindDialogEvent(wxEVT_FIND_NEXT);
  findEvent->SetFindString(m_findReplaceData->GetFindString());
  findEvent->SetFlags(m_findReplaceData->GetFlags());
  GetParent()->GetParent()->GetEventHandler()->QueueEvent(findEvent);
}

void FindReplacePane::OnReplace(wxCommandEvent& event)
{
  wxFindDialogEvent *findEvent = new wxFindDialogEvent(wxEVT_FIND_REPLACE);
  findEvent->SetFindString(m_findReplaceData->GetFindString());
  findEvent->SetReplaceString(m_findReplaceData->GetReplaceString());
  findEvent->SetFlags(m_findReplaceData->GetFlags());
  GetParent()->GetParent()->GetEventHandler()->QueueEvent(findEvent);
}

void FindReplacePane::OnReplaceAll(wxCommandEvent& event)
{
  wxFindDialogEvent *findEvent = new wxFindDialogEvent(wxEVT_FIND_REPLACE_ALL);
  findEvent->SetFindString(m_findReplaceData->GetFindString());
  findEvent->SetReplaceString(m_findReplaceData->GetReplaceString());
  findEvent->SetFlags(m_findReplaceData->GetFlags());
  GetParent()->GetParent()->GetEventHandler()->QueueEvent(findEvent);
}

void FindReplacePane::OnDirectionChange(wxCommandEvent& event)
{
  m_findReplaceData->SetFlags(
    !((m_findReplaceData->GetFlags()&(!wxFR_DOWN)) | (m_forward->GetValue()*wxFR_DOWN)));
}


void FindReplacePane::OnMatchCase(wxCommandEvent& event)
{
  m_findReplaceData->SetFlags(
    (m_findReplaceData->GetFlags()&(!wxFR_MATCHCASE)) | (event.IsChecked()*wxFR_MATCHCASE));
}

void FindReplacePane::OnActivate(wxActivateEvent& event)
{
  if(event.GetActive())
    SetTransparent(255);
  else
    SetTransparent(180);
  m_active = true;
}

void FindReplacePane::OnFindStringChange(wxCommandEvent& event)
{
  m_findReplaceData->SetFindString(m_searchText->GetValue());
  std::cerr<<"test:"<<m_searchText->GetValue()<<"\n";
}

void FindReplacePane::OnReplaceStringChange(wxCommandEvent& event)
{
  m_findReplaceData->SetReplaceString(m_replaceText->GetValue());
}

void FindReplacePane::OnKeyDown(wxKeyEvent& event)
{
  if(event.GetKeyCode()==WXK_RETURN)
  {
    if(m_searchText->HasFocus())
    {
      wxCommandEvent dummyEvent;
      OnSearch(dummyEvent);
    }
    else if(m_replaceText->HasFocus())
    {
      wxCommandEvent dummyEvent;
      OnReplace(dummyEvent);
    }
    else
      event.Skip();
  }
  else
    event.Skip();
}

BEGIN_EVENT_TABLE(FindReplacePane, wxPanel)
 EVT_CHAR_HOOK(FindReplacePane::OnKeyDown)
 EVT_ACTIVATE(FindReplacePane::OnActivate)
END_EVENT_TABLE()