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

    // 🆕 NEW: Count word occurrences in a string
    int countWordOccurrences(const string &text, const string &searchTerm) {
        string lowerText = toLowerCase(text);
        string lowerSearchTerm = toLowerCase(searchTerm);
        
        int count = 0;
        size_t pos = lowerText.find(lowerSearchTerm);
        while (pos != string::npos) {
            count++;
            pos = lowerText.find(lowerSearchTerm, pos + lowerSearchTerm.length());
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

    // 🆕 NEW: Calculate Levenshtein distance between two strings
    int levenshteinDistance(const string &s1, const string &s2) {
        int m = s1.length();
        int n = s2.length();
        vector<vector<int>> dp(m + 1, vector<int>(n + 1));

        for (int i = 0; i <= m; i++) dp[i][0] = i;
        for (int j = 0; j <= n; j++) dp[0][j] = j;

        for (int i = 1; i <= m; i++) {
            for (int j = 1; j <= n; j++) {
                int cost = (s1[i - 1] == s2[j - 1]) ? 0 : 1;
                dp[i][j] = min({dp[i - 1][j] + 1,      // deletion
                               dp[i][j - 1] + 1,       // insertion
                               dp[i - 1][j - 1] + cost}); // substitution
            }
        }
        return dp[m][n];
    }

    // 🆕 NEW: Get best spelling correction
    string getSpellingCorrection(const string &searchTerm) {
        if (searchTerm.length() < 3) return ""; // Too short for correction
        
        string bestMatch;
        int bestDistance = 9999;
        string lowerTerm = toLowerCase(searchTerm);

        // Find closest match in dictionary
        for (const auto &word : dictionary) {
            int distance = levenshteinDistance(lowerTerm, word);
            
            // If exact match found, no correction needed
            if (distance == 0) return "";
            
            // Find the closest match (max 2 edits allowed)
            if (distance < bestDistance && distance <= 2) {
                bestDistance = distance;
                bestMatch = word;
            }
        }

        // Only return correction if it's significantly better
        if (!bestMatch.empty() && bestDistance <= 2 && bestDistance < searchTerm.length() / 2) {
            return bestMatch;
        }

        return "";
    }

    // 🆕 NEW: Extract sentences containing search term
    vector<pair<string, int>> extractSentencesWithTerm(const string &filename, const string &searchTerm) {
        vector<pair<string, int>> sentencesWithCounts;
        
        ifstream file(filename);
        if (!file.is_open()) {
            cout << " ❌ Cannot open file: " << filename << endl;
            return sentencesWithCounts;
        }

        // Read entire file content
        string content, line;
        while (getline(file, line)) {
            content += line + " ";
        }
        file.close();

        string lowerSearchTerm = toLowerCase(searchTerm);

        // Multiple sentence delimiters
        vector<char> sentenceDelimiters = {'.', '!', '?'};
        
        size_t start = 0;
        size_t end = 0;
        
        while (end < content.length()) {
            // Find the next sentence delimiter
            size_t nextDelimiter = string::npos;
            char foundDelimiter = '.';
            
            for (char delimiter : sentenceDelimiters) {
                size_t pos = content.find(delimiter, start);
                if (pos != string::npos && (nextDelimiter == string::npos || pos < nextDelimiter)) {
                    nextDelimiter = pos;
                    foundDelimiter = delimiter;
                }
            }
            
            if (nextDelimiter == string::npos) {
                // No more delimiters, take the rest as one sentence
                end = content.length();
            } else {
                end = nextDelimiter + 1; // Include the delimiter
            }
            
            // Extract sentence
            string sentence = content.substr(start, end - start);
            
            // Trim whitespace
            sentence.erase(0, sentence.find_first_not_of(" \t\n\r"));
            sentence.erase(sentence.find_last_not_of(" \t\n\r") + 1);
            
            // Check if sentence contains search term
            string lowerSentence = toLowerCase(sentence);
            if (!sentence.empty() && 
                lowerSentence.find(lowerSearchTerm) != string::npos) {
                
                // Count occurrences of search term in this sentence
                int occurrences = countWordOccurrences(sentence, searchTerm);
                sentencesWithCounts.push_back({sentence, occurrences});
            }
            
            if (nextDelimiter == string::npos) break;
            start = end;
        }

        // If no sentences found with delimiters, try line-by-line
        if (sentencesWithCounts.empty()) {
            ifstream file2(filename);
            if (file2.is_open()) {
                string line;
                while (getline(file2, line)) {
                    // Trim the line
                    line.erase(0, line.find_first_not_of(" \t\n\r"));
                    line.erase(line.find_last_not_of(" \t\n\r") + 1);
                    
                    if (!line.empty()) {
                        string lowerLine = toLowerCase(line);
                        if (lowerLine.find(lowerSearchTerm) != string::npos) {
                            int occurrences = countWordOccurrences(line, searchTerm);
                            sentencesWithCounts.push_back({line, occurrences});
                        }
                    }
                }
                file2.close();
            }
        }

        // Sort by occurrence count (descending)
        sort(sentencesWithCounts.begin(), sentencesWithCounts.end(), 
             [](const pair<string, int> &a, const pair<string, int> &b) {
                 return a.second > b.second;
             });

        return sentencesWithCounts;
    }

    // 🆕 NEW: Highlight search term in text
    string highlightTerm(const string &text, const string &searchTerm) {
        string result = text;
        string lowerText = toLowerCase(text);
        string lowerSearchTerm = toLowerCase(searchTerm);
        
        size_t pos = lowerText.find(lowerSearchTerm);
        while (pos != string::npos) {
            // Insert highlight markers
            result.insert(pos + searchTerm.length(), "[-]");
            result.insert(pos, "[+]");
            
            // Update positions for next search
            lowerText = toLowerCase(result);
            pos = lowerText.find(lowerSearchTerm, pos + searchTerm.length() + 4); // +4 for markers
        }
        
        return result;
    }

    // 🆕 NEW: Read full file content
    string readFullFile(const string &filename) {
        ifstream file(filename);
        if (!file.is_open()) {
            return "❌ Error: Cannot open file: " + filename;
        }

        stringstream buffer;
        buffer << file.rdbuf();
        file.close();
        
        return buffer.str();
    }

    // 🆕 NEW: Display full file content
    void displayFullFile(const string &filename) {
        string fileContent = readFullFile(filename);
        
        cout << "\n=============================================\n";
        cout << " FULL FILE CONTENT: " << filename << endl;
        cout << "=============================================\n";
        
        cout << "\n📊 File Statistics:\n";
        cout << "   Characters: " << fileContent.length() << endl;
        cout << "   Lines: " << count(fileContent.begin(), fileContent.end(), '\n') + 1 << endl;
        cout << "\n------------------------------------------------------------\n";
        cout << "CONTENT:\n";
        cout << "------------------------------------------------------------\n";
        
        cout << fileContent << endl;
        
        cout << "------------------------------------------------------------\n";
        cout << "END OF FILE\n";
        cout << "------------------------------------------------------------\n";
    }

    // 🆕 NEW: Check if string is a valid number
    bool isValidNumber(const string& str) {
        if (str.empty()) return false;
        for (char c : str) {
            if (!isdigit(c)) return false;
        }
        return true;
    }

public:
    MiniSearchEngine() { buildDictionary(); }

    void displayLogo() {
        cout << "\n=============================================\n";
        cout << "             MINI SEARCH ENGINE\n";
        cout << "       Your Personal Search Companion\n";
        cout << "=============================================\n";
        cout << "           Files Loaded: 20 | Ready!\n";
        cout << "        🔤 Spelling Correction: ACTIVE\n";
        cout << "        📝 Sentence Extraction: ACTIVE\n";
        cout << "        📄 Full File View: ACTIVE\n";
    }

    // 🆕 ENHANCED: Search with sentence extraction option
    void searchFiles(const string& searchTerm) {
        vector<pair<string, int>> foundFiles;
        string spellingCorrection = getSpellingCorrection(searchTerm);

        cout << "\n Searching for: \"" << searchTerm << "\"\n";
        
        if (!spellingCorrection.empty()) {
            cout << " 💡 Did you mean: \"" << spellingCorrection << "\"?\n";
            cout << "    (Searching for both terms...)\n";
        }
        
        cout << "------------------------------------------------------------\n";

        for (const string& filename : fileNames) {
            int count = countOccurrences(filename, searchTerm);
            if (count > 0) foundFiles.push_back({filename, count});
        }

        // Also search for corrected term if available
        if (!spellingCorrection.empty() && spellingCorrection != searchTerm) {
            for (const string& filename : fileNames) {
                int count = countOccurrences(filename, spellingCorrection);
                if (count > 0) {
                    // Check if file already exists in results
                    bool exists = false;
                    for (auto& file : foundFiles) {
                        if (file.first == filename) {
                            file.second += count; // Add occurrences
                            exists = true;
                            break;
                        }
                    }
                    if (!exists) {
                        foundFiles.push_back({filename, count});
                    }
                }
            }
        }

        if (foundFiles.empty()) {
            cout << "\n No results found for \"" << searchTerm << "\".\n";
            if (!spellingCorrection.empty()) {
                cout << " Try searching for: \"" << spellingCorrection << "\"\n";
            }
        } else {
            // 🔧 FIXED: C++11 compatible sort
            sort(foundFiles.begin(), foundFiles.end(), [](const pair<string, int> &a, const pair<string, int> &b) {
                return a.second > b.second;
            });

            cout << "\n Found " << foundFiles.size() << " result(s):\n\n";
            for (int i = 0; i < foundFiles.size(); i++) {
                cout << "   " << (i + 1) << ". " << foundFiles[i].first
                     << "  (Occurrences: " << foundFiles[i].second << ")\n";
            }
            
            // Show which results came from correction
            if (!spellingCorrection.empty()) {
                cout << "\n 📝 Note: Some results include matches for \"" << spellingCorrection << "\"\n";
            }

            // 🆕 NEW: Enhanced options menu
            cout << "\n------------------------------------------------------------\n";
            cout << " 📖 OPTIONS:\n";
            cout << "   Enter file number to view sentences\n";
            cout << "   Enter 'f' + file number (e.g., f1) to view full file\n";
            cout << "   Enter 0 to continue\n";
            cout << "   Your choice: ";
            
            string choice;
            cin >> choice;
            cin.ignore();
            
            if (choice == "0") {
                return;
            }
            else if (!choice.empty() && (choice[0] == 'f' || choice[0] == 'F')) {
                // View full file - SAFE VERSION
                if (choice.length() > 1) {
                    string numberPart = choice.substr(1);
                    if (isValidNumber(numberPart)) {
                        int fileNumber = stoi(numberPart);
                        if (fileNumber >= 1 && fileNumber <= foundFiles.size()) {
                            string selectedFile = foundFiles[fileNumber - 1].first;
                            displayFullFile(selectedFile);
                            cout << "\n Press Enter to return to main menu...";
                            cin.ignore();
                        } else {
                            cout << " ❌ Invalid file number. Please choose between 1 and " << foundFiles.size() << ".\n";
                            cout << " Press Enter to continue...";
                            cin.ignore();
                        }
                    } else {
                        cout << " ❌ Invalid format. Please use 'f1', 'f2', etc. (e.g., f1 for file 1).\n";
                        cout << " Press Enter to continue...";
                        cin.ignore();
                    }
                } else {
                    cout << " ❌ Please specify a file number. Format: 'f1', 'f2', etc.\n";
                    cout << " Press Enter to continue...";
                    cin.ignore();
                }
            }
            else {
                // View sentences - SAFE VERSION
                if (isValidNumber(choice)) {
                    int fileChoice = stoi(choice);
                    if (fileChoice >= 1 && fileChoice <= foundFiles.size()) {
                        string selectedFile = foundFiles[fileChoice - 1].first;
                        showSentences(selectedFile, searchTerm);
                    } else if (fileChoice == 0) {
                        return;
                    } else {
                        cout << " ❌ Invalid file number. Please choose between 1 and " << foundFiles.size() << ".\n";
                        cout << " Press Enter to continue...";
                        cin.ignore();
                    }
                } else {
                    cout << " ❌ Invalid input. Please enter a number (1-" << foundFiles.size() << ") or 'f1', 'f2', etc.\n";
                    cout << " Press Enter to continue...";
                    cin.ignore();
                }
            }
        }
        cout << "------------------------------------------------------------\n";
    }

    // 🆕 NEW: Display sentences from a file
    void showSentences(const string& filename, const string& searchTerm) {
        cout << "\n=============================================\n";
        cout << " EXTRACTING SENTENCES FROM: " << filename << endl;
        cout << " SEARCH TERM: \"" << searchTerm << "\"\n";
        cout << "=============================================\n";

        vector<pair<string, int>> sentences = extractSentencesWithTerm(filename, searchTerm);
        
        if (sentences.empty()) {
            cout << "\n ❌ No sentences found containing \"" << searchTerm << "\"\n";
            cout << "    (Only complete sentences ending with . ! ? are considered)\n";
        } else {
            cout << "\n 📊 Found " << sentences.size() << " sentence(s) containing \"" << searchTerm << "\"\n";
            cout << "    (Sorted by most occurrences first)\n";
            cout << "------------------------------------------------------------\n\n";

            for (int i = 0; i < sentences.size(); i++) {
                cout << " [" << (i + 1) << "] Occurrences: " << sentences[i].second << endl;
                string highlighted = highlightTerm(sentences[i].first, searchTerm);
                cout << "     " << highlighted << endl;
                cout << endl;
                
                // Pause every 3 sentences for readability
                if ((i + 1) % 3 == 0 && i != sentences.size() - 1) {
                    cout << " --- Press Enter to continue ---";
                    cin.ignore();
                    cout << endl;
                }
            }
            
            cout << "------------------------------------------------------------\n";
            
            // 🆕 NEW: Option to view full file after sentences
            cout << "\n 📄 OPTIONS:\n";
            cout << "   Press 'f' to view full file content\n";
            cout << "   Press any other key to return to main menu\n";
            cout << "   Your choice: ";
            
            char choice;
            cin >> choice;
            cin.ignore();
            
            if (choice == 'f' || choice == 'F') {
                displayFullFile(filename);
                cout << "\n Press Enter to return to main menu...";
                cin.ignore();
            }
        }
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
        
        // 🆕 NEW: Option to view full file from file list
        cout << "\n 📄 Enter file number to view full content, or 0 to return: ";
        string choice;
        cin >> choice;
        cin.ignore();
        
        if (isValidNumber(choice)) {
            int fileChoice = stoi(choice);
            if (fileChoice >= 1 && fileChoice <= fileNames.size()) {
                displayFullFile(fileNames[fileChoice - 1]);
                cout << "\n Press Enter to return to main menu...";
                cin.ignore();
            } else if (fileChoice != 0) {
                cout << " ❌ Invalid file number. Please choose between 1 and " << fileNames.size() << ".\n";
                cout << " Press Enter to continue...";
                cin.ignore();
            }
        } else {
            cout << " ❌ Invalid input. Please enter a number.\n";
            cout << " Press Enter to continue...";
            cin.ignore();
        }
    }

    // Interactive typing with suggestions
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