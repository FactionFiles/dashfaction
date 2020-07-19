#pragma once

#include <wxx_wincore.h>
#include "resource.h"
#include <crash_handler_stub/WatchDogTimer.h>
#include <optional>
#include <string>

class LauncherApp : public CWinApp
{
    WatchDogTimer m_watch_dog_timer;

public:
    virtual BOOL InitInstance() override;

    bool LaunchGame(HWND hwnd, const char* mod_name = nullptr, std::optional<std::string> rf_exe_path = {});
    bool LaunchEditor(HWND hwnd, const char* mod_name = nullptr, std::optional<std::string> rf_exe_path = {});

private:
    void MigrateConfig();
    int Message(HWND hwnd, const char *text, const char *title, int flags);
};

inline LauncherApp* GetLauncherApp()
{
    return static_cast<LauncherApp*>(GetApp());
}
