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

#include "hex_view.hpp"

#include <wx/dcbuffer.h>
#include <algorithm>
#include <cmath>
#include <chrono>

IMPLEMENT_DYNAMIC_CLASS(HexView, wxView)

HexView::HexView() {
   wxLogDebug("[HexView] created at 0x%llX.", reinterpret_cast<uint64_t>(this));

   m_base = wxRect(0, 0, 0, 0);
   m_frame = 0;
   view_data = 0;
   view_data_sz = 0;

   // -- Colors setup
   line_clr.Set(0, 105, 153);      // dark cyan
   addrhex_clr.Set(128, 0, 0);     // dark red
   addrbg_clr.Set(192, 224, 240);  // cyan
   addrhl_clr.Set(255, 160, 128);  // beige

   HtPrefs &prefs = wxGetApp().prefs;

   m_columns = prefs.getInt(wxT("document/visualization/columns"));
   m_group_by = prefs.getInt(wxT("document/visualization/group-by"));
   m_show_ruler = prefs.getBool(wxT("workspace/display/show-ruler"));
   m_show_cursor_pos = prefs.getBool(wxT("workspace/display/show-cursor-position"));
   m_show_mouse_pos = prefs.getBool(wxT("workspace/display/show-mouse-position"));
   m_scroll_past_ends = prefs.getBool(wxT("workspace/display/scroll-past-ends"));

   m_start_line = 0;
   m_end_line = 0;

   m_edit_char = false;   // caret starts in hex area
   m_mouse_down = false;  // mouse is initially up
   m_mouse_over = wxPoint(-1, -1);
   m_scrollpos = wxPoint(0, 0);
   m_caretpos = wxPoint(0, 0);
   m_text_y_offset = 0;
}

HexView::~HexView() {
   wxLogDebug("[HexView] at 0x%llX destroyed.", reinterpret_cast<uint64_t>(this));

   if (view_data)
      delete[] view_data;

   m_frame->SetView(nullptr);
}

bool HexView::OnCreate(wxDocument *doc, long WXUNUSED(flags)) {
   // HexDoc *hdoc = (HexDoc *)doc;

   return true;
}

void HexView::OnLeftDown(wxMouseEvent &event) {
   wxPoint point = event.GetPosition();

   bool same_pos = false;
   int row = ((point.y - m_base.y) / text_sz.y) + m_start_line;

   // a click in the horizontal ruler
   if (!m_base.Contains(point))
      return;

   if (point.x >= 0 && point.x <= CharCol2Pos(0) - text_sz.x / 2) {
      // -- we have a click in the hex area
      int col = Pos2HexCol(point.x);

      // past last column must put the caret at last column
      if (col == m_columns)
         col = m_columns - 1;
      // col = 0, row += 1;  // this makes it behave quite like hexedit, but still needs some work

      // if we are past file size, go back
      if (row * m_columns + col >= view_data_sz)
         row = view_data_sz / m_columns, col = view_data_sz % m_columns;

      if (col == m_caretpos.x && row == m_caretpos.y)
         same_pos = true;

      // m_frame->SetCaretPos(wxPoint(col, row));
      m_caretpos = wxPoint(col, row);
      m_frame->GetCanvas()->GetCaret()->Hide();

      m_edit_char = false;
      m_mouse_down = true;
      UpdateCaret();
   }
   else if (point.x >= CharCol2Pos(0) && point.x <= CharCol2Pos(m_columns) + text_sz.x) {
      // -- we have a click in the char area
      int col = Pos2CharCol(point.x);

      // if we are past the last column, go to next line
      if (col == m_columns)
         col = 0, row += 1;

      // if we are past file size, go back
      if (row * m_columns + col >= view_data_sz)
         row = view_data_sz / m_columns, col = view_data_sz % m_columns;

      if (col == m_caretpos.x && row == m_caretpos.y)
         same_pos = true;

      m_caretpos = wxPoint(col, row);
      m_frame->GetCanvas()->GetCaret()->Hide();

      m_edit_char = true;
      m_mouse_down = true;
      UpdateCaret();
   }

   if (!same_pos)
      m_frame->GetCanvas()->Refresh(), m_frame->GetCanvas()->Update();
}

void HexView::OnLeftUp(wxMouseEvent &WXUNUSED(event)) {
   if (!m_frame->GetCanvas()->GetCaret()->IsVisible())
      m_frame->GetCanvas()->GetCaret()->Show();

   m_mouse_down = false;
   m_frame->GetCanvas()->Refresh(), m_frame->GetCanvas()->Update();
}

void HexView::OnMotion(wxMouseEvent &event) {
   const wxPoint pos = event.GetPosition();
   wxRect edit_area = wxRect(
      wxPoint(m_addr_width * text_sz.x, m_base.y), wxPoint(CharCol2Pos(m_columns) + text_sz.x, m_base.GetBottom()));

   // update cursor based on what it's hovering
   m_frame->GetCanvas()->SetCursor(edit_area.Contains(pos) ? wxCURSOR_IBEAM : wxCURSOR_ARROW);

   bool over_hex = false;
   if (pos.x >= 0 && pos.x <= CharCol2Pos(0) - text_sz.x / 2)
      over_hex = true;

   // update mouse hovering row
   edit_area.x += text_sz.x, edit_area.width -= text_sz.x;
   int row = edit_area.Contains(pos) ? (pos.y - m_base.y) / text_sz.y : -1;
   int col = edit_area.Contains(pos) ? over_hex ? Pos2HexCol(pos.x) : Pos2CharCol(pos.x) : -1;

   if (col == m_columns)
      col = m_columns - 1;

   bool refresh = false;

   if (row != m_mouse_over.y)
      m_mouse_over.y = row, refresh = true;

   if (col != m_mouse_over.x)
      m_mouse_over.x = col, refresh = true;

   if (refresh)
      m_frame->GetCanvas()->Refresh(), m_frame->GetCanvas()->Update();
}

void HexView::OnLeaveWindow(wxMouseEvent &WXUNUSED(event)) {
   m_mouse_over = wxPoint(-1, -1);
   m_frame->GetCanvas()->Refresh(), m_frame->GetCanvas()->Update();
}

void HexView::OnKeyDown(wxKeyEvent &event) {
   switch (event.GetKeyCode()) {
      case WXK_LEFT:
      case WXK_RIGHT:
      case WXK_UP:
      case WXK_DOWN:
      case WXK_END:
      case WXK_HOME:
         MovePos(event.GetKeyCode(), event.ControlDown(), event.ShiftDown(), event.AltDown());
         break;

      case WXK_TAB:
         m_edit_char = !m_edit_char;
         UpdateCaret();

         m_frame->GetCanvas()->Refresh(), m_frame->GetCanvas()->Update();
         break;

      default:
         event.Skip();
   }
}

void HexView::OnScroll(wxScrollWinEvent &event) {
   if (event.GetOrientation() == wxVERTICAL) {
      const int &type = event.GetEventType();

      if (type == wxEVT_SCROLLWIN_LINEUP)
         m_scrollpos.y -= 1;
      else if (type == wxEVT_SCROLLWIN_LINEDOWN)
         m_scrollpos.y += 1;
      else if (type == wxEVT_SCROLLWIN_PAGEUP)
         m_scrollpos.y -= m_numshown;
      else if (type == wxEVT_SCROLLWIN_PAGEDOWN)
         m_scrollpos.y += m_numshown;

      else if (type == wxEVT_SCROLLWIN_THUMBTRACK) {
         m_scrollpos.y = event.GetPosition();
      }
      else if (type == wxEVT_SCROLL_THUMBRELEASE) {
         UpdateScrollbars();
      }
      else if (type == wxEVT_SCROLLWIN_BOTTOM) {
         m_scrollpos.y = m_numlines;
      }
   }

   ValidateScroll(m_scrollpos);
   UpdateScrollbars();

   // m_view->ValidateCaret(m_caretpos);
   m_frame->GetCanvas()->Refresh();
   UpdateCaret();
}

void HexView::OnSize(wxSizeEvent &WXUNUSED(event)) {
   RecalcDisplay();
}

void HexView::OnSetFocus(wxFocusEvent &event) {
   if (!m_frame->GetCanvas()->GetCaret()->IsVisible())
      m_frame->GetCanvas()->GetCaret()->Show();
}

void HexView::OnKillFocus(wxFocusEvent &event) {
   if (m_frame->GetCanvas()->GetCaret()->IsVisible())
      m_frame->GetCanvas()->GetCaret()->Hide();
}

/*
No HexEdit, a renderiza��o � composta das seguintes etapas:
+ pr�-processamento e c�lculo de valores posteriormente necess�rios
+ c�lculo da parte do arquivo que precisar ser renderizada
+ renderiza��o dos resultados da busca
+ renderiza��o das �reas em highlight
+ renderiza��o do(s) marcador(es)
+ renderiza��o dos offsets
+ renderiza��o dos hexadecimais e caracteres, alternadamente
+ invers�o das colunas que estiverem selecionadas
*/
void HexView::OnDraw(wxDC *dc) {
   wxASSERT(m_columns > 0);
   wxASSERT(m_group_by > 0);

   if (!view_data) {
      HexDoc *doc = (HexDoc *)GetDocument();
      view_data = new unsigned char[doc->length()];
      view_data_sz = doc->GetData(view_data, doc->length());
      RecalcDisplay();
   }

   wxSize client_sz(m_frame->GetCanvas()->GetClientSize());
   int rulerHeight = m_show_ruler ? text_sz.y + 4 : 0;
   m_base = wxRect(0, rulerHeight, client_sz.x, client_sz.y - rulerHeight);

   m_start_line = m_scrollpos.y;
   m_end_line = std::min(m_start_line + m_numshown, m_numlines);

   SetupDC(dc);
   DrawBackground(dc);
   DrawSeparators(dc);
   DrawCursorHighlight(dc);
   DrawRuler(dc);
   DrawMouseHighlight(dc);
   DrawLines(dc);
   DrawCaret(dc);
}

void HexView::SetupDC(wxDC *dc) const {
   wxFont &font = *wxTheFontList->FindOrCreateFont(
      12, wxFONTFAMILY_MODERN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxT("Courier"));
   dc->SetFont(font);
}

void HexView::DrawSeparators(wxDC *dc) const {
   dc->SetPen(wxPen(line_clr));
   DrawVerticalLine(*dc, m_addr_width * text_sz.x, m_base.y - 3, m_base.GetBottom());
   DrawVerticalLine(*dc, CharCol2Pos(0) - text_sz.x / 2, m_base.y - 3, m_base.GetBottom());
   DrawVerticalLine(*dc, CharCol2Pos(m_columns) + text_sz.x / 2, m_base.y - 3, m_base.GetBottom());
}

void HexView::DrawRuler(wxDC *dc) const {
   if (!m_show_ruler)
      return;

   wxSize client_sz(m_frame->GetCanvas()->GetClientSize());
   dc->SetPen(wxPen(line_clr));
   dc->DrawLine(m_addr_width * text_sz.x, m_base.y - 3, client_sz.x, m_base.y - 3);

   dc->SetTextForeground(addrhex_clr);
   dc->SetPen(wxPen(addrhex_clr));

   for (int i = 0; i < m_columns; i++) {
      dc->DrawText(wxString::Format(wxT("%02X"), i), HexCol2Pos(i), m_text_y_offset);
      dc->DrawText(wxString::Format(wxT("%02X"), i).substr(1), CharCol2Pos(i), m_text_y_offset);
   }
}

void HexView::DrawPositionIndicator(wxDC *dc, int view_row, int col, bool filled) const {
   dc->SetPen(wxPen(addrhex_clr));
   dc->SetBrush(filled ? wxBrush(addrhl_clr) : *wxTRANSPARENT_BRUSH);

   if (view_row >= 0 && (view_row + m_start_line) < m_numlines)
      dc->DrawRectangle(0, m_base.y + view_row * text_sz.y, m_addr_width * text_sz.x - 1, text_sz.y);

   if (m_show_ruler) {
      dc->DrawRectangle(HexCol2Pos(col), 0, 2 * text_sz.x, text_sz.y);
      dc->DrawRectangle(CharCol2Pos(col), 0, text_sz.x, text_sz.y);
   }
}

void HexView::DrawCursorHighlight(wxDC *dc) const {
   if (m_mouse_down || !m_show_cursor_pos)
      return;

   DrawPositionIndicator(dc, m_caretpos.y - m_start_line, m_caretpos.x, /*filled=*/true);
}

void HexView::DrawMouseHighlight(wxDC *dc) const {
   if (m_mouse_over == wxPoint(-1, -1) || !m_show_mouse_pos)
      return;

   DrawPositionIndicator(dc, m_mouse_over.y, m_mouse_over.x, /*filled=*/false);
}

void HexView::DrawLines(wxDC *dc) const {
   static const wxChar hexDigits[] = wxT("0123456789ABCDEF");

   for (int i = m_start_line; i < m_end_line; i++) {
      const int line_y = m_base.y + (i - m_start_line) * text_sz.y;
      const int text_y = line_y + m_text_y_offset;

      dc->SetTextForeground(addrhex_clr);
      dc->DrawText(wxString::Format(m_addr_fmt, i * m_columns), 0, text_y);

      dc->SetTextForeground(*wxBLACK);

      wxString hexRow;
      wxString charRow;
      for (int j = i * m_columns; j < (i * m_columns) + m_columns; j++) {
         if (j >= view_data_sz)
            break;

         const unsigned char value = view_data[j];
         hexRow += hexDigits[value >> 4];
         hexRow += hexDigits[value & 0x0F];

         const wxChar c = (value > 0x1F && value < 0x7F) ? value : wxT('.');
         charRow += c;

         const int col = j % m_columns;
         if (col + 1 < m_columns) {
            hexRow += wxT(' ');
            if ((col + 1) % m_group_by == 0)
               hexRow += wxT(' ');
         }
      }

      if (!hexRow.IsEmpty()) {
         dc->DrawText(hexRow, HexCol2Pos(0), text_y);
         dc->DrawText(charRow, CharCol2Pos(0), text_y);
      }
   }
}

void HexView::DrawCaret(wxDC *dc) const {
   if (m_caretpos.y < m_start_line || m_caretpos.y > m_end_line)
      return;

   static const wxColour caret_fill_clr(0, 102, 153);  // dark cyan, matches palette

   const wxFileOffset caretOffset = m_caretpos.y * m_columns + m_caretpos.x;
   const int cy = (m_caretpos.y - m_start_line) * text_sz.y + m_base.y;
   const int hx = HexCol2Pos(m_caretpos.x);
   const int cx = CharCol2Pos(m_caretpos.x);

   const wxString hexText =
      caretOffset < view_data_sz ? wxString::Format(wxT("%02X"), view_data[caretOffset]) : wxString();
   const wxString charText =
      caretOffset < view_data_sz
         ? wxString::Format(wxT("%c"),
              (view_data[caretOffset] > 0x1F && view_data[caretOffset] < 0x7F) ? (char)view_data[caretOffset] : '.')
         : wxString();

   if (!m_edit_char) {
      DrawCaretCell(dc, hx, cy, 2 * text_sz.x, hexText, caret_fill_clr, CaretCellStyle::Filled);
      DrawCaretCell(dc, cx, cy, text_sz.x, charText, caret_fill_clr, CaretCellStyle::Outline);
   }
   else {
      DrawCaretCell(dc, cx, cy, text_sz.x, charText, caret_fill_clr, CaretCellStyle::Filled);
      DrawCaretCell(dc, hx, cy, 2 * text_sz.x, hexText, caret_fill_clr, CaretCellStyle::Outline);
   }
}

void HexView::DrawCaretCell(
   wxDC *dc, int x, int y, int w, const wxString &text, const wxColour &clr, CaretCellStyle style) const {
   if (style == CaretCellStyle::Filled) {
      dc->SetPen(*wxTRANSPARENT_PEN);
      dc->SetBrush(wxBrush(clr));
      dc->DrawRectangle(x, y, w, text_sz.y);
      if (!text.IsEmpty()) {
         dc->SetTextForeground(*wxWHITE);
         dc->DrawText(text, x, y + m_text_y_offset);
      }
   }
   else {  // Outline
      dc->SetBrush(*wxTRANSPARENT_BRUSH);
      dc->SetPen(wxPen(clr));
      dc->DrawRectangle(x, y, w, text_sz.y);
   }
}

void HexView::DrawBackground(wxDC *dc) const {
   // fill the background
   dc->SetBackground(*wxWHITE_BRUSH);
   dc->Clear();

   int offw = m_addr_width * text_sz.x;
   wxRect client = m_frame->GetClientRect();

   dc->SetBrush(wxBrush(addrbg_clr));
   dc->SetPen(*wxTRANSPARENT_PEN);

   // offsets column
   dc->DrawRectangle(wxRect(0, 0, offw, m_frame->GetClientRect().height));

   // horizontal ruler
   if (m_show_ruler)
      dc->DrawRectangle(wxRect(0, 0, m_base.width, m_base.y - 3));
}

void HexView::MovePos(int key, bool ctrl_down, bool shift_down, bool alt_down) {
   int col = m_caretpos.x;
   int row = m_caretpos.y;

   if (key == WXK_LEFT) {
      col -= 1;

      if (col < 0 && row == 0)
         col = 0;

      if (col < 0 && row > 0)
         row -= 1, col = m_columns - 1;
   }
   else if (key == WXK_RIGHT) {
      if (++col == m_columns)
         col = 0, row += 1;
   }
   else if (key == WXK_UP) {
      if (--row < 0)
         row = 0;
   }
   else if (key == WXK_DOWN) {
      row += 1;
   }
   else if (key == WXK_END && ctrl_down) {
      // just make it ajust the position to the last one in the view
      row = m_numlines + 1;
      col = m_columns + 1;
   }
   else if (key == WXK_HOME && ctrl_down) {
      row = 0;
      col = 0;
   }

   if (row * m_columns + col > view_data_sz)
      col = view_data_sz % m_columns, row = view_data_sz / m_columns;

   m_caretpos = wxPoint(col, row);

   /* as vari�veis m_start_pos e m_end_pos s�o atualizadas apenas na fun��o
    OnDraw(), e talvez seja necess�rio mudar isso porque tem outras partes da HexView
    que dependem de valores atualizados dessas vari�veis antes do pr�ximo OnDraw.

    A navega��o n�o-linear do documento por exemplo requer c�lculos para reposicionamento
    dos elementos gr�ficos da view antes do pr�ximo Draw, ent�o talvez a melhor op��o seja
    passar esse c�lculo de atualiza��o das vari�veis para uma fun��o e cham�-la caso necess�rio,
    ou ainda atualizar esses valores ap�s qualquer altera��o que os afete.
    */
   if (m_caretpos.y >= m_end_line - m_additional_line ? 1 : 0) {
      m_scrollpos.y += 1;
   }
   else if (m_caretpos.y < m_start_line) {
      m_scrollpos.y -= 1;
   }

   ValidateScroll(m_scrollpos);
   UpdateScrollbars();

   m_frame->GetCanvas()->Refresh(), m_frame->GetCanvas()->Update();
   UpdateCaret();
}

/**
 * Updates all the attributes of the view in response to a change
 * of settings or when the application window is resized or changed
 * in any other way.
 */
void HexView::RecalcDisplay() {
   // wxLogDebug("<<Flow>> HexView::RecalcDisplay");

   wxClientDC dc(m_frame->GetCanvas());
   dc.SetFont(*wxTheFontList->FindOrCreateFont(
      12, wxFONTFAMILY_MODERN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxT("Courier")));

   text_sz = wxSize(dc.GetCharWidth(), dc.GetCharHeight());
   wxFontMetrics fm = dc.GetFontMetrics();
   m_text_y_offset = std::max(0, (fm.descent - fm.internalLeading) / 2);
   m_addr_width = wxString::Format(wxT("%lX:"), view_data_sz).size();
   m_addr_fmt = wxString::Format(wxT("%%0%dX:"), m_addr_width - 1);

   wxSize client_sz = m_frame->GetCanvas()->GetClientSize();
   // m_base = wxRect(0, m_show_ruler ? text_sz.y + 4 : 0, client_sz.x, client_sz.y + (m_show_ruler ? text_sz.y + 4 :
   // 0));
   int rulerHeight = m_show_ruler ? text_sz.y + 4 : 0;
   m_base = wxRect(0, rulerHeight, client_sz.x, client_sz.y - rulerHeight);

   // scrolling
   wxFileOffset doc_len = static_cast<HexDoc *>(GetDocument())->length();
   m_numlines = ceil(float(doc_len) / m_columns);
   m_additional_line = ((client_sz.y - m_base.y) % text_sz.y) > 0;

   // if the file data ends exactly in the end of an line, the next one must be shown
   m_numlines += ((doc_len % m_columns) == 0) ? 1 : 0;

   m_caretpos = wxPoint(0, 0);
   UpdateScrollbars();
   UpdateCaret();
}

/**
 * Updates the current state of the attributes of this view.
 * This may be called anytime another method needs an updates state
 * before refreshing the view (OnDraw).
 */
void HexView::RecalcViewState() {
   m_start_line = m_scrollpos.y;
   m_end_line = std::min(m_start_line + m_numshown, m_numlines);
}

void HexView::UpdateScrollbars() {
   // wxLogDebug("<<Flow>> HexView::UpdateScrollbars");
   wxSize clientsz = m_frame->GetCanvas()->GetClientSize();
   m_numshown = floor(float(clientsz.y) / text_sz.y);

   // if the client area can't shown a round number of complete lines, we need an extra line
   int extra = m_additional_line ? 1 : 0;

   // if we are scrolling past ends, we should make the scrollbar bigger
   // TODO: almost correct! when it reaches its last position, it grows one more unit (where it shouldn't)
   if (m_scrollpos.y + m_numshown > m_numlines + extra)
      extra += m_numshown - (m_end_line - m_start_line);

   // show/adjust or hide the vertical scrollbar
   if (m_numlines >= m_numshown)
      m_frame->GetCanvas()->SetScrollbar(
         wxVERTICAL, m_scrollpos.y, m_numshown, m_numlines + extra /* + m_numshown - 1*/, true);
   else
      m_frame->GetCanvas()->SetScrollbar(wxVERTICAL, 0, 2, 1, true);
}

void HexView::DoMouseWheel(const int lines) {
   // negative scrolls upwards, not downwards
   m_scrollpos.y += -lines;

   // validate and scroll
   ValidateScroll(m_scrollpos);
   UpdateScrollbars();

   m_frame->GetCanvas()->Refresh();
   UpdateCaret();
}

/**
 * Returns closest hex area column given x display position
 * @param pos mouse pointer position
 * @return Closest hex area column
 */
int HexView::Pos2HexCol(const int pos) const {
   int col = pos - m_addr_width * text_sz.x;
   col -= (col / (text_sz.x * (m_group_by * 3 + 1))) * text_sz.x;
   col /= 3 * text_sz.x;

   if (col < 0)
      col = 0;
   else if (col > m_columns)
      col = m_columns;

   return col;
}

// returns closet char area column given x display position
int HexView::Pos2CharCol(const int pos) const {
   int col = pos - (1 + m_addr_width) * text_sz.x;
   col -= (3 * m_columns + m_columns / m_group_by - 1) * text_sz.x;
   col /= text_sz.x;

   if (col < 0)
      col = 0;
   if (col > m_columns)
      col = m_columns;

   return col;
}

#ifdef HT_DEBUG_PANEL_ENABLED

void HexView::AddToDebugPanel(DebugPanel *panel) {
   if (!panel) {
      return;
   }

   m_debugWatches.clear();
   const wxString tag = wxString::Format("[%s] ", wxFileNameFromPath(GetDocument()->GetFilename()));

   auto watch = [&](const char *name, const auto *var) {
      m_debugWatches.push_back(panel->AddWatchRef(tag + name, var));
   };

   watch("m_numlines", &m_numlines);
   watch("m_numshown", &m_numshown);
   watch("m_start_line", &m_start_line);
   watch("m_end_line", &m_end_line);
   watch("m_caretpos", &m_caretpos);
   watch("m_base", &m_base);
   watch("m_scrollpos", &m_scrollpos);

   m_debugWatches.push_back(panel->AddWatch(
      tag + "offset", [this] { return wxString::Format("%lld", static_cast<long long>(GetCurrentOffset())); }));
}

#endif
