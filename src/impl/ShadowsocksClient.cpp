﻿//
// Created by Climber on 2020/6/8.
//

#include <utility>
#include "ShadowsocksClient.h"
#include "../Paths.h"
#include "../Configuration.h"
#include "../utils.h"

ShadowsocksClient::ShadowsocksClient(const wxString &bin, json obj) : BaseClient(bin, std::move(obj)) {}

json ShadowsocksClient::OverrideListening(const wxString &localAddr, int localPort) const {
    auto data = m_data;
    data["data"]["local_address"] = localAddr.ToStdString();
    data["data"]["local_port"] = localPort;
    return data;
}

wxVector<wxVariant> ShadowsocksClient::GetDataViewRowData() const {
    wxVector<wxVariant> row;
    std::string host = m_data["data"]["server"];
    int port = m_data["data"]["server_port"];
    row.push_back(m_name);
    row.push_back(m_type);
    row.push_back(host);
    row.push_back(wxString::Format("%d", port));
    return row;
}

wxString ShadowsocksClient::GetSystemTrayTitle() const {
    std::string host = m_data["data"]["server"];
    int port = m_data["data"]["server_port"];
    return wxString::Format("%s (%s@%s:%d)", m_name, m_type, wxString(host), port);
}

void ShadowsocksClient::Start() const {
    auto clientTmpConfigFile = Paths::GetTmpDirFile("shadowsocks.json");
    auto clientLogFile = Paths::GetLogDirFile("shadowsocks.log");
    auto localAddr = CONFIGURATION.GetShareOnLan() ? "0.0.0.0" : "127.0.0.1";
    auto localPort = CONFIGURATION.GetSocksPort();
    this->WriteTo(clientTmpConfigFile, localAddr, localPort);

    execRedirect(wxString::Format("\"%s\" -c \"%s\"", m_bin, clientTmpConfigFile), clientLogFile);
    wxLogMessage("Shadowsocks client started at %s:%d", localAddr, localPort);
}
