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

#ifndef HT_CONSTANTS_HPP
#define HT_CONSTANTS_HPP

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#define HT_VERSION wxT("0.1")
#define HT_CONFIG_FILE wxT("config.xml")
#define HT_BUILD wxT(__DATE__) wxT(" ") wxT(__TIME__)

enum
{
   // menu items with predefined IDs
   HT_FileNew = wxID_NEW,
   HT_FileOpen = wxID_OPEN,
   HT_FileClose = wxID_CLOSE,
   HT_FileCloseAll = wxID_CLOSE_ALL,
   HT_FileSave = wxID_SAVE,
   HT_FileSaveAs = wxID_SAVEAS,
   HT_FileExit = wxID_EXIT,
   HT_EditUndo = wxID_UNDO,
   HT_EditRedo = wxID_REDO,
   HT_EditCut = wxID_CUT,
   HT_EditCopy = wxID_COPY,
   HT_EditPaste = wxID_PASTE,
   HT_EditSelectAll = wxID_SELECTALL,
   HT_HelpAbout = wxID_ABOUT,

   // other menu items
   HT_FileSaveAll = wxID_HIGHEST + 1,
   HT_EditCopyAsHexText,
   HT_EditCopyAsCSource,
   HT_EditFindForw,
   HT_EditFindBack,
   HT_ViewStatusBar,
   HT_ViewAddTable,
   HT_ToolsTblManager,
   HT_ToolsLogger,

   // toolbars
   HT_Toolbar_Main,
   HT_Toolbar_View,
   HT_Toolbar_Navigation,

   // toolbar controls
   HT_Tool_TableList,
   HT_Tool_SearchBox,
   HT_Tool_JumpHex,
   HT_Tool_JumpDec,

#ifdef WXDEBUG
   // debug panel
   HT_DebugPanel,
   HT_DebugPanel_List,
   HT_DebugPanel_UpdateTimer,
   HT_ViewDebugPanel,
#endif

   // recent files history (must be the last item here)
   HT_FileHistory_Base,
};

/*----------------------------------------------------------*
 * HtArtProvider Art Ids                                    *
 *----------------------------------------------------------*/
#define HTArtId_MainTb  wxART_MAKE_ART_ID(HTArtId_MainTb)
enum
{
   HTArt_New,
   HTArt_Open,
   HTArt_Save,
   HTArt_SaveGr,
   HTArt_SaveAs,
   HTArt_SaveAll,
   HTArt_Cut,
   HTArt_CutGr,
   HTArt_Copy,
   HTArt_CopyGr,
   HTArt_Paste,
   HTArt_PasteGr,
   HTArt_Undo,
   HTArt_UndoGr,
   HTArt_Redo,
   HTArt_RedoGr
};

#define HTArtId_NavTb  wxART_MAKE_ART_ID(HTArtId_NavTb)
enum
{
   HTArt_SearchBack,
   HTArt_SearchForw
};

#define HTArtId_ViewTb  wxART_MAKE_ART_ID(HTArtId_ViewTb)
enum
{
   HTArt_AddTable
};

#define HTArtId_Common   wxART_MAKE_ART_ID(HTArtId_Icons)
enum
{
   HTArt_Close,
   HTArt_Exit,
   HTArt_IncreaseFont,
   HTArt_DecreaseFont,
   HTArt_AutoAdjust,
   HTArt_OffsetMode,
   HTArt_AllowChanges,
   HTArt_Search,
   HTArt_Highlight,
   HTArt_ClearHighlight,
   HTArt_MarkPos,
   HTArt_GoToMark,
   HTArt_Properties,
   HTArt_CopyAsHex,
   HTArt_CopyAsCSrc,
   HTArt_PasteAsASCII,
   HTArt_PasteAsUnicode,
   HTArt_PasteAsEBCDIC,
   HTArt_WriteSelection,
   HTArt_ReadFile,
   HTArt_Help,
   HTArt_ContextHelp,
   HTArt_About,
   HTArt_Options
};

#endif //~HT_CONSTANTS_HPP