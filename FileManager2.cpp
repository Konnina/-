#include <iostream>
#include <vector>
#include <string>
#include <filesystem>
#include <fstream>
#include <conio.h>
#include <windows.h>
#include <io.h>
#include <fcntl.h>

using namespace std;
namespace fs = std::filesystem;

struct Item
{
    fs::path path;
    bool isDir;
};

fs::path currentDir = fs::current_path();
vector<Item> items;
int cursor = 0;
int topIndex = 0;
fs::path marked;
bool hasMarked = false;

wstring toWide(const string& s)
{
    UINT cp = CP_UTF8;
    int n = MultiByteToWideChar(cp, MB_ERR_INVALID_CHARS, s.data(), (int)s.size(), 0, 0);

    if (!n)
    {
        cp = 1251;
        n = MultiByteToWideChar(cp, 0, s.data(), (int)s.size(), 0, 0);
    }

    wstring w(n, 0);
    MultiByteToWideChar(cp, 0, s.data(), (int)s.size(), &w[0], n);
    return w;
}

void load()
{
    items.clear();

    for (auto& p : fs::directory_iterator(currentDir))
        items.push_back({ p.path(), fs::is_directory(p.path()) });

    cursor = 0;
    topIndex = 0;
}

void draw()
{
    system("cls");

    wcout << "FILE MANAGER\n";
    wcout << "Current: " << currentDir.wstring() << "\n";

    if (hasMarked)
        wcout << "Marked: " << marked.wstring() << "\n";
    else
        wcout << "Marked: none\n";

    wcout << "----------------------------------------\n";

    int visible = 10;

    if (cursor < topIndex) topIndex = cursor;
    if (cursor >= topIndex + visible) topIndex = cursor - visible + 1;

    int last = min(topIndex + visible, (int)items.size());

    for (int i = topIndex; i < last; i++)
    {
        wcout << (i == cursor ? "> " : "  ");
        wcout << (hasMarked && items[i].path == marked ? "* " : "  ");
        wcout << (items[i].isDir ? "[D] " : "[F] ");
        wcout << items[i].path.filename().wstring() << "\n";
    }

    wcout << "\n";
    wcout << "Arrows - move | Enter - open dir | Backspace - parent\n";
    wcout << "Del - delete | Space - mark | F1 - copy | F2 - move | F3 - view | Esc - exit\n";
}

int key()
{
    int c = _getch();

    if (c == 0 || c == 224)
    {
        c = _getch();

        if (c == 72) return 1; // up
        if (c == 80) return 2; // down
        if (c == 83) return 3; // delete
        if (c == 59) return 4; // F1
        if (c == 60) return 5; // F2
        if (c == 61) return 6; // F3
    }

    if (c == 13) return 7; // enter
    if (c == 8)  return 8; // backspace
    if (c == 32) return 9; // space
    if (c == 27) return 10; // esc

    return 0;
}

void openDir()
{
    if (items.empty()) return;

    if (items[cursor].isDir)
    {
        currentDir = items[cursor].path;
        load();
    }
}

void goParent()
{
    currentDir = currentDir.parent_path();
    load();
}

void del()
{
    if (items.empty()) return;

    fs::remove_all(items[cursor].path);
    load();
}

void mark()
{
    if (items.empty()) return;

    marked = items[cursor].path;
    hasMarked = true;
}

void copyMarked()
{
    if (!hasMarked) return;

    fs::path dest = currentDir / marked.filename();

    if (fs::is_directory(marked))
        fs::copy(marked, dest, fs::copy_options::recursive | fs::copy_options::overwrite_existing);
    else
        fs::copy_file(marked, dest, fs::copy_options::overwrite_existing);

    load();
}

void moveMarked()
{
    if (!hasMarked) return;

    fs::path dest = currentDir / marked.filename();

    fs::rename(marked, dest);
    marked = dest;

    load();
}

void viewFile()
{
    if (items.empty() || items[cursor].isDir) return;

    system("cls");

    ifstream file(items[cursor].path, ios::binary);
    string text((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());

    wcout << L"VIEW: " << items[cursor].path.filename().wstring() << L"\n";
    wcout << L"----------------------------------------\n";
    wcout << toWide(text);
    wcout << L"\n\nPress any key to return...";

    _getch();

}

int main()
{
    _setmode(_fileno(stdout), _O_U16TEXT);
    _setmode(_fileno(stderr), _O_U16TEXT);

    load();

    while (true)
    {
        draw();

        switch (key())
        {
        case 1: if (cursor > 0) --cursor; break;
        case 2: if (cursor + 1 < (int)items.size()) ++cursor; break;
        case 3: del(); break;
        case 4: copyMarked(); break;
        case 5: moveMarked(); break;
        case 6: viewFile(); break;
        case 7: openDir(); break;
        case 8: goParent(); break;
        case 9: mark(); break;
        case 10: return 0;
        }
    }
}
