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

BEGIN_EVENT_TABLE(HexFrame, wxAuiMDIChildFrame)
   EVT_PAINT(HexFrame::OnPaint)
   EVT_ERASE_BACKGROUND(HexFrame::OnEraseBackground)
   EVT_LEFT_DOWN(HexFrame::OnLeftDown)
   EVT_LEFT_UP(HexFrame::OnLeftUp)
   EVT_MOTION(HexFrame::OnMotion)
   EVT_LEAVE_WINDOW(HexFrame::OnLeaveWindow)
   EVT_KEY_DOWN(HexFrame::OnKeyDown)
   EVT_CHAR_HOOK(HexFrame::OnKeyDown)
   EVT_SIZE(HexFrame::OnSize)
   EVT_MOUSEWHEEL(HexFrame::OnMouseWheel)
   EVT_SCROLLWIN(HexFrame::OnScroll)
   EVT_SET_FOCUS(HexFrame::OnSetFocus)
   EVT_KILL_FOCUS(HexFrame::OnKillFocus)
END_EVENT_TABLE()

HexFrame::HexFrame(HexView *vw, wxAuiMDIParentFrame *parent, wxWindowID id, const wxString &title):
wxAuiMDIChildFrame(parent, id, title, wxDefaultPosition, wxDefaultSize, wxDEFAULT_FRAME_STYLE | wxVSCROLL | wxHSCROLL | wxALWAYS_SHOW_SB),
m_view(vw)
{
   wxLogDebug("[HexFrame] created at 0x%X.", reinterpret_cast<int>(this));

   wxSize screen = wxGetDisplaySize();
   m_buffer.reset(new wxBitmap(screen.x, screen.y));

    // -- Caret Creation
   wxCaret *caret = new wxCaret(this, 2, 16);
   caret->Show();

   SetCaret(caret);
   m_view->SetFrame(this);
}

HexFrame::~HexFrame()
{
   wxLogDebug("[HexFrame] at 0x%X destroyed.", reinterpret_cast<int>(this));
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

/* Processa a entrada do teclado quando o frame está com o foco.
   O evento EVT_CHAR_HOOK é direcionado para cá porque senão outros Event Handlers
   na hierarquia acabam monopolizando algumas teclas que são usadas na navegação do HexView.
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

#ifdef WXDEBUG
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
#endif