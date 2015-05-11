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

/*BEGIN_EVENT_TABLE(HexView, wxView)
   EVT_PAINT(HexFrame::OnPaint)
   EVT_ERASE_BACKGROUND(HexFrame::OnEraseBackground)
   EVT_LEFT_DOWN(HexFrame::OnLeftDown)
   EVT_LEFT_UP(HexFrame::OnLeftUp)
   EVT_MOTION(HexFrame::OnMotion)
   EVT_LEAVE_WINDOW(HexFrame::OnLeaveWindow)
   EVT_KEY_DOWN(HexView::OnKeyDown)
   EVT_SIZE(HexFrame::OnSize)
END_EVENT_TABLE()*/


HexView::HexView ()
{
   wxLogDebug("[HexView] created at 0x%X.", reinterpret_cast<int>(this));

   m_frame = 0;
   view_data = 0;
   view_data_sz = 0;

   // -- Colors setup
   line_clr.Set(0, 105, 153);     // dark cyan
   addrhex_clr.Set(128, 0, 0);    // dark red
   addrbg_clr.Set(192, 224, 240); // cyan
   addrhl_clr.Set(255, 160, 128); // beige

   HtPrefs &prefs = wxGetApp().prefs;

   m_columns = prefs.getInt(wxT("document/visualization/columns"));
   m_group_by = prefs.getInt(wxT("document/visualization/group-by"));
   m_show_ruler = prefs.getBool(wxT("workspace/display/show-ruler"));
   m_show_cursor_pos = prefs.getBool(wxT("workspace/display/show-cursor-position"));
   m_show_mouse_pos = prefs.getBool(wxT("workspace/display/show-mouse-position"));
   m_scroll_past_ends = prefs.getBool(wxT("workspace/display/scroll-past-ends"));

   m_start_line = 0;
   m_end_line = 0;

   m_edit_char = false;  // caret starts in hex area
   m_mouse_down = false; // mouse is initially up
   m_mouse_over = wxPoint(-1, -1);
   m_scrollpos = wxPoint(0, 0);
   m_caretpos = wxPoint(0, 0);
}

HexView::~HexView ()
{
   wxLogDebug("[HexView] at 0x%X destroyed.", reinterpret_cast<int>(this));

   if (view_data)
      delete [] view_data;

   m_frame->SetView(nullptr);
}

bool HexView::OnCreate (wxDocument *doc, long WXUNUSED(flags))
{
   wxLogStatus("[HexView] OnCreate called.");
   //HexDoc *hdoc = (HexDoc *)doc;

   return true;
}

void HexView::OnLeftDown (wxMouseEvent &event)
{
   wxPoint point = event.GetPosition();

   bool same_pos = false;
   int row = ((point.y - m_base.y) / text_sz.y) + m_start_line;

   // a click in the horizontal ruler
   if (!m_base.Contains(point)) return;

   if (point.x >= 0 && point.x <= CharCol2Pos(0) - text_sz.x / 2)
   {
      // -- we have a click in the hex area
      int col = Pos2HexCol(point.x);

      // past last column must put the caret at last column
      if (col == m_columns)
         col = m_columns - 1;
         //col = 0, row += 1;  // this makes it behave quite like hexedit, but still needs some work

      // if we are past file size, go back
      if (row * m_columns + col >= view_data_sz)
         row = view_data_sz / m_columns, col = view_data_sz % m_columns;

      if (col == m_caretpos.x && row == m_caretpos.y)
         same_pos = true;

      //m_frame->SetCaretPos(wxPoint(col, row));
      m_caretpos = wxPoint(col, row);
      m_frame->GetCaret()->Hide();

      m_edit_char = false;
      m_mouse_down = true;
      UpdateCaret();
   }
   else if (point.x >= CharCol2Pos(0) && point.x <= CharCol2Pos(m_columns) + text_sz.x)
   {
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
      m_frame->GetCaret()->Hide();
      
      m_edit_char = true;
      m_mouse_down = true;
      UpdateCaret();
   }

   if (!same_pos)
      m_frame->Refresh(), m_frame->Update();
}

void HexView::OnLeftUp (wxMouseEvent &WXUNUSED(event))
{
   if (!m_frame->GetCaret()->IsVisible())
      m_frame->GetCaret()->Show();

   m_mouse_down = false;
   m_frame->Refresh(), m_frame->Update();
}

void HexView::OnMotion (wxMouseEvent &event)
{
   const wxPoint pos = event.GetPosition();
   wxRect edit_area = wxRect(wxPoint(m_addr_width * text_sz.x, m_base.y), wxPoint(CharCol2Pos(m_columns) + text_sz.x, m_base.GetBottom()));

   // update cursor based on what it's hovering
   m_frame->SetCursor(edit_area.Contains(pos) ? wxCURSOR_IBEAM : wxCURSOR_ARROW);

   bool over_hex = false;
   if (pos.x >= 0 && pos.x <= CharCol2Pos(0) - text_sz.x / 2)
      over_hex = true;

   // update mouse hovering row
   edit_area.x += text_sz.x, edit_area.width -= text_sz.x;
   int row = edit_area.Contains(pos) ? (pos.y - m_base.y) / text_sz.y : -1;
   int col = edit_area.Contains(pos) ? over_hex ? Pos2HexCol(pos.x) : Pos2CharCol(pos.x) : -1;

   if (col == m_columns) col = m_columns - 1;

   bool refresh = false;

   if (row != m_mouse_over.y)
      m_mouse_over.y = row, refresh = true;

   if (col != m_mouse_over.x)
      m_mouse_over.x = col, refresh = true;

   if (refresh)
      m_frame->Refresh(), m_frame->Update();
}

void HexView::OnLeaveWindow (wxMouseEvent &WXUNUSED(event))
{
   m_mouse_over = wxPoint(-1, -1);
   m_frame->Refresh(), m_frame->Update();
}

void HexView::OnKeyDown (wxKeyEvent &event)
{
   switch (event.GetKeyCode())
   {
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
         
         m_frame->Refresh(), m_frame->Update();
         break;

      default:
         event.Skip();
   }
}

void HexView::OnScroll (wxScrollWinEvent &event)
{
   if (event.GetOrientation() == wxVERTICAL)
   {
      const int &type = event.GetEventType();

      if      (type == wxEVT_SCROLLWIN_LINEUP)   m_scrollpos.y -= 1;
      else if (type == wxEVT_SCROLLWIN_LINEDOWN) m_scrollpos.y += 1;
      else if (type == wxEVT_SCROLLWIN_PAGEUP)   m_scrollpos.y -= m_numshown;
      else if (type == wxEVT_SCROLLWIN_PAGEDOWN) m_scrollpos.y += m_numshown;

      else if (type == wxEVT_SCROLLWIN_THUMBTRACK)
      {
         m_scrollpos.y = event.GetPosition();
      }
      else if (type == wxEVT_SCROLL_THUMBRELEASE)
      {
         UpdateScrollbars();
      }
      else if (type == wxEVT_SCROLLWIN_BOTTOM)
      {
         wxLogStatus(wxT("[HexFrame] Vertical SB: Received EVT_SCROLLWIN_BOTTOM event"));
         m_scrollpos.y = m_numlines;
      }
   }

   ValidateScroll(m_scrollpos);
   UpdateScrollbars();

   //m_view->ValidateCaret(m_caretpos);
   m_frame->Refresh(), m_frame->Update();
   UpdateCaret();
}

void HexView::OnSize(wxSizeEvent &WXUNUSED(event))
{
   RecalcDisplay();
}


void HexView::OnSetFocus(wxFocusEvent &event)
{
   if (!m_frame->GetCaret()->IsVisible())
      m_frame->GetCaret()->Show();
}

void HexView::OnKillFocus(wxFocusEvent &event)
{
   if (m_frame->GetCaret()->IsVisible())
      m_frame->GetCaret()->Hide();
}





/*
No HexEdit, a renderização é composta das seguintes etapas:
+ pré-processamento e cálculo de valores posteriormente necessários
+ cálculo da parte do arquivo que precisar ser renderizada
+ renderização dos resultados da busca
+ renderização das áreas em highlight
+ renderização do(s) marcador(es)
+ renderização dos offsets
+ renderização dos hexadecimais e caracteres, alternadamente
+ inversão das colunas que estiverem selecionadas
*/
void HexView::OnDraw (wxDC *dc)
{
#if 1
   auto start = std::chrono::system_clock::now();
#endif


   // -- setup
   wxFont &font = *wxTheFontList->FindOrCreateFont(12, wxFONTFAMILY_MODERN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxT("Courier"));
   dc->SetFont(font);

   wxASSERT(m_columns > 0);
   wxASSERT(m_group_by > 0);

   if (!view_data)
   {
      HexDoc *doc = (HexDoc *)GetDocument();

      view_data = new unsigned char[doc->length()];
      view_data_sz = doc->GetData(view_data, doc->length());

      RecalcDisplay();
   }

   wxSize client_sz(m_frame->GetClientSize());
   int rulerHeight = m_show_ruler ? text_sz.y + 4 : 0;
   m_base = wxRect(0, rulerHeight, client_sz.x, client_sz.y - rulerHeight);
   //m_base = wxRect(0, m_show_ruler ? text_sz.y + 4 : 0, client_sz.x, client_sz.y + (m_show_ruler ? text_sz.y + 4 : 0));

   // calculate the amount of lines we need to display and the amount of lines used by our file
   // TODO: ensure that every change in the window updates those 2 values
   //m_numlines = static_cast <int> (ceil(float(view_data_sz) / m_columns)) + (view_data_sz % m_columns ? 0 : 1);
   //m_numshown = static_cast <int> (ceil(float(m_base.height) / text_sz.y));

   // which rows in the file context represent the rows displayed in the view
   m_start_line = m_scrollpos.y;
   m_end_line = std::min(m_start_line + m_numshown, m_numlines);
   
   unsigned char *data_ptr = view_data + m_start_line * m_columns;

   // -- rendering
   DrawBackground(dc);

   // display area lines
   dc->SetPen(wxPen(line_clr));
   DrawVerticalLine(*dc, m_addr_width * text_sz.x, m_base.y - 3, m_base.GetBottom());
   DrawVerticalLine(*dc, CharCol2Pos(0) - text_sz.x / 2, m_base.y - 3, m_base.GetBottom());
   DrawVerticalLine(*dc, CharCol2Pos(m_columns) + text_sz.x / 2, m_base.y - 3, m_base.GetBottom());

   // display current caret position highlight
   if (!m_mouse_down && m_show_cursor_pos)
   {
      dc->SetPen(wxPen(addrhex_clr));
      dc->SetBrush(wxBrush(addrhl_clr));

      if ((m_caretpos.y - m_start_line) >= 0)
         dc->DrawRectangle(0, m_base.y + (m_caretpos.y - m_start_line) * text_sz.y, m_addr_width * text_sz.x - 1, text_sz.y);
      
      if (m_show_ruler)
      {
         dc->DrawRectangle(CharCol2Pos(m_caretpos.x), 0, text_sz.x + 1, text_sz.y);
         dc->DrawRectangle(HexCol2Pos(m_caretpos.x), 0, 2 * text_sz.x + 1, text_sz.y);
      }
   }

   // display the horizontal ruler
   if (m_show_ruler)
   {
      dc->SetPen(wxPen(line_clr));
      dc->DrawLine(m_addr_width * text_sz.x, m_base.y - 3, client_sz.x, m_base.y - 3);

      dc->SetTextForeground(addrhex_clr);
      dc->SetPen(wxPen(addrhex_clr));

      for (int i = 0; i < m_columns; i++)
      {
         dc->DrawText(wxString::Format(wxT("%02X"), i), HexCol2Pos(i), 0);
         dc->DrawText(wxString::Format(wxT("%02X"), i).substr(1), CharCol2Pos(i), 0);
      }
   }

   // display which row is currently under the mouse pointer
   if (m_mouse_over != wxPoint(-1, -1) && m_show_mouse_pos)
   {
      dc->SetPen(wxPen(addrhex_clr));
      dc->SetBrush(*wxTRANSPARENT_BRUSH);

      // row
      if ((m_mouse_over.y + m_start_line) < m_numlines)
         dc->DrawRectangle(0, m_base.y + m_mouse_over.y * text_sz.y, m_addr_width * text_sz.x - 1, text_sz.y);

      // columns
      if (m_show_ruler)
      {
         dc->DrawRectangle(CharCol2Pos(m_mouse_over.x), 0, text_sz.x + 1, text_sz.y);
         dc->DrawRectangle(HexCol2Pos(m_mouse_over.x), 0, 2 * text_sz.x + 1, text_sz.y);
      }
   }

   // display each line, from top to bottom
   for (int i = m_start_line; i < m_end_line; i++)
   {
      // display formatted offset
      wxString offset = wxString::Format(m_addr_fmt, i * m_columns);
      dc->SetTextForeground(addrhex_clr);
      dc->DrawText(offset, 0, m_base.y + (i - m_start_line) * text_sz.y);

      dc->SetTextForeground(*wxBLACK);

      // display each byte in hex (and char)
      for (int j = i * m_columns; j < (i * m_columns) + m_columns; j++)
      {
         if (j >= view_data_sz) break;

         wxString byte = wxString::Format(wxT("%02X"), view_data[j]);
         dc->DrawText(byte, HexCol2Pos(j % m_columns), m_base.y + (i - m_start_line) * text_sz.y);

         const char &c = (view_data[j] > 0x1F && view_data[j] < 0x7F) ? view_data[j] : '.';
         dc->DrawText(wxString::Format(wxT("%c"), c), CharCol2Pos(j % m_columns), m_base.y + (i - m_start_line) * text_sz.y);
      }
   }
   
   // display the selected bytes in reverse video
   wxPoint caret_pos = m_caretpos;//m_frame->GetCaretPos();
   
   dc->SetLogicalFunction(wxINVERT);
   dc->SetBrush(*wxBLACK_BRUSH);
   
   if (caret_pos.y >= m_start_line && caret_pos.y <= m_end_line)
   {
      caret_pos.y = (caret_pos.y - m_start_line) * text_sz.y + m_base.y;

      if (!m_edit_char)
      {
         int cx = CharCol2Pos(caret_pos.x);
         dc->DrawRectangle(cx, caret_pos.y, text_sz.x, text_sz.y);
      }
      else
      {
         int hx = HexCol2Pos(caret_pos.x);
         dc->DrawRectangle(hx, caret_pos.y, 2 * text_sz.x, text_sz.y);
      }
   }

#if 1
   auto end = std::chrono::system_clock::now();
   auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

   //wxLogStatus("[HexView] ::OnDraw run time: %I64d ms", elapsed.count());
#endif
}

void HexView::DrawBackground(wxDC *dc) const
{
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

#if 0
   wxSize client_sz = m_frame->GetClientSize();
   const int lines_shown = int(client_sz.y / text_sz.y) + (client_sz.y % text_sz.y ? 1 : 0);

   for (int i = 0; i < lines_shown; i++)
   {
      dc->SetPen(*wxBLACK_PEN);
      dc->DrawLine(0, i * text_sz.y, client_sz.x, i * text_sz.y);

      if (!i)
      {
         int colcount = 0;
         int curhexb_x = offw + text_sz.x;
         int curcharb_x = curhexb_x + (m_columns * text_sz.x * 2 + m_columns * 9) + (m_group_by ? (m_group_by - 1) * text_sz.x : 0);

         for (int j = 0; j < m_columns; j++)
         {
            dc->SetPen(*wxLIGHT_GREY_PEN);
            dc->DrawLine(curhexb_x, 0, curhexb_x, client_sz.y);

            dc->DrawLine(curhexb_x + text_sz.x * 2, 0, curhexb_x + text_sz.x * 2, client_sz.y);
            curhexb_x += text_sz.x + text_sz.x + text_sz.x;

            dc->SetPen(*wxBLACK_PEN);
            dc->DrawLine(curcharb_x, 0, curcharb_x, client_sz.y);
            curcharb_x += text_sz.x;

            if (m_group_by && ++colcount == m_group_by)
               colcount = 0, curhexb_x += text_sz.x;
         }
      }
   }

   dc->SetPen(wxPen(wxColour(0, 105, 153)));
#endif

#if 0
   dc->SetPen(*wxRED_PEN);

   // horizontal
   dc->DrawLine(m_base.GetTopLeft(), m_base.GetTopRight());
   dc->DrawLine(m_base.GetLeft(), m_base.GetBottom(), m_base.GetRight(), m_base.GetBottom());

   // vertical
   dc->DrawLine(m_base.GetTopLeft(), m_base.GetBottomLeft());
   dc->DrawLine(m_base.GetTopRight(), m_base.GetBottomRight());


   dc->SetPen(wxPen(wxColour(0, 105, 153)));
#endif
}

void HexView::MovePos(int key, bool ctrl_down, bool shift_down, bool alt_down)
{
   int col = m_caretpos.x;
   int row = m_caretpos.y;

   if (key == WXK_LEFT)
   {
      col -= 1;

      if (col < 0 && row == 0)
         col = 0;

      if (col < 0 && row > 0)
         row -= 1, col = m_columns - 1;
   }
   else if (key == WXK_RIGHT)
   {
      if (++col == m_columns)
         col = 0, row += 1;
   }
   else if (key == WXK_UP)
   {
      if (--row < 0)
         row = 0;
   }
   else if (key == WXK_DOWN)
   {
      row += 1;
   }
   else if (key == WXK_END && ctrl_down)
   {
      // just make it ajust the position to the last one in the view
      row = m_numlines + 1;
      col = m_columns + 1;
   }
   else if (key == WXK_HOME && ctrl_down)
   {
      row = 0;
      col = 0;
   }

   if (row * m_columns + col > view_data_sz)
      col = view_data_sz % m_columns, row = view_data_sz / m_columns;
      
   m_caretpos = wxPoint(col, row);

   /* as variáveis m_start_pos e m_end_pos são atualizadas apenas na função
    OnDraw(), e talvez seja necessário mudar isso porque tem outras partes da HexView
    que dependem de valores atualizados dessas variáveis antes do próximo OnDraw.

    A navegação não-linear do documento por exemplo requer cálculos para reposicionamento
    dos elementos gráficos da view antes do próximo Draw, então talvez a melhor opção seja
    passar esse cálculo de atualização das variáveis para uma função e chamá-la caso necessário,
    ou ainda atualizar esses valores após qualquer alteração que os afete.
    */
   if (m_caretpos.y >= m_end_line - m_additional_line ? 1 : 0)
   {
      wxLogStatus("I need to scroll down 1 line");
      m_scrollpos.y += 1;
   }
   else if (m_caretpos.y < m_start_line)
   {
      wxLogStatus("I need to scroll up 1 line");
      m_scrollpos.y -= 1;
   }

   ValidateScroll(m_scrollpos);
   UpdateScrollbars();

   m_frame->Refresh(), m_frame->Update();
   UpdateCaret();
}

/**
* Updates all the attributes of the view in response to a change
* of settings or when the application window is resized or changed
* in any other way.
*/
void HexView::RecalcDisplay ()
{
   //wxLogDebug("<<Flow>> HexView::RecalcDisplay");

   wxClientDC dc(m_frame);
   dc.SetFont(*wxTheFontList->FindOrCreateFont(12, wxFONTFAMILY_MODERN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxT("Courier")));

   text_sz = wxSize(dc.GetCharWidth(), dc.GetCharHeight());
   m_addr_width = wxString::Format(wxT("%X:"), view_data_sz).size();
   m_addr_fmt = wxString::Format(wxT("%%0%dX:"), m_addr_width - 1);

   wxSize client_sz = m_frame->GetClientSize();
   //m_base = wxRect(0, m_show_ruler ? text_sz.y + 4 : 0, client_sz.x, client_sz.y + (m_show_ruler ? text_sz.y + 4 : 0));
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
void HexView::RecalcViewState ()
{
   m_start_line = m_scrollpos.y;
   m_end_line = std::min(m_start_line + m_numshown, m_numlines);
}

void HexView::UpdateScrollbars ()
{
   //wxLogDebug("<<Flow>> HexView::UpdateScrollbars");
   wxSize clientsz = m_frame->GetClientSize();
   m_numshown = floor(float(clientsz.y) / text_sz.y);

   // if the client area can't shown a round number of complete lines, we need an extra line
   int extra = m_additional_line ? 1 : 0;

   // if we are scrolling past ends, we should make the scrollbar bigger
   // TODO: almost correct! when it reaches its last position, it grows one more unit (where it shouldn't)
   if (m_scrollpos.y + m_numshown > m_numlines + extra)
      extra += m_numshown - (m_end_line - m_start_line);

   if (m_frame->HasFocus())
      wxLogStatus("[HexView] UpdateScrollbars: m_numlines(%d), m_numshown(%d), extra(%d)", m_numlines + extra, m_numshown, extra);

   // show/adjust or hide the vertical scrollbar
   if (m_numlines >= m_numshown)
      m_frame->SetScrollbar(wxVERTICAL, m_scrollpos.y, m_numshown, m_numlines + extra/* + m_numshown - 1*/, true);
   else
      m_frame->SetScrollbar(wxVERTICAL, 0, 2, 1, true);
}

void HexView::DoMouseWheel (const int lines)
{
   // negative scrolls upwards, not downwards
   m_scrollpos.y += -lines;

   // validate and scroll
   ValidateScroll(m_scrollpos);
   UpdateScrollbars();

   m_frame->Refresh(), m_frame->Update();
   UpdateCaret();
}

/**
* Returns closest hex area column given x display position
* @param pos mouse pointer position
* @return Closest hex area column
*/
int HexView::Pos2HexCol (const int pos) const
{
   int col = pos - m_addr_width * text_sz.x;
   col -= (col / (text_sz.x * (m_group_by * 3 + 1))) * text_sz.x;
   col /= 3 * text_sz.x;

   if (col < 0) col = 0;
   else if (col > m_columns) col = m_columns;

   return col;
}

// returns closet char area column given x display position
int HexView::Pos2CharCol (const int pos) const
{
   int col = pos - (1 + m_addr_width) * text_sz.x;
   col -= (3 * m_columns + m_columns / m_group_by - 1) * text_sz.x;
   col /= text_sz.x;

   if (col < 0) col = 0;
   if (col > m_columns) col = m_columns;

   return col;
}

#ifdef WXDEBUG
void HexView::AddToDebugPanel (DebugPanel *dbgPanel)
{
   dbgPanel->AddProperty("[View] m_numlines", DebugPanel::Type_Long, &m_numlines);
   dbgPanel->AddProperty("[View] m_numshown", DebugPanel::Type_Long, &m_numshown);
   dbgPanel->AddProperty("[View] m_start_line", DebugPanel::Type_Long, &m_start_line);
   dbgPanel->AddProperty("[View] m_end_line", DebugPanel::Type_Long, &m_end_line);
   dbgPanel->AddProperty("[View] m_caretpos", DebugPanel::Type_wxPoint, &m_caretpos);
   dbgPanel->AddProperty("[View] m_base", DebugPanel::Type_wxRect, &m_base);
   dbgPanel->AddProperty("[View] m_scrollpos", DebugPanel::Type_wxPoint, &m_scrollpos);
}

void HexView::RemoveFromDebugPanel (DebugPanel *dbgPanel)
{
   dbgPanel->RemoveProperty("[View] m_numlines");
   dbgPanel->RemoveProperty("[View] m_numshown");
   dbgPanel->RemoveProperty("[View] m_start_line");
   dbgPanel->RemoveProperty("[View] m_end_line");
   dbgPanel->RemoveProperty("[View] m_caretpos");
   dbgPanel->RemoveProperty("[View] m_base");
   dbgPanel->RemoveProperty("[View] m_scrollpos");
}
#endif