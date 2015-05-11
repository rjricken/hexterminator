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

#ifndef HT_DEBUGPANEL_HPP
#define HT_DEBUGPANEL_HPP

#include <wx/wxprec.h>
 
#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <wx/listctrl.h>
#include <memory>
#include <map>

class DebugPanel : public wxPanel
{
public:
   DebugPanel (wxWindow *parent, wxWindowID id);

   virtual ~DebugPanel ();

   bool AddProperty (wxString name, int type, void *dataPtr);
   bool RemoveProperty (wxString name);

   void StopUpdateTimer () { updateTimer->Stop(); }

   void OnUpdateTimer (wxTimerEvent &event) { updatePropertyList(); }

   enum {
      Type_Int,
      Type_Long,
      Type_Bool,
      Type_wxSize,
      Type_wxPoint,
      Type_wxRect
   };


private:
   void updatePropertyList();

   std::unique_ptr<wxTimer> updateTimer;
   std::map<wxString, std::pair<void *, int>> props;

   DECLARE_EVENT_TABLE()
};

#endif //~HT_DEBUGPANEL_HPP