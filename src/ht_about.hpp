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

#ifndef HT_ABOUT_HPP
#define HT_ABOUT_HPP

#include <wx/wxprec.h>

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include "constants.hpp"
#include <wx/hyperlink.h>
#include <boost/scoped_ptr.hpp>

/**
* Implements the about dialog box, which displays information about the program.
* ie: version, build, license, author(s).
*/
class HtAbout : public wxDialog
{
public:
   HtAbout (wxWindow* parent, const wxString& title, const wxSize& size = wxDefaultSize) :
   wxDialog(parent, wxID_ANY, title, wxDefaultPosition, size)
   {
      wxBoxSizer *global_sz = new wxBoxSizer(wxVERTICAL);
      wxButton *close = new wxButton(this, wxID_OK, _("&Close"), wxPoint(225, 240));
      global_sz->AddStretchSpacer(1);
      global_sz->Add(close, wxSizerFlags().Right().Border(wxLEFT | wxBOTTOM | wxRIGHT, 10));
      SetSizer(global_sz);

      //wxHyperlinkCtrl *url = new wxHyperlinkCtrl(this, wxID_ANY, _("Project's Website"), MM_PROJECT_URL);
      //url->SetPosition(wxPoint(192, 195));
      //url->SetToolTip(_("Visit project homepage, at F.U.T."));

      ht_logo.reset(new wxImage(wxT("images/logo.png"), wxT("image/png")));
   }

   ~HtAbout () {}

   /**
   * This method is called when the window needs to be repainted.
   * @param event not used
   */
   void OnPaint(wxPaintEvent &WXUNUSED(event))
   {
      wxPaintDC dc(this);

      wxSize clientsz = GetClientSize();
      dc.DrawBitmap(wxBitmap(*ht_logo), (clientsz.x / 2) - (ht_logo->GetWidth() / 2), 10, true);

      //dc.SetFont(wxFont(10, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));
      //dc.DrawText(wxT("MONKEY-MOORE"), 190, 50);

      /*dc.SetFont(wxFont(8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
      dc.DrawText(wxString::Format(_("Version %s (build %s)"), MM_VERSION, MM_BUILD), 192, 65);
      dc.DrawText(_("2007-2009, writen by Darkl0rd."), 192, 90);
      dc.DrawText(_("All rights reserved."), 192, 105);
      dc.DrawText(_("This is a free software and it's"), 192, 130);
      dc.DrawText(_("under GNU's General Public"), 192, 145);
      dc.DrawText(_("Lisense (GPL)."), 192, 160);*/
   }

private:
   boost::scoped_ptr <wxImage> ht_logo;

   DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(HtAbout, wxDialog)
   EVT_PAINT(HtAbout::OnPaint)
END_EVENT_TABLE()

#endif //~HT_ABOUT_HPP
