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

#ifndef HT_HEXDOC_HPP
#define HT_HEXDOC_HPP

#include <wx/wxprec.h>

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <wx/docview.h>
#include <wx/file.h>

class HexDoc : public wxDocument
{
public:
   HexDoc ();
   ~HexDoc ();

   wxFileOffset length () {
      wxASSERT(m_file.IsOpened());
      return m_file.Length();
   }

   size_t GetData (unsigned char *buf, size_t len, wxFileOffset address = 0)
   {
      wxASSERT(address >= 0);

      m_file.Seek(address);
      return m_file.Read(buf, len);
   }

   virtual bool OnOpenDocument (const wxString &filename);
   virtual bool OnCreate (const wxString &path, long flags = 0);

private:
   //wxString m_filepath;
   wxFile m_file;

   DECLARE_DYNAMIC_CLASS(HexDoc)
};

#endif //~HT_HEXDOC_HPP