#include <iostream>
#include <filesystem>
#include <string>
#include <windows.h>

namespace fs = std::filesystem;

// Функція для перевірки, чи файл є прихованим
bool isHidden(const fs::path& p) {
    DWORD attrs = GetFileAttributes(p.c_str());
    return (attrs != INVALID_FILE_ATTRIBUTES) && (attrs & FILE_ATTRIBUTE_HIDDEN);
}

// Функція для підрахунку кількості файлів, відповідних заданому шаблону в певному каталозі
int countFiles(const fs::path& directory, const std::string& pattern, bool include_hidden, bool include_readonly, bool include_archive) {
    int count = 0;
    for (const auto& entry : fs::recursive_directory_iterator(directory)) {
        if (fs::is_regular_file(entry) && entry.path().filename().string().find(pattern) != std::string::npos) {
            auto perms = fs::status(entry).permissions();
            bool is_readonly = (perms & fs::perms::owner_write) == fs::perms::none;
            bool is_archive = (perms & fs::perms::owner_exec) == fs::perms::none;
            bool matches = true;

            // Фільтрація файлів за атрибутами
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

    // Перевірка наявності параметрів командного рядка для режиму підказки
    if (argc == 1) {
        std::cout << "Usage: " << argv[0] << " <directory> [pattern] [options]\n";
        std::cout << "Options:\n";
        std::cout << "  --help        Show this help message\n";
        std::cout << "  --hidden      Include hidden files\n";
        std::cout << "  --readonly    Include read-only files\n";
        std::cout << "  --archive     Include archive files\n";
        return 2; // Код завершення для помилок аргументів
    }

    // Обробка опцій командного рядка
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
            return 0; // Код завершення для успіху
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

    // Отримання каталогу та шаблону із аргументів командного рядка
    std::string directory;
    std::string pattern = "*"; // За замовчуванням, шаблон '*' означає всі файли

    if (argc >= 2) {
        directory = argv[1];
    }
    if (argc >= 3) {
        pattern = argv[2];
    }

    try {
        // Підрахунок кількості файлів в підкаталогах, що відповідають заданому шаблону
        int fileCount = countFiles(directory, pattern, include_hidden, include_readonly, include_archive);

        // Вивід результату
        std::cout << "Number of files matching pattern '" << pattern << "' in directory '" << directory << "': " << fileCount << std::endl;
        exit_code = 0; // Код завершення для успіху
    }
    catch (const fs::filesystem_error& e) {
        std::cerr << "Filesystem error: " << e.what() << std::endl;
        exit_code = 3; // Код завершення для помилок файлової системи
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        exit_code = 1; // Код завершення для загальної помилки
    }

    // Вивід коду завершення
    std::cout << "Exit code: " << exit_code << std::endl;
    return exit_code;
}