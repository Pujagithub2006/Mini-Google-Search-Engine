#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <cctype>
#include <set>
#include <map>
#include <sstream>
#include <json/json.h> // You'll need to install libjsoncpp-dev

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

public:
    MiniSearchEngineAPI() { 
        buildDictionary(); 
    }

    // Search function that returns JSON
    string searchFiles(const string& searchTerm) {
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
        Json::Value jsonResponse(Json::arrayValue);
        
        for (const auto& result : foundFiles) {
            Json::Value item;
            item["filename"] = result.first;
            item["occurrences"] = result.second;
            jsonResponse.append(item);
        }

        Json::StreamWriterBuilder builder;
        return Json::writeString(builder, jsonResponse);
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
        Json::Value jsonResponse(Json::arrayValue);
        
        for (const string& suggestion : suggestions) {
            Json::Value item;
            item["text"] = suggestion;
            item["type"] = "search";
            jsonResponse.append(item);
        }

        Json::StreamWriterBuilder builder;
        return Json::writeString(builder, jsonResponse);
    }

    // Process HTTP-like requests
    void processRequest() {
        string method, path, query;
        
        // Read HTTP method and path
        getline(cin, method);
        
        if (method.find("POST /api/search") != string::npos) {
            // Handle search request
            string jsonInput;
            getline(cin, jsonInput);
            
            Json::Value root;
            Json::Reader reader;
            
            if (reader.parse(jsonInput, root)) {
                string searchTerm = root["query"].asString();
                string result = searchFiles(searchTerm);
                
                cout << "HTTP/1.1 200 OK\r\n";
                cout << "Content-Type: application/json\r\n";
                cout << "Access-Control-Allow-Origin: *\r\n";
                cout << "Content-Length: " << result.length() << "\r\n";
                cout << "\r\n";
                cout << result << endl;
            }
        }
        else if (method.find("GET /api/suggestions") != string::npos) {
            // Extract query parameter
            size_t pos = method.find("q=");
            if (pos != string::npos) {
                string queryParam = method.substr(pos + 2);
                size_t endPos = queryParam.find(' ');
                if (endPos != string::npos) {
                    queryParam = queryParam.substr(0, endPos);
                }
                
                // URL decode (basic)
                replace(queryParam.begin(), queryParam.end(), '+', ' ');
                
                string result = getSuggestions(queryParam);
                
                cout << "HTTP/1.1 200 OK\r\n";
                cout << "Content-Type: application/json\r\n";
                cout << "Access-Control-Allow-Origin: *\r\n";
                cout << "Content-Length: " << result.length() << "\r\n";
                cout << "\r\n";
                cout << result << endl;
            }
        }
    }
};

int main() {
    MiniSearchEngineAPI engine;
    
    // Process requests in a loop
    while (true) {
        try {
            engine.processRequest();
        } catch (const exception& e) {
            cerr << "Error processing request: " << e.what() << endl;
        }
    }
    
    return 0;
}