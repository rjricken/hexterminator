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

#include "debug_panel.hpp"
#include "constants.hpp"


BEGIN_EVENT_TABLE(DebugPanel, wxPanel)
   EVT_TIMER(HT_DebugPanel_UpdateTimer, DebugPanel::OnUpdateTimer)
END_EVENT_TABLE()


DebugPanel::DebugPanel (wxWindow *parent, wxWindowID id) :
wxPanel(parent, id, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL | wxBORDER_NONE)
{
   wxLogDebug("[DebugPanel] created at 0x%X.", reinterpret_cast<int>(this));

   /*wxStaticBoxSizer *resultbox_sz = new wxStaticBoxSizer(new wxStaticBox(this, wxID_ANY, _("Results")), wxVERTICAL);
   wxListCtrl *results = new wxListCtrl(this, MonkeyMoore_Results, wxDefaultPosition, wxDefaultSize, wxLC_REPORT | wxLC_SINGLE_SEL);
   wxCheckBox *show_all = new wxCheckBox(this, MonkeyMoore_AllResults, _(" Show repeated results"));
   resultbox_sz->Add(show_all, wxSizerFlags().Left().Border(wxLEFT, 5));
   resultbox_sz->Add(results, wxSizerFlags(1).Top().Border(wxALL, 4).Expand());
   resultbox_sz->Add(resultopt_sz, wxSizerFlags().Border(wxALL, 2).Expand());

   results->InsertColumn(0, _("Offset"));
   results->InsertColumn(1, _("Values"), wxLIST_FORMAT_LEFT, 90);
   results->InsertColumn(2, _("Preview"), wxLIST_FORMAT_LEFT, 200);*/

   wxBoxSizer *panelSz = new wxBoxSizer(wxVERTICAL);

   wxListView *list = new wxListView(this, HT_DebugPanel_List, wxDefaultPosition, wxDefaultSize, wxLC_REPORT | wxLC_SINGLE_SEL);
   list->InsertColumn(0, "Property", wxLIST_FORMAT_LEFT, 120);
   list->InsertColumn(1, "Value", wxLIST_AUTOSIZE, 100);

   panelSz->Add(list, wxSizerFlags(1).Border(wxALL, 5).Expand());
   SetSizer(panelSz);

   updateTimer.reset(new wxTimer(GetEventHandler(), HT_DebugPanel_UpdateTimer));
   updateTimer->Start(100);
}

DebugPanel::~DebugPanel ()
{
   wxLogDebug("[DebugPanel] at 0x%X destroyed.", reinterpret_cast<int>(this));
}

bool DebugPanel::AddProperty (wxString name, int type, void *dataPtr)
{
   auto dup = props.find(name);

   // se já existir um elemento com o nome especificado, substitui
   if (dup != props.end())
      dup->second = std::make_pair(dataPtr, type);
   else
   {
      auto ret = props.insert(std::make_pair(name, std::make_pair(dataPtr, type)));
      return ret.second;
   }

   return true;
}

bool DebugPanel::RemoveProperty (wxString name)
{
   auto count = props.erase(name);
   return count ? true : false;
}

void DebugPanel::updatePropertyList ()
{
   wxListView *list = dynamic_cast <wxListView *>(FindWindowById(HT_DebugPanel_List));

   list->DeleteAllItems();

   if (!props.empty())
   {
      auto itemNum = 0;
      for (auto i = props.begin(); i != props.end(); ++i, ++itemNum)
      {
         list->InsertItem(itemNum, i->first);

         auto valuePair = i->second;
         wxString valueStr;

         switch (valuePair.second)
         {
            case Type_Bool:
               valueStr = *static_cast <bool *>(valuePair.first) ? "true" : "false";
               break;

            case Type_Int:
               valueStr = wxString::Format("%d", *static_cast<int *>(valuePair.first));
               break;

            case Type_Long:
               valueStr = wxString::Format("%d", *static_cast<long *>(valuePair.first));
               break;

            case Type_wxPoint: {
               wxPoint *valuePtr = static_cast<wxPoint *>(valuePair.first);
               valueStr = wxString::Format("(%d,%d)", valuePtr->x, valuePtr->y);
               break;
            }

            case Type_wxSize: {
               wxSize *valuePtr = static_cast<wxSize *>(valuePair.first);
               valueStr = wxString::Format("(%dx%d)", valuePtr->x, valuePtr->y);
               break;
            }

            case Type_wxRect: {
               wxRect *valuePtr = static_cast<wxRect *>(valuePair.first);
               valueStr = wxString::Format(
                  "(%d,%d,%d,%d)", valuePtr->x, valuePtr->y, valuePtr->width, valuePtr->height
               );
               break;
            }

            default:
               valueStr = "<type not supported>";
         }

         list->SetItem(itemNum, 1, valueStr);
      }
   }
}
