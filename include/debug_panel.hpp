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

#ifdef HT_DEBUG_PANEL_ENABLED

#include <wx/wxprec.h>

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <wx/listctrl.h>

#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <type_traits>
#include <utility>

class DebugPanel : public wxPanel {
public:
   using Getter = std::function<wxString()>;

   struct Entry {
      Getter get;
      wxString last;
      std::uint64_t id = 0;
   };

   using WatchMap = std::map<wxString, Entry>;

   class Watch {
   public:
      Watch() = default;

      Watch(std::weak_ptr<WatchMap> map, wxString name, std::uint64_t id)
          : m_map(std::move(map)), m_name(std::move(name)), m_id(id) {}

      Watch(Watch &&other) noexcept
          : m_map(std::exchange(other.m_map, {})), m_name(std::move(other.m_name)), m_id(std::exchange(other.m_id, 0)) {
      }

      Watch &operator=(Watch &&other) noexcept {
         if (this != &other) {
            release();

            m_map = std::exchange(other.m_map, {});
            m_name = std::move(other.m_name);
            m_id = std::exchange(other.m_id, 0);
         }

         return *this;
      }

      Watch(const Watch &) = delete;
      Watch &operator=(const Watch &) = delete;

      ~Watch() { release(); }

   private:
      void release() {
         if (auto map = m_map.lock()) {
            auto it = map->find(m_name);

            if (it != map->end() && it->second.id == m_id) {
               map->erase(it);
            }
         }

         m_map.reset();
      }

      std::weak_ptr<WatchMap> m_map;
      wxString m_name;
      std::uint64_t m_id = 0;
   };

   DebugPanel(wxWindow *parent, wxWindowID id);
   ~DebugPanel() override;

   [[nodiscard]] Watch AddWatch(const wxString &name, Getter getter);

   template <typename T>
   [[nodiscard]] Watch AddWatchRef(const wxString &name, const T *var) {
      return AddWatch(name, [var] { return ToString(*var); });
   }

   void StartUpdateTimer(int intervalMs = 100);
   void StopUpdateTimer();

   template <typename T>
   static wxString ToString(const T &v) {
      using U = std::remove_cv_t<T>;

      if constexpr (std::is_same_v<U, bool>) {
         return v ? "true" : "false";
      }
      else if constexpr (std::is_integral_v<U> && std::is_signed_v<U>) {
         return wxString::Format("%lld", static_cast<long long>(v));
      }
      else if constexpr (std::is_integral_v<U>) {
         return wxString::Format("%llu", static_cast<unsigned long long>(v));
      }
      else if constexpr (std::is_same_v<U, wxPoint>) {
         return wxString::Format("(%d,%d)", v.x, v.y);
      }
      else if constexpr (std::is_same_v<U, wxSize>) {
         return wxString::Format("(%dx%d)", v.GetWidth(), v.GetHeight());
      }
      else if constexpr (std::is_same_v<U, wxRect>) {
         return wxString::Format("(%d,%d,%d,%d)", v.x, v.y, v.width, v.height);
      }
      else {
         static_assert(!sizeof(U), "DebugPanel::ToString: type not supported");
      }
   }

private:
   void OnUpdateTimer(wxTimerEvent &event);
   void updateList();

   wxListView *m_list = nullptr;
   std::shared_ptr<WatchMap> m_watches;
   std::unique_ptr<wxTimer> m_timer;
   std::uint64_t m_nextId = 1;
};

#endif  // HT_DEBUG_PAMEL_ENABLED

#endif  // HT_DEBUGPANEL_HPP