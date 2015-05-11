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

#ifndef HT_HEXFRAME_HPP
#define HT_HEXFRAME_HPP

#include <wx/wxprec.h>

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <boost/scoped_ptr.hpp>
#include <wx/aui/aui.h>
#include <wx/docview.h>
#include <wx/caret.h>

class HexView;

class HexFrame : public wxAuiMDIChildFrame
{
#ifdef WXDEBUG
public:
   void OnAddToDebugPanel () ;
   void OnRemoveFromDebugPanel ();
#endif

public:
   HexFrame (HexView *view, wxAuiMDIParentFrame *parent, wxWindowID id, const wxString &title);
   virtual ~HexFrame ();

   HexView *GetView () const { return m_view; }
   void SetView (HexView *view) { m_view = view; }
 
   // will be used to adjust the caret when selection gots implemented
   void SetSel (wxPoint &start, wxPoint &end);

   void ShowCaret (bool show = true)
   {
      if (show && !GetCaret()->IsVisible())
         GetCaret()->Show();//, wxLogStatus("Caret is now visible");
      else if (!show && GetCaret()->IsVisible())
         GetCaret()->Hide();//, wxLogStatus("Caret is now hidden");
   }

   void HideCaret () {
      ShowCaret(false);
   }

   // -- Event handlers
   void OnPaint (wxPaintEvent &event);
   void OnEraseBackground (wxEraseEvent &event);
   void OnSize (wxSizeEvent &event);
   void OnLeftDown (wxMouseEvent &event);
   void OnLeftUp (wxMouseEvent &event);
   void OnMouseWheel (wxMouseEvent &event);
   void OnMotion (wxMouseEvent &event);
   void OnLeaveWindow (wxMouseEvent &event);
   void OnKeyDown (wxKeyEvent &event);
   void OnScroll(wxScrollWinEvent &event);
   void OnSetFocus(wxFocusEvent &event);
   void OnKillFocus(wxFocusEvent &event);

private:
   boost::scoped_ptr <wxBitmap> m_buffer;
   HexView *m_view;

   DECLARE_EVENT_TABLE()
};

#endif //~HT_HEXFRAME_HPP