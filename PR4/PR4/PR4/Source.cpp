#include <iostream>
#include <filesystem>
#include <string>
#include <windows.h>

namespace fs = std::filesystem;

// ������� ��� ��������, �� ���� � ����������
bool isHidden(const fs::path& p) {
    DWORD attrs = GetFileAttributes(p.c_str());
    return (attrs != INVALID_FILE_ATTRIBUTES) && (attrs & FILE_ATTRIBUTE_HIDDEN);
}

// ������� ��� ��������� ������� �����, ��������� �������� ������� � ������� �������
int countFiles(const fs::path& directory, const std::string& pattern, bool include_hidden, bool include_readonly, bool include_archive) {
    int count = 0;
    for (const auto& entry : fs::recursive_directory_iterator(directory)) {
        if (fs::is_regular_file(entry) && entry.path().filename().string().find(pattern) != std::string::npos) {
            auto perms = fs::status(entry).permissions();
            bool is_readonly = (perms & fs::perms::owner_write) == fs::perms::none;
            bool is_archive = (perms & fs::perms::owner_exec) == fs::perms::none;
            bool matches = true;

            // Գ�������� ����� �� ����������
            if ((isHidden(entry) && !include_hidden) || (is_readonly && !include_readonly) || (is_archive && !include_archive)) {
                matches = false;
            }

            if (matches) {
                count++;
            }
        }
    }
    return count;
}

int main(int argc, char* argv[]) {
    int exit_code = 0;

    // �������� �������� ��������� ���������� ����� ��� ������ �������
    if (argc == 1) {
        std::cout << "Usage: " << argv[0] << " <directory> [pattern] [options]\n";
        std::cout << "Options:\n";
        std::cout << "  --help        Show this help message\n";
        std::cout << "  --hidden      Include hidden files\n";
        std::cout << "  --readonly    Include read-only files\n";
        std::cout << "  --archive     Include archive files\n";
        return 2; // ��� ���������� ��� ������� ���������
    }

    // ������� ����� ���������� �����
    bool include_hidden = false;
    bool include_readonly = false;
    bool include_archive = false;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--help") {
            std::cout << "Usage: " << argv[0] << " <directory> [pattern] [options]\n";
            std::cout << "Options:\n";
            std::cout << "  --help        Show this help message\n";
            std::cout << "  --hidden      Include hidden files\n";
            std::cout << "  --readonly    Include read-only files\n";
            std::cout << "  --archive     Include archive files\n";
            return 0; // ��� ���������� ��� �����
        }
        else if (arg == "--hidden") {
            include_hidden = true;
        }
        else if (arg == "--readonly") {
            include_readonly = true;
        }
        else if (arg == "--archive") {
            include_archive = true;
        }
    }

    // ��������� �������� �� ������� �� ��������� ���������� �����
    std::string directory;
    std::string pattern = "*"; // �� �������������, ������ '*' ������ �� �����

    if (argc >= 2) {
        directory = argv[1];
    }
    if (argc >= 3) {
        pattern = argv[2];
    }

    try {
        // ϳ�������� ������� ����� � �����������, �� ���������� �������� �������
        int fileCount = countFiles(directory, pattern, include_hidden, include_readonly, include_archive);

        // ���� ����������
        std::cout << "Number of files matching pattern '" << pattern << "' in directory '" << directory << "': " << fileCount << std::endl;
        exit_code = 0; // ��� ���������� ��� �����
    }
    catch (const fs::filesystem_error& e) {
        std::cerr << "Filesystem error: " << e.what() << std::endl;
        exit_code = 3; // ��� ���������� ��� ������� ������� �������
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        exit_code = 1; // ��� ���������� ��� �������� �������
    }

    // ���� ���� ����������
    std::cout << "Exit code: " << exit_code << std::endl;
    return exit_code;
}