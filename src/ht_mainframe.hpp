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

#ifndef HT_MAINFRAME_HPP
#define HT_MAINFRAME_HPP

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <wx/aui/aui.h>
#include <wx/docview.h> 
#include "ht_artprovider.hpp"
#include "hex_view.hpp"

class HtMainFrame : public wxAuiMDIParentFrame
{
public:
   HtMainFrame (const wxString &title, const wxPoint &pos, const wxSize &size, long flags = wxDEFAULT_FRAME_STYLE);
   ~HtMainFrame ();

   void OnEraseBackground (wxEraseEvent &event);
   void OnFileOpen (wxCommandEvent &event);
   void OnFileExit (wxCommandEvent &event);
   void OnViewStatusBar (wxCommandEvent &event);
   void OnToolsLog (wxCommandEvent &event);
   void OnHelpAbout (wxCommandEvent &event);
   void OnClose (wxCloseEvent &event);
   void OnUpdateUI (wxUpdateUIEvent &event);
   void OnNotebookPageClose (wxAuiNotebookEvent &event);
   void OnNotebookPageChange (wxAuiNotebookEvent &event);
   void OnEditUndoDropDown (wxAuiToolBarEvent &event);
   void OnFileOpenDropDown (wxAuiToolBarEvent &event);
   void OnRecentFilesSelected (wxCommandEvent &event);

   void OnKeyDown (wxKeyEvent &event);
   void OnJumpHexTextEnter (wxCommandEvent &event);

   void DoOpenFile (const wxString &path);

   /**
   * Returns a pointer to the active document view, if there is one.
   * @return Pointer to the active HexView instance if any, 0 otherwise.
   */
   HexView *GetActiveView () const {
      wxAuiNotebook *nb = GetNotebook();

      return nb->GetPageCount() ?
         static_cast<HexFrame *>(nb->GetPage(nb->GetSelection()))->GetView() : 0;
   }

private:
   void CreateMenu (HtArtProvider &art);

   wxAuiManager aui_manager;
   wxStatusBar *statusbar;
   wxLogWindow *logger;
   wxFileHistory history;

   DECLARE_EVENT_TABLE()
};

#endif //~HT_MAINFRAME_HPP
