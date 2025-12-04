// Global variables
let currentUser = null;
let isLoggedIn = false;
let searchHistory = [];

// DOM Elements
const searchInput = document.getElementById('searchInput');
const suggestionsDropdown = document.getElementById('suggestionsDropdown');
const resultsSection = document.getElementById('resultsSection');
const resultsContainer = document.getElementById('resultsContainer');
const resultsCount = document.getElementById('resultsCount');
const loadingSpinner = document.getElementById('loadingSpinner');
const searchHistorySection = document.getElementById('searchHistory');
const historyItems = document.getElementById('historyItems');

// Initialize the application
document.addEventListener('DOMContentLoaded', function() {
    initializeApp();
    setupEventListeners();
    checkCurrentUser();
});

// Initialize application
function initializeApp() {
    createModals();
    updateUIForLoginState();
}

// Setup event listeners
function setupEventListeners() {
    // Search input events
    if (searchInput) {
        searchInput.addEventListener('input', handleSearchInput);
        searchInput.addEventListener('focus', handleSearchFocus);
        searchInput.addEventListener('blur', handleSearchBlur);
        searchInput.addEventListener('keydown', handleSearchKeydown);
    }
    
    // Click outside to close dropdowns
    document.addEventListener('click', function(e) {
        if (!e.target.closest('.search-container')) {
            hideSuggestions();
        }
        if (!e.target.closest('.user-menu')) {
            hideThreeDotsMenu();
        }
        if (!e.target.closest('.search-history') && !e.target.closest('.three-dots-menu')) {
            if (searchHistorySection) searchHistorySection.style.display = 'none';
        }
    });
    
    // Camera button for image search
    const cameraBtn = document.querySelector('.camera-btn');
    if (cameraBtn) {
        cameraBtn.addEventListener('click', handleImageSearch);
    }
    
    // Microphone button for voice search
    const micBtn = document.querySelector('.mic-btn');
    if (micBtn) {
        micBtn.addEventListener('click', handleVoiceSearch);
    }
}

// Check current user on page load
async function checkCurrentUser() {
    try {
        const response = await fetch('/api/current-user');
        const userInfo = await response.json();
        
        if (userInfo.loggedIn) {
            currentUser = userInfo;
            isLoggedIn = true;
            await loadSearchHistory();
            updateUIForLoginState();
        }
    } catch (error) {
        console.error('Error checking current user:', error);
    }
}

// Update UI based on login state
function updateUIForLoginState() {
    const authButtons = document.querySelector('.auth-buttons');
    const userAvatar = document.querySelector('.user-avatar');
    
    if (isLoggedIn && currentUser) {
        if (authButtons) authButtons.style.display = 'none';
        if (userAvatar) {
            userAvatar.textContent = currentUser.firstName ? currentUser.firstName.charAt(0).toUpperCase() : 'U';
            userAvatar.style.backgroundColor = '#4285f4';
            userAvatar.style.color = 'white';
        }
        loadSearchHistory();
    } else {
        if (authButtons) authButtons.style.display = 'flex';
        if (userAvatar) {
            userAvatar.textContent = 'G';
            userAvatar.style.backgroundColor = '#f1f3f4';
            userAvatar.style.color = '#5f6368';
        }
        searchHistory = [];
        updateSearchHistoryUI();
    }
    updateThreeDotsMenu();
}

// Update three dots menu based on login state
function updateThreeDotsMenu() {
    const historyItem = document.querySelector('.menu-item[onclick="toggleSearchHistory()"]');
    const clearHistoryItem = document.querySelector('.menu-item[onclick="clearSearchHistory()"]');
    const logoutItem = document.getElementById('logoutMenuItem');
    const loginItem = document.getElementById('loginMenuItem');
    const signupItem = document.getElementById('signupMenuItem');
    
    if (isLoggedIn) {
        if (historyItem) historyItem.style.display = 'flex';
        if (clearHistoryItem) clearHistoryItem.style.display = 'flex';
        if (logoutItem) logoutItem.style.display = 'flex';
        if (loginItem) loginItem.style.display = 'none';
        if (signupItem) signupItem.style.display = 'none';
    } else {
        if (historyItem) historyItem.style.display = 'none';
        if (clearHistoryItem) clearHistoryItem.style.display = 'none';
        if (logoutItem) logoutItem.style.display = 'none';
        if (loginItem) loginItem.style.display = 'flex';
        if (signupItem) signupItem.style.display = 'flex';
    }
}

// Toggle three dots menu
function toggleThreeDotsMenu() {
    const menu = document.querySelector('.three-dots-menu');
    if (menu) {
        menu.classList.toggle('show');
    }
}

// Hide three dots menu
function hideThreeDotsMenu() {
    const menu = document.querySelector('.three-dots-menu');
    if (menu) {
        menu.classList.remove('show');
    }
}

// Search input handlers
function handleSearchInput(e) {
    const query = e.target.value.trim();
    
    if (query.length > 0) {
        showSuggestions(query);
    } else {
        hideSuggestions();
        showRecentSearches();
    }
}

function handleSearchFocus() {
    const query = searchInput.value.trim();
    if (query.length === 0) {
        showRecentSearches();
    }
}

function handleSearchBlur() {
    setTimeout(hideSuggestions, 200);
}

function handleSearchKeydown(e) {
    if (e.key === 'Enter') {
        performSearch();
    } else if (e.key === 'Escape') {
        hideSuggestions();
        searchInput.blur();
    }
}

// Show suggestions
async function showSuggestions(query) {
    try {
        const response = await fetch(`/api/suggestions?q=${encodeURIComponent(query)}`);
        const suggestions = await response.json();
        displaySuggestions(query, suggestions);
    } catch (error) {
        console.error('Error fetching suggestions:', error);
    }
}

// Display suggestions in dropdown
function displaySuggestions(query, suggestions) {
    if (!suggestionsDropdown) return;
    
    suggestionsDropdown.innerHTML = '';
    
    if (suggestions.length === 0) {
        hideSuggestions();
        return;
    }
    
    suggestions.forEach(suggestion => {
        const suggestionItem = document.createElement('div');
        suggestionItem.className = 'suggestion-item';
        suggestionItem.innerHTML = `
            <i class="fas fa-search"></i>
            <span>${escapeHtml(suggestion.text)}</span>
        `;
        suggestionItem.addEventListener('click', () => {
            searchInput.value = suggestion.text;
            performSearch();
        });
        suggestionsDropdown.appendChild(suggestionItem);
    });
    
    suggestionsDropdown.style.display = 'block';
}

// Show recent searches
function showRecentSearches() {
    if (!suggestionsDropdown || !isLoggedIn || searchHistory.length === 0) {
        hideSuggestions();
        return;
    }
    
    suggestionsDropdown.innerHTML = '';
    
    const header = document.createElement('div');
    header.className = 'suggestions-divider';
    header.textContent = 'Recent searches';
    suggestionsDropdown.appendChild(header);
    
    searchHistory.slice(0, 5).forEach(historyItem => {
        const suggestionItem = document.createElement('div');
        suggestionItem.className = 'suggestion-item history-suggestion';
        suggestionItem.innerHTML = `
            <div>
                <i class="fas fa-history"></i>
                <span>${escapeHtml(historyItem.query)}</span>
            </div>
            <button class="remove-suggestion-btn" onclick="removeFromSearchHistory('${escapeHtml(historyItem.query)}', event)">
                <i class="fas fa-times"></i>
            </button>
        `;
        suggestionItem.addEventListener('click', (e) => {
            if (!e.target.closest('.remove-suggestion-btn')) {
                searchInput.value = historyItem.query;
                performSearch();
                hideSuggestions();
            }
        });
        suggestionsDropdown.appendChild(suggestionItem);
    });
    
    suggestionsDropdown.style.display = 'block';
}

// Hide suggestions
function hideSuggestions() {
    if (suggestionsDropdown) {
        suggestionsDropdown.style.display = 'none';
    }
}

// Perform search
async function performSearch() {
    const query = searchInput.value.trim();
    
    if (!query) return;
    
    hideSuggestions();
    showLoading();
    
    try {
        const response = await fetch('/api/search', {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json',
            },
            body: JSON.stringify({ query: query })
        });
        
        const data = await response.json();
        displayResults(data);
        
        if (isLoggedIn) {
            addToSearchHistory(query);
        }
    } catch (error) {
        console.error('Search error:', error);
        displayError('Failed to perform search. Please try again.');
    } finally {
        hideLoading();
    }
}

// Feeling Lucky search
async function feelingLucky() {
    const query = searchInput.value.trim();
    
    if (!query) return;
    
    showLoading();
    
    try {
        const response = await fetch('/api/search', {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json',
            },
            body: JSON.stringify({ query: query })
        });
        
        const data = await response.json();
        
        if (data.results && data.results.length > 0) {
            displayResults(data);
            document.querySelector('.result-item')?.scrollIntoView({ behavior: 'smooth' });
        } else {
            displayResults(data);
        }
        
        if (isLoggedIn) {
            addToSearchHistory(query);
        }
    } catch (error) {
        console.error('Feeling Lucky error:', error);
        displayError('Failed to perform search. Please try again.');
    } finally {
        hideLoading();
    }
}

// Display search results
function displayResults(data) {
    if (!resultsContainer || !resultsSection) return;
    
    resultsContainer.innerHTML = '';
    resultsSection.style.display = 'block';
    resultsSection.scrollIntoView({ behavior: 'smooth' });
    
    const correction = data.correction || data.spellingCorrection;
    const originalQuery = data.originalQuery || searchInput.value;
    
    if (correction && correction.trim() !== '' && correction !== originalQuery) {
        const correctionHtml = `
            <div class="spelling-correction">
                <div class="correction-text">
                    <i class="fas fa-lightbulb"></i>
                    Showing results for: 
                    <span class="corrected-search" onclick="searchCorrectedTerm('${escapeHtml(correction)}')">
                        ${escapeHtml(correction)}
                    </span>
                </div>
                <div class="search-instead">
                    Search instead for: 
                    <a onclick="searchOriginalTerm('${escapeHtml(originalQuery)}')">
                        ${escapeHtml(originalQuery)}
                    </a>
                </div>
            </div>
        `;
        resultsContainer.innerHTML = correctionHtml;
    }
    
    if (data.results && data.results.length > 0) {
        if (resultsCount) {
            resultsCount.textContent = `About ${data.results.length} results`;
        }
        
        data.results.forEach((result, index) => {
            const resultItem = document.createElement('div');
            resultItem.className = 'result-item';
            
            const fileName = result.filename.split('/').pop() || result.filename;
            const displayName = fileName.replace('.txt', '').replace(/_/g, ' ');
            
            resultItem.innerHTML = `
                <div class="result-header">
                    <div class="result-icon">
                        <i class="fas fa-file-alt"></i>
                    </div>
                    <a href="#" class="result-title" onclick="viewFullFile('${escapeHtml(result.filename)}', '${escapeHtml(originalQuery)}')">
                        ${escapeHtml(displayName)}
                    </a>
                </div>
                <div class="result-path">${escapeHtml(result.filename)}</div>
                <div class="result-occurrences">
                    <span class="occurrence-count">${result.occurrences} occurrences</span>
                    <button class="extract-sentences-btn" onclick="extractSentences('${escapeHtml(result.filename)}', '${escapeHtml(originalQuery)}')" ${!isLoggedIn ? 'disabled' : ''}>
                        <i class="fas fa-paragraph"></i>
                        Extract Sentences
                        ${!isLoggedIn ? '(Login Required)' : ''}
                    </button>
                </div>
            `;
            
            resultsContainer.appendChild(resultItem);
        });
    } else {
        if (resultsCount) {
            resultsCount.textContent = 'No results found';
        }
        resultsContainer.innerHTML = `
            <div class="no-results">
                <i class="fas fa-search fa-3x"></i>
                <h3>No results found</h3>
                <p>Try different keywords or check your spelling</p>
                ${correction ? `
                <div class="did-you-mean">
                    <p>Did you mean: 
                        <a onclick="searchCorrectedTerm('${escapeHtml(correction)}')" style="color: #1a73e8; cursor: pointer; text-decoration: underline;">
                            ${escapeHtml(correction)}
                        </a>
                    </p>
                </div>
                ` : ''}
            </div>
        `;
    }
}

// Search with corrected term
// FIXED: Search with corrected term - properly handles both regular and image search
function searchCorrectedTerm(correctedTerm) {
    console.log('🔍 Searching corrected term:', correctedTerm);
    
    if (searchInput) {
        searchInput.value = correctedTerm;
        
        // Reset image search results and show regular results
        resetToRegularSearch();
        
        // Perform the search
        performSearch();
    }
}

// FIXED: Search with original term - properly handles both regular and image search
function searchOriginalTerm(originalTerm) {
    console.log('🔍 Searching original term:', originalTerm);
    
    if (searchInput) {
        searchInput.value = originalTerm;
        
        // Reset image search results and show regular results
        resetToRegularSearch();
        
        // Perform the search
        performSearch();
    }
}

// Extract sentences from file
// Extract sentences from file - FIXED VERSION
// View real file content - FIXED VERSION
// Extract sentences from file - FIXED VERSION
async function extractSentences(filename, searchTerm) {
    if (!isLoggedIn) {
        showLoginModal();
        showNotification('Please login to extract sentences', 'warning');
        return;
    }
    
    showLoading();
    
    try {
        const response = await fetch('/api/extract-sentences', {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json',
            },
            body: JSON.stringify({
                filename: filename,
                searchTerm: searchTerm
            })
        });
        
        if (response.ok) {
            const sentences = await response.json();
            displaySentencesModal(filename, searchTerm, sentences);
        } else {
            const errorData = await response.json();
            throw new Error(errorData.error || 'Sentence extraction failed');
        }
    } catch (error) {
        console.error('Sentence extraction error:', error);
        showNotification('Failed to extract sentences: ' + error.message, 'error');
    } finally {
        hideLoading();
    }
}

// Display sentences modal - FIXED VERSION with separate closing
function displaySentencesModal(filename, searchTerm, sentences) {
    // Remove any existing sentences modal first
    const existingModal = document.querySelector('.sentences-modal-overlay');
    if (existingModal) {
        existingModal.remove();
    }
    
    const modalOverlay = document.createElement('div');
    modalOverlay.className = 'sentences-modal-overlay';
    modalOverlay.style.cssText = `
        position: fixed;
        top: 0;
        left: 0;
        width: 100%;
        height: 100%;
        background: rgba(0, 0, 0, 0.5);
        display: flex;
        justify-content: center;
        align-items: center;
        z-index: 10000;
    `;
    
    const fileName = filename.split('/').pop() || filename;
    const displayName = fileName.replace('.txt', '').replace(/_/g, ' ');
    
    let sentencesHtml = '';
    
    if (sentences && sentences.length > 0) {
        sentencesHtml = `
            <div style="background: #e8f0fe; padding: 15px; border-radius: 8px; margin-bottom: 20px; border-left: 4px solid #1a73e8;">
                <strong>Found ${sentences.length} sentences containing "${searchTerm}"</strong>
            </div>
            <div style="max-height: 400px; overflow-y: auto;">
        `;
        
        sentences.forEach((sentence, index) => {
            const sentenceText = sentence.sentence || sentence.text || sentence;
            const occurrences = sentence.occurrences || sentence.count || 1;
            
            sentencesHtml += `
                <div style="background: white; border: 1px solid #e8eaed; border-radius: 8px; padding: 16px; margin-bottom: 10px;">
                    <div style="display: flex; justify-content: space-between; align-items: center; margin-bottom: 8px;">
                        <strong style="color: #1a73e8;">Sentence ${index + 1}</strong>
                        <span style="background: #34a853; color: white; padding: 4px 8px; border-radius: 12px; font-size: 12px;">
                            ${occurrences} occurrence(s)
                        </span>
                    </div>
                    <div style="line-height: 1.6; color: #3c4043;">
                        ${highlightSearchTerm(sentenceText, searchTerm)}
                    </div>
                </div>
            `;
        });
        
        sentencesHtml += `</div>`;
    } else {
        sentencesHtml = `
            <div style="text-align: center; padding: 40px 20px; color: #5f6368;">
                <div style="font-size: 48px; margin-bottom: 16px;">🔍</div>
                <h3>No Sentences Found</h3>
                <p>No sentences found containing "${searchTerm}"</p>
            </div>
        `;
    }
    
    modalOverlay.innerHTML = `
        <div style="background: white; border-radius: 12px; width: 90%; max-width: 700px; max-height: 80vh; display: flex; flex-direction: column;">
            <div style="padding: 20px; border-bottom: 1px solid #e8eaed; display: flex; justify-content: space-between; align-items: center;">
                <h3 style="margin: 0; color: #1a73e8;">📝 Sentences from ${displayName}</h3>
                <button onclick="closeSentencesModal()" style="background: none; border: none; font-size: 20px; cursor: pointer; color: #5f6368;">×</button>
            </div>
            <div style="padding: 20px; flex: 1; overflow-y: auto;">
                ${sentencesHtml}
            </div>
            <div style="padding: 20px; border-top: 1px solid #e8eaed; display: flex; gap: 10px; justify-content: flex-end;">
                <button onclick="closeSentencesModal()" style="padding: 10px 20px; border: 1px solid #dadce0; background: white; border-radius: 4px; cursor: pointer;">Close</button>
                <button onclick="viewFullFile('${escapeHtml(filename)}', '${escapeHtml(searchTerm)}')" style="padding: 10px 20px; background: #1a73e8; color: white; border: none; border-radius: 4px; cursor: pointer;">View Full File</button>
            </div>
        </div>
    `;
    
    // Add click outside to close
    modalOverlay.addEventListener('click', function(e) {
        if (e.target === modalOverlay) {
            closeSentencesModal();
        }
    });
    
    document.body.appendChild(modalOverlay);
    document.body.style.overflow = 'hidden';
}

// Close sentences modal - SEPARATE FUNCTION
function closeSentencesModal() {
    const modal = document.querySelector('.sentences-modal-overlay');
    if (modal) {
        modal.remove();
        document.body.style.overflow = '';
    }
}

// View real file content - FIXED VERSION
async function viewFullFile(filename, searchTerm) {
    showLoading();
    
    try {
        console.log('📖 Loading file:', filename);
        
        // Try multiple approaches to load the file
        let response;
        
        // First try the exact filename
        response = await fetch(filename);
        
        if (!response.ok) {
            // Try with relative path
            const relativePath = filename.startsWith('/') ? filename.substring(1) : filename;
            response = await fetch(relativePath);
        }
        
        if (!response.ok) {
            // Try with Docs/ prefix
            const docsPath = 'Docs/' + filename.split('/').pop();
            response = await fetch(docsPath);
        }
        
        if (!response.ok) {
            // Try absolute path from root
            const absolutePath = '/' + filename;
            response = await fetch(absolutePath);
        }
        
        if (!response.ok) {
            throw new Error(`File not found (${response.status}) - Tried: ${filename}`);
        }
        
        const content = await response.text();
        console.log('✅ File loaded successfully, length:', content.length);
        displayFullFileModal(filename, searchTerm, content);
        
    } catch (error) {
        console.error('❌ Error loading file:', error);
        showNotification('Failed to load file content: ' + error.message, 'error');
    } finally {
        hideLoading();
    }
}

// Display full file modal - FIXED VERSION with separate closing
function displayFullFileModal(filename, searchTerm, content) {
    // Remove any existing file modal first
    const existingModal = document.querySelector('.full-file-modal-overlay');
    if (existingModal) {
        existingModal.remove();
    }
    
    const modalOverlay = document.createElement('div');
    modalOverlay.className = 'full-file-modal-overlay';
    modalOverlay.style.cssText = `
        position: fixed;
        top: 0;
        left: 0;
        width: 100%;
        height: 100%;
        background: rgba(0, 0, 0, 0.5);
        display: flex;
        justify-content: center;
        align-items: center;
        z-index: 10001;
    `;
    
    const fileName = filename.split('/').pop() || filename;
    const displayName = fileName.replace('.txt', '').replace(/_/g, ' ');
    
    // Highlight search term in content
    const highlightedContent = highlightSearchTerm(content, searchTerm);
    
    modalOverlay.innerHTML = `
        <div style="background: white; border-radius: 12px; width: 90%; max-width: 900px; max-height: 80vh; display: flex; flex-direction: column;">
            <div style="padding: 20px; border-bottom: 1px solid #e8eaed; display: flex; justify-content: space-between; align-items: center;">
                <h3 style="margin: 0; color: #1a73e8;">📄 ${displayName}</h3>
                <button onclick="closeFullFileModal()" style="background: none; border: none; font-size: 20px; cursor: pointer; color: #5f6368;">×</button>
            </div>
            <div style="padding: 20px; border-bottom: 1px solid #e8eaed;">
                <div style="display: flex; gap: 20px; font-size: 14px; color: #5f6368;">
                    <div><strong>File:</strong> ${filename}</div>
                    <div><strong>Search term:</strong> "${searchTerm}"</div>
                    <div><strong>Size:</strong> ${content.length} characters</div>
                </div>
            </div>
            <div style="padding: 20px; flex: 1; overflow-y: auto; background: #f8f9fa;">
                <div style="background: white; padding: 20px; border-radius: 8px; border: 1px solid #e8eaed; white-space: pre-wrap; font-family: monospace; line-height: 1.5;">
                    ${highlightedContent}
                </div>
            </div>
            <div style="padding: 20px; border-top: 1px solid #e8eaed; display: flex; gap: 10px; justify-content: flex-end;">
                <button onclick="closeFullFileModal()" style="padding: 10px 20px; border: 1px solid #dadce0; background: white; border-radius: 4px; cursor: pointer;">Close</button>
                <button onclick="downloadFile('${escapeHtml(filename)}', '${escapeHtml(displayName)}')" style="padding: 10px 20px; background: #1a73e8; color: white; border: none; border-radius: 4px; cursor: pointer;">Download</button>
            </div>
        </div>
    `;
    
    // Add click outside to close
    modalOverlay.addEventListener('click', function(e) {
        if (e.target === modalOverlay) {
            closeFullFileModal();
        }
    });
    
    document.body.appendChild(modalOverlay);
    document.body.style.overflow = 'hidden';
}

// Close full file modal - SEPARATE FUNCTION
function closeFullFileModal() {
    const modal = document.querySelector('.full-file-modal-overlay');
    if (modal) {
        modal.remove();
        document.body.style.overflow = '';
    }
}

// Download file function
function downloadFile(filename, displayName) {
    const link = document.createElement('a');
    link.href = filename;
    link.download = displayName + '.txt';
    document.body.appendChild(link);
    link.click();
    document.body.removeChild(link);
}

// Highlight search term function
function highlightSearchTerm(text, searchTerm) {
    if (!searchTerm) return escapeHtml(text);
    const regex = new RegExp(`(${escapeRegex(searchTerm)})`, 'gi');
    return escapeHtml(text).replace(regex, '<mark style="background: #fef7e0; padding: 2px 4px; border-radius: 3px; font-weight: 600; color: #e37400;">$1</mark>');
}

// Escape regex function
function escapeRegex(string) {
    return string.replace(/[.*+?^${}()|[\]\\]/g, '\\$&');
}

// Handle image search
// Enhanced: Handle image search with Tesseract.js OCR
// SIMPLIFIED: Handle image search - only upload option
function handleImageSearch() {
    const input = document.createElement('input');
    input.type = 'file';
    input.accept = 'image/*';
    input.capture = 'environment'; // For mobile camera
    input.onchange = (e) => {
        const file = e.target.files[0];
        if (file) {
            // Validate file type
            if (!file.type.startsWith('image/')) {
                showNotification('Please select an image file', 'error');
                return;
            }
            
            // Validate file size (max 10MB)
            if (file.size > 10 * 1024 * 1024) {
                showNotification('Image too large. Please select image under 10MB.', 'error');
                return;
            }
            
            processImageWithOCR(file);
        }
    };
    input.click();
}

// NEW: Process image with Tesseract.js OCR
// ENHANCED: Process image with Tesseract.js OCR with better preprocessing
async function processImageWithOCR(file) {
    console.log('🖼️ Processing image with enhanced OCR:', file.name);
    
    showNotification('Processing image with enhanced OCR...', 'info');
    
    try {
        // Show loading state
        const imageResultsContainer = document.getElementById('imageSearchResults');
        const resultsSection = document.getElementById('resultsSection');
        
        if (resultsSection) {
            resultsSection.style.display = 'block';
        }
        
        if (imageResultsContainer) {
            imageResultsContainer.style.display = 'block';
            imageResultsContainer.innerHTML = `
                <div class="loading-ocr">
                    <div class="spinner"></div>
                    <p>Processing image with enhanced OCR...</p>
                    <small>Preprocessing image for better text extraction...</small>
                </div>
            `;
        }

        // Step 1: Preprocess the image
        const processedImageData = await preprocessImageForOCR(file);
        
        // Step 2: Use Tesseract.js for OCR with enhanced configuration
        const { data: { text, confidence } } = await Tesseract.recognize(
            processedImageData,
            'eng', // English language
            {
                logger: progress => {
                    console.log('OCR Progress:', progress);
                    if (progress.status === 'recognizing text') {
                        const percent = Math.round(progress.progress * 100);
                        if (imageResultsContainer) {
                            imageResultsContainer.innerHTML = `
                                <div class="loading-ocr">
                                    <div class="spinner"></div>
                                    <p>Processing image with OCR... ${percent}%</p>
                                    <small>Extracting text from preprocessed image...</small>
                                </div>
                            `;
                        }
                    }
                },
                // Enhanced OCR settings for better accuracy
                tessedit_pageseg_mode: Tesseract.PSM.SINGLE_BLOCK,
                tessedit_char_whitelist: 'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789 .,!?()-',
                preserve_interword_spaces: '1'
            }
        );

        console.log('✅ OCR Extracted Text:', text);
        console.log('📊 OCR Confidence:', confidence);
        
        // Clean the extracted text with enhanced cleaning
        const cleanedText = cleanOCRText(text);
        console.log('🧹 Cleaned Text:', cleanedText);

        if (!cleanedText || cleanedText.trim().length < 2) {
            throw new Error('No readable text found in image. Try a clearer image.');
        }

        // Show confidence level to user
        if (confidence < 50) {
            showNotification(`Low confidence (${Math.round(confidence)}%). Results may not be accurate.`, 'warning');
        }

        // Auto-fill search and perform search
        if (searchInput) {
            searchInput.value = cleanedText;
        }
        
        // Perform search with extracted text
        await performSearchWithOCRText(cleanedText, file);

    } catch (error) {
        console.error('❌ OCR Processing error:', error);
        showNotification('OCR failed: ' + error.message, 'error');
        
        const imageResultsContainer = document.getElementById('imageSearchResults');
        if (imageResultsContainer) {
            imageResultsContainer.innerHTML = `
                <div class="error">
                    <i class="fas fa-exclamation-triangle"></i>
                    <h3>OCR Processing Failed</h3>
                    <p>${error.message}</p>
                    <p><small>Try: Clearer image, Better lighting, Horizontal text</small></p>
                    <div style="margin-top: 15px;">
                        <button onclick="handleImageSearch()" class="btn-primary">
                            <i class="fas fa-camera"></i>
                            Try Another Image
                        </button>
                        <button onclick="resetToRegularSearch()" class="btn-secondary">
                            Regular Search
                        </button>
                    </div>
                </div>
            `;
        }
    }
}

// NEW: Enhanced image preprocessing for OCR
async function preprocessImageForOCR(file) {
    return new Promise((resolve, reject) => {
        const img = new Image();
        const canvas = document.createElement('canvas');
        const ctx = canvas.getContext('2d');
        
        img.onload = function() {
            // Auto-rotate based on EXIF data (for vertical images)
            const orientation = getImageOrientation(file);
            
            // Set canvas dimensions based on orientation
            if (orientation >= 5 && orientation <= 8) {
                // Swap dimensions for vertical images
                canvas.width = img.height;
                canvas.height = img.width;
            } else {
                canvas.width = img.width;
                canvas.height = img.height;
            }
            
            // Apply orientation correction
            applyImageOrientation(ctx, img, orientation);
            
            // Apply image enhancements for better OCR
            enhanceImageForOCR(ctx, canvas.width, canvas.height);
            
            // Convert back to blob for Tesseract
            canvas.toBlob(blob => {
                resolve(blob);
            }, 'image/jpeg', 0.8);
        };
        
        img.onerror = reject;
        img.src = URL.createObjectURL(file);
    });
}

// NEW: Get image orientation from EXIF data
function getImageOrientation(file) {
    // Default to horizontal (1)
    return 1; // You can implement EXIF reading here if needed
}

// NEW: Apply orientation correction
function applyImageOrientation(ctx, img, orientation) {
    const width = img.width;
    const height = img.height;
    
    ctx.clearRect(0, 0, ctx.canvas.width, ctx.canvas.height);
    
    switch (orientation) {
        case 2:
            ctx.transform(-1, 0, 0, 1, width, 0);
            break;
        case 3:
            ctx.transform(-1, 0, 0, -1, width, height);
            break;
        case 4:
            ctx.transform(1, 0, 0, -1, 0, height);
            break;
        case 5:
            ctx.transform(0, 1, 1, 0, 0, 0);
            break;
        case 6:
            ctx.transform(0, 1, -1, 0, height, 0);
            break;
        case 7:
            ctx.transform(0, -1, -1, 0, height, width);
            break;
        case 8:
            ctx.transform(0, -1, 1, 0, 0, width);
            break;
    }
    
    ctx.drawImage(img, 0, 0);
}

// NEW: Enhance image for better OCR
function enhanceImageForOCR(ctx, width, height) {
    const imageData = ctx.getImageData(0, 0, width, height);
    const data = imageData.data;
    
    // Apply contrast enhancement
    const contrast = 1.5;
    const factor = (259 * (contrast + 255)) / (255 * (259 - contrast));
    
    for (let i = 0; i < data.length; i += 4) {
        // Increase contrast
        data[i] = factor * (data[i] - 128) + 128;     // Red
        data[i + 1] = factor * (data[i + 1] - 128) + 128; // Green
        data[i + 2] = factor * (data[i + 2] - 128) + 128; // Blue
        
        // Convert to grayscale for better OCR
        const gray = 0.299 * data[i] + 0.587 * data[i + 1] + 0.114 * data[i + 2];
        data[i] = data[i + 1] = data[i + 2] = gray;
        
        // Increase brightness for dark images
        if (gray < 100) {
            data[i] = data[i + 1] = data[i + 2] = Math.min(255, gray + 50);
        }
    }
    
    ctx.putImageData(imageData, 0, 0);
}

// ENHANCED: Clean OCR extracted text with better handling
function cleanOCRText(text) {
    if (!text) return '';
    
    return text
        .replace(/\n/g, ' ')          // Replace newlines with spaces
        .replace(/\s+/g, ' ')         // Replace multiple spaces with single space
        .replace(/[^\w\s.,!?()-]/g, '') // Keep basic punctuation
        .replace(/\b(\w)\b/g, '')     // Remove single characters (likely OCR errors)
        .replace(/\b\w{1,2}\b/g, '')  // Remove very short words
        .trim()                       // Trim whitespace
        .replace(/\s+/g, ' ')         // Clean up spaces again
        .substring(0, 200);           // Reasonable length limit
}



// NEW: Perform search with OCR text
// FIXED: Perform search with OCR text and handle corrections
async function performSearchWithOCRText(extractedText, originalFile) {
    console.log('🔍 Performing search with OCR text:', extractedText);
    
    try {
        const response = await fetch('/api/search', {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json',
            },
            body: JSON.stringify({ query: extractedText })
        });
        
        if (!response.ok) {
            throw new Error('Search request failed');
        }
        
        const data = await response.json();
        
        // FIX: Update search input with corrected term if available
        const correction = data.correction || data.spellingCorrection;
        if (correction && correction.trim() !== '' && searchInput) {
            searchInput.value = correction;
        }
        
        // Display OCR-specific results
        displayOCRSearchResults(data, extractedText, originalFile);
        
        // Add to search history if logged in
        if (isLoggedIn) {
            const termToSave = correction && correction.trim() !== '' ? correction : extractedText;
            addToSearchHistory(termToSave);
        }
        
    } catch (error) {
        console.error('Search with OCR error:', error);
        showNotification('Search failed after OCR', 'error');
        
        const imageResultsContainer = document.getElementById('imageSearchResults');
        if (imageResultsContainer) {
            imageResultsContainer.innerHTML = `
                <div class="error">
                    <i class="fas fa-exclamation-triangle"></i>
                    <h3>Search Failed</h3>
                    <p>OCR worked but search failed: ${error.message}</p>
                    <p>Extracted text: "${extractedText}"</p>
                    <div style="margin-top: 15px;">
                        <button onclick="searchInput.value='${extractedText}'; performSearch();" class="btn-primary">
                            Try Again
                        </button>
                        <button onclick="resetToRegularSearch()" class="btn-secondary">
                            Regular Search
                        </button>
                    </div>
                </div>
            `;
        }
    }
}

// NEW: Display OCR search results
// FIXED: Display OCR search results with proper "Did you mean" feature
function displayOCRSearchResults(data, extractedText, originalFile) {
    const imageResultsContainer = document.getElementById('imageSearchResults');
    const regularResultsContainer = document.getElementById('resultsContainer');
    
    if (!imageResultsContainer) return;

    // Hide regular results
    if (regularResultsContainer) {
        regularResultsContainer.style.display = 'none';
    }

    // FIX: Check for spelling correction in the data
    const correction = data.correction || data.spellingCorrection;
    const originalQuery = data.originalQuery || extractedText;

    let resultsHtml = `
        <div class="ocr-search-results">
            <div class="ocr-header">
                <div class="ocr-success">
                    <i class="fas fa-check-circle"></i>
                    <h3>Image Search Complete</h3>
                </div>
                <div class="ocr-extracted-text">
                    <strong>Extracted Text:</strong> 
                    <span class="extracted-text">"${escapeHtml(extractedText)}"</span>
                </div>
            </div>
    `;

    // FIX: Add spelling correction section if available
            // FIX: Add spelling correction section if available
    if (correction && correction.trim() !== "" && correction !== originalQuery) {
    resultsHtml += `
            <div class="spelling-correction">
                <div class="correction-text">
                    <i class="fas fa-lightbulb"></i>
                    Showing results for: 
                    <span class="corrected-search" onclick="searchCorrectedTerm('${escapeHtml(
                    correction
                    )}')" style="color: #1a73e8; cursor: pointer; text-decoration: underline; font-weight: 600;">
                        ${escapeHtml(correction)}
                    </span>
                </div>
                <div class="search-instead">
                    Search instead for: 
                    <a onclick="searchOriginalTerm('${escapeHtml(
                    originalQuery
                    )}')" style="color: #1a73e8; cursor: pointer; text-decoration: underline;">
                        ${escapeHtml(originalQuery)}
                    </a>
                </div>
            </div>
        `;
    }

    if (data.results && data.results.length > 0) {
        resultsHtml += `
            <div class="search-results">
                <h4>Found ${data.results.length} matching files:</h4>
                <div class="results-list">
        `;

        data.results.forEach((result, index) => {
            const fileName = result.filename.split('/').pop() || result.filename;
            const displayName = fileName.replace('.txt', '').replace(/_/g, ' ');
            
            // FIX: Use the corrected term for sentence extraction if available
            const searchTermToUse = correction && correction !== originalQuery ? correction : extractedText;
            
            resultsHtml += `
                <div class="image-result-item">
                    <div class="result-rank">${index + 1}</div>
                    <div class="file-icon">
                        <i class="fas fa-file-alt"></i>
                    </div>
                    <div class="file-info">
                        <div class="file-name">
                            <a href="#" onclick="viewFullFile('${escapeHtml(result.filename)}', '${escapeHtml(searchTermToUse)}')">
                                ${escapeHtml(displayName)}
                            </a>
                        </div>
                        <div class="file-path">${escapeHtml(result.filename)}</div>
                        <div class="occurrences">
                            <span class="occurrence-count">${result.occurrences} occurrences</span>
                            <button class="extract-sentences-btn" onclick="extractSentences('${escapeHtml(result.filename)}', '${escapeHtml(searchTermToUse)}')" ${!isLoggedIn ? 'disabled' : ''}>
                                <i class="fas fa-paragraph"></i>
                                Extract Sentences
                                ${!isLoggedIn ? '(Login Required)' : ''}
                            </button>
                        </div>
                    </div>
                </div>
            `;
        });

        resultsHtml += `
                </div>
            </div>
        `;
    } else {
        resultsHtml += `
            <div class="no-results">
                <i class="fas fa-search fa-2x"></i>
                <h3>No matching files found</h3>
                <p>No results found for: "${escapeHtml(extractedText)}"</p>
                ${correction ? `
                <div class="did-you-mean">
                    <p>Did you mean: 
                        <a onclick="searchCorrectedTerm('${escapeHtml(correction)}')" style="color: #1a73e8; cursor: pointer; text-decoration: underline;">
                            ${escapeHtml(correction)}
                        </a>
                    </p>
                </div>
                ` : ''}
            </div>
        `;
    }

    resultsHtml += `
            <div class="ocr-actions">
                <button onclick="resetToRegularSearch()" class="btn-secondary">
                    <i class="fas fa-search"></i>
                    Back to Regular Search
                </button>
                <button onclick="handleImageSearch()" class="btn-primary">
                    <i class="fas fa-camera"></i>
                    Search Another Image
                </button>
            </div>
        </div>
    `;

    imageResultsContainer.innerHTML = resultsHtml;
    showNotification('Image search completed successfully!', 'success');
}

// Display image search results
function displayImageSearchResults(data) {
    const imageResultsContainer = document.getElementById('imageSearchResults');
    if (!imageResultsContainer) return;

    const regularResultsContainer = document.getElementById('resultsContainer');
    if (regularResultsContainer) {
        regularResultsContainer.style.display = 'none';
    }

    if (data.success !== false && !data.error) {
        let resultsHtml = `
            <div class="image-search-success">
                <h3>Image Search Results</h3>
                <div class="ocr-text">
                    <strong>OCR Detected Text:</strong> "${data.ocrText || 'No text detected'}"
                </div>
        `;

        if (data.results && data.results.length > 0) {
            resultsHtml += `
                <div class="search-results">
                    <h4>Found ${data.results.length} matching files:</h4>
                    <div class="results-list">
            `;

            data.results.forEach(result => {
                const fileName = result.filename.split('/').pop() || result.filename;
                const displayName = fileName.replace('.txt', '').replace(/_/g, ' ');
                
                resultsHtml += `
                    <div class="image-result-item">
                        <div class="file-icon">
                            <i class="fas fa-file-alt"></i>
                        </div>
                        <div class="file-info">
                            <div class="file-name">${escapeHtml(displayName)}</div>
                            <div class="file-path">${escapeHtml(result.filename)}</div>
                            <div class="occurrences">${result.occurrences} occurrences</div>
                        </div>
                    </div>
                `;
            });

            resultsHtml += `
                    </div>
                </div>
            `;
        } else {
            resultsHtml += `
                <div class="no-results">
                    <i class="fas fa-search fa-2x"></i>
                    <p>No matching files found for the detected text.</p>
                </div>
            `;
        }

        resultsHtml += `</div>`;
        imageResultsContainer.innerHTML = resultsHtml;
        
    } else {
        imageResultsContainer.innerHTML = `
            <div class="error">
                <i class="fas fa-exclamation-triangle"></i>
                <p>Image search failed</p>
            </div>
        `;
    }
}

// SIMPLE VOICE SEARCH - No popups, just basic functionality
function handleVoiceSearch() {
    if (!('webkitSpeechRecognition' in window) && !('SpeechRecognition' in window)) {
        showNotification('Voice search not supported in your browser', 'error');
        return;
    }
    
    const SpeechRecognition = window.SpeechRecognition || window.webkitSpeechRecognition;
    const recognition = new SpeechRecognition();
    
    recognition.continuous = false;
    recognition.interimResults = false;
    recognition.lang = 'en-US';
    
    // Simple visual feedback - just change mic button color
    const micBtn = document.querySelector('.mic-btn');
    if (micBtn) {
        micBtn.style.background = '#34a853';
        micBtn.style.color = 'white';
    }
    
    showNotification('Listening... Speak now', 'info');
    
    recognition.start();
    
    recognition.onresult = function(event) {
        const transcript = event.results[0][0].transcript;
        
        // Reset mic button
        if (micBtn) {
            micBtn.style.background = '';
            micBtn.style.color = '';
        }
        
        if (searchInput) {
            searchInput.value = transcript;
            performSearch();
        }
        
        showNotification(`Voice search: "${transcript}"`, 'success');
    };
    
    recognition.onerror = function(event) {
        // Reset mic button
        if (micBtn) {
            micBtn.style.background = '';
            micBtn.style.color = '';
        }
        
        if (event.error === 'no-speech') {
            showNotification('No speech detected', 'warning');
        } else {
            showNotification('Voice search failed', 'error');
        }
    };
    
    recognition.onend = function() {
        // Reset mic button
        if (micBtn) {
            micBtn.style.background = '';
            micBtn.style.color = '';
        }
    };
}

// Reset to regular search
// FIXED: Reset to regular search - completely clear image search state
function resetToRegularSearch() {
    const imageResultsContainer = document.getElementById('imageSearchResults');
    const regularResultsContainer = document.getElementById('resultsContainer');
    const resultsSection = document.getElementById('resultsSection');
    
    if (imageResultsContainer) {
        imageResultsContainer.style.display = 'none';
        imageResultsContainer.innerHTML = '';
    }
    
    if (regularResultsContainer) {
        regularResultsContainer.style.display = 'block';
    }
    
    if (resultsSection) {
        resultsSection.style.display = 'block';
    }
    
    // Ensure search input is focused and ready
    if (searchInput) {
        searchInput.focus();
    }
}

// Search history management
async function loadSearchHistory() {
    if (!isLoggedIn) return;
    
    try {
        const response = await fetch('/api/search-history');
        const history = await response.json();
        searchHistory = history;
        updateSearchHistoryUI();
    } catch (error) {
        console.error('Error loading search history:', error);
    }
}

function updateSearchHistoryUI() {
    if (!historyItems) return;
    
    if (!isLoggedIn) {
        historyItems.innerHTML = '<div class="no-history">No search history available</div>';
        return;
    }
    
    if (searchHistory.length === 0) {
        historyItems.innerHTML = '<div class="no-history">No search history yet</div>';
        return;
    }
    
    historyItems.innerHTML = '';
    searchHistory.forEach(item => {
        const historyItem = document.createElement('div');
        historyItem.className = 'history-item';
        historyItem.innerHTML = `
            <span class="history-query">${escapeHtml(item.query)}</span>
            <button class="history-remove" onclick="removeFromSearchHistory('${escapeHtml(item.query)}', event)">
                <i class="fas fa-times"></i>
            </button>
        `;
        historyItem.addEventListener('click', (e) => {
            if (!e.target.closest('.history-remove')) {
                if (searchInput) {
                    searchInput.value = item.query;
                }
                performSearch();
                toggleSearchHistory();
            }
        });
        historyItems.appendChild(historyItem);
    });
}

async function addToSearchHistory(query) {
    if (!isLoggedIn) return;
    
    try {
        await loadSearchHistory();
    } catch (error) {
        console.error('Error adding to search history:', error);
    }
}

async function removeFromSearchHistory(query, event) {
    if (event) event.stopPropagation();
    if (!isLoggedIn) return;
    await loadSearchHistory();
}

async function clearSearchHistory() {
    if (!isLoggedIn) {
        showLoginModal();
        return;
    }
    
    try {
        const response = await fetch('/api/clear-history', { method: 'POST' });
        const result = await response.json();
        
        if (result.success) {
            searchHistory = [];
            updateSearchHistoryUI();
            hideThreeDotsMenu();
            showNotification('Search history cleared', 'success');
        }
    } catch (error) {
        console.error('Error clearing search history:', error);
        showNotification('Failed to clear history', 'error');
    }
}

function toggleSearchHistory() {
    if (!isLoggedIn) {
        showLoginModal();
        return;
    }
    
    if (!searchHistorySection) return;
    
    const isVisible = searchHistorySection.style.display !== 'none';
    searchHistorySection.style.display = isVisible ? 'none' : 'block';
    hideThreeDotsMenu();
    
    if (!isVisible) {
        updateSearchHistoryUI();
    }
}

// User authentication functions
async function loginUser(email, password) {
    try {
        const response = await fetch('/api/login', {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json',
            },
            body: JSON.stringify({ email, password })
        });
        
        const result = await response.json();
        
        if (result.success) {
            isLoggedIn = true;
            await checkCurrentUser();
            hideAllModals();
            showNotification('Login successful!', 'success');
            return true;
        } else {
            showNotification('Invalid email or password', 'error');
            return false;
        }
    } catch (error) {
        console.error('Login error:', error);
        showNotification('Login failed. Please try again.', 'error');
        return false;
    }
}

async function registerUser(userData) {
    try {
        const response = await fetch('/api/register', {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json',
            },
            body: JSON.stringify(userData)
        });
        
        const result = await response.json();
        
        if (result.success) {
            showNotification('Registration successful! Please login.', 'success');
            switchToLoginTab();
            return true;
        } else {
            showNotification(result.errorMessage || 'Registration failed', 'error');
            return false;
        }
    } catch (error) {
        console.error('Registration error:', error);
        showNotification('Registration failed. Please try again.', 'error');
        return false;
    }
}

async function logoutUser() {
    try {
        await fetch('/api/logout', { method: 'POST' });
        
        currentUser = null;
        isLoggedIn = false;
        searchHistory = [];
        
        updateUIForLoginState();
        hideThreeDotsMenu();
        hideAllModals();
        showNotification('Logged out successfully', 'success');
    } catch (error) {
        console.error('Logout error:', error);
    }
}

// Modal management
function createModals() {
    const modalsContainer = document.createElement('div');
    modalsContainer.id = 'modals-container';
    document.body.appendChild(modalsContainer);
}

function showLoginModal() {
    createAuthModal('login');
}

function showSignupModal() {
    createAuthModal('signup');
}

function createAuthModal(activeTab = 'login') {
    document.querySelectorAll('.modal-overlay').forEach(modal => modal.remove());
    
    const modalOverlay = document.createElement('div');
    modalOverlay.className = 'modal-overlay';
    modalOverlay.innerHTML = `
        <div class="modal-content login-modal">
            <div class="modal-header">
                <h3>${activeTab === 'login' ? 'Sign In' : 'Create Account'}</h3>
                <button class="modal-close" onclick="closeCurrentModal(this)">
                    <i class="fas fa-times"></i>
                </button>
            </div>
            <div class="modal-body">
                <div class="login-tabs">
                    <button class="tab-btn ${activeTab === 'login' ? 'active' : ''}" onclick="switchAuthTab('login')">Sign In</button>
                    <button class="tab-btn ${activeTab === 'signup' ? 'active' : ''}" onclick="switchAuthTab('signup')">Sign Up</button>
                </div>
                
                <div class="tab-content ${activeTab === 'login' ? 'active' : ''}" id="login-tab">
                    <form id="login-form" onsubmit="handleLogin(event)">
                        <div class="form-group">
                            <label>Email</label>
                            <input type="email" id="login-email" required>
                        </div>
                        <div class="form-group">
                            <label>Password</label>
                            <input type="password" id="login-password" required>
                        </div>
                        <div class="modal-footer">
                            <button type="button" class="btn-secondary" onclick="closeCurrentModal(this)">Cancel</button>
                            <button type="submit" class="btn-primary">Sign In</button>
                        </div>
                    </form>
                </div>
                
                <div class="tab-content ${activeTab === 'signup' ? 'active' : ''}" id="signup-tab">
                    <form id="signup-form" onsubmit="handleSignup(event)">
                        <div class="form-row">
                            <div class="form-group">
                                <label>First Name</label>
                                <input type="text" id="signup-firstname" required>
                            </div>
                            <div class="form-group">
                                <label>Last Name</label>
                                <input type="text" id="signup-lastname" required>
                            </div>
                        </div>
                        <div class="form-group">
                            <label>Email</label>
                            <input type="email" id="signup-email" required>
                        </div>
                        <div class="form-group">
                            <label>Password</label>
                            <input type="password" id="signup-password" required minlength="6">
                        </div>
                        <div class="form-group">
                            <label>Confirm Password</label>
                            <input type="password" id="signup-confirm-password" required>
                        </div>
                        <div class="modal-footer">
                            <button type="button" class="btn-secondary" onclick="closeCurrentModal(this)">Cancel</button>
                            <button type="submit" class="btn-primary">Create Account</button>
                        </div>
                    </form>
                </div>
            </div>
        </div>
    `;
    
    const modalsContainer = document.getElementById('modals-container');
    if (modalsContainer) {
        modalsContainer.appendChild(modalOverlay);
    }
}

function switchAuthTab(tab) {
    createAuthModal(tab);
}

function switchToLoginTab() {
    createAuthModal('login');
}

async function handleLogin(event) {
    event.preventDefault();
    
    const email = document.getElementById('login-email');
    const password = document.getElementById('login-password');
    
    if (!email || !password) return;
    
    const success = await loginUser(email.value, password.value);
    if (success) {
        hideAllModals();
    }
}

async function handleSignup(event) {
    event.preventDefault();

    const firstName = document.getElementById('signup-firstname');
    const lastName = document.getElementById('signup-lastname');
    const email = document.getElementById('signup-email');
    const password = document.getElementById('signup-password');
    const confirmPassword = document.getElementById('signup-confirm-password');

    if (!firstName || !lastName || !email || !password || !confirmPassword) return;

    const userData = {
        firstName: firstName.value,
        lastName: lastName.value,
        email: email.value,
        password: password.value,
        confirmPassword: confirmPassword.value,
    };

    const success = await registerUser(userData);
    if (success) {
        // Form will be cleared when modal is recreated for login
    }
}

// Close only the current modal
function closeCurrentModal(element) {
    const modal = element.closest('.modal-overlay');
    if (modal) {
        modal.remove();
    }
}

function hideAllModals() {
    document.querySelectorAll('.modal-overlay').forEach(modal => modal.remove());
}

// Utility functions
function showLoading() {
    if (loadingSpinner) {
        loadingSpinner.style.display = 'flex';
    }
}

function hideLoading() {
    if (loadingSpinner) {
        loadingSpinner.style.display = 'none';
    }
}

function escapeHtml(unsafe) {
    if (typeof unsafe !== 'string') return unsafe;
    return unsafe
        .replace(/&/g, "&amp;")
        .replace(/</g, "&lt;")
        .replace(/>/g, "&gt;")
        .replace(/"/g, "&quot;")
        .replace(/'/g, "&#039;");
}

function highlightSearchTerm(text, searchTerm) {
    if (!searchTerm) return escapeHtml(text);
    const regex = new RegExp(`(${escapeRegex(searchTerm)})`, 'gi');
    return escapeHtml(text).replace(regex, '<span class="search-highlight">$1</span>');
}

function escapeRegex(string) {
    return string.replace(/[.*+?^${}()|[\]\\]/g, '\\$&');
}

function displayError(message) {
    if (!resultsContainer) return;
    
    resultsContainer.innerHTML = `
        <div class="no-results">
            <i class="fas fa-exclamation-triangle fa-3x"></i>
            <h3>Search Error</h3>
            <p>${message}</p>
        </div>
    `;
}

function showNotification(message, type = 'info') {
    // Remove existing notifications
    document.querySelectorAll('.notification').forEach(notification => notification.remove());
    
    const notification = document.createElement('div');
    notification.className = `notification notification-${type}`;
    notification.innerHTML = `
        <i class="fas fa-${getNotificationIcon(type)}"></i>
        <span>${message}</span>
        <button onclick="this.parentElement.remove()">
            <i class="fas fa-times"></i>
        </button>
    `;
    
    notification.style.cssText = `
        position: fixed;
        top: 20px;
        right: 20px;
        background: ${getNotificationColor(type)};
        color: white;
        padding: 12px 16px;
        border-radius: 8px;
        box-shadow: 0 4px 12px rgba(0,0,0,0.15);
        display: flex;
        align-items: center;
        gap: 10px;
        z-index: 10001;
        max-width: 400px;
    `;
    
    document.body.appendChild(notification);
    
    setTimeout(() => {
        if (notification.parentElement) {
            notification.remove();
        }
    }, 5000);
}

function getNotificationIcon(type) {
    const icons = {
        success: 'check-circle',
        error: 'exclamation-circle',
        warning: 'exclamation-triangle',
        info: 'info-circle'
    };
    return icons[type] || 'info-circle';
}

function getNotificationColor(type) {
    const colors = {
        success: '#34a853',
        error: '#ea4335',
        warning: '#f9ab00',
        info: '#4285f4'
    };
    return colors[type] || '#4285f4';
}