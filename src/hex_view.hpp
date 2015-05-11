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

#ifndef HT_HEXVIEW_HPP
#define HT_HEXVIEW_HPP

#include <wx/wxprec.h>

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include "hex_frame.hpp"
#include "hex_doc.hpp"
#include "ht_app.hpp"
#include <wx/docview.h>

#ifdef WXDEBUG
   #include "debug_panel.hpp"
#endif

class HexView : public wxView
{
#ifdef WXDEBUG
public:
   void AddToDebugPanel (DebugPanel *dbgPanel);
   void RemoveFromDebugPanel (DebugPanel *dbgPanel);
#endif

public:
   HexView ();
   ~HexView ();

   void SetFrame (HexFrame *f)
   {
      //wxLogDebug("<<Flow>> HexView::SetFrame");

      m_frame = f;
      RecalcDisplay();
   }

   void UpdateCaret ()
   {
      int x = m_edit_char ? CharCol2Pos(m_caretpos.x) : HexCol2Pos(m_caretpos.x);
      int y = (m_caretpos.y - m_start_line) * text_sz.y + m_base.y;

      m_frame->GetCaret()->Move(x, y);
      (m_caretpos.y - m_start_line) >= 0 ? m_frame->ShowCaret() : m_frame->HideCaret();

      wxLogStatus(
         "Caret is now %svisible at [%d,%d]/[%d,%d] in the %s area",
         m_frame->GetCaret()->IsVisible() ? "" : "in",
         m_caretpos.x, m_caretpos.y, m_frame->GetCaret()->GetPosition().x, m_frame->GetCaret()->GetPosition().y,
         m_edit_char ? "CHAR" : "HEX"
      );

      

      //if (y + text_sz.y > m_base.GetBottom())
      //   wxLogStatus("I need to scroll down 1 line");




   }

   void UpdateScrollbars ();

   wxFileOffset GetCurrentOffset () const {
      return m_caretpos.y * m_columns + m_caretpos.x;
   }

   HexFrame *GetFrame () const { return m_frame; }
   HexDoc *GetDocument () const { return static_cast <HexDoc *>(wxView::GetDocument()); }

   virtual bool OnCreate (wxDocument *doc, long flags = 0);
   virtual void OnDraw (wxDC *dc);
   virtual void OnUpdate (wxView *WXUNUSED(sender), wxObject *WXUNUSED(hint) = 0) {
      //wxLogDebug("<<Flow>> HexView::OnUpdate");
   }
   virtual bool OnClose (bool WXUNUSED(deleteWindow) = true) {
      wxLogDebug("[HexView] ::OnClose");
      m_frame->OnRemoveFromDebugPanel();
      return true;
   }

   /**
   * Scrolla o documento para a posição especificada.
   * A posição anterior é nécessária para terminar a direção
   * do scroll e o consequente posicionamento do cursor, ex.:
   * ele vai para o topo da view caso a direção de navegação é para cima,
   * e para o fundo da view caso a direção de navegação seja para baixo.
   */
   void ScrollToPosition (wxPoint from, wxPoint to);

   // -- Event handlers
   void OnLeftDown (wxMouseEvent &event);
   void OnLeftUp (wxMouseEvent &event);
   void OnMotion (wxMouseEvent &event);
   void OnLeaveWindow (wxMouseEvent &event);
   void OnKeyDown (wxKeyEvent &event);
   void OnScroll (wxScrollWinEvent &event);
   void OnSize (wxSizeEvent &event);

   void OnSetFocus (wxFocusEvent &event);
   void OnKillFocus (wxFocusEvent &event);

   void DoMouseWheel (const int lines);



private:
   void RecalcDisplay ();
   void RecalcViewState ();
   void DrawBackground (wxDC *dc) const;

   void DrawVerticalLine (wxDC &dc, const int x, const int y1, const int y2) const
   { dc.DrawLine(x, y1, x, y2); }

   void MovePos (int key, bool ctrl_down, bool shift_down, bool alt_down);

   int Pos2HexCol (const int pos) const;
   int Pos2CharCol (const int pos) const;

   // returns x position of given hex area column
   inline int HexCol2Pos (const int col) const
   {
      wxASSERT(col <= m_columns);
      return (1 + m_addr_width + col * 3 + col / m_group_by) * text_sz.x;
   }

   // returns x position of given char area column
   inline int CharCol2Pos (const int col) const
   {
      wxASSERT(col <= m_columns);
      int grouping_width = m_columns / m_group_by + (m_columns % m_group_by ? 1 : 0);
      return (1 + m_addr_width + m_columns * 3 + grouping_width) * text_sz.x + (col - 1) * text_sz.x;
   }

   // return file offset of given position
   wxFileOffset Pos2Offset (wxPoint pos) const
   {
      wxFileOffset address = (m_start_line + pos.y / text_sz.y) * m_columns;
      address += m_edit_char ? Pos2CharCol(pos.x) : address += Pos2HexCol(pos.x);

      return address;
   }

   // return position in view of given file offset
   wxPoint Offset2Pos (wxFileOffset address) const
   {
      return m_edit_char ?
         wxPoint(CharCol2Pos(address % m_columns), (address / m_columns) * text_sz.y) :
         wxPoint(HexCol2Pos(address % m_columns), (address / m_columns) * text_sz.y);
   }

   void ValidateScroll (wxPoint &scr_pos)
   {
      //wxFileOffset doclen = GetDocument()->length();

      if (scr_pos.y < 0)
         scr_pos.y = 0;

      if (m_scroll_past_ends)
      {
         if (scr_pos.y > m_numlines - 1)
            scr_pos.y = m_numlines - 1;
      }
      else
      {
         // TODO: needs a little bit more working ^^ (SEE: UpdateScrollbars)
         if (scr_pos.y > m_numlines - m_numshown + (m_additional_line ? 1 : 0))
            scr_pos.y = m_numlines - m_numshown + (m_additional_line ? 1 : 0);
      }

      //if (scr_pos.y > m_numlines - 1)
      //   scr_pos.y = m_scroll_past_ends ? m_numlines - 1 : m_numlines - m_numshown;

      wxLogStatus("[HexView] ValidateScroll: m_scrollpos   y=%d", m_scrollpos.y);
   }

   // -- Attributes
   HexFrame *m_frame;

   // file related
   unsigned char *view_data;
   long view_data_sz;

   // color scheme
   wxColor line_clr;        /**< line color */
   wxColor addrhex_clr;     /**< addresses text color */
   wxColor addrbg_clr;      /**< addresses background color */
   wxColor addrhl_clr;      /**< addresses highlight color */

   // metrics
   wxSize text_sz;          /**< average character size (in pixels) */
   wxString m_addr_fmt;     /**< file address/offset format */
   int m_addr_width;        /**< width of the address column */

   // position and sizes
   long m_numlines;         /**< number of lines in the view */
   long m_numshown;         /**< number of lines that can be displayed */
   long m_start_line;       /**< line displayed in the top of the view */
   long m_end_line;         /**< line displayed in the bottom of the view */
   wxRect m_base;           /**< the area where we should actually draw on */
   bool m_additional_line;  /**< needed if the line at the bottom of the view is clipped */

   // view state
   bool m_edit_char;        /**< is the char area being edited? */
   bool m_mouse_down;       /**< is the left mouse button down? */
   wxPoint m_mouse_over;    /**< row/col currently under the mouse pointer */
   wxPoint m_scrollpos;     /**< which position in the doc is the top left of the display */
   wxPoint m_caretpos;      /**< position of the caret in the doc (also, selection begin point) */

   // view settings
   int m_columns;           /**< number of columns to display */
   int m_group_by;          /**< columns grouping */
   bool m_show_ruler;       /**< show the horizontal ruler? */
   bool m_show_cursor_pos;  /**< show the current cursor position */
   bool m_show_mouse_pos;   /**< show the current mouse position */
   bool m_scroll_past_ends; /**< scroll past the end of the file? */

   DECLARE_DYNAMIC_CLASS(HexView)
   //DECLARE_EVENT_TABLE()
};

#endif //~HT_HEXVIEW_HPP