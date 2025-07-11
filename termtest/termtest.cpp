#include <windows.h>
#include <iostream>
#include <string>
#include <sstream>

HANDLE hSerial;

bool openSerialPort(const std::string& portName) {
    std::string fullName = portName;
    if (portName.length() > 4)  // COM10 and above
        fullName = "\\\\.\\" + portName;

    hSerial = CreateFileA(fullName.c_str(), GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, 0, nullptr);
    if (hSerial == INVALID_HANDLE_VALUE) {
        std::cerr << "Error opening serial port\n";
        return false;
    }

    DCB dcb = { 0 };
    dcb.DCBlength = sizeof(dcb);
    if (!GetCommState(hSerial, &dcb)) return false;

    dcb.BaudRate = CBR_115200;
    dcb.ByteSize = 8;
    dcb.StopBits = ONESTOPBIT;
    dcb.Parity = NOPARITY;

    return SetCommState(hSerial, &dcb);
}

void sendString(const std::string& str) {
    DWORD bytesWritten;
    WriteFile(hSerial, str.c_str(), str.size(), &bytesWritten, nullptr);
}

void clearScreen() {
    sendString("\x1B[2J\x1B[H");  // Clear screen and move cursor to top-left
}

void drawFrame() {
    sendString("\x1B[1;1H+----------------------+");
    for (int i = 2; i < 10; ++i) {
        std::ostringstream oss;
        oss << "\x1B[" << i << ";1H|";
        oss << "\x1B[" << i << ";23H|";
        sendString(oss.str());
    }
    sendString("\x1B[10;1H+----------------------+");
}

void showColorTest() {
    clearScreen();
    sendString("\x1B[1;1H256-color demo:\r\n");
    for (int i = 0; i < 256; ++i) {
        std::ostringstream oss;
        oss << "\x1B[48;5;" << i << "m ";
        sendString(oss.str());
        if ((i + 1) % 16 == 0) sendString("\x1B[0m\r\n");
    }
    sendString("\x1B[0m\r\n");
}

void printMenu() {
    std::cout << "\n=== ANSI UART Demo ===\n";
    std::cout << "1. Bold, Underline, Inverse\n";
    std::cout << "2. 256-Color Gradient\n";
    std::cout << "3. Cursor Move Example\n";
    std::cout << "4. Clear Screen + Draw Frame\n";
    std::cout << "5. Custom ANSI String\n";
    std::cout << "6. Show Status Bar\n";
    std::cout << "7. Show Progress Bar\n";
    std::cout << "8. Simulated Menu\n";
    std::cout << "0. Exit\n";
    std::cout << "Select option: ";
}


void demoStyles() {
    sendString("\x1B[1mBold Text\x1B[0m\r\n");
    sendString("\x1B[4mUnderline Text\x1B[0m\r\n");
    sendString("\x1B[7mInverted Colors\x1B[0m\r\n");
    sendString("\x1B[3mItalic (may not work)\x1B[0m\r\n");
}

void demoCursorMove() {
    clearScreen();
    sendString("\x1B[5;10HHello at (5,10)");
    sendString("\x1B[6;20HAnother line at (6,20)");
    sendString("\x1B[10;1HBack to bottom\r\n");
}
void drawStatusBar(const std::string& text) {
    sendString("\x1B[s");                    // Save cursor
    sendString("\x1B[24;1H");                // Move to bottom
    sendString("\x1B[7m");                   // Inverted colors
    sendString(" " + text + std::string(80 - text.length(), ' ')); // Pad line
    sendString("\x1B[0m");                   // Reset
    sendString("\x1B[u");                    // Restore cursor
    sendString("alma");                    // Restore cursor
}
void drawProgressBar(int percent) {
    int width = 50;
    int filled = percent * width / 100;
    std::ostringstream oss;
    oss << "\x1B[10;5H[";
    for (int i = 0; i < width; ++i) {
        if (i < filled) oss << "\x1B[42m*\x1B[0m";  // Green block
        else oss << "-";
    }
    oss << "] " << percent << "%";
    sendString(oss.str());
}
void showFakeInteractiveMenu() {
    clearScreen();
    const char* items[] = { "Option 1", "Option 2", "Option 3", "Exit Menu" };
    int selected = 1;

    for (int i = 0; i < 4; ++i) {
        std::ostringstream oss;
        oss << "\x1B[" << (5 + i) << ";10H";
        if (i == selected)
            oss << "\x1B[7m> " << items[i] << " \x1B[0m";
        else
            oss << "  " << items[i];
        sendString(oss.str());
    }

    sendString("\x1B[12;1H(This is a placeholder � add real key input later.)\r\n");
}


int main() {
    std::string port;
    std::cout << "Enter COM port (e.g., COM3): ";
    std::getline(std::cin, port);

    if (!openSerialPort(port)) return 1;

    int choice;
    do {
        printMenu();
        std::cin >> choice;
        std::cin.ignore();

        switch (choice) {
        case 1: demoStyles(); break;
        case 2: showColorTest(); break;
        case 3: demoCursorMove(); break;
        case 4: clearScreen(); drawFrame(); break;
        case 5: {
            std::string custom;
            std::cout << "Enter raw ANSI string: ";
            std::getline(std::cin, custom);
            sendString(custom + "\r\n");
            break;
        }
        case 6:
            drawStatusBar("Connected to STM32 - UART OK");
            break;
        case 7: {
            clearScreen();
            for (int i = 0; i <= 100; i += 10) {
                drawProgressBar(i);
                Sleep(200);  // Windows Sleep(ms)
            }
            break;
        }
        case 8:
            showFakeInteractiveMenu();
            break;
        }

    } while (choice != 0);

    CloseHandle(hSerial);
    return 0;
}
