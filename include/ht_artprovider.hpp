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

#ifndef HT_ARTPROVIDER_HPP
#define HT_ARTPROVIDER_HPP

#include <wx/wxprec.h>
 
#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <memory>
#include <wx/imaglist.h>
#include <map>

/**
* Provides runtime art to the application. The images are loaded
* dynamically and can be build up from tilesets.
* @author Ricardo J. Ricken
*/
class HtArtProvider
{
public:
   HtArtProvider () {}
   ~HtArtProvider () {}

   /**
   * Adds a new image to the art provider. The image can
   * be split in sub-images if specified.
   * @param fname file named
   * @param id identifier for this image/image set
   * @param mime image MIME-type
   * @param tiles specifies a tiled image
   * @param tile size (width and height)
   * @return True on success, false on failure.
   */
   bool Add (const wxString &fname, const wxString &id, const wxString &mime, bool tiles = false, wxSize tile_sz = wxDefaultSize)
   {
      wxImage img(fname, mime);
      if (!img.IsOk()) return false;

      if (tiles)
      {
         const int num_images = img.GetWidth() / tile_sz.x;
         std::shared_ptr <wxImageList> imglist(new wxImageList(tile_sz.x, tile_sz.y, true, num_images));

         for (int i = 0; i < num_images; i++)
            imglist->Add(img.GetSubImage(wxRect(i * tile_sz.x, 0, tile_sz.x, tile_sz.y)));

         art_set[id] = imglist;
      }
      else
         art_other[id] = img;

      return true;
   }

   /**
   * Gets a bitmap from the art provider.
   * @param id image/image set identifier
   * @param index tile index (valid only for image sets)
   * @return The specified bitmap if found, wxNullBitmap otherwise.
   */
   wxBitmap GetBitmap (const wxString &id, int index = -1)
   {
      if (index >= 0 && art_set.count(id))
      {
         std::shared_ptr <wxImageList> list = art_set[id];
         wxASSERT(index < list->GetImageCount());

         return list->GetBitmap(index);
      }
      else if (art_other.count(id))
         return wxBitmap(art_other[id]);

      return wxNullBitmap;
   }

private:
   std::map <wxString, std::shared_ptr <wxImageList>> art_set;   /**< image sets */
   std::map <wxString, wxImage> art_other;                       /**< other images */
};

#endif //~HT_ARTPROVIDER_HPP