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
#include <cstdint>

using ht::core::ByteOffset;
using ht::core::CaretMove;
using ht::core::EditorPane;

IMPLEMENT_DYNAMIC_CLASS(HexView, wxView)

HexView::HexView() {
   wxLogDebug("[HexView] created at 0x%llX.", reinterpret_cast<uint64_t>(this));

   m_base = wxRect(0, 0, 0, 0);
   m_frame = 0;
   view_data = 0;
   view_data_sz = 0;

   HtPrefs &prefs = wxGetApp().prefs;

   m_columns = prefs.getInt(wxT("document/visualization/columns"));
   m_group_by = prefs.getInt(wxT("document/visualization/group-by"));
   m_show_ruler = prefs.getBool(wxT("workspace/display/show-ruler"));
   m_show_cursor_pos = prefs.getBool(wxT("workspace/display/show-cursor-position"));
   m_show_mouse_pos = prefs.getBool(wxT("workspace/display/show-mouse-position"));
   m_scroll_past_ends = prefs.getBool(wxT("workspace/display/scroll-past-ends"));

   m_start_line = 0;
   m_end_line = 0;

   m_mouse_down = false;  // mouse is initially up
   m_mouse_over = wxPoint(-1, -1);
   m_scrollpos = wxPoint(0, 0);
   m_text_y_offset = 0;
}

HexView::~HexView() {
   wxLogDebug("[HexView] at 0x%llX destroyed.", reinterpret_cast<uint64_t>(this));

   if (view_data)
      delete[] view_data;
}

bool HexView::OnCreate(wxDocument *doc, long WXUNUSED(flags)) {
   // HexDoc *hdoc = (HexDoc *)doc;

   return true;
}

void HexView::OnLeftDown(wxMouseEvent &event) {
   wxPoint point = event.GetPosition();

   if (!m_base.Contains(point))
      return;

   EditorPane pane;
   int col;
   if (point.x >= 0 && point.x <= CharCol2Pos(0) - text_sz.x / 2) {
      pane = EditorPane::Hex;
      // The hex trailing edge selects the last column of this row.
      col = std::min(Pos2HexCol(point.x), m_columns - 1);
   }
   else if (point.x >= CharCol2Pos(0) && point.x <= CharCol2Pos(m_columns) + text_sz.x) {
      pane = EditorPane::Character;
      // The character trailing edge can select the first byte of the next row.
      col = Pos2CharCol(point.x);
   }
   else {
      return;
   }

   const wxFileOffset length = GetDocument()->length();
   if (length < 0)
      return;

   const std::int64_t row = static_cast<std::int64_t>(m_start_line) + (point.y - m_base.y) / text_sz.y;
   m_editorController.PlaceCaret(row * m_columns + col, pane, length);
   m_frame->GetCanvas()->GetCaret()->Hide();
   m_mouse_down = true;
   UpdateCaret();
   m_frame->GetCanvas()->Refresh();
   m_frame->GetCanvas()->Update();
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
         MoveCaret(CaretMove::Left);
         break;
      case WXK_RIGHT:
         MoveCaret(CaretMove::Right);
         break;
      case WXK_UP:
         MoveCaret(CaretMove::Up);
         break;
      case WXK_DOWN:
         MoveCaret(CaretMove::Down);
         break;
      case WXK_HOME:
         if (event.ControlDown())
            MoveCaret(CaretMove::DocumentStart);
         break;
      case WXK_END:
         if (event.ControlDown())
            MoveCaret(CaretMove::DocumentEnd);
         break;
      case WXK_TAB:
         m_editorController.SwitchPane();
         UpdateCaret();

         m_frame->GetCanvas()->Refresh(), m_frame->GetCanvas()->Update();
         break;

      default:
         event.Skip();
   }
}

std::optional<ByteOffset> HexView::JumpToOffset(ByteOffset requestedOffset) {
   const wxFileOffset documentLength = GetDocument()->length();
   if (documentLength < 0)
      return std::nullopt;

   const ByteOffset caretOffset = m_editorController.JumpToOffset(requestedOffset, documentLength);
   const int caretRow = static_cast<int>(caretOffset / m_columns);
   const int visibleRows = std::max(1, m_base.height / text_sz.y);

   if (caretRow < m_scrollpos.y || caretRow - m_scrollpos.y >= visibleRows) {
      m_scrollpos.y = caretRow - visibleRows / 2;
   }

   ValidateScroll(m_scrollpos);
   RecalcViewState();
   UpdateScrollbars();

   m_frame->GetCanvas()->Refresh();
   m_frame->GetCanvas()->Update();
   UpdateCaret();
   return caretOffset;
}

wxPoint HexView::GetCaretPosition() const {
   const ByteOffset offset = GetCurrentOffset();
   return wxPoint(static_cast<int>(offset % m_columns), static_cast<int>(offset / m_columns));
}

void HexView::UpdateCaret() {
   const wxPoint caretPosition = GetCaretPosition();
   const int x = m_editorController.GetState().activePane == EditorPane::Character ? CharCol2Pos(caretPosition.x)
                                                                                   : HexCol2Pos(caretPosition.x);
   const int y = (caretPosition.y - m_start_line) * text_sz.y + m_base.y;

   m_frame->GetCanvas()->GetCaret()->Move(x, y);
   caretPosition.y >= m_start_line ? m_frame->ShowCaret() : m_frame->HideCaret();
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
   dc->SetPen(wxPen(wxGetApp().GetColorByRole(ColorRole::Separator)));
   DrawVerticalLine(*dc, m_addr_width * text_sz.x, m_base.y - 3, m_base.GetBottom());
   DrawVerticalLine(*dc, CharCol2Pos(0) - text_sz.x / 2, m_base.y - 3, m_base.GetBottom());
   DrawVerticalLine(*dc, CharCol2Pos(m_columns) + text_sz.x / 2, m_base.y - 3, m_base.GetBottom());
}

void HexView::DrawRuler(wxDC *dc) const {
   if (!m_show_ruler)
      return;

   wxSize client_sz(m_frame->GetCanvas()->GetClientSize());
   dc->SetPen(wxPen(wxGetApp().GetColorByRole(ColorRole::Separator)));
   dc->DrawLine(m_addr_width * text_sz.x, m_base.y - 3, client_sz.x, m_base.y - 3);

   const wxColour &rulerText = wxGetApp().GetColorByRole(ColorRole::RulerText);
   dc->SetTextForeground(rulerText);
   dc->SetPen(wxPen(rulerText));

   for (int i = 0; i < m_columns; i++) {
      dc->DrawText(wxString::Format(wxT("%02X"), i), HexCol2Pos(i), m_text_y_offset);
      dc->DrawText(wxString::Format(wxT("%02X"), i).substr(1), CharCol2Pos(i), m_text_y_offset);
   }
}

void HexView::DrawPositionIndicator(wxDC *dc, int view_row, int col, bool filled) const {
   dc->SetPen(wxPen(wxGetApp().GetColorByRole(ColorRole::RulerText)));
   dc->SetBrush(filled ? wxBrush(wxGetApp().GetColorByRole(ColorRole::HighlightBackground)) : *wxTRANSPARENT_BRUSH);

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

   const wxPoint caretPosition = GetCaretPosition();
   DrawPositionIndicator(dc, caretPosition.y - m_start_line, caretPosition.x, true);
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

      dc->SetTextForeground(wxGetApp().GetColorByRole(ColorRole::RulerText));
      dc->DrawText(wxString::Format(m_addr_fmt, i * m_columns), 0, text_y);

      dc->SetTextForeground(wxGetApp().GetColorByRole(ColorRole::ViewText));

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
   const wxPoint caretPosition = GetCaretPosition();
   if (caretPosition.y < m_start_line || caretPosition.y > m_end_line)
      return;

   const wxColour &caret_fill_clr = wxGetApp().GetColorByRole(ColorRole::EditPosition);

   const ByteOffset caretOffset = GetCurrentOffset();
   const int cy = (caretPosition.y - m_start_line) * text_sz.y + m_base.y;
   const int hx = HexCol2Pos(caretPosition.x);
   const int cx = CharCol2Pos(caretPosition.x);

   const wxString hexText =
      caretOffset < view_data_sz ? wxString::Format(wxT("%02X"), view_data[caretOffset]) : wxString();
   const wxString charText =
      caretOffset < view_data_sz
         ? wxString::Format(wxT("%c"),
              (view_data[caretOffset] > 0x1F && view_data[caretOffset] < 0x7F) ? (char)view_data[caretOffset] : '.')
         : wxString();

   if (m_editorController.GetState().activePane == EditorPane::Hex) {
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
         dc->SetTextForeground(wxGetApp().GetColorByRole(ColorRole::EditText));
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
   dc->SetBackground(wxBrush(wxGetApp().GetColorByRole(ColorRole::ViewBackground)));
   dc->Clear();

   int offw = m_addr_width * text_sz.x;
   wxRect client = m_frame->GetClientRect();

   dc->SetBrush(wxBrush(wxGetApp().GetColorByRole(ColorRole::RulerBackground)));
   dc->SetPen(*wxTRANSPARENT_PEN);

   // offsets column
   dc->DrawRectangle(wxRect(0, 0, offw, m_frame->GetClientRect().height));

   // horizontal ruler
   if (m_show_ruler)
      dc->DrawRectangle(wxRect(0, 0, m_base.width, m_base.y - 3));
}

void HexView::MoveCaret(CaretMove move) {
   const wxFileOffset documentLength = GetDocument()->length();
   if (documentLength < 0)
      return;

   m_editorController.MoveCaret(move, documentLength, m_columns);
   const std::int64_t caretRow = GetCurrentOffset() / m_columns;

   if (caretRow >= m_end_line - m_additional_line) {
      m_scrollpos.y += 1;
   }
   else if (caretRow < m_start_line) {
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
   watch("m_base", &m_base);
   watch("m_scrollpos", &m_scrollpos);
   watch("offset", &m_editorController.GetState().caretOffset);

   m_debugWatches.push_back(
      panel->AddWatch(tag + "caret position", [this] { return DebugPanel::ToString(GetCaretPosition()); }));
}

#endif
