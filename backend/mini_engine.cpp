#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <cctype>
#include <set>
#include <map>
#include <conio.h> // for _getch() on Windows

using namespace std;

class MiniSearchEngine {
private:
    vector<string> fileNames = {
    "Docs/technology.txt", "Docs/sports.txt", "Docs/food.txt", "Docs/nature.txt", "Docs/education.txt",
    "Docs/music.txt", "Docs/travel.txt", "Docs/science.txt", "Docs/movies.txt", "Docs/health.txt",
    "Docs/history.txt", "Docs/books.txt", "Docs/games.txt", "Docs/weather.txt", "Docs/animals.txt",
    "Docs/colors.txt", "Docs/transportation.txt", "Docs/friendship.txt", "Docs/work.txt", "Docs/hobbies.txt"
};


    set<string> dictionary;
    map<string, int> wordFrequency;

    string toLowerCase(string str) {
        transform(str.begin(), str.end(), str.begin(), ::tolower);
        return str;
    }

    int countOccurrences(const string& filename, const string& searchTerm) {
        ifstream file(filename);
        if (!file.is_open()) return 0;

        string content, line;
        while (getline(file, line)) content += line + " ";
        file.close();

        string lowerContent = toLowerCase(content);
        string lowerSearchTerm = toLowerCase(searchTerm);

        int count = 0;
        size_t pos = lowerContent.find(lowerSearchTerm);
        while (pos != string::npos) {
            count++;
            pos = lowerContent.find(lowerSearchTerm, pos + lowerSearchTerm.length());
        }
        return count;
    }

    void buildDictionary() {
        for (const string& filename : fileNames) {
            ifstream file(filename);
            if (!file.is_open()) continue;

            string word;
            while (file >> word) {
                word.erase(remove_if(word.begin(), word.end(),
                    [](char c) { return ispunct(c); }), word.end());
                word = toLowerCase(word);
                if (!word.empty()) {
                    dictionary.insert(word);
                    wordFrequency[word]++;
                }
            }
            file.close();
        }
    }

    vector<string> getSuggestions(const string& prefix) {
        vector<string> suggestions;
        string lowerPrefix = toLowerCase(prefix);

        for (const auto& word : dictionary) {
            if (word.rfind(lowerPrefix, 0) == 0)
                suggestions.push_back(word);
        }

        sort(suggestions.begin(), suggestions.end(), [&](const string& a, const string& b) {
            return wordFrequency[a] > wordFrequency[b];
        });

        if (suggestions.size() > 5)
            suggestions.resize(5);
        return suggestions;
    }

public:
    MiniSearchEngine() { buildDictionary(); }

    void displayLogo() {
        cout << "\n=============================================\n";
        cout << "             MINI SEARCH ENGINE\n";
        cout << "       Your Personal Search Companion\n";
        cout << "=============================================\n";
        cout << "           Files Loaded: 20 | Ready!\n";
    }

    void searchFiles(const string& searchTerm) {
        vector<pair<string, int>> foundFiles;

        cout << "\n Searching for: \"" << searchTerm << "\"\n";
        cout << "------------------------------------------------------------\n";

        for (const string& filename : fileNames) {
            int count = countOccurrences(filename, searchTerm);
            if (count > 0) foundFiles.push_back({filename, count});
        }

        if (foundFiles.empty()) {
            cout << "\n No results found for \"" << searchTerm << "\".\n";
        } else {
            sort(foundFiles.begin(), foundFiles.end(), [](auto &a, auto &b) {
                return a.second > b.second;
            });

            cout << "\n Found " << foundFiles.size() << " result(s):\n\n";
            for (int i = 0; i < foundFiles.size(); i++) {
                cout << "   " << (i + 1) << ". " << foundFiles[i].first
                     << "  (Occurrences: " << foundFiles[i].second << ")\n";
            }
        }
        cout << "------------------------------------------------------------\n";
    }

    void showMenu() {
        cout << "\n MENU:\n";
        cout << "   1. Search\n";
        cout << "   2. View All Files\n";
        cout << "   3. Exit\n";
        cout << "\nChoose an option (1-3): ";
    }

    void showAllFiles() {
        cout << "\n Available Files:\n";
        cout << "------------------------------------------------------------\n";
        for (int i = 0; i < fileNames.size(); i++) {
            cout << "   " << (i + 1) << ". " << fileNames[i] << endl;
        }
        cout << "------------------------------------------------------------\n";
    }

    // 🔥 Live typing + numeric suggestion selection
    string interactiveTyping() {
        string input = "";
        char ch;

        cout << "\n Type your search term (Press ENTER to search):\n\n";
        cout << "> ";

        while (true) {
            ch = _getch(); // instant key read

            if (ch == 13) { // ENTER
                cout << endl;
                return input;
            }
            else if (ch == 8) { // BACKSPACE
                if (!input.empty()) {
                    input.pop_back();
                    cout << "\b \b";
                }
            }
            else if (ch >= '1' && ch <= '5') { // Numeric selection
                vector<string> suggestions = getSuggestions(input);
                int index = ch - '1';
                if (index < suggestions.size()) {
                    string selected = suggestions[index];
                    // Autocomplete the word
                    input = selected;
                    cout << "\r> " << input << "          \n"; // clear old line
                    cout << "\n Selected: " << selected << "\n";
                    return input; // auto-search after selection
                }
            }
            else {
                input += ch;
                cout << ch;
            }

            // Show suggestions below input
            cout << "\n\n Suggestions:\n";
            vector<string> suggestions = getSuggestions(input);
            if (suggestions.empty()) {
                cout << "   (no suggestions)\n";
            } else {
                for (int i = 0; i < suggestions.size(); i++) {
                    cout << "   " << (i + 1) << ". " << suggestions[i] << endl;
                }
                cout << "\n (Press 1–" << suggestions.size() << " to choose a suggestion)";
            }

            cout << "\n\n> " << input;
        }
    }

    void run() {
        displayLogo();

        int choice;
        while (true) {
            showMenu();
            cin >> choice;
            cin.ignore();

            switch (choice) {
                case 1: {
                    string searchTerm = interactiveTyping();
                    if (!searchTerm.empty()) searchFiles(searchTerm);
                    else cout << "Please enter a valid search term.\n";
                    break;
                }
                case 2:
                    showAllFiles();
                    break;
                case 3:
                    cout << "\nThank you for using Mini Search Engine!\n";
                    cout << "Goodbye!\n";
                    return;
                default:
                    cout << "Invalid option! Please choose 1, 2, or 3.\n";
                    break;
            }
        }
    }
};

int main() {
    MiniSearchEngine engine;
    engine.run();
    return 0;
}
