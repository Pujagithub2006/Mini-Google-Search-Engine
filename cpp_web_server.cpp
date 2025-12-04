#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <cctype>
#include <set>
#include <map>
#include <ctime>
#include <regex>
#include <sys/stat.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <dirent.h>
#endif

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#include <direct.h>
#pragma comment(lib, "ws2_32.lib")
#define mkdir(path, mode) _mkdir(path)
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/stat.h>
#endif

using namespace std;

// Enhanced User management structure with first name, last name
struct User {
    string firstName;
    string lastName;
    string email;
    string password;
    vector<string> searchHistory;
};

class MiniSearchEngine
{
private:
    vector<string> fileNames;

    // Enhanced: User management with persistence
    vector<User> users;
    string currentUserEmail;
    
    // Data persistence files
    const string USERS_FILE = "data/users.json";
    const string SEARCH_HISTORY_FILE = "data/search_history.json";

    void ensureDataDirectory() {
        mkdir("data", 0755);
        mkdir("uploads", 0755);
        mkdir("Docs", 0755);
    }

    // Auto-detect all .txt files in Docs folder
    void loadFilesFromDocsFolder()
    {
        fileNames.clear();

#ifdef _WIN32
        // Windows implementation
        WIN32_FIND_DATAA findData;
        HANDLE hFind = FindFirstFileA("Docs\\*.txt", &findData);

        if (hFind != INVALID_HANDLE_VALUE)
        {
            do
            {
                string filename = "Docs/" + string(findData.cFileName);
                fileNames.push_back(filename);
                
            } while (FindNextFileA(hFind, &findData) != 0);
            FindClose(hFind);
        }
        else
        {
            cout << "⚠️ Warning: Docs folder not found or empty" << endl;
        }
#else
        // Linux/Mac implementation
        DIR *dir = opendir("Docs");
        if (dir != nullptr)
        {
            struct dirent *entry;
            while ((entry = readdir(dir)) != nullptr)
            {
                string filename = entry->d_name;
                // Check if file ends with .txt
                if (filename.length() > 4 &&
                    filename.substr(filename.length() - 4) == ".txt")
                {
                    string fullPath = "Docs/" + filename;
                    fileNames.push_back(fullPath);
                    
                }
            }
            closedir(dir);
        }
        else
        {
            cout << "⚠️ Warning: Docs folder not found or empty" << endl;
        }
#endif

        cout << "✅ Loaded " << fileNames.size() << " files from Docs folder" << endl;
    }

    string toLowerCase(string str)
    {
        transform(str.begin(), str.end(), str.begin(), ::tolower);
        return str;
    }

    // Enhanced: Find ONLY words starting with search term
    int countOccurrences(const string &filename, const string &searchTerm)
    {
        ifstream file(filename);
        if (!file.is_open())
            return 0;

        string content, line;
        while (getline(file, line))
            content += line + " ";
        file.close();

        string lowerContent = toLowerCase(content);
        string lowerSearchTerm = toLowerCase(searchTerm);

        int count = 0;
        
        // Split content into words and check each word
        istringstream iss(lowerContent);
        string word;
        while (iss >> word) {
            // Remove punctuation from word
            word.erase(remove_if(word.begin(), word.end(), 
                [](char c) { return ispunct(c); }), word.end());
            
            // Check if word starts with search term (exact start match)
            if (word.find(lowerSearchTerm) == 0) {
                count++;
            }
        }
        return count;
    }

    // Enhanced: Search by filename with better matching
    int countFilenameMatches(const string &filename, const string &searchTerm)
    {
        // Extract just the filename without path and extension
        size_t lastSlash = filename.find_last_of("/\\");
        string justFilename = (lastSlash != string::npos) ? 
                             filename.substr(lastSlash + 1) : filename;
        
        // Remove file extension
        size_t dotPos = justFilename.find_last_of(".");
        if (dotPos != string::npos) {
            justFilename = justFilename.substr(0, dotPos);
        }
        
        string lowerFilename = toLowerCase(justFilename);
        string lowerSearchTerm = toLowerCase(searchTerm);
        
        // Check if filename contains search term
        if (lowerFilename.find(lowerSearchTerm) != string::npos) {
            return 2; // Give higher weight to filename matches
        }
        return 0;
    }

    // Enhanced Levenshtein distance for better spelling correction
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

    // UPDATED: "Did you mean" feature using ONLY Levenshtein distance
    string getSpellingCorrection(const string &searchTerm) {
        if (searchTerm.length() < 3) return ""; // Too short for meaningful correction
        
        string lowerTerm = toLowerCase(searchTerm);
        
        set<string> dictionary;

        // Build dictionary from all files
        for (const string &filename : fileNames) {
            ifstream file(filename);
            if (!file.is_open()) continue;

            string word;
            while (file >> word) {
                word.erase(remove_if(word.begin(), word.end(),
                    [](char c) { return ispunct(c); }), word.end());
                word = toLowerCase(word);
                if (!word.empty() && word.length() > 2) {
                    dictionary.insert(word);
                }
            }
            file.close();
        }

        string bestMatch;
        int bestDistance = 9999;

        // Find closest match in dictionary using Levenshtein distance
        for (const auto &word : dictionary) {
            int distance = levenshteinDistance(lowerTerm, word);
            
            // If exact match found, no correction needed
            if (distance == 0) return "";
            
            // Consider matches with edit distance <= 2 and same first letter
            if (distance <= 2 && 
                !word.empty() && !lowerTerm.empty() && 
                word[0] == lowerTerm[0]) {
                
                if (distance < bestDistance) {
                    bestDistance = distance;
                    bestMatch = word;
                }
            }
        }

        // Only return correction if it's significantly better
        if (!bestMatch.empty() && bestDistance <= 2) {
            // Capitalize first letter if original had it capitalized
            string corrected = bestMatch;
            if (isupper(searchTerm[0])) {
                corrected[0] = toupper(corrected[0]);
            }
            
            cout << "🔤 Spelling correction: '" << searchTerm << "' -> '" << corrected << "' (distance: " << bestDistance << ")" << endl;
            return corrected;
        }

        return "";
    }

    // Enhanced: Count words starting with search term in text
    int countWordOccurrences(const string &text, const string &searchTerm)
    {
        string lowerText = toLowerCase(text);
        string lowerSearchTerm = toLowerCase(searchTerm);

        int count = 0;
        
        // Split text into words and check each word
        istringstream iss(lowerText);
        string word;
        while (iss >> word) {
            // Remove punctuation from word
            word.erase(remove_if(word.begin(), word.end(), 
                [](char c) { return ispunct(c); }), word.end());
            
            // Check if word starts with search term
            if (word.find(lowerSearchTerm) == 0) {
                count++;
            }
        }
        return count;
    }

    // Data persistence methods
    void loadUsers() {
        ifstream file(USERS_FILE);
        if (!file.is_open()) {
            cout << "ℹ️ No existing users file found. Starting with demo users." << endl;
            // Initialize with demo users
            users.push_back({"John", "Doe", "user1@demo.com", "password123", {}});
            users.push_back({"Jane", "Smith", "user2@demo.com", "password123", {}});
            saveUsers();
            return;
        }

        string jsonContent((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
        file.close();

        // Simple JSON parsing for users
        size_t pos = 0;
        while ((pos = jsonContent.find("{\"firstName\":", pos)) != string::npos) {
            User user;
            
            // Extract firstName
            size_t start = jsonContent.find(":\"", pos) + 2;
            size_t end = jsonContent.find("\"", start);
            user.firstName = jsonContent.substr(start, end - start);
            
            // Extract lastName
            start = jsonContent.find("lastName\":\"", end) + 11;
            end = jsonContent.find("\"", start);
            user.lastName = jsonContent.substr(start, end - start);
            
            // Extract email
            start = jsonContent.find("email\":\"", end) + 8;
            end = jsonContent.find("\"", start);
            user.email = jsonContent.substr(start, end - start);
            
            // Extract password
            start = jsonContent.find("password\":\"", end) + 11;
            end = jsonContent.find("\"", start);
            user.password = jsonContent.substr(start, end - start);
            
            users.push_back(user);
            pos = end;
        }
        cout << "✅ Loaded " << users.size() << " users from disk" << endl;
    }

    void saveUsers() {
        ofstream file(USERS_FILE);
        if (!file.is_open()) {
            cerr << "❌ Failed to save users data" << endl;
            return;
        }

        file << "[";
        for (size_t i = 0; i < users.size(); ++i) {
            if (i > 0) file << ",";
            file << "{";
            file << "\"firstName\":\"" << escapeJsonString(users[i].firstName) << "\",";
            file << "\"lastName\":\"" << escapeJsonString(users[i].lastName) << "\",";
            file << "\"email\":\"" << escapeJsonString(users[i].email) << "\",";
            file << "\"password\":\"" << escapeJsonString(users[i].password) << "\"";
            file << "}";
        }
        file << "]";
        file.close();
        cout << "💾 Saved " << users.size() << " users to disk" << endl;
    }

    void loadSearchHistory() {
        ifstream file(SEARCH_HISTORY_FILE);
        if (!file.is_open()) {
            cout << "ℹ️ No existing search history file found." << endl;
            return;
        }

        string jsonContent((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
        file.close();

        // Parse search history
        size_t pos = 0;
        while ((pos = jsonContent.find("{\"email\":", pos)) != string::npos) {
            string email;
            size_t start = jsonContent.find(":\"", pos) + 2;
            size_t end = jsonContent.find("\"", start);
            email = jsonContent.substr(start, end - start);
            
            // Find the user and load their history
            for (auto &user : users) {
                if (user.email == email) {
                    // Look for history array
                    size_t historyStart = jsonContent.find("history\":[", end);
                    if (historyStart != string::npos) {
                        historyStart += 10;
                        size_t historyEnd = jsonContent.find("]", historyStart);
                        string historyStr = jsonContent.substr(historyStart, historyEnd - historyStart);
                        
                        // Parse individual search queries
                        size_t queryPos = 0;
                        while ((queryPos = historyStr.find("\"", queryPos)) != string::npos) {
                            size_t queryStart = queryPos + 1;
                            size_t queryEnd = historyStr.find("\"", queryStart);
                            if (queryEnd == string::npos) break;
                            
                            string query = historyStr.substr(queryStart, queryEnd - queryStart);
                            user.searchHistory.push_back(query);
                            queryPos = queryEnd + 1;
                        }
                    }
                    break;
                }
            }
            pos = end;
        }
        cout << "✅ Loaded search history from disk" << endl;
    }

    void saveSearchHistory() {
        ofstream file(SEARCH_HISTORY_FILE);
        if (!file.is_open()) {
            cerr << "❌ Failed to save search history" << endl;
            return;
        }

        file << "[";
        bool firstUser = true;
        for (const auto &user : users) {
            if (user.searchHistory.empty()) continue;
            
            if (!firstUser) file << ",";
            firstUser = false;
            
            file << "{";
            file << "\"email\":\"" << escapeJsonString(user.email) << "\",";
            file << "\"history\":[";
            
            for (size_t i = 0; i < user.searchHistory.size(); ++i) {
                if (i > 0) file << ",";
                file << "\"" << escapeJsonString(user.searchHistory[i]) << "\"";
            }
            file << "]";
            file << "}";
        }
        file << "]";
        file.close();
        cout << "💾 Saved search history to disk" << endl;
    }

public:
    MiniSearchEngine() {
        ensureDataDirectory();
        
        loadUsers();
        loadSearchHistory();
        currentUserEmail = ""; // No user logged in initially
    }

    ~MiniSearchEngine() {
        saveUsers();
        saveSearchHistory();
    }

    void refreshFiles() {
        cout << "🔄 Refreshing file list..." << endl;
        loadFilesFromDocsFolder();
    }

    // FIXED: REAL OCR Text Extraction with proper Tesseract path
    string extractTextFromImage(const string &imagePath) {
        cout << "🖼️ REAL OCR Processing image: " << imagePath << endl;
        
        // Try multiple Tesseract paths - FIXED for your installation
        vector<string> tesseractPaths = {
            "tesseract-ocr/tesseract.exe",  // Your installed location
            "tesseract.exe",                // Global installation
            "C:/Program Files/Tesseract-OCR/tesseract.exe" // Default Windows install
        };
        
        string tesseractCmd;
        for (const auto& path : tesseractPaths) {
            // Check if file exists
            ifstream testFile(path);
            if (testFile.good()) {
                tesseractCmd = path;
                testFile.close();
                cout << "✅ Found Tesseract at: " << path << endl;
                break;
            }
        }
        
        if (tesseractCmd.empty()) {
            cout << "❌ Tesseract not found in any expected location." << endl;
            cout << "📥 Please ensure Tesseract is installed in 'tesseract-ocr' folder" << endl;
            return "tesseract_not_installed";
        }
        
        // Create output filename
        string outputFile = "uploads/ocr_output_" + to_string(time(0));
        string command;
        
        #ifdef _WIN32
            // Windows command - FIXED: Use quotes for paths with spaces
            command = "\"" + tesseractCmd + "\" \"" + imagePath + "\" \"" + outputFile + "\" -l eng 2>nul";
        #else
            // Linux/Mac command
            command = "\"" + tesseractCmd + "\" \"" + imagePath + "\" \"" + outputFile + "\" -l eng 2>/dev/null";
        #endif
        
        cout << "🔧 Running OCR command: " << command << endl;
        
        // Execute OCR command
        int result = system(command.c_str());
        
        if (result != 0) {
            cout << "❌ OCR command failed with result: " << result << endl;
            cout << "💡 Try running this command manually to debug: " << command << endl;
            return "ocr_processing_failed";
        }
        
        // Read the OCR result
        string outputFilePath = outputFile + ".txt"; // Tesseract adds .txt extension
        ifstream ocrFile(outputFilePath);
        if (!ocrFile.is_open()) {
            cout << "❌ Cannot open OCR output file: " << outputFilePath << endl;
            return "cannot_read_ocr_output";
        }
        
        string extractedText;
        string line;
        while (getline(ocrFile, line)) {
            extractedText += line + " ";
        }
        ocrFile.close();
        
        // Clean up temporary file
        remove(outputFilePath.c_str());
        
        // Clean and process the extracted text
        extractedText = regex_replace(extractedText, regex("\\s+"), " ");
        extractedText = regex_replace(extractedText, regex("^\\s+|\\s+$"), "");
        
        if (extractedText.empty()) {
            extractedText = "no_text_detected";
        }
        
        cout << "✅ REAL OCR Result: '" << extractedText << "'" << endl;
        return extractedText;
    }

    // Enhanced: User authentication with proper validation and persistence
    bool loginUser(const string &email, const string &password) {
        for (auto &user : users) {
            if (user.email == email && user.password == password) {
                currentUserEmail = email;
                cout << "🔐 User logged in: " << email << endl;
                saveUsers(); // Persist login state
                return true;
            }
        }
        return false;
    }

    // Enhanced: User registration with persistence
    bool registerUser(const string &firstName, const string &lastName, const string &email, const string &password) {
        // Check if user already exists
        for (const auto &user : users) {
            if (user.email == email) {
                cout << "❌ Registration failed - email already exists: " << email << endl;
                return false;
            }
        }
        
        // Add user with first name and last name
        users.push_back({firstName, lastName, email, password, {}});
        cout << "✅ New user registered: " << firstName << " " << lastName << " (" << email << ")" << endl;
        saveUsers(); // Persist immediately
        return true;
    }

    // Check if user is logged in
    bool isUserLoggedIn() {
        return !currentUserEmail.empty();
    }

    // Add to search history for current user with persistence
    void addToSearchHistory(const string &query) {
        if (currentUserEmail.empty() || query.empty()) return;
        
        for (auto &user : users) {
            if (user.email == currentUserEmail) {
                // Remove if already exists (to avoid duplicates)
                user.searchHistory.erase(
                    remove(user.searchHistory.begin(), user.searchHistory.end(), query),
                    user.searchHistory.end()
                );
                
                // Add to front of history
                user.searchHistory.insert(user.searchHistory.begin(), query);
                
                // Keep only last 20 searches
                if (user.searchHistory.size() > 20) {
                    user.searchHistory.pop_back();
                }
                cout << "📝 Added to history for " << currentUserEmail << ": " << query << endl;
                
                // Save history to disk
                saveSearchHistory();
                break;
            }
        }
    }

    // Get search history for current user
    string getSearchHistoryJSON() {
        if (currentUserEmail.empty()) {
            return "[]";
        }
        
        for (const auto &user : users) {
            if (user.email == currentUserEmail) {
                ostringstream json;
                json << "[";
                
                for (size_t i = 0; i < user.searchHistory.size(); ++i) {
                    if (i > 0) json << ",";
                    json << "{";
                    json << "\"query\":\"" << escapeJsonString(user.searchHistory[i]) << "\"";
                    json << "}";
                }
                
                json << "]";
                cout << "📋 Returning search history for " << currentUserEmail << ": " << user.searchHistory.size() << " items" << endl;
                return json.str();
            }
        }
        return "[]";
    }

    // Clear search history for current user with persistence
    void clearSearchHistory() {
        if (currentUserEmail.empty()) {
            cout << "❌ Cannot clear history - no user logged in" << endl;
            return;
        }
        
        for (auto &user : users) {
            if (user.email == currentUserEmail) {
                int historySize = user.searchHistory.size();
                user.searchHistory.clear();
                cout << "🗑️ Cleared " << historySize << " search history items for: " << currentUserEmail << endl;
                
                // Save cleared history to disk
                saveSearchHistory();
                break;
            }
        }
    }

    // Enhanced: Sentence extraction with login requirement
    string extractSentencesWithTerm(const string &filename, const string &searchTerm)
    {
        // FIXED: Require login for sentence extraction
        loadFilesFromDocsFolder();
        if (currentUserEmail.empty()) {
            cout << "❌ Sentence extraction requires login" << endl;
            return "{\"error\":\"Login required to extract sentences\"}";
        }
        
        cout << "📝 Extracting sentences from: " << filename << " for term: '" << searchTerm << "'" << endl;

        ifstream file(filename);
        if (!file.is_open())
        {
            cout << "❌ Cannot open file: " << filename << endl;
            return "[]";
        }

        stringstream buffer;
        buffer << file.rdbuf();
        string content = buffer.str();
        file.close();

        vector<pair<string, int>> sentencesWithCounts;
        string lowerSearchTerm = toLowerCase(searchTerm);

        vector<char> sentenceDelimiters = {'.', '!', '?'};

        size_t start = 0;
        size_t end = 0;

        while (end < content.length())
        {
            size_t nextDelimiter = string::npos;
            char foundDelimiter = '.';

            for (char delimiter : sentenceDelimiters)
            {
                size_t pos = content.find(delimiter, start);
                if (pos != string::npos && (nextDelimiter == string::npos || pos < nextDelimiter))
                {
                    nextDelimiter = pos;
                    foundDelimiter = delimiter;
                }
            }

            if (nextDelimiter == string::npos)
            {
                end = content.length();
            }
            else
            {
                end = nextDelimiter + 1;
            }

            string sentence = content.substr(start, end - start);
            sentence.erase(0, sentence.find_first_not_of(" \t\n\r"));
            sentence.erase(sentence.find_last_not_of(" \t\n\r") + 1);

            string lowerSentence = toLowerCase(sentence);
            if (!sentence.empty() &&
                lowerSentence.find(lowerSearchTerm) != string::npos)
            {
                int occurrences = countWordOccurrences(sentence, searchTerm);
                if (occurrences > 0) { // Only add if word starts with search term
                    sentencesWithCounts.push_back({sentence, occurrences});
                    cout << "✅ Found sentence: '" << sentence << "' (" << occurrences << " occurrences)" << endl;
                }
            }

            if (nextDelimiter == string::npos)
                break;
            start = end;
        }

        if (sentencesWithCounts.empty())
        {
            cout << "🔍 No sentences found with delimiters, trying line-by-line..." << endl;

            ifstream file2(filename);
            if (file2.is_open())
            {
                string line;
                while (getline(file2, line))
                {
                    line.erase(0, line.find_first_not_of(" \t\n\r"));
                    line.erase(line.find_last_not_of(" \t\n\r") + 1);

                    if (!line.empty())
                    {
                        string lowerLine = toLowerCase(line);
                        if (lowerLine.find(lowerSearchTerm) != string::npos)
                        {
                            int occurrences = countWordOccurrences(line, searchTerm);
                            if (occurrences > 0) {
                                sentencesWithCounts.push_back({line, occurrences});
                                cout << "✅ Found line: '" << line << "' (" << occurrences << " occurrences)" << endl;
                            }
                        }
                    }
                }
                file2.close();
            }
        }

        sort(sentencesWithCounts.begin(), sentencesWithCounts.end(),
             [](const pair<string, int> &a, const pair<string, int> &b)
             {
                 return a.second > b.second;
             });

        ostringstream json;
        json << "[";

        for (size_t i = 0; i < sentencesWithCounts.size(); ++i)
        {
            if (i > 0)
                json << ",";
            json << "{";
            json << "\"sentence\":\"" << escapeJsonString(sentencesWithCounts[i].first) << "\",";
            json << "\"occurrences\":" << sentencesWithCounts[i].second;
            json << "}";
        }

        json << "]";

        cout << "📊 Found " << sentencesWithCounts.size() << " sentences with term '" << searchTerm << "'" << endl;
        return json.str();
    }

    string escapeJsonString(const string &input)
    {
        ostringstream escaped;
        for (char c : input)
        {
            switch (c)
            {
            case '"':
                escaped << "\\\"";
                break;
            case '\\':
                escaped << "\\\\";
                break;
            case '\b':
                escaped << "\\b";
                break;
            case '\f':
                escaped << "\\f";
                break;
            case '\n':
                escaped << "\\n";
                break;
            case '\r':
                escaped << "\\r";
                break;
            case '\t':
                escaped << "\\t";
                break;
            default:
                escaped << c;
                break;
            }
        }
        return escaped.str();
    }

    // UPDATED: Enhanced search with Levenshtein-based "Did you mean" feature
    string searchJSONWithCorrection(const string &searchTerm) {
        cout << "🎯 C++ SEARCHING: " << searchTerm << endl;
        loadFilesFromDocsFolder();
        vector<pair<string, int>> foundFiles;
        string spellingCorrection = getSpellingCorrection(searchTerm);

        // Enhanced: Search both content and filenames
        for (const string &filename : fileNames) {
            int contentCount = countOccurrences(filename, searchTerm);
            int filenameCount = countFilenameMatches(filename, searchTerm);
            int totalCount = contentCount + filenameCount;
            
            if (totalCount > 0 || filenameCount > 0) { // Show files even if only filename matches
                foundFiles.push_back({filename, totalCount});
                cout << "📁 Found: " << filename << " (" << contentCount << " content + " << filenameCount << " filename = " << totalCount << " occurrences)" << endl;
            }
        }

        // Add to search history if user is logged in
        if (!searchTerm.empty()) {
            addToSearchHistory(searchTerm);
        }

        ostringstream json;
        json << "{";
        
        // UPDATED: "Did you mean" feature using only Levenshtein distance
        if (!spellingCorrection.empty()) {
            // Show correction if few or no results found
            if (foundFiles.empty() || foundFiles.size() < 3) {
                json << "\"correction\":\"" << spellingCorrection << "\",";
                cout << "💡 Did you mean: " << spellingCorrection << endl;
            } else {
                json << "\"correction\":\"\",";
            }
        } else {
            json << "\"correction\":\"\",";
        }
        
        json << "\"originalQuery\":\"" << searchTerm << "\",";
        
        json << "\"results\":[";
        
        sort(foundFiles.begin(), foundFiles.end(), [](const pair<string, int> &a, const pair<string, int> &b)
             { return a.second > b.second; });

        for (size_t i = 0; i < foundFiles.size(); ++i) {
            if (i > 0) json << ",";
            json << "{";
            json << "\"filename\":\"" << foundFiles[i].first << "\",";
            json << "\"occurrences\":" << foundFiles[i].second;
            json << "}";
        }

        json << "]}";
        
        cout << "📊 Search completed. Results: " << foundFiles.size() << ", Correction: " << (spellingCorrection.empty() ? "None" : spellingCorrection) << endl;
        return json.str();
    }

    string searchJSON(const string &searchTerm) {
        return searchJSONWithCorrection(searchTerm);
    }

    string suggestJSON(const string &prefix)
    {
        cout << "💡 C++ SUGGESTIONS for: " << prefix << endl;
        loadFilesFromDocsFolder();
        set<string> dictionary;
        map<string, int> wordFrequency;

        for (const string &filename : fileNames)
        {
            ifstream file(filename);
            if (!file.is_open())
                continue;

            string word;
            while (file >> word)
            {
                word.erase(remove_if(word.begin(), word.end(),
                                     [](char c)
                                     { return ispunct(c); }),
                           word.end());
                word = toLowerCase(word);
                if (!word.empty() && word.length() > 2)
                {
                    dictionary.insert(word);
                    wordFrequency[word]++;
                }
            }
            file.close();
        }

        vector<string> suggestions;
        string lowerPrefix = toLowerCase(prefix);

        for (const auto &word : dictionary)
        {
            if (word.find(lowerPrefix) == 0)
            {
                suggestions.push_back(word);
            }
        }

        sort(suggestions.begin(), suggestions.end(), [&](const string &a, const string &b)
             { return wordFrequency[a] > wordFrequency[b]; });

        if (suggestions.size() > 5)
            suggestions.resize(5);

        cout << "📋 Suggestions found: ";
        for (const auto &s : suggestions)
        {
            cout << s << " ";
        }
        cout << endl;

        ostringstream json;
        json << "[";

        for (size_t i = 0; i < suggestions.size(); ++i)
        {
            if (i > 0)
                json << ",";
            json << "{";
            json << "\"text\":\"" << suggestions[i] << "\",";
            json << "\"type\":\"search\"";
            json << "}";
        }

        json << "]";
        return json.str();
    }

    // Get current user email
    string getCurrentUser() {
        return currentUserEmail;
    }

    // Get current user full info
    string getCurrentUserInfo() {
        if (currentUserEmail.empty()) {
            return "{\"loggedIn\":false}";
        }
        
        for (const auto &user : users) {
            if (user.email == currentUserEmail) {
                ostringstream json;
                json << "{";
                json << "\"loggedIn\":true,";
                json << "\"firstName\":\"" << user.firstName << "\",";
                json << "\"lastName\":\"" << user.lastName << "\",";
                json << "\"email\":\"" << user.email << "\"";
                json << "}";
                return json.str();
            }
        }
        return "{\"loggedIn\":false}";
    }

    // Logout user with persistence
    void logoutUser() {
        cout << "👋 User logged out: " << currentUserEmail << endl;
        currentUserEmail = "";
        saveUsers(); // Persist logout state
    }
};

class CppWebServer
{
private:
    int server_fd;
    MiniSearchEngine engine;

    string readFile(const string &filename)
    {
        ifstream file(filename);
        if (!file)
            return "";
        stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }

    // Save uploaded image with proper base64 decoding
    string saveUploadedImage(const string &base64Data, const string &filename) {
        cout << "💾 Saving uploaded image, base64 length: " << base64Data.length() << endl;
        
        // Create unique filename
        string imagePath = "uploads/" + filename + "_" + to_string(time(0)) + ".png";
        
        // Base64 decoding
        string base64_chars = 
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz"
            "0123456789+/";
        
        vector<int> decoding(256, -1);
        for (int i = 0; i < 64; i++) {
            decoding[base64_chars[i]] = i;
        }
        
        string decodedData;
        int val = 0, valb = -8;
        
        for (unsigned char c : base64Data) {
            if (c == '=') break; // Padding character
            if (decoding[c] == -1) continue; // Skip invalid characters
            
            val = (val << 6) + decoding[c];
            valb += 6;
            
            if (valb >= 0) {
                decodedData.push_back(char((val >> valb) & 0xFF));
                valb -= 8;
            }
        }
        
        cout << "📊 Decoded image size: " << decodedData.size() << " bytes" << endl;
        
        // Save the decoded image
        ofstream file(imagePath, ios::binary);
        if (file) {
            file.write(decodedData.c_str(), decodedData.size());
            file.close();
            cout << "✅ Saved image to: " << imagePath << endl;
        } else {
            cout << "❌ Failed to save image: " << imagePath << endl;
            throw runtime_error("Cannot save image file");
        }
        
        return imagePath;
    }

public:
    bool start(int port = 8000)
    {
#ifdef _WIN32
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
        {
            cerr << "WSAStartup failed" << endl;
            return false;
        }
#endif

        server_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (server_fd < 0)
        {
            cerr << "Socket creation failed" << endl;
            return false;
        }

        sockaddr_in address;
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons(port);

        if (bind(server_fd, (sockaddr *)&address, sizeof(address)) < 0)
        {
            cerr << "Bind failed" << endl;
            return false;
        }

        if (listen(server_fd, 10) < 0)
        {
            cerr << "Listen failed" << endl;
            return false;
        }

        cout << "🚀 C++ WEB SERVER running on http://localhost:" << port << endl;
        cout << "🔍 Enhanced Search Engine with REAL OCR!" << endl;
        cout << "✅ UPDATED: Pure Levenshtein 'Did you mean' Feature" << endl;
        cout << "✅ FIXED: Real Tesseract OCR Integration" << endl;
        cout << "✅ NEW: Intelligent Spelling Correction" << endl;
        cout << "✅ NEW: Data Persistence - Users & History Saved" << endl;
        cout << "👤 Demo: Try searching 'athrav' to see 'Did you mean: atharv'" << endl;
        return true;
    }

    void run()
    {
        while (true)
        {
            sockaddr_in client_addr;
#ifdef _WIN32
            int addrlen = sizeof(client_addr);
#else
            socklen_t addrlen = sizeof(client_addr);
#endif

            int client_fd = accept(server_fd, (sockaddr *)&client_addr, &addrlen);
            if (client_fd < 0)
                continue;

            char buffer[16384] = {0};
            int bytes_received = recv(client_fd, buffer, 16384, 0);
            
            if (bytes_received <= 0) {
#ifdef _WIN32
                closesocket(client_fd);
#else
                close(client_fd);
#endif
                continue;
            }

            string request(buffer, bytes_received);
            string response;

            cout << "📨 Request: " << request.substr(0, min(200, (int)request.length())) << "..." << endl;

            // Enhanced: User login endpoint
            if (request.find("POST /api/login") != string::npos) {
                size_t json_start = request.find("{\"email\"");
                if (json_start != string::npos) {
                    string json_str = request.substr(json_start);
                    size_t email_start = json_str.find(":\"") + 2;
                    size_t email_end = json_str.find("\"", email_start);
                    size_t pass_start = json_str.find("password\":\"") + 11;
                    size_t pass_end = json_str.find("\"", pass_start);
                    
                    if (email_start != string::npos && email_end != string::npos && 
                        pass_start != string::npos && pass_end != string::npos) {
                        string email = json_str.substr(email_start, email_end - email_start);
                        string password = json_str.substr(pass_start, pass_end - pass_start);
                        
                        bool success = engine.loginUser(email, password);
                        
                        ostringstream json;
                        json << "{\"success\":" << (success ? "true" : "false") << "}";
                        
                        response = "HTTP/1.1 200 OK\r\n";
                        response += "Content-Type: application/json\r\n";
                        response += "Access-Control-Allow-Origin: *\r\n";
                        response += "Content-Length: " + to_string(json.str().length()) + "\r\n";
                        response += "\r\n";
                        response += json.str();
                    }
                }
            }
            // Enhanced: User registration endpoint
            else if (request.find("/api/register") != string::npos && request.find("POST") != string::npos)
            {
                size_t json_start = request.find("{");
                if (json_start != string::npos)
                {
                    string json_str = request.substr(json_start);
                    
                    string firstName, lastName, email, password, confirmPassword;

                    // Extract fields
                    vector<string> firstNames = {"firstName", "firstname", "first_name", "first"};
                    for (const string &field : firstNames) {
                        string searchStr = "\"" + field + "\":\"";
                        size_t start = json_str.find(searchStr);
                        if (start != string::npos) {
                            start += searchStr.length();
                            size_t end = json_str.find("\"", start);
                            if (end != string::npos) {
                                firstName = json_str.substr(start, end - start);
                                break;
                            }
                        }
                    }

                    vector<string> lastNames = {"lastName", "lastname", "last_name", "last"};
                    for (const string &field : lastNames) {
                        string searchStr = "\"" + field + "\":\"";
                        size_t start = json_str.find(searchStr);
                        if (start != string::npos) {
                            start += searchStr.length();
                            size_t end = json_str.find("\"", start);
                            if (end != string::npos) {
                                lastName = json_str.substr(start, end - start);
                                break;
                            }
                        }
                    }

                    vector<string> emails = {"email", "Email"};
                    for (const string &field : emails) {
                        string searchStr = "\"" + field + "\":\"";
                        size_t start = json_str.find(searchStr);
                        if (start != string::npos) {
                            start += searchStr.length();
                            size_t end = json_str.find("\"", start);
                            if (end != string::npos) {
                                email = json_str.substr(start, end - start);
                                break;
                            }
                        }
                    }

                    vector<string> passwords = {"password", "Password"};
                    for (const string &field : passwords) {
                        string searchStr = "\"" + field + "\":\"";
                        size_t start = json_str.find(searchStr);
                        if (start != string::npos) {
                            start += searchStr.length();
                            size_t end = json_str.find("\"", start);
                            if (end != string::npos) {
                                password = json_str.substr(start, end - start);
                                break;
                            }
                        }
                    }

                    vector<string> confirmPasswords = {"confirmPassword", "confirmpassword", "confirm_password", "confirmPassword", "confirm"};
                    for (const string &field : confirmPasswords) {
                        string searchStr = "\"" + field + "\":\"";
                        size_t start = json_str.find(searchStr);
                        if (start != string::npos) {
                            start += searchStr.length();
                            size_t end = json_str.find("\"", start);
                            if (end != string::npos) {
                                confirmPassword = json_str.substr(start, end - start);
                                break;
                            }
                        }
                    }

                    bool success = false;
                    string errorMessage = "";

                    if (firstName.empty() || lastName.empty() || email.empty() || 
                        password.empty() || confirmPassword.empty()) {
                        errorMessage = "All fields are required";
                    }
                    else if (password != confirmPassword) {
                        errorMessage = "Passwords do not match";
                    }
                    else if (password.length() < 6) {
                        errorMessage = "Password must be at least 6 characters";
                    }
                    else {
                        success = engine.registerUser(firstName, lastName, email, password);
                        if (!success) {
                            errorMessage = "Email already exists";
                        }
                    }

                    ostringstream json;
                    json << "{";
                    json << "\"success\":" << (success ? "true" : "false") << ",";
                    json << "\"errorMessage\":\"" << errorMessage << "\"";
                    json << "}";

                    response = "HTTP/1.1 200 OK\r\n";
                    response += "Content-Type: application/json\r\n";
                    response += "Access-Control-Allow-Origin: *\r\n";
                    response += "Content-Length: " + to_string(json.str().length()) + "\r\n";
                    response += "\r\n";
                    response += json.str();
                }
            }
            // Enhanced: Get current user info endpoint
            else if (request.find("GET /api/current-user") != string::npos) {
                string userInfo = engine.getCurrentUserInfo();
                
                response = "HTTP/1.1 200 OK\r\n";
                response += "Content-Type: application/json\r\n";
                response += "Access-Control-Allow-Origin: *\r\n";
                response += "Content-Length: " + to_string(userInfo.length()) + "\r\n";
                response += "\r\n";
                response += userInfo;
            }
            // Enhanced: Logout endpoint
            else if (request.find("POST /api/logout") != string::npos) {
                engine.logoutUser();
                
                ostringstream json;
                json << "{\"success\":true}";
                
                response = "HTTP/1.1 200 OK\r\n";
                response += "Content-Type: application/json\r\n";
                response += "Access-Control-Allow-Origin: *\r\n";
                response += "Content-Length: " + to_string(json.str().length()) + "\r\n";
                response += "\r\n";
                response += json.str();
            }
            // Enhanced: Get search history endpoint
            else if (request.find("GET /api/search-history") != string::npos) {
                string history = engine.getSearchHistoryJSON();
                
                response = "HTTP/1.1 200 OK\r\n";
                response += "Content-Type: application/json\r\n";
                response += "Access-Control-Allow-Origin: *\r\n";
                response += "Content-Length: " + to_string(history.length()) + "\r\n";
                response += "\r\n";
                response += history;
            }
            // Enhanced: Clear search history endpoint
            else if (request.find("POST /api/clear-history") != string::npos) {
                engine.clearSearchHistory();
                
                ostringstream json;
                json << "{\"success\":true}";
                
                response = "HTTP/1.1 200 OK\r\n";
                response += "Content-Type: application/json\r\n";
                response += "Access-Control-Allow-Origin: *\r\n";
                response += "Content-Length: " + to_string(json.str().length()) + "\r\n";
                response += "\r\n";
                response += json.str();
            }
            // FIXED: Image search endpoint with REAL OCR
            else if (request.find("POST /api/search-by-image") != string::npos)
            {
                cout << "🖼️ Image search request received" << endl;

                // Find the start of the base64 data (after headers)
                size_t header_end = request.find("\r\n\r\n");
                if (header_end == string::npos)
                {
                    cout << "❌ No data found in request" << endl;
                    response = "HTTP/1.1 400 Bad Request\r\nContent-Type: application/json\r\nAccess-Control-Allow-Origin: *\r\n\r\n{\"error\":\"No image data found\"}";
                }
                else
                {
                    string imageData = request.substr(header_end + 4); // Skip \r\n\r\n

                    cout << "📥 Received image data, length: " << imageData.length() << endl;

                    if (imageData.length() < 100)
                    {
                        cout << "❌ Image data too short: " << imageData << endl;
                        response = "HTTP/1.1 400 Bad Request\r\nContent-Type: application/json\r\nAccess-Control-Allow-Origin: *\r\n\r\n{\"error\":\"Image data too short\"}";
                    }
                    else
                    {
                        try
                        {
                            string imagePath = saveUploadedImage(imageData, "search_image");
                            string extractedText = engine.extractTextFromImage(imagePath);

                            cout << "🔍 Searching for: " << extractedText << endl;
                            string searchResults = engine.searchJSONWithCorrection(extractedText);

                            // Parse the search results to extract just the results array
                            size_t results_start = searchResults.find("\"results\":");
                            if (results_start != string::npos)
                            {
                                results_start += 10; // Move past "\"results\":"
                                size_t results_end = searchResults.find("]", results_start) + 1;

                                ostringstream enhancedResults;
                                enhancedResults << "{";
                                enhancedResults << "\"success\":true,";
                                enhancedResults << "\"ocrText\":\"" << engine.escapeJsonString(extractedText) << "\",";
                                enhancedResults << "\"imagePath\":\"" << imagePath << "\",";
                                enhancedResults << "\"results\":" << searchResults.substr(results_start, results_end - results_start);
                                enhancedResults << "}";

                                response = "HTTP/1.1 200 OK\r\n";
                                response += "Content-Type: application/json\r\n";
                                response += "Access-Control-Allow-Origin: *\r\n";
                                response += "Content-Length: " + to_string(enhancedResults.str().length()) + "\r\n";
                                response += "\r\n";
                                response += enhancedResults.str();

                                cout << "✅ Image search completed successfully" << endl;
                            }
                            else
                            {
                                throw runtime_error("Invalid search results format");
                            }
                        }
                        catch (const exception &e)
                        {
                            cout << "❌ Image processing error: " << e.what() << endl;
                            ostringstream json;
                            json << "{\"error\":\"Image processing failed: " << e.what() << "\"}";

                            response = "HTTP/1.1 500 Internal Server Error\r\n";
                            response += "Content-Type: application/json\r\n";
                            response += "Access-Control-Allow-Origin: *\r\n";
                            response += "Content-Length: " + to_string(json.str().length()) + "\r\n";
                            response += "\r\n";
                            response += json.str();
                        }
                    }
                }
            }
            // Enhanced: Search endpoint with "Did you mean" feature
            else if (request.find("POST /api/search") != string::npos)
            {
                size_t json_start = request.find("{\"query\"");
                if (json_start != string::npos)
                {
                    string json_str = request.substr(json_start);
                    size_t query_start = json_str.find(":\"") + 2;
                    size_t query_end = json_str.find("\"", query_start);
                    if (query_start != string::npos && query_end != string::npos && query_end > query_start) {
                        string query = json_str.substr(query_start, query_end - query_start);

                        cout << "🔍 C++ Processing search: " << query << endl;
                        string results = engine.searchJSONWithCorrection(query);

                        response = "HTTP/1.1 200 OK\r\n";
                        response += "Content-Type: application/json\r\n";
                        response += "Access-Control-Allow-Origin: *\r\n";
                        response += "Content-Length: " + to_string(results.length()) + "\r\n";
                        response += "\r\n";
                        response += results;
                    }
                }
            }
            // Fixed: Sentence extraction endpoint - requires login
            else if (request.find("POST /api/extract-sentences") != string::npos)
            {
                if (!engine.isUserLoggedIn()) {
                    response = "HTTP/1.1 401 Unauthorized\r\n";
                    response += "Content-Type: application/json\r\n";
                    response += "Access-Control-Allow-Origin: *\r\n";
                    response += "Content-Length: 46\r\n";
                    response += "\r\n";
                    response += "{\"error\":\"Login required for this feature\"}";
                }
                else {
                    size_t json_start = request.find("{\"filename\"");
                    if (json_start != string::npos)
                    {
                        string json_str = request.substr(json_start);
                        
                        size_t file_start = json_str.find("filename\":\"") + 11;
                        size_t file_end = json_str.find("\"", file_start);
                        string filename = json_str.substr(file_start, file_end - file_start);
                        
                        size_t term_start = json_str.find("searchTerm\":\"") + 13;
                        size_t term_end = json_str.find("\"", term_start);
                        string searchTerm = json_str.substr(term_start, term_end - term_start);

                        cout << "📝 C++ Extracting sentences from: " << filename << " for term: " << searchTerm << endl;
                        
                        string sentences = engine.extractSentencesWithTerm(filename, searchTerm);

                        response = "HTTP/1.1 200 OK\r\n";
                        response += "Content-Type: application/json\r\n";
                        response += "Access-Control-Allow-Origin: *\r\n";
                        response += "Content-Length: " + to_string(sentences.length()) + "\r\n";
                        response += "\r\n";
                        response += sentences;
                    }
                }
            }
            else if (request.find("GET /api/suggestions") != string::npos)
            {
                size_t q_pos = request.find("q=");
                if (q_pos != string::npos)
                {
                    size_t query_start = q_pos + 2;
                    size_t query_end = request.find(" HTTP/1.1", query_start);
                    if (query_end == string::npos)
                        query_end = request.find(" ", query_start);
                    if (query_end != string::npos && query_end > query_start) {
                        string query = request.substr(query_start, query_end - query_start);

                        size_t plus_pos;
                        while ((plus_pos = query.find('+')) != string::npos)
                        {
                            query.replace(plus_pos, 1, " ");
                        }

                        cout << "💡 C++ Processing suggestions: '" << query << "'" << endl;
                        string suggestions = engine.suggestJSON(query);

                        response = "HTTP/1.1 200 OK\r\n";
                        response += "Content-Type: application/json\r\n";
                        response += "Access-Control-Allow-Origin: *\r\n";
                        response += "Content-Length: " + to_string(suggestions.length()) + "\r\n";
                        response += "\r\n";
                        response += suggestions;

                        cout << "📤 Sending suggestions: " << suggestions << endl;
                    }
                }
            }
            else if (request.find("GET / ") != string::npos || request.find("GET /index.html") != string::npos)
            {
                string html = readFile("index.html");
                if (html.empty())
                {
                    html = "<h1>C++ Search Engine Working!</h1><p>Enhanced with REAL OCR and Levenshtein-based 'Did you mean' feature!</p>";
                }

                response = "HTTP/1.1 200 OK\r\n";
                response += "Content-Type: text/html\r\n";
                response += "Content-Length: " + to_string(html.length()) + "\r\n";
                response += "\r\n";
                response += html;
            }
            else if (request.find("GET /") == 0)
            {
                size_t method_end = request.find(" HTTP/");
                if (method_end == string::npos) {
                    method_end = request.find(" ");
                }
                
                if (method_end != string::npos && method_end > 5) {
                    string path = request.substr(4, method_end - 4);
                    
                    if (!path.empty() && path[0] == '/') {
                        path = path.substr(1);
                    }
                    if (path.empty()) {
                        path = "index.html";
                    }
                    
                    if (path.find("..") != string::npos) {
                        path = "index.html";
                    }

                    cout << "📁 Serving file: " << path << endl;

                    ifstream file(path, ios::binary);
                    if (file)
                    {
                        string content((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
                        file.close();

                        string content_type = "text/plain";
                        if (path.find(".css") != string::npos)
                            content_type = "text/css";
                        else if (path.find(".js") != string::npos)
                            content_type = "application/javascript";
                        else if (path.find(".png") != string::npos)
                            content_type = "image/png";
                        else if (path.find(".jpg") != string::npos || path.find(".jpeg") != string::npos)
                            content_type = "image/jpeg";
                        else if (path.find(".html") != string::npos)
                            content_type = "text/html";

                        response = "HTTP/1.1 200 OK\r\n";
                        response += "Content-Type: " + content_type + "\r\n";
                        response += "Content-Length: " + to_string(content.length()) + "\r\n";
                        response += "\r\n";
                        response += content;

                        cout << "✅ Served: " << path << " (" << content.length() << " bytes)" << endl;
                    }
                    else
                    {
                        cout << "❌ File not found: " << path << endl;
                        response = "HTTP/1.1 404 Not Found\r\n\r\nFile not found: " + path;
                    }
                } else {
                    response = "HTTP/1.1 400 Bad Request\r\n\r\nInvalid request";
                }
            }
            else
            {
                response = "HTTP/1.1 404 Not Found\r\n\r\nPage not found";
            }

            send(client_fd, response.c_str(), response.length(), 0);
#ifdef _WIN32
            closesocket(client_fd);
#else
            close(client_fd);
#endif
        }
    }
};

int main()
{
    CppWebServer server;
    if (server.start(8000))
    {
        cout << "🎯 Enhanced C++ Search Engine Ready!" << endl;
        cout << "📁 Serving from current directory" << endl;
        cout << "🔍 REAL OCR: Tesseract integration for image text extraction" << endl;
        cout << "💡 UPDATED: Pure Levenshtein 'Did you mean' feature" << endl;
        cout << "👤 Demo: Try searching 'athrav' to see correction to 'atharv'" << endl;
        cout << "💾 Data persistence: Users and search history saved to disk" << endl;
        server.run();
    }
    else
    {
        cerr << "❌ Failed to start C++ server" << endl;
    }
    return 0;
}