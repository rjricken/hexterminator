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

#include "ht_app.hpp"
#include "constants.hpp"
#include "hex_doc.hpp"
#include "hex_view.hpp"
#include "ht_mainframe.hpp"

IMPLEMENT_APP(HtApp);

HtApp::~HtApp ()
{

}

bool HtApp::OnInit ()
{
   if (!wxApp::OnInit())
      return false;

   // set up language selection system
   loc.Init(wxLANGUAGE_DEFAULT, wxLOCALE_LOAD_DEFAULT);
   wxLocale::AddCatalogLookupPathPrefix("./language");
   loc.AddCatalog("hexterminator");

   wxImage::AddHandler(new wxPNGHandler());

   doc_manager.reset(new wxDocManager);
   new wxDocTemplate(doc_manager.get(), "All Files", "*.*", "", "*", "Hex Doc", "Hex View", CLASSINFO(HexDoc), CLASSINFO(HexView));

   HtMainFrame *frame = new HtMainFrame(wxString::Format("HEXterminator %s", HT_VERSION), wxPoint(0, 0) /*wxDefaultPosition*/, wxSize(/*920*/980, 740));
   frame->SetMinSize(wxSize(480, 420));
   frame->Show(true);

   SetTopWindow(frame);
   return true;
}

int HtApp::OnExit ()
{
   return wxApp::OnExit();
}