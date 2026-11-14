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

#include "hex_frame.hpp"
#include "hex_view.hpp"
#include <wx/dcbuffer.h>
#include <wx/sizer.h>

BEGIN_EVENT_TABLE(HexCanvas, wxScrolledWindow)
   EVT_PAINT(HexCanvas::OnPaint)
   EVT_ERASE_BACKGROUND(HexCanvas::OnEraseBackground)
   EVT_LEFT_DOWN(HexCanvas::OnLeftDown)
   EVT_LEFT_UP(HexCanvas::OnLeftUp)
   EVT_MOTION(HexCanvas::OnMotion)
   EVT_LEAVE_WINDOW(HexCanvas::OnLeaveWindow)
   EVT_KEY_DOWN(HexCanvas::OnKeyDown)
   EVT_CHAR_HOOK(HexCanvas::OnKeyDown)
   EVT_SIZE(HexCanvas::OnSize)
   EVT_MOUSEWHEEL(HexCanvas::OnMouseWheel)
   //EVT_SCROLLWIN(HexCanvas::OnScroll)
   EVT_SET_FOCUS(HexCanvas::OnSetFocus)
   EVT_KILL_FOCUS(HexCanvas::OnKillFocus)
END_EVENT_TABLE()

HexCanvas::HexCanvas(wxWindow *parent, HexView *view)
:  wxScrolledWindow(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxVSCROLL | wxHSCROLL | wxALWAYS_SHOW_SB | wxWANTS_CHARS),
   m_view(view) {
   wxSize screen = wxGetDisplaySize();
   m_buffer.reset(new wxBitmap(screen.x, screen.y));

   wxCaret *caret = new wxCaret(this, 2, 16);
   caret->Show();
   SetCaret(caret);
}

HexCanvas::~HexCanvas() {}

void HexCanvas::OnPaint(wxPaintEvent &WXUNUSED(event)) {
   wxBufferedPaintDC bdc(this, *m_buffer);
   if (m_view) {
      m_view->OnDraw(&bdc);
   }
}

void HexCanvas::OnSize(wxSizeEvent &event) {
   if (m_view) {
      m_view->OnSize(event);
   }
   Refresh();
   Update();
}

// Anti-flicker
void HexCanvas::OnEraseBackground(wxEraseEvent &WXUNUSED(event)) {}

void HexCanvas::OnLeftDown(wxMouseEvent &event) {
   SetFocus();
   if (m_view) {
      m_view->OnLeftDown(event);
   }
}

void HexCanvas::OnLeftUp(wxMouseEvent &event) {
   if (m_view) {
      m_view->OnLeftUp(event);
   }
}

void HexCanvas::OnMotion(wxMouseEvent &event) {
   if (m_view) {
      m_view->OnMotion(event);
   }
}

void HexCanvas::OnLeaveWindow(wxMouseEvent &event) {
   if (m_view) {
      m_view->OnLeaveWindow(event);
   }
}

void HexCanvas::OnMouseWheel(wxMouseEvent &event) {
   if (event.GetWheelAxis() == 0 && m_view) {
      int lines = event.GetLinesPerAction();
      int count = event.GetWheelRotation() / event.GetWheelDelta();
      count *= lines;
      
      m_view->DoMouseWheel(count);
   }
}

void HexCanvas::OnKeyDown(wxKeyEvent &event) {
   if (event.GetKeyCode() == WXK_TAB && event.ControlDown()) {
      event.Skip();
      return;
   }

   if (m_view) {
      m_view->OnKeyDown(event);
   }
}

void HexCanvas::OnSetFocus(wxFocusEvent &event) {
   if (m_view) {
      m_view->OnSetFocus(event);
   }
}

void HexCanvas::OnKillFocus(wxFocusEvent &event) {
   if (m_view) {
      m_view->OnKillFocus(event);
   }
}

BEGIN_EVENT_TABLE(HexFrame, wxAuiMDIChildFrame)
END_EVENT_TABLE()

HexFrame::HexFrame(HexView *vw, wxAuiMDIParentFrame *parent, wxWindowID id, const wxString &title):
wxAuiMDIChildFrame(parent, id, title, wxDefaultPosition, wxDefaultSize, wxDEFAULT_FRAME_STYLE),
m_view(vw)
{
   wxLogDebug("[HexFrame] created at 0x%llX.", reinterpret_cast<uint64_t>(this));

   m_canvas = new HexCanvas(this, vw);
   wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);
   sizer->Add(m_canvas, 1, wxEXPAND);
   SetSizer(sizer);

   m_canvas->Bind(wxEVT_SCROLLWIN_TOP, &HexView::OnScroll, m_view);
   m_canvas->Bind(wxEVT_SCROLLWIN_BOTTOM, &HexView::OnScroll, m_view);
   m_canvas->Bind(wxEVT_SCROLLWIN_LINEUP, &HexView::OnScroll, m_view);
   m_canvas->Bind(wxEVT_SCROLLWIN_LINEDOWN, &HexView::OnScroll, m_view);
   m_canvas->Bind(wxEVT_SCROLLWIN_PAGEUP, &HexView::OnScroll, m_view);
   m_canvas->Bind(wxEVT_SCROLLWIN_PAGEDOWN, &HexView::OnScroll, m_view);
   m_canvas->Bind(wxEVT_SCROLLWIN_THUMBTRACK, &HexView::OnScroll, m_view);
   m_canvas->Bind(wxEVT_SCROLLWIN_THUMBRELEASE, &HexView::OnScroll, m_view);

  
   m_view->SetFrame(this);
}

HexFrame::~HexFrame()
{
   wxLogDebug("[HexFrame] at 0x%llX destroyed.", reinterpret_cast<uint64_t>(this));
}

void HexFrame::OnSize(wxSizeEvent &event)
{
   wxLogDebug("<<Flow>> HwxFrame::OnSize");

   m_view->OnSize(event);
   Refresh(), Update();
}

void HexFrame::OnEraseBackground(wxEraseEvent &WXUNUSED(event))
{
   // do nothing, prevents flickering
}

void HexFrame::OnLeftDown (wxMouseEvent &event)
{
   SetFocus();
   m_view->OnLeftDown(event);
}

void HexFrame::OnLeftUp (wxMouseEvent &event)
{
   m_view->OnLeftUp(event);
}

void HexFrame::OnMouseWheel (wxMouseEvent &event)
{
   // y axis
   if (event.GetWheelAxis() == 0)
   {
      /* lines - the number of lines per wheel rotation configured in the operational system
         delta - a value which represents a whole rotation
         rotation - number of times the wheel rotated, expressed in number os deltas */
      int lines = event.GetLinesPerAction();
      int count = event.GetWheelRotation() / event.GetWheelDelta();
      count *= lines;

      wxLogStatus("Mouse Wheel rotation: %d", count);
      m_view->DoMouseWheel(count);
   }
}

void HexFrame::OnScroll (wxScrollWinEvent &event)
{
   wxLogDebug("<<Flow>> HwxFrame::OnScroll");

   m_view->OnScroll(event);
}

void HexFrame::OnMotion (wxMouseEvent &event)
{
   m_view->OnMotion(event);
}

void HexFrame::OnLeaveWindow (wxMouseEvent &event)
{
   m_view->OnLeaveWindow(event);
}

/* Processa a entrada do teclado quando o frame est� com o foco.
   O evento EVT_CHAR_HOOK � direcionado para c� porque sen�o outros Event Handlers
   na hierarquia acabam monopolizando algumas teclas que s�o usadas na navega��o do HexView.
*/
void HexFrame::OnKeyDown (wxKeyEvent &event)
{
   switch (event.GetKeyCode())
   {
      case WXK_TAB:
         if (event.ControlDown())
            break;

      default:
         m_view->OnKeyDown(event);
         return;
   }

   event.Skip();
}

void HexFrame::OnPaint(wxPaintEvent &WXUNUSED(event))
{
   wxBufferedPaintDC bdc(this, *m_buffer);
   m_view->OnDraw(&bdc);
}

void HexFrame::OnSetFocus(wxFocusEvent &event)
{
   wxLogStatus("[HexFrame] ::OnSetFocus event");

   if (m_view)
      m_view->OnSetFocus(event);

}

void HexFrame::OnKillFocus (wxFocusEvent &event)
{
   wxLogStatus("[HexFrame] ::OnKillFocus event");

   if (m_view)
      m_view->OnKillFocus(event);
}

/*#ifndef NDEBUG
#include "constants.hpp"
#include "debug_panel.hpp"

void HexFrame::OnAddToDebugPanel ()
{
   auto *dbgPanel = dynamic_cast<DebugPanel *>(FindWindowById(HT_DebugPanel));
   GetView()->AddToDebugPanel(dbgPanel);
}

void HexFrame::OnRemoveFromDebugPanel ()
{
   auto *dbgPanel = dynamic_cast<DebugPanel *>(FindWindowById(HT_DebugPanel));

   if (dbgPanel)
      GetView()->RemoveFromDebugPanel(dbgPanel);
}
#endif*/