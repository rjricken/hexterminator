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

#include "ht_mainframe.hpp"
#include "constants.hpp"
#include "hex_doc.hpp"
#include "ht_app.hpp"
#include "ht_about.hpp"
#include "bookmarks_panel.hpp"
#include "debug_panel.hpp"

#include <wx/artprov.h>
#include <wx/spinctrl.h>
#include <wx/combo.h>

BEGIN_EVENT_TABLE(HtMainFrame, wxAuiMDIParentFrame)
   EVT_MENU(HT_FileExit, HtMainFrame::OnFileExit)
   EVT_MENU(HT_FileOpen, HtMainFrame::OnFileOpen)
   EVT_MENU(HT_ViewStatusBar, HtMainFrame::OnViewStatusBar)
   EVT_MENU(HT_ToolsLogger, HtMainFrame::OnToolsLog)
   EVT_MENU(HT_HelpAbout, HtMainFrame::OnHelpAbout)
   EVT_ERASE_BACKGROUND(HtMainFrame::OnEraseBackground)
   EVT_CLOSE(HtMainFrame::OnClose)
   EVT_UPDATE_UI(HT_FileSave, HtMainFrame::OnUpdateUI)
   EVT_UPDATE_UI(HT_FileClose, HtMainFrame::OnUpdateUI)
   EVT_UPDATE_UI(HT_EditCut, HtMainFrame::OnUpdateUI)
   EVT_UPDATE_UI(HT_EditCopy, HtMainFrame::OnUpdateUI)
   EVT_UPDATE_UI(HT_EditPaste, HtMainFrame::OnUpdateUI)
   EVT_UPDATE_UI(HT_Tool_JumpHex, HtMainFrame::OnUpdateUI)
   EVT_UPDATE_UI(HT_Tool_JumpDec, HtMainFrame::OnUpdateUI)
   EVT_UPDATE_UI(HT_Tool_SearchBox, HtMainFrame::OnUpdateUI)
   EVT_AUINOTEBOOK_PAGE_CLOSE(wxID_ANY, HtMainFrame::OnNotebookPageClose)
   EVT_AUITOOLBAR_TOOL_DROPDOWN(HT_EditUndo, HtMainFrame::OnEditUndoDropDown)
   EVT_AUITOOLBAR_TOOL_DROPDOWN(HT_FileOpen, HtMainFrame::OnFileOpenDropDown)

   EVT_KEY_DOWN(HtMainFrame::OnKeyDown)
   EVT_CHAR_HOOK(HtMainFrame::OnKeyDown)
END_EVENT_TABLE()

#if !defined(__WXMSW__) && !defined(__WXPM__)
#include "../resources/hterminator.xpm"
#endif

HtMainFrame::HtMainFrame (const wxString &title, const wxPoint &pos, const wxSize &size, long flags) :
   wxAuiMDIParentFrame(0, wxID_ANY, title, pos, size, flags | wxBORDER_NONE | wxFRAME_NO_WINDOW_MENU),
   history(9, HT_FileHistory_Base)
{
   // logger
   logger = new wxLogWindow(this, wxT("Logger"), false);
   wxLog::SetActiveTarget(logger);

   aui_manager.SetManagedWindow(this);
   SetIcon(wxICON(hterminator));

   HtArtProvider &art = wxGetApp().art_provider;
   art.Add(wxT("images/common.png"), HTArtId_Common, wxT("image/png"), true, wxSize(16, 15));
   art.Add(wxT("images/main_tb.png"), HTArtId_MainTb, wxT("image/png"), true, wxSize(16, 16));
   art.Add(wxT("images/view_tb.png"), HTArtId_ViewTb, wxT("image/png"), true, wxSize(16, 15));
   art.Add(wxT("images/nav_tb.png"), HTArtId_NavTb, wxT("image/png"), true, wxSize(16, 15));

#if wxUSE_MENUS
   CreateMenu(art);
#endif //~wxUSE_MENUS

   BookmarksPanel *bookmarks = new BookmarksPanel(this, wxID_ANY);

   wxAuiToolBar *tb_main = new wxAuiToolBar(this, HT_Toolbar_Main/*, wxDefaultPosition, wxDefaultSize, wxAUI_TB_DEFAULT_STYLE | wxAUI_TB_OVERFLOW*/);
   tb_main->SetToolBitmapSize(wxSize(16, 15));
   tb_main->AddTool(HT_FileNew, _("New"), art.GetBitmap(HTArtId_MainTb, HTArt_New), _("New (Ctrl+N)"));
   tb_main->AddTool(HT_FileOpen, _("Open"), art.GetBitmap(HTArtId_MainTb, HTArt_Open), _("Open (Ctrl+O)"));
   tb_main->AddTool(HT_FileSave, _("Save"), art.GetBitmap(HTArtId_MainTb, HTArt_Save), _("Save (Ctrl+S)"))->SetDisabledBitmap(art.GetBitmap(HTArtId_MainTb, HTArt_SaveGr));
   tb_main->AddSeparator();
   tb_main->AddTool(HT_EditCut, _("Cut"), art.GetBitmap(HTArtId_MainTb, HTArt_Cut), _("Cut (Ctrl+X)"))->SetDisabledBitmap(art.GetBitmap(HTArtId_MainTb, HTArt_CutGr));
   tb_main->AddTool(HT_EditCopy, _("Copy"), art.GetBitmap(HTArtId_MainTb, HTArt_Copy), _("Copy (Ctrl+C)"))->SetDisabledBitmap(art.GetBitmap(HTArtId_MainTb, HTArt_CopyGr));
   tb_main->AddTool(HT_EditPaste, _("Paste"), art.GetBitmap(HTArtId_MainTb, HTArt_Paste), _("Paste (Ctrl+V)"))->SetDisabledBitmap(art.GetBitmap(HTArtId_MainTb, HTArt_PasteGr));
   tb_main->AddSeparator();
   tb_main->AddTool(HT_EditUndo, _("Undo"), art.GetBitmap(HTArtId_MainTb, HTArt_Undo), _("Undo (Ctrl+Z)"))->SetDisabledBitmap(art.GetBitmap(HTArtId_MainTb, HTArt_UndoGr));
   tb_main->AddTool(HT_EditRedo, _("Redo"), art.GetBitmap(HTArtId_MainTb, HTArt_Redo), _("Redo (Ctrl+Y)"))->SetDisabledBitmap(art.GetBitmap(HTArtId_MainTb, HTArt_RedoGr));
   tb_main->SetToolDropDown(HT_EditUndo, true);
   tb_main->SetToolDropDown(HT_FileOpen, true);
   tb_main->Realize();

   wxAuiToolBar *tb_view = new wxAuiToolBar(this, HT_Toolbar_View/*, wxDefaultPosition, wxDefaultSize, wxAUI_TB_DEFAULT_STYLE | wxAUI_TB_OVERFLOW*/);
   wxChoice *tablelist = new wxChoice(tb_view, HT_Tool_TableList);
   tb_view->AddControl(tablelist, _("Table"));
   tb_view->AddTool(HT_ViewAddTable, _("Add Table"), art.GetBitmap(HTArtId_ViewTb, HTArt_AddTable), _("Add table"));

   tablelist->Append(wxT("ASCII default"));
   tablelist->SetSelection(0);

   wxAuiToolBar *tb_nav = new wxAuiToolBar(this, HT_Toolbar_Navigation/*, wxDefaultPosition, wxDefaultSize, wxAUI_TB_DEFAULT_STYLE | wxAUI_TB_OVERFLOW*/);
   tb_nav->SetToolBitmapSize(wxSize(16, 15));
   tb_nav->AddTool(HT_EditFindForw, _("Find backward"), art.GetBitmap(HTArtId_NavTb, HTArt_SearchBack), _("Find Backward"));
   tb_nav->AddControl(new wxComboBox(tb_nav, HT_Tool_SearchBox, wxT(""), wxDefaultPosition, wxSize(200, 20)));
   tb_nav->AddTool(HT_EditFindBack, _("Find forward"), art.GetBitmap(HTArtId_NavTb, HTArt_SearchForw), _("Find Forward"));
   wxComboBox *jump_hex = new wxComboBox(tb_nav, HT_Tool_JumpHex, wxT(""), wxDefaultPosition, wxSize(115, 20));
   jump_hex->SetForegroundColour(wxColor(128, 0, 0));
   tb_nav->AddControl(jump_hex, _("Jump hex"));
   wxComboBox *jump_dec = new wxComboBox(tb_nav, HT_Tool_JumpDec, wxT(""), wxDefaultPosition, wxSize(140, 20));
   //jump_dec->SetFont(wxFont(10, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false));
   jump_dec->SetForegroundColour(wxColor(0, 0, 128));
   tb_nav->AddControl(jump_dec, _("Jump decimal"));
   tb_nav->Realize();

   aui_manager.AddPane(tb_main, wxAuiPaneInfo().ToolbarPane().Top().LeftDockable(false).RightDockable(false));
   aui_manager.AddPane(tb_view, wxAuiPaneInfo().ToolbarPane().Top().Position(1).LeftDockable(false).RightDockable(false));
   aui_manager.AddPane(tb_nav, wxAuiPaneInfo().ToolbarPane().Top().Row(1).LeftDockable(false).RightDockable(false));
   
   aui_manager.AddPane(bookmarks, wxAuiPaneInfo().Right().PinButton().CloseButton().MinimizeButton().MinSize(150, 100)
      .BestSize(180, 100).FloatingSize(250, 200).Caption("Bookmarks").Name("bookmarks"));

   statusbar = new wxStatusBar(this, wxID_ANY, wxSB_FLAT);
   statusbar->SetFieldsCount(1);
   statusbar->PushStatusText(_("Ready"));
   SetStatusBar(statusbar);
  
   wxAuiMDIParentFrame::GetClientWindow()->Connect(
      wxEVT_COMMAND_AUINOTEBOOK_PAGE_CLOSE, wxAuiNotebookEventHandler(HtMainFrame::OnNotebookPageClose));

   jump_hex->Connect(wxEVT_TEXT_ENTER, wxCommandEventHandler(HtMainFrame::OnJumpHexTextEnter));
   //jump_hex->Connect(wxEVT_TEXT, wxCommandEventHandler(HtMainFrame::OnJumpHexTextEnter));


#ifdef WXDEBUG
   DebugPanel *debugPanel = new DebugPanel(this, HT_DebugPanel);
   
   aui_manager.AddPane(debugPanel, wxAuiPaneInfo().Right().PinButton().CloseButton().MinimizeButton()
      .MinSize(150, 100).BestSize(240, 100).FloatingSize(250, 200).Caption("Debug Info").Name("debug_pane"));

   aui_manager.GetPane("bookmarks").Hide();

   wxAuiMDIParentFrame::GetClientWindow()->Connect(
      wxEVT_COMMAND_AUINOTEBOOK_PAGE_CHANGED, wxAuiNotebookEventHandler(HtMainFrame::OnNotebookPageChange));
#endif

   aui_manager.Update();

   // TEMP -----------------
   //DoOpenFile(wxT("D:/Projects/ricardojricken/trunk/hexterminator/test/1452 - Metroid - Zero Mission (EU) (M5).gba"));
   DoOpenFile("test/1452 - Metroid - Zero Mission (EU) (M5).gba");
}

HtMainFrame::~HtMainFrame ()
{
   // The open tabs/windows must be closed manually before this code executes (I guess)... oO
   // So the OnClose/OnExit (whatever) should be responsible of doing it (again, I guess).

   aui_manager.UnInit();
}

void HtMainFrame::OnFileOpen (wxCommandEvent &WXUNUSED(event))
{
   wxFileDialog dialog(this, wxT("Escolha um arquivo"), wxT(""), wxT(""), _("All files (*.*)|*.*"), wxFD_OPEN | wxFD_FILE_MUST_EXIST);

   if (dialog.ShowModal() == wxID_OK)
   {
      wxString path = dialog.GetPath();
      DoOpenFile(path);
   }
}

void HtMainFrame::OnFileExit (wxCommandEvent &event)
{
   wxMessageDialog confirm(this, _("Are you sure you want to quit?"), _("HEXterminator"), wxYES_NO | wxYES_DEFAULT);
   confirm.ShowModal() == wxID_YES ? Close() : event.Skip();
}

void HtMainFrame::OnClose (wxCloseEvent &WXUNUSED(event))
{
#ifdef WXDEBUG
   dynamic_cast<DebugPanel *>(FindWindowById(HT_DebugPanel))->StopUpdateTimer();
#endif

   wxDocManager *docm = wxGetApp().doc_manager.get();

   wxList &docs = docm->GetDocuments();

   // delete all pages currently open in the notebook.
   // this will automatically destroy the views/doc/frames associated to them.
   wxAuiNotebook *nb = GetNotebook();
   while (nb->GetPageCount())
      nb->DeletePage(0);

   docm->Clear(true);
   Destroy();
}

void HtMainFrame::OnUpdateUI (wxUpdateUIEvent &event)
{
   switch (event.GetId())
   {
      case HT_FileSave:
         event.Enable(GetActiveView() != 0);
         break;

      case HT_FileClose:
         event.Enable(GetActiveView() != 0);
         break;

      case HT_EditCut:
      case HT_EditCopy:
      case HT_EditPaste:
         event.Enable(false);
         break;

      // updates the current offset in the selected tab
      case HT_Tool_JumpHex:
      case HT_Tool_JumpDec:
      {
         HexView *view = GetActiveView();
         event.Enable(view != 0);

         if (view)
         {
            wxFileOffset off = view->GetCurrentOffset();

            event.GetId() == HT_Tool_JumpHex ?
               event.SetText(wxString::Format("%I64X", off)) :
               event.SetText(wxString::Format("%I64u", off));
         }
      }
      break;

      case HT_Tool_SearchBox:
         event.Enable(GetActiveView() != 0);
         break;
   }
}

void HtMainFrame::OnViewStatusBar (wxCommandEvent &event)
{
   GetStatusBar()->Show(event.IsChecked());
   Layout();
}

void HtMainFrame::OnHelpAbout (wxCommandEvent &WXUNUSED(event))
{
   HtAbout about(this, _("About HEXterminator"), wxSize(370, 300));
   about.CenterOnParent();
   about.ShowModal();
}

void HtMainFrame::OnEditUndoDropDown (wxAuiToolBarEvent &event)
{
   if (event.IsDropDownClicked())
   {
      wxAuiToolBar *tb = static_cast<wxAuiToolBar *>(event.GetEventObject());
      tb->SetToolSticky(event.GetId(), true);

      wxMenu menuPopup;
      menuPopup.Append(wxID_ANY, wxT("Not implemented"));

      // line up our menu with the button
      wxRect rect = tb->GetToolRect(event.GetId());
      wxPoint pt = tb->ClientToScreen(rect.GetBottomLeft());
      pt = ScreenToClient(pt);

      PopupMenu(&menuPopup, pt);

      // make sure the button is "un-stuck"
      tb->SetToolSticky(event.GetId(), false);
   }
}

void HtMainFrame::OnFileOpenDropDown (wxAuiToolBarEvent &event)
{
   if (event.IsDropDownClicked())
   {
      wxAuiToolBar *tb = static_cast<wxAuiToolBar *>(event.GetEventObject());
      tb->SetToolSticky(event.GetId(), true);

      wxMenu menuPopup;
      history.AddFilesToMenu(&menuPopup);

      // event handler to deal with the menu items
      const int start = history.GetBaseId();
      const int end = history.GetBaseId() + history.GetMaxFiles();

      Connect(start, end, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(HtMainFrame::OnRecentFilesSelected));

      // line up our menu with the button
      wxRect rect = tb->GetToolRect(event.GetId());
      wxPoint pt = tb->ClientToScreen(rect.GetBottomLeft());
      pt = ScreenToClient(pt);

      PopupMenu(&menuPopup, pt);

      // make sure the button is "un-stuck"
      tb->SetToolSticky(event.GetId(), false);
   }
   else
   {
      wxCommandEvent dummy;
      OnFileOpen(dummy);
   }
}

void HtMainFrame::OnRecentFilesSelected (wxCommandEvent &event)
{
   int index = event.GetId() - history.GetBaseId();
   wxString selFile = history.GetHistoryFile(index);

   DoOpenFile(selFile);
}

void HtMainFrame::OnNotebookPageClose (wxAuiNotebookEvent &event)
{
   wxAuiNotebook *nb = static_cast<wxAuiNotebook *>(event.GetEventObject());
   HexFrame *frame = static_cast<HexFrame *>(nb->GetPage(event.GetSelection()));

   wxDocManager *docm = wxGetApp().doc_manager.get();

   HexView *vw = frame->GetView();
   docm->CloseDocument(vw->GetDocument());
}

void HtMainFrame::OnNotebookPageChange (wxAuiNotebookEvent &event)
{


#ifdef WXDEBUG
   wxAuiNotebook *nb = static_cast<wxAuiNotebook *>(event.GetEventObject());
   HexFrame *frame = static_cast<HexFrame *>(nb->GetPage(event.GetSelection()));

   frame->OnAddToDebugPanel();
#endif
}

void HtMainFrame::OnToolsLog (wxCommandEvent &WXUNUSED(event))
{
   wxSize display_sz = wxGetDisplaySize();
   wxSize logger_sz = logger->GetFrame()->GetSize();

   logger->GetFrame()->SetPosition(wxPoint(display_sz.x - logger_sz.x, display_sz.y - logger_sz.y - 30));
   logger->Show();
}

void HtMainFrame::OnEraseBackground (wxEraseEvent &WXUNUSED(event))
{
   // does nothing to prevent flicker
}

void HtMainFrame::OnKeyDown (wxKeyEvent &event)
{
   // Keyboard shortcuts to change the current active notebook tab.
   if (event.ControlDown() && event.GetKeyCode() == WXK_TAB)
   {
      bool goForward = !event.ShiftDown();

      GetNotebook()->AdvanceSelection(goForward);
      return;
   }

   // resume default event propagation
   event.Skip();
}

void HtMainFrame::OnJumpHexTextEnter (wxCommandEvent &event)
{
   auto *cbxPtr = dynamic_cast<wxComboBox *>(event.GetEventObject());
   wxString addressStr = cbxPtr->GetValue();

   wxLogStatus("[HtMainFrame] ::OnJumpHexTextEnter (%s)", addressStr);
}

/**
* This method is responsible for setting everything up in the doc/view framework
* and opening the request file for editing in a new notebook tab.
* @param path Path to the filename being opened
*/
void HtMainFrame::DoOpenFile (const wxString &path)
{
   wxDocManager *docm = wxGetApp().doc_manager.get();

   // TODO: fazer as checagens no arquivo antes de tentar abrir.
   //       Ex: verificar se ele pode ser aberto como leitura/escrita, se o usuário tem permissão etc.

   // check if the requested file ain't already open
   wxList &docs = docm->GetDocuments();
   for (wxList::iterator i = docs.begin(); i != docs.end(); i++)
   {
      HexDoc *doc = dynamic_cast <HexDoc *> (*i);

      if (doc->GetFilename() == path)
      {
         // it's open already... focus it and we're done
         HexFrame *frame = static_cast<HexView *>(doc->GetFirstView())->GetFrame();

         auto *nb = GetNotebook();
         auto frmIdx = nb->FindPage(static_cast<wxWindow *>(frame));

         if (frmIdx != wxNOT_FOUND)
         {
            nb->SetSelection(frmIdx);
            frame->SetFocus();
         }

         return;
      }
   }

   // the view is created automatically when we create the doc
   HexDoc *doc = (HexDoc *)docm->CreateDocument(path, wxDOC_SILENT);

   if (doc)
   {
      HexView *view = (HexView *)doc->GetFirstView();

      HexFrame *frame = new HexFrame(view, this, wxID_ANY, wxFileNameFromPath(path));
      history.AddFileToHistory(path);

#ifdef WXDEBUG
      DebugPanel *dbgPanel = dynamic_cast<DebugPanel *>(FindWindowById(HT_DebugPanel));
      view->AddToDebugPanel(dbgPanel);
#endif
   }
   else // TODO: melhorar essa mensagem de erro.
      wxMessageBox(wxString::Format("Não foi possível abrir o arquivo\n%s", path), "Erro", wxOK | wxICON_ERROR, this);
}

void HtMainFrame::CreateMenu (HtArtProvider &art)
{
   // -- File --
   wxMenu *file_menu = new wxMenu;

   wxMenuItem *file_new = new wxMenuItem(file_menu, HT_FileNew, _("&New\tCtrl+N"));
   wxMenuItem *file_open = new wxMenuItem(file_menu, HT_FileOpen, _("&Open...\tCtrl+O"));
   wxMenuItem *file_close = new wxMenuItem(file_menu, HT_FileClose, _("&Close\tCtrl+W"));
   wxMenuItem *file_closeall = new wxMenuItem(file_menu, HT_FileCloseAll, _("Close All"));
   wxMenuItem *file_save = new wxMenuItem(file_menu, HT_FileSave, _("&Save\tCtrl+S"));
   wxMenuItem *file_saveas = new wxMenuItem(file_menu, HT_FileSaveAs, _("Save &As..."));
   wxMenuItem *file_saveall = new wxMenuItem(file_menu, HT_FileSaveAll, _("Save All\tCtrl+Shift+S"));
   wxMenuItem *file_exit = new wxMenuItem(file_menu, HT_FileExit, _("E&xit\tCtrl+Q"));
   file_new->SetBitmap(art.GetBitmap(HTArtId_MainTb, HTArt_New));
   file_open->SetBitmap(art.GetBitmap(HTArtId_MainTb, HTArt_Open));
   file_close->SetBitmap(art.GetBitmap(HTArtId_Common, HTArt_Close));
   file_save->SetBitmap(art.GetBitmap(HTArtId_MainTb, HTArt_Save));
   file_exit->SetBitmap(art.GetBitmap(HTArtId_Common, HTArt_Exit));

   file_menu->Append(file_new);
   file_menu->Append(file_open);
   file_menu->AppendSeparator();
   file_menu->Append(file_close);
   file_menu->Append(file_closeall);
   file_menu->AppendSeparator();
   file_menu->Append(file_save);
   file_menu->Append(file_saveas);
   file_menu->Append(file_saveall);
   file_menu->AppendSeparator();
   file_menu->Append(file_exit);

   // -- Edit -> Copy As --
   wxMenu *edit_copyas = new wxMenu;

   wxMenuItem *edit_copyas_hextxt = new wxMenuItem(edit_copyas, HT_EditCopyAsHexText, _("&Hex Text"));
   wxMenuItem *edit_copyas_csrc = new wxMenuItem(edit_copyas, HT_EditCopyAsCSource, _("&C Source..."));
   edit_copyas_hextxt->SetBitmap(art.GetBitmap(HTArtId_Common, HTArt_CopyAsHex));
   edit_copyas_csrc->SetBitmap(art.GetBitmap(HTArtId_Common, HTArt_CopyAsCSrc));

   edit_copyas->Append(edit_copyas_hextxt);
   edit_copyas->Append(edit_copyas_csrc);

   // -- Edit --
   wxMenu *edit_menu = new wxMenu;

   wxMenuItem *edit_undo = new wxMenuItem(edit_menu, HT_EditUndo, _("&Undo\tCtrl+Z"));
   wxMenuItem *edit_redo = new wxMenuItem(edit_menu, HT_EditRedo, _("&Redo\tCtrl+Y"));
   wxMenuItem *edit_cut = new wxMenuItem(edit_menu, HT_EditCut, _("Cu&t\tCtrl+X"));
   wxMenuItem *edit_copy = new wxMenuItem(edit_menu, HT_EditCopy, _("Copy\tCtrl+C"));
   wxMenuItem *edit_paste = new wxMenuItem(edit_menu, HT_EditPaste, _("Paste\tCtrl+V"));
   wxMenuItem *edit_selectall = new wxMenuItem(edit_menu, HT_EditSelectAll, _("Select All\tCtrl+A"));
   edit_undo->SetBitmap(art.GetBitmap(HTArtId_MainTb, HTArt_Undo));
   edit_redo->SetBitmap(art.GetBitmap(HTArtId_MainTb, HTArt_Redo));
   edit_cut->SetBitmap(art.GetBitmap(HTArtId_MainTb, HTArt_Cut));
   edit_copy->SetBitmap(art.GetBitmap(HTArtId_MainTb, HTArt_Copy));
   edit_paste->SetBitmap(art.GetBitmap(HTArtId_MainTb, HTArt_Paste));

   edit_menu->Append(edit_undo);
   edit_menu->Append(edit_redo);
   edit_menu->AppendSeparator();
   edit_menu->Append(edit_cut);
   edit_menu->Append(edit_copy);
   edit_menu->Append(edit_paste);
   edit_menu->AppendSeparator();
   edit_menu->AppendSubMenu(edit_copyas, _("&Copy As"));
   edit_menu->Append(edit_selectall);

   // -- View --
   wxMenu *view_menu = new wxMenu;

   view_menu->AppendCheckItem(HT_ViewStatusBar, _("&Status Bar"), _("Show/hide status bar"));
   view_menu->Check(HT_ViewStatusBar, true);

#ifdef WXDEBUG
   view_menu->AppendCheckItem(HT_ViewDebugPanel, "&Debug Panel", "Show/hide the debug panel");
   view_menu->Check(HT_ViewDebugPanel, true);
#endif

   // -- Tools --
   wxMenu *tools_menu = new wxMenu;

   wxMenuItem *tools_tblmanager = new wxMenuItem(tools_menu, HT_ToolsTblManager, _("&Table Editor\tCtrl+Shift+T"));
   wxMenuItem *tools_logger = new wxMenuItem(tools_menu, HT_ToolsLogger, _("&Logger\tCtrl+Shift+E"));

   tools_menu->Append(tools_tblmanager);
   tools_menu->Append(tools_logger);

   // -- Help --
   wxMenu *help_menu = new wxMenu;
   
   help_menu->Append(wxID_ABOUT, _("&About"));

   // -------------------------------------------
   wxMenuBar *mb = new wxMenuBar;

   mb->Append(file_menu, _("&File"));
   mb->Append(edit_menu, _("&Edit"));
   mb->Append(view_menu, _("&View"));
   mb->Append(tools_menu, _("&Tools"));
   mb->Append(help_menu, _("&Help"));
 
   SetMenuBar(mb);
}

   



