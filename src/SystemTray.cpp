﻿//
// Created by Climber on 2020/6/3.
//

#include "SystemTray.h"
#include "Configuration.h"
#include "Climber.h"
#include "ServerConfManager.h"
#include "Paths.h"
#include "utils.h"

#ifdef CLIMBER_DARWIN
#define PREFERENCES_MENU_TITLE _("Preferences")
#else
#define PREFERENCES_MENU_TITLE _("Settings")
#endif

SystemTray::SystemTray() {
    wxTaskBarIcon::SetIcon(wxIcon(Paths::GetAssetsDirFile("icon.png"), wxBITMAP_TYPE_ANY));
}

wxMenu *SystemTray::CreatePopupMenu() {
    m_taskBarMenu = new wxMenu();

    auto *statusMenuItem =
            m_taskBarMenu->Append(ID_MENU_STATUS, CLIMBER.IsRunning() ? _("Climber: On") : _("Climber: Off"));
    statusMenuItem->Enable(false);
    m_taskBarMenu->Append(ID_MENU_TOGGLE, CLIMBER.IsRunning() ? _("Stop Climber") : _("Start Climber"));

    m_taskBarMenu->AppendSeparator();

    m_taskBarMenu->AppendSubMenu(CreateProxyModeMenu(), _("Proxy Mode"));
    m_taskBarMenu->AppendSubMenu(CreateServersListMenu(), _("Servers"));
    m_taskBarMenu->AppendSubMenu(CreatePacMenu(), _("PAC"));

    m_taskBarMenu->AppendSeparator();

    m_taskBarMenu->Append(ID_MENU_PREFERENCES, PREFERENCES_MENU_TITLE);
    m_taskBarMenu->AppendSubMenu(CreateCopyCommandMenu(), _("Copy Terminal Proxy Command"));
    m_taskBarMenu->Append(ID_MENU_OPEN_CONFIG_DIRECTORY, _("Open Config Directory"));
    m_taskBarMenu->Append(ID_MENU_OPEN_LOG_DIRECTORY, _("Open Log Directory"));
    m_taskBarMenu->Append(ID_MENU_CHECK_FOR_UPDATES, _("Check for Updates"));
    m_taskBarMenu->Append(ID_MENU_ABOUT, _("About"));

    m_taskBarMenu->AppendSeparator();

    m_taskBarMenu->Append(ID_MENU_EXIT, _("Quit"));
    return m_taskBarMenu;
}

wxMenu *SystemTray::CreateProxyModeMenu() {
    auto proxyMode = CONFIGURATION.GetProxyMode();

    auto *proxyModeMenu = new wxMenu();

    auto *proxyDirectModeMenuItem = proxyModeMenu->AppendRadioItem(ID_MENU_PROXY_DIRECT_MODE, _("Direct Mode"));
    if (proxyMode == PROXY_MODE_DIRECT) {
        proxyDirectModeMenuItem->Check(true);
    }

    auto *proxyPacModeMenuItem = proxyModeMenu->AppendRadioItem(ID_MENU_PROXY_PAC_MODE, _("PAC Mode"));
    if (proxyMode == PROXY_MODE_PAC) {
        proxyPacModeMenuItem->Check(true);
    }

    auto *proxyGlobalModeMenuItem = proxyModeMenu->AppendRadioItem(ID_MENU_PROXY_GLOBAL_MODE, _("Global Mode"));
    if (proxyMode == PROXY_MODE_GLOBAL) {
        proxyGlobalModeMenuItem->Check(true);
    }

    return proxyModeMenu;
}

wxMenu *SystemTray::CreatePacMenu() {
    auto *pacMenu = new wxMenu();
    pacMenu->Append(ID_MENU_UPDATE_GFWLIST, _("Update gfwlist.txt"));
    pacMenu->Append(ID_MENU_EDIT_USER_RULE, _("Edit User Rules"));
    return pacMenu;
}

wxMenu *SystemTray::CreateServersListMenu() {
    auto *serverListMenu = new wxMenu();
    serverListMenu->Append(ID_MENU_SERVERS_REFRESH, _("Refresh"));

    const auto &serverList = SERVER_CONF_MANAGER.GetServersList();
    if (serverList.size() > 0) {
        serverListMenu->AppendSeparator();
    }

    const int maxCount = ID_MENU_SERVER_ITEM_END - ID_MENU_SERVER_ITEM_START;
    if (serverList.size() > maxCount) {
        wxLogWarning("Max servers count %d, %lu provided", maxCount, serverList.size());
    }
    const int count = wxMin(serverList.size(), maxCount);
    const int selected = CONFIGURATION.GetSelectedServerIndex();
    m_serverMenuItemList.clear();
    for (int i = 0; i < count; ++i) {
        const auto *serverItem = serverList[i];
        auto menuItem = serverListMenu->AppendCheckItem(
                ID_MENU_SERVER_ITEM_START + i,
                serverItem->GetSystemTrayTitle());
        menuItem->Check(i == selected);
        m_taskBarMenu->Bind(wxEVT_MENU, [count, i, this](wxCommandEvent &event) {
            CONFIGURATION.SetSelectedServerIndex(i);
            if (CLIMBER.IsRunning()) {
                CLIMBER.Restart();
            }
            for (int n = 0; n < count; ++n) {
                m_taskBarMenu->FindItem(ID_MENU_SERVER_ITEM_START + n)->Check(n == i);
            }
        }, ID_MENU_SERVER_ITEM_START + i);
        m_serverMenuItemList.push_back(menuItem);
    }
    return serverListMenu;
}

wxMenu *SystemTray::CreateCopyCommandMenu() {
    auto *copyCommandMenu = new wxMenu();
    copyCommandMenu->Append(ID_MENU_COPY_TERMINAL_PROXY_COMMAND_BASH, _("For Bash"));
#ifdef CLIMBER_WINDOWS
    copyCommandMenu->Append(ID_MENU_COPY_TERMINAL_PROXY_COMMAND_CMD, _("For CMD"));
#endif
    return copyCommandMenu;
}

void SystemTray::OnToggleClimber(wxCommandEvent &event) {
    if (CLIMBER.IsRunning()) {
        CLIMBER.Stop();
        CONFIGURATION.SetEnable(false);
    } else {
        CLIMBER.Start();
        CONFIGURATION.SetEnable(true);
    }
    m_taskBarMenu->FindItem(ID_MENU_STATUS)->SetItemLabel(CLIMBER.IsRunning() ? _("Climber: On") : _("Climber: Off"));
    m_taskBarMenu->FindItem(ID_MENU_TOGGLE)->SetItemLabel(CLIMBER.IsRunning() ? _("Stop Climber") : _("Start Climber"));
}

void SystemTray::OnSelectDirectProxyMode(wxCommandEvent &event) {
    if (event.IsChecked()) {
        ((wxMenuItem *) event.GetEventObject())->Check(true);
        CONFIGURATION.SetProxyMode(PROXY_MODE_DIRECT);
        if (CLIMBER.IsRunning()) {
            CLIMBER.SetSystemProxy();
        }
    }
}

void SystemTray::OnSelectPacProxyMode(wxCommandEvent &event) {
    if (event.IsChecked()) {
        ((wxMenuItem *) event.GetEventObject())->Check(true);
        CONFIGURATION.SetProxyMode(PROXY_MODE_PAC);
        if (CLIMBER.IsRunning()) {
            CLIMBER.SetSystemProxy();
        }
    }
}

void SystemTray::OnSelectGlobalProxyMode(wxCommandEvent &event) {
    if (event.IsChecked()) {
        ((wxMenuItem *) event.GetEventObject())->Check(true);
        CONFIGURATION.SetProxyMode(PROXY_MODE_GLOBAL);
        if (CLIMBER.IsRunning()) {
            CLIMBER.SetSystemProxy();
        }
    }
}

void SystemTray::OnUpdateGfwlist(wxCommandEvent &event) {
    if (m_updateGfwlistThread != nullptr) {
        return;
    }
    if (!CLIMBER.IsRunning()) {
        wxMessageDialog(nullptr, _("You must turn on Climber first to update gfwlist!"), _("Warning")).ShowModal();
        return;
    }

    m_updateGfwlistThread = new UpdateGfwlistThread(this);
    m_updateGfwlistThread->Run();
}

wxThread::ExitCode UpdateGfwlistThread::Entry() {
    httplib::SSLClient client("github.com", 443);
    client.set_proxy("127.0.0.1", CONFIGURATION.GetHttpPort());
    auto res = client.Get("/gfwlist/gfwlist/blob/master/gfwlist.txt");
    if (res && res->status == 200) {
        writeTextFile(Paths::GetRuleDirFile("gfwlist.txt"), wxString(res->body));
        wxCommandEvent event(wxEVT_UPDATE_GFWLIST_FINISHED, wxID_ANY);
        event.SetInt(1);
        m_parent->AddPendingEvent(event);
    } else {
        wxCommandEvent event(wxEVT_UPDATE_GFWLIST_FINISHED, wxID_ANY);
        event.SetInt(0);
        m_parent->AddPendingEvent(event);
    }
    return 0;
}

void SystemTray::OnUpdateGfwlistFinished(wxCommandEvent &event) {
    m_updateGfwlistThread = nullptr;
    if (event.GetInt()) {
        wxMessageDialog(nullptr, _("Update gfwlist suc!"), _("Information")).ShowModal();
    } else {
        wxMessageDialog(nullptr, _("Update gfwlist failed!"), _("Warning")).ShowModal();
    }
}

void SystemTray::OnEditUserRules(wxCommandEvent &event) {
    if (!wxFileExists(Paths::GetRuleDirFile("user-rule.txt"))) {
        wxCopyFile(Paths::GetAssetsDirFile("user-rule.txt"), Paths::GetRuleDirFile("user-rule.txt"));
    }
    openDirectory(Paths::GetRuleDir());
}

void SystemTray::OnRefreshServers(wxCommandEvent &event) {
    SERVER_CONF_MANAGER.Reload();
    if (CLIMBER.IsRunning()) {
        CLIMBER.Restart();
    }
}

void SystemTray::OnShowPreferencesFrame(wxCommandEvent &event) {
    if (m_preferencesFrame == nullptr) {
        m_preferencesFrame = new PreferencesFrame(nullptr, ID_FRAME_PREFERENCES);
        m_preferencesFrame->Bind(wxEVT_CLOSE_WINDOW, [&](wxCloseEvent &event) {
            m_preferencesFrame = nullptr;
            event.Skip();
        });
    }

//    Raise do not work, why?
//    m_preferencesFrame->Raise();

    long style = m_preferencesFrame->GetWindowStyle();
    m_preferencesFrame->SetWindowStyle(style | wxSTAY_ON_TOP);
    m_preferencesFrame->Show();
    m_preferencesFrame->SetWindowStyle(style);
}

void SystemTray::OnCopyProxyCommandBash(wxCommandEvent &event) {
    setClipboardText(wxString::Format(
            "export all_proxy=socks5://127.0.0.1:%d; export http_proxy=http://127.0.0.1:%d; export https_proxy=http://127.0.0.1:%d;",
            CONFIGURATION.GetSocksPort(),
            CONFIGURATION.GetHttpPort(),
            CONFIGURATION.GetHttpPort()
    ));
}

void SystemTray::OnCopyProxyCommandCmd(wxCommandEvent &event) {
    setClipboardText(wxString::Format(
            "set all_proxy=socks5://127.0.0.1:%d&&set http_proxy=http://127.0.0.1:%d&&set https_proxy=http://127.0.0.1:%d",
            CONFIGURATION.GetSocksPort(),
            CONFIGURATION.GetHttpPort(),
            CONFIGURATION.GetHttpPort()
    ));
}

void SystemTray::OnOpenConfigDirectory(wxCommandEvent &event) {
    openDirectory(Paths::GetConfigDir());
}

void SystemTray::OnOpenLogDirectory(wxCommandEvent &event) {
    openDirectory(Paths::GetLogDir());
}

void SystemTray::OnCheckForUpdates(wxCommandEvent &event) {
    // TODO
}

void SystemTray::OnShowAboutFrame(wxCommandEvent &event) {
    if (m_aboutFrame == nullptr) {
        m_aboutFrame = new AboutFrame(nullptr, ID_FRAME_ABOUT);
        m_aboutFrame->Bind(wxEVT_CLOSE_WINDOW, [&](wxCloseEvent &event) {
            m_aboutFrame = nullptr;
            event.Skip();
        });
    }

//    Raise do not work, why?
//    m_aboutFrame->Raise();

    long style = m_aboutFrame->GetWindowStyle();
    m_aboutFrame->SetWindowStyle(style | wxSTAY_ON_TOP);
    m_aboutFrame->Show();
    m_aboutFrame->SetWindowStyle(style);
}

void SystemTray::OnQuit(wxCommandEvent &event) {
    RemoveIcon();
    wxExit();
}

BEGIN_EVENT_TABLE(SystemTray, wxTaskBarIcon)
                EVT_MENU(ID_MENU_TOGGLE, SystemTray::OnToggleClimber)
                EVT_MENU(ID_MENU_PROXY_DIRECT_MODE, SystemTray::OnSelectDirectProxyMode)
                EVT_MENU(ID_MENU_PROXY_PAC_MODE, SystemTray::OnSelectPacProxyMode)
                EVT_MENU(ID_MENU_PROXY_GLOBAL_MODE, SystemTray::OnSelectGlobalProxyMode)
                EVT_MENU(ID_MENU_UPDATE_GFWLIST, SystemTray::OnUpdateGfwlist)
                EVT_COMMAND(wxID_ANY, wxEVT_UPDATE_GFWLIST_FINISHED, SystemTray::OnUpdateGfwlistFinished)
                EVT_MENU(ID_MENU_EDIT_USER_RULE, SystemTray::OnEditUserRules)
                EVT_MENU(ID_MENU_SERVERS_REFRESH, SystemTray::OnRefreshServers)
                EVT_MENU(ID_MENU_PREFERENCES, SystemTray::OnShowPreferencesFrame)
                EVT_MENU(ID_MENU_COPY_TERMINAL_PROXY_COMMAND_BASH, SystemTray::OnCopyProxyCommandBash)
                EVT_MENU(ID_MENU_COPY_TERMINAL_PROXY_COMMAND_CMD, SystemTray::OnCopyProxyCommandCmd)
                EVT_MENU(ID_MENU_OPEN_CONFIG_DIRECTORY, SystemTray::OnOpenConfigDirectory)
                EVT_MENU(ID_MENU_OPEN_LOG_DIRECTORY, SystemTray::OnOpenLogDirectory)
                EVT_MENU(ID_MENU_CHECK_FOR_UPDATES, SystemTray::OnCheckForUpdates)
                EVT_MENU(ID_MENU_ABOUT, SystemTray::OnShowAboutFrame)
                EVT_MENU(ID_MENU_EXIT, SystemTray::OnQuit)
END_EVENT_TABLE()
