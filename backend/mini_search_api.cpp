#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <cctype>
#include <set>
#include <map>
#include <sstream>
#include <ctime>


using namespace std;

class MiniSearchEngineAPI {
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
                // Remove punctuation
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

    string escapeJson(const string& str) {
        string escaped;
        for (char c : str) {
            switch (c) {
                case '"': escaped += "\\\""; break;
                case '\\': escaped += "\\\\"; break;
                case '\b': escaped += "\\b"; break;
                case '\f': escaped += "\\f"; break;
                case '\n': escaped += "\\n"; break;
                case '\r': escaped += "\\r"; break;
                case '\t': escaped += "\\t"; break;
                default: escaped += c; break;
            }
        }
        return escaped;
    }

public:
    MiniSearchEngineAPI() { 
        buildDictionary(); 
    }

    // Search function that returns JSON
    string searchFiles(const string& searchTerm) {
        ofstream logfile("c++_debug.log", ios::app);
        logfile << "🎯 C++ IS PROCESSING SEARCH: " << searchTerm << " at " << time(0) << endl;
        logfile.close();
        vector<pair<string, int>> foundFiles;

        for (const string& filename : fileNames) {
            int count = countOccurrences(filename, searchTerm);
            if (count > 0) foundFiles.push_back({filename, count});
        }

        // Sort by occurrence count (descending)
        sort(foundFiles.begin(), foundFiles.end(), [](auto &a, auto &b) {
            return a.second > b.second;
        });

        // Create JSON response
        ostringstream json;
        json << "[";
        
        for (size_t i = 0; i < foundFiles.size(); ++i) {
            if (i > 0) json << ",";
            json << "{";
            json << "\"filename\":\"" << escapeJson(foundFiles[i].first) << "\",";
            json << "\"occurrences\":" << foundFiles[i].second;
            json << "}";
        }
        
        json << "]";
        return json.str();
    }

    // Get suggestions function that returns JSON
    string getSuggestions(const string& prefix) {
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

        // Create JSON response
        ostringstream json;
        json << "[";
        
        for (size_t i = 0; i < suggestions.size(); ++i) {
            if (i > 0) json << ",";
            json << "{";
            json << "\"text\":\"" << escapeJson(suggestions[i]) << "\",";
            json << "\"type\":\"search\"";
            json << "}";
        }
        
        json << "]";
        return json.str();
    }
};

int main(int argc, char* argv[]) {
    if (argc < 3) {
        cerr << "Usage: " << argv[0] << " <command> <query>" << endl;
        cerr << "Commands: search, suggestions" << endl;
        return 1;
    }

    string command = argv[1];
    string query = argv[2];

    MiniSearchEngineAPI engine;

    try {
        if (command == "search") {
            cout << engine.searchFiles(query) << endl;
        } else if (command == "suggestions") {
            cout << engine.getSuggestions(query) << endl;
        } else {
            cerr << "Unknown command: " << command << endl;
            return 1;
        }
    } catch (const exception& e) {
        cerr << "Error: " << e.what() << endl;
        return 1;
    }

    return 0;
}