#include <iostream>
#include <fstream>
#include <string>
#include <vector>

using namespace std;

int main() {
    // Data for 20 different files
    vector<pair<string, string>> fileData = {
        {"technology.txt", "Artificial intelligence is revolutionizing the world. Machine learning algorithms are becoming more sophisticated every day. Robotics and automation are changing industries globally."},
        {"sports.txt", "Football is the most popular sport worldwide. Basketball has millions of fans across the globe. Tennis tournaments attract viewers from every continent."},
        {"food.txt", "Pizza originated in Italy and became popular worldwide. Sushi is a traditional Japanese cuisine. Pasta comes in hundreds of different shapes and varieties."},
        {"nature.txt", "Mountains provide breathtaking views and fresh air. Oceans cover most of our planet's surface. Forests are home to countless species of animals and plants."},
        {"education.txt", "Schools play a vital role in child development. Universities offer higher education opportunities. Online learning has become increasingly popular recently."},
        {"music.txt", "Classical music has been appreciated for centuries. Rock music emerged in the 1950s and changed everything. Jazz originated in New Orleans and spread worldwide."},
        {"travel.txt", "Paris is known as the city of love and lights. Tokyo offers a perfect blend of tradition and modernity. London has a rich history and vibrant culture."},
        {"science.txt", "Physics explains the fundamental laws of nature. Chemistry studies matter and its properties extensively. Biology explores all forms of life on Earth."},
        {"movies.txt", "Hollywood produces blockbuster films every year. Bollywood creates colorful musical entertainment regularly. Independent films offer unique storytelling perspectives always."},
        {"health.txt", "Exercise is essential for maintaining good health. Nutrition plays a crucial role in wellness. Sleep is important for physical and mental recovery."},
        {"history.txt", "Ancient civilizations built magnificent structures that still stand. Medieval times were marked by castles and knights. Modern history includes two major world wars."},
        {"books.txt", "Novels transport readers to different worlds completely. Poetry expresses emotions through beautiful language carefully. Biographies tell inspiring life stories of remarkable people."},
        {"games.txt", "Chess is considered the ultimate strategy game. Video games have evolved tremendously over decades. Board games bring families together for fun."},
        {"weather.txt", "Summer brings warm temperatures and sunny days. Winter is characterized by cold weather and snow. Spring flowers bloom after the harsh winter season."},
        {"animals.txt", "Lions are known as the kings of jungle. Elephants are the largest land mammals on Earth. Dolphins are highly intelligent marine creatures that fascinate scientists."},
        {"colors.txt", "Blue is often associated with calmness and serenity. Red symbolizes passion and energy in many cultures. Green represents nature and growth in most societies."},
        {"transportation.txt", "Cars have revolutionized personal transportation methods completely. Airplanes make long-distance travel quick and convenient. Trains offer comfortable travel for medium distances efficiently."},
        {"friendship.txt", "Friends provide emotional support during difficult times. Loyalty is the foundation of lasting friendships. Trust must be earned and maintained in relationships."},
        {"work.txt", "Programming requires logical thinking and problem-solving skills. Teaching shapes the minds of future generations positively. Engineering involves creating solutions for real-world problems creatively."},
        {"hobbies.txt", "Reading expands knowledge and improves vocabulary significantly. Gardening connects people with nature and provides relaxation. Photography captures beautiful moments that last forever."}
    };

    cout << "Creating 20 text files with content...\n";
    cout << "======================================\n";

    // Create and write to each file
    for (int i = 0; i < fileData.size(); i++) {
        ofstream file(fileData[i].first);
        
        if (file.is_open()) {
            file << fileData[i].second;
            file.close();
            cout << "Created: " << fileData[i].first << endl;
        } else {
            cout << "Error creating: " << fileData[i].first << endl;
        }
    }

    cout << "\nAll files created successfully!\n";
    cout << "\nFiles created:\n";
    for (int i = 0; i < fileData.size(); i++) {
        cout << (i + 1) << ". " << fileData[i].first << endl;
    }

    return 0;
}
