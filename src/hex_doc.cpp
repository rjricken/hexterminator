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

#include "hex_doc.hpp"

IMPLEMENT_DYNAMIC_CLASS(HexDoc, wxDocument)

HexDoc::HexDoc ()
{
   wxLogDebug("[HexDoc] created at 0x%X.", reinterpret_cast<int>(this));

}

HexDoc::~HexDoc ()
{
   wxLogDebug("[HexDoc] at 0x%X destroyed.", reinterpret_cast<int>(this));

}

bool HexDoc::OnCreate (const wxString &path, long flags)
{
   wxLogDebug("[HexDoc] ::OnCreate [%s]", path);
   return wxDocument::OnCreate(path, flags);
}

bool HexDoc::OnOpenDocument (const wxString &filename)
{
   if (!wxFile::Access(filename, wxFile::read))
      return false;

   if (m_file.Open(filename, wxFile::read) && m_file.IsOpened())
   {
      //m_filepath = filename;
      return true;
   }

   return false;
}