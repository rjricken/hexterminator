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

#include "editor_controller.hpp"
#include "hex_frame.hpp"
#include "hex_doc.hpp"
#include "ht_app.hpp"
#include <wx/docview.h>
#include <optional>

#ifdef HT_DEBUG_PANEL_ENABLED
#include "debug_panel.hpp"
#include <vector>
#endif

class HexView : public wxView {
#ifdef HT_DEBUG_PANEL_ENABLED
public:
   void AddToDebugPanel(DebugPanel *dbgPanel);
#endif

public:
   HexView();
   ~HexView();

   void SetFrame(HexFrame *f) {
      m_frame = f;
      RecalcDisplay();
   }

   void UpdateCaret();
   void UpdateScrollbars();

   ht::core::ByteOffset GetCurrentOffset() const { return m_editorController.GetState().caretOffset; }
   std::optional<ht::core::ByteOffset> JumpToOffset(ht::core::ByteOffset requestedOffset);

   HexFrame *GetFrame() const { return m_frame; }
   HexDoc *GetDocument() const { return static_cast<HexDoc *>(wxView::GetDocument()); }

   virtual bool OnCreate(wxDocument *doc, long flags = 0);
   virtual void OnDraw(wxDC *dc);

   virtual void OnUpdate(wxView *WXUNUSED(sender), wxObject *WXUNUSED(hint) = 0) {}

   virtual bool OnClose(bool WXUNUSED(deleteWindow) = true) {

#ifdef HT_DEBUG_PANEL_ENABLED
      m_debugWatches.clear();
#endif
      if (m_frame) {
         m_frame->DetachView();
         m_frame = nullptr;
      }

      return true;
   }

   void ScrollToPosition(wxPoint from, wxPoint to);

   // -- Event handlers
   void OnLeftDown(wxMouseEvent &event);
   void OnLeftUp(wxMouseEvent &event);
   void OnMotion(wxMouseEvent &event);
   void OnLeaveWindow(wxMouseEvent &event);
   void OnKeyDown(wxKeyEvent &event);
   void OnScroll(wxScrollWinEvent &event);
   void OnSize(wxSizeEvent &event);

   void OnSetFocus(wxFocusEvent &event);
   void OnKillFocus(wxFocusEvent &event);

   void DoMouseWheel(const int lines);

private:
   wxPoint GetCaretPosition() const;
   void RecalcDisplay();
   void RecalcViewState();

   void SetupDC(wxDC *dc) const;
   void DrawBackground(wxDC *dc) const;
   void DrawSeparators(wxDC *dc) const;
   void DrawCursorHighlight(wxDC *dc) const;
   void DrawRuler(wxDC *dc) const;
   void DrawMouseHighlight(wxDC *dc) const;
   void DrawLines(wxDC *dc) const;
   void DrawCaret(wxDC *dc) const;

   void DrawVerticalLine(wxDC &dc, const int x, const int y1, const int y2) const { dc.DrawLine(x, y1, x, y2); }
   void DrawPositionIndicator(wxDC *dc, int view_row, int col, bool filled) const;

   enum class CaretCellStyle { Filled, Outline };
   void DrawCaretCell(
      wxDC *dc, int x, int y, int w, const wxString &text, const wxColour &clr, CaretCellStyle style) const;

   void MoveCaret(ht::core::CaretMove move);

   int Pos2HexCol(const int pos) const;
   int Pos2CharCol(const int pos) const;

   // returns x position of given hex area column
   inline int HexCol2Pos(const int col) const {
      wxASSERT(col <= m_columns);
      return (1 + m_addr_width + col * 3 + col / m_group_by) * text_sz.x;
   }

   // returns x position of given char area column
   inline int CharCol2Pos(const int col) const {
      wxASSERT(col <= m_columns);
      int grouping_width = m_columns / m_group_by + (m_columns % m_group_by ? 1 : 0);
      return (1 + m_addr_width + m_columns * 3 + grouping_width) * text_sz.x + (col - 1) * text_sz.x;
   }

   void ValidateScroll(wxPoint &scr_pos) {
      // wxFileOffset doclen = GetDocument()->length();

      if (scr_pos.y < 0)
         scr_pos.y = 0;

      if (m_scroll_past_ends) {
         if (scr_pos.y > m_numlines - 1)
            scr_pos.y = m_numlines - 1;
      }
      else {
         // TODO: needs a little bit more working ^^ (SEE: UpdateScrollbars)
         if (scr_pos.y > m_numlines - m_numshown + (m_additional_line ? 1 : 0))
            scr_pos.y = m_numlines - m_numshown + (m_additional_line ? 1 : 0);
      }

      // if (scr_pos.y > m_numlines - 1)
      //    scr_pos.y = m_scroll_past_ends ? m_numlines - 1 : m_numlines - m_numshown;
   }

   // -- Attributes
   HexFrame *m_frame;

   // file related
   unsigned char *view_data;
   long view_data_sz;

   // metrics
   wxSize text_sz;      /**< average character size (in pixels) */
   int m_text_y_offset; /**< vertical offset to center characters within row height */
   wxString m_addr_fmt; /**< file address/offset format */
   int m_addr_width;    /**< width of the address column */

   // position and sizes
   long m_numlines;        /**< number of lines in the view */
   long m_numshown;        /**< number of lines that can be displayed */
   long m_start_line;      /**< line displayed in the top of the view */
   long m_end_line;        /**< line displayed in the bottom of the view */
   wxRect m_base;          /**< the area where we should actually draw on */
   bool m_additional_line; /**< needed if the line at the bottom of the view is clipped */

   // view state
   bool m_mouse_down;    /**< is the left mouse button down? */
   wxPoint m_mouse_over; /**< row/col currently under the mouse pointer */
   wxPoint m_scrollpos;  /**< which position in the doc is the top left of the display */
   ht::core::EditorController m_editorController;

   // view settings
   int m_columns;           /**< number of columns to display */
   int m_group_by;          /**< columns grouping */
   bool m_show_ruler;       /**< show the horizontal ruler? */
   bool m_show_cursor_pos;  /**< show the current cursor position */
   bool m_show_mouse_pos;   /**< show the current mouse position */
   bool m_scroll_past_ends; /**< scroll past the end of the file? */

#ifdef HT_DEBUG_PANEL_ENABLED
   std::vector<DebugPanel::Watch> m_debugWatches;
#endif

   DECLARE_DYNAMIC_CLASS(HexView)
   // DECLARE_EVENT_TABLE()
};

#endif  //~HT_HEXVIEW_HPP
