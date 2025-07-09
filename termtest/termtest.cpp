#include <windows.h>
#include <iostream>
#include <string>

HANDLE hSerial;

bool openSerialPort(const std::string& portName) {
    std::string fullName = portName;
    if (portName.length() > 2) // Handle COM10 and above
        fullName = "\\\\.\\" + portName;

    hSerial = CreateFileA(fullName.c_str(), GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, 0, nullptr);


    if (hSerial == INVALID_HANDLE_VALUE) {
        std::cerr << "Error opening serial port\n";
        return false;
    }

    DCB dcb = { 0 };
    dcb.DCBlength = sizeof(dcb);
    if (!GetCommState(hSerial, &dcb)) {
        std::cerr << "Failed to get COM state\n";
        return false;
    }

    dcb.BaudRate = CBR_115200;
    dcb.ByteSize = 8;
    dcb.StopBits = ONESTOPBIT;
    dcb.Parity = EVENPARITY;

    if (!SetCommState(hSerial, &dcb)) {
        std::cerr << "Failed to set COM state\n";
        return false;
    }

    return true;
}

void sendString(const std::string& str) {
    DWORD bytesWritten;
    WriteFile(hSerial, str.c_str(), str.size(), &bytesWritten, nullptr);
}

void closeSerialPort() {
    if (hSerial != INVALID_HANDLE_VALUE)
        CloseHandle(hSerial);
}

void printMenu() {
    std::cout << "\n=== ANSI UART Test Menu ===\n";
    std::cout << "1. Red text\n";
    std::cout << "2. Green text\n";
    std::cout << "3. Move cursor to (5,10)\n";
    std::cout << "4. Clear screen\n";
    std::cout << "5. Reset attributes\n";
    std::cout << "6. Custom string\n";
    std::cout << "0. Exit\n";
    std::cout << "Select option: ";
}

int main() {
    std::string port;
   // std::cout << "Enter COM port (e.g., COM3): ";
    //std::getline(std::cin, port);

    if (!openSerialPort("COM15"))
        return 1;

    int choice;
    do {
        printMenu();
        std::cin >> choice;
        std::cin.ignore(); // clear newline

        switch (choice) {
        case 1:
            sendString("\x1B[31mRed text\x1B[0m\r\n");
            break;
        case 2:
            sendString("\x1B[32mGreen text\x1B[0m\r\n");
            break;
        case 3:
            sendString("\x1B[5;10HPositioned Text\r\n");
            break;
        case 4:
            sendString("\x1B[2J\x1B[H"); // clear + home
            break;
        case 5:
            sendString("\x1B[0mAttributes reset\r\n");
            break;
        case 6: {
            std::string custom;
            std::cout << "Enter raw string (escape as needed): ";
            std::getline(std::cin, custom);
            sendString(custom + "\r\n");
            break;
        }
        case 0:
            break;
        default:
            std::cout << "Invalid option\n";
        }

    } while (choice != 0);

    closeSerialPort();
    return 0;
}
