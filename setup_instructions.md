# Mini Google Search Engine - Setup Instructions

## 📁 Project Structure

Your project directory should look like this:
```
D:\SY\DS lab\Mini Google Search Engine 2\
├── frontend/                    # Frontend files
│   ├── index.html              # Main HTML page
│   ├── styles.css              # CSS styling
│   ├── script.js               # JavaScript functionality
│   └── logo.png                # Logo image
├── backend/                     # Backend files
│   ├── mini_engine.cpp         # Your original C++ code
│   ├── mini_search_api.cpp     # API version of C++ backend
│   └── mini_search_api.exe     # Compiled API executable
├── server/                      # Server files
│   └── server.py               # Python web server
├── Docs/                        # Your document files
│   ├── technology.txt
│   ├── sports.txt
│   ├── food.txt
│   └── ... (all your other txt files)
└── README.md                   # This file
```

## 🚀 Step-by-Step Setup Guide

### Step 1: Create Directory Structure
1. Create these folders in your main project directory:
   ```
   mkdir frontend
   mkdir backend
   mkdir server
   ```

### Step 2: Place Files in Correct Locations

**Frontend Files (put in `frontend/` folder):**
- `index.html`
- `styles.css` 
- `script.js`
- `logo.png` (download from the generated image above)

**Backend Files (put in `backend/` folder):**
- `mini_search_api.cpp` (new API version)
- Keep your original `mini_engine.cpp` as backup

**Server Files (put in `server/` folder):**
- `server.py`

**Document Files (keep in `Docs/` folder):**
- All your existing `.txt` files should remain in the `Docs/` folder

### Step 3: Download and Save Logo
1. Right-click on the logo image generated above
2. Save it as `logo.png` in your `frontend/` folder

### Step 4: Compile C++ Backend

**Option A: Using g++ (if you have MinGW or GCC installed)**
```bash
cd backend
g++ -o mini_search_api mini_search_api.cpp
```

**Option B: Using Visual Studio (if you have it installed)**
1. Open Visual Studio
2. Create a new C++ Console Application project
3. Replace the default code with `mini_search_api.cpp`
4. Build the project (Ctrl+Shift+B)
5. Copy the generated `.exe` file to your `backend/` folder

**Option C: Using Dev-C++ or Code::Blocks**
1. Open your IDE
2. Create new project with `mini_search_api.cpp`
3. Compile and build
4. Copy executable to `backend/` folder

### Step 5: Install Python (if not already installed)
1. Download Python from https://python.org
2. Make sure to check "Add Python to PATH" during installation
3. Verify installation by opening command prompt and typing: `python --version`

### Step 6: Start the Server

1. **Open Command Prompt/Terminal**
2. **Navigate to your project directory:**
   ```bash
   cd "D:\SY\DS lab\Mini Google Search Engine 2"
   ```

3. **Copy frontend files to main directory for the server:**
   ```bash
   copy frontend\*.* .
   ```

4. **Start the Python server:**
   ```bash
   python server.py
   ```
   
   Or if you have Python 3 specifically:
   ```bash
   python3 server.py
   ```

5. **You should see output like:**
   ```
   🚀 Mini Google Search Engine Server started!
   📍 Server running at: http://localhost:8000
   🌐 Open your browser and visit: http://localhost:8000
   ```

### Step 7: Access Your Search Engine
1. Open your web browser
2. Go to: `http://localhost:8000`
3. You should see your Mini Google Search Engine!

## 🔧 Alternative Server Start Methods

**Method 1: Using the server from main directory**
```bash
cd "D:\SY\DS lab\Mini Google Search Engine 2"
python server\server.py
```

**Method 2: If you want to run on a different port**
```bash
python server.py localhost 3000
```

**Method 3: To make it accessible from other computers on your network**
```bash
python server.py 0.0.0.0 8000
```

## 🆕 New Features Added

### 1. Search History
- Automatically saves your last 10 searches
- Click the "Search History" button to view past searches
- Click on any history item to search again
- Delete individual items or clear all history

### 2. Auto-suggestions
- Start typing and get real-time suggestions
- Suggestions are based on available content in your files
- Use arrow keys to navigate suggestions
- Press Enter to select

### 3. Responsive Design
- Works on desktop, tablet, and mobile devices
- Google-like clean interface
- Smooth animations and transitions

## 🛠️ Troubleshooting

### Problem: "Python is not recognized"
**Solution:** Add Python to your system PATH or use full path to python.exe

### Problem: Server won't start
**Solution:** 
1. Make sure no other program is using port 8000
2. Try a different port: `python server.py localhost 3001`

### Problem: C++ backend not working
**Solution:** 
1. The server has fallback functionality and will work without C++ backend
2. Make sure `mini_search_api.exe` is in the same directory as `server.py`
3. Ensure your `Docs/` folder is in the same directory

### Problem: Files not loading
**Solution:** 
1. Make sure all files are in the correct directories
2. Check that `index.html`, `styles.css`, and `script.js` are in the same folder as `server.py`

### Problem: Search not working
**Solution:**
1. Ensure your `Docs/` folder contains all the `.txt` files
2. Check the browser console (F12) for any JavaScript errors

## 📊 Testing Your Setup

1. **Test Basic Functionality:**
   - Open `http://localhost:8000`
   - Try searching for "technology" or "sports"
   - Check if you get results

2. **Test Search History:**
   - Perform a few searches
   - Click "Search History" button
   - Verify your searches are saved

3. **Test Suggestions:**
   - Start typing in the search box
   - Check if suggestions appear
   - Try selecting a suggestion

## 🎨 Customization Options

### Change Colors
Edit `styles.css` and modify the CSS custom properties at the top of the file.

### Add More File Types
Edit `mini_search_api.cpp` and add more file extensions to the `fileNames` vector.

### Change Port
Start server with: `python server.py localhost [YOUR_PORT]`

## 🔄 Development Workflow

1. **Make changes to frontend:** Edit HTML/CSS/JS files
2. **Make changes to backend:** Edit `mini_search_api.cpp` and recompile
3. **Restart server:** Stop with Ctrl+C and run `python server.py` again
4. **Refresh browser:** Press F5 to see changes

## 📝 Next Steps for Enhancement

1. **Add file content preview**
2. **Implement advanced search filters**
3. **Add search result highlighting**
4. **Implement user authentication**
5. **Add search analytics dashboard**

---

🎉 **Congratulations!** Your Mini Google Search Engine is now ready to use!

For any issues, make sure all files are in the correct locations and that Python is properly installed.