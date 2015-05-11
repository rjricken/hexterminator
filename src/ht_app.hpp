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

#ifndef HT_APP_HPP
#define HT_APP_HPP

#include <wx/wxprec.h>
 
#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <memory>
#include <wx/docview.h>
#include "ht_artprovider.hpp"
#include "ht_prefs.hpp"

/**
* Starts HEXterminator execution and bring up its user interface.
* @author Ricardo J. Ricken
*/
class HtApp : public wxApp
{
public:
   virtual ~HtApp ();

   /**
   * This method is called during program initialization. It chooses the
   * correct language based on the current operational system's language,
   * picking it from a catalog, when available. It also creates and shows
   * the main window frame.
   * @return True if initializated with success, false otherwise.
   */
   virtual bool OnInit();

   /**
   * This method is called when the program is ending its execution. It
   * basically saves the user preferences to a configuration file.
   * @return Exit code
   */
   virtual int OnExit();

   std::unique_ptr <wxDocManager> doc_manager;   /**< document manager           */
   HtArtProvider art_provider;                   /**< app-wide art provider      */
   HtPrefs prefs;                                /**< app-wide preferences       */

private:
   wxLocale loc;                                 /**< defines our current locale */
};

DECLARE_APP(HtApp);


#endif //~HT_APP_HPP
