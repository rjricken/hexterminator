/*
 * HEXterminator - Terminating your hex editing problems
 * Copyright (C) 2008 Ricardo J. Ricken (Darkl0rd)
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#ifndef HT_BOOKMARKSPANEL_HPP
#define HT_BOOKMARKSPANEL_HPP

#include <wx/wxprec.h>
 
#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <wx/dcbuffer.h>

class BookmarksPanel : public wxPanel
{
public:
   BookmarksPanel (wxWindow *parent, wxWindowID id)
      : wxPanel(parent, id, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL | wxBORDER_NONE)
   {
      SetBackgroundColour(wxColour(255, 255, 255));
      new wxStaticText(this, wxID_ANY, _("Not implemented yet!"), wxPoint(10, 20));
   }

   ~BookmarksPanel () { };

   void OnPaint (wxPaintEvent &WXUNUSED(event))
   {
      wxPaintDC dc(this);

      // draws a nice grey line below the pane title
      dc.SetPen(*wxGREY_PEN);
      dc.DrawLine(0, 0, GetClientSize().x, 0);
   }

private:
   DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(BookmarksPanel, wxPanel)
   EVT_PAINT(BookmarksPanel::OnPaint)
END_EVENT_TABLE()

#endif //~HT_BOOKMARKSPANEL_HPP