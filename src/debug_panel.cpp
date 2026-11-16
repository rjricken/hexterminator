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

#ifdef HT_DEBUG_PANEL_ENABLED

#include "debug_panel.hpp"
#include "constants.hpp"

DebugPanel::DebugPanel(wxWindow *parent, wxWindowID id)
    : wxPanel(parent, id, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL | wxBORDER_NONE),
      m_watches(std::make_shared<WatchMap>()) {
   m_list = new wxListView(this, HT_DebugPanel_List, wxDefaultPosition, wxDefaultSize, wxLC_REPORT | wxLC_SINGLE_SEL);
   m_list->InsertColumn(0, "Property", wxLIST_FORMAT_LEFT, 200);
   m_list->InsertColumn(1, "Value", wxLIST_FORMAT_LEFT, 130);

   auto *sizer = new wxBoxSizer(wxVERTICAL);
   sizer->Add(m_list, wxSizerFlags(1).Border(wxALL, 5).Expand());
   SetSizer(sizer);

   m_timer = std::make_unique<wxTimer>(this, HT_DebugPanel_UpdateTimer);
   Bind(wxEVT_TIMER, &DebugPanel::OnUpdateTimer, this, HT_DebugPanel_UpdateTimer);
   StartUpdateTimer();
}

DebugPanel::~DebugPanel() {
   StopUpdateTimer();
}

DebugPanel::Watch DebugPanel::AddWatch(const wxString &name, Getter getter) {
   const std::uint64_t id = m_nextId++;

   m_watches->insert_or_assign(name, Entry {std::move(getter), wxString(), id});

   return Watch(m_watches, name, id);
}

void DebugPanel::StartUpdateTimer(int intervalMs) {
   if (m_timer) {
      m_timer->Start(intervalMs);
   }
}

void DebugPanel::StopUpdateTimer() {
   if (m_timer) {
      m_timer->Stop();
   }
}

void DebugPanel::OnUpdateTimer(wxTimerEvent &WXUNUSED(event)) {
   updateList();
}

void DebugPanel::updateList() {
   bool rebuild = m_list->GetItemCount() != static_cast<long>(m_watches->size());

   if (!rebuild) {
      long row = 0;
      for (const auto &kv : *m_watches) {
         if (m_list->GetItemText(row++) != kv.first) {
            rebuild = true;
            break;
         }
      }
   }

   if (rebuild) {
      m_list->DeleteAllItems();

      long row = 0;
      for (auto &kv : *m_watches) {
         m_list->InsertItem(row++, kv.first);
         kv.second.last.clear();
      }
   }

   long row = 0;
   for (auto &kv : *m_watches) {
      Entry &entry = kv.second;
      wxString now = entry.get();

      if (rebuild || now != entry.last) {
         m_list->SetItem(row, 1, now);
         entry.last = std::move(now);
      }

      ++row;
   }
}

#endif