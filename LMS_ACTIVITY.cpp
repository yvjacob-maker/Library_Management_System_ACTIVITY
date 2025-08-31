#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
using namespace std;

// ====================== Book Class ======================
class Book {
private:
    string title;
    string author;
    string isbn;
    bool available;

public:
    Book(string t, string a, string i, bool avail = true)
        : title(t), author(a), isbn(i), available(avail) {}

    // Getters
    string getTitle() const { return title; }
    string getAuthor() const { return author; }
    string getISBN() const { return isbn; }
    bool isAvailable() const { return available; }

    // Borrow and return books
    void borrowBook() { available = false; }
    void returnBook() { available = true; }

    // Save book info to file
    string toString() const {
        return title + "|" + author + "|" + isbn + "|" + (available ? "1" : "0");
    }

    // Load book from string
    static Book fromString(const string &data) {
        size_t pos1 = data.find("|");
        size_t pos2 = data.find("|", pos1 + 1);
        size_t pos3 = data.find("|", pos2 + 1);
        string title = data.substr(0, pos1);
        string author = data.substr(pos1 + 1, pos2 - pos1 - 1);
        string isbn = data.substr(pos2 + 1, pos3 - pos2 - 1);
        bool available = (data.substr(pos3 + 1) == "1");
        return Book(title, author, isbn, available);
    }
};

// ====================== User Class ======================
class User {
private:
    string id;
    string name;
    vector<string> borrowedBooks; // Store ISBNs

public:
    User(string i, string n) : id(i), name(n) {}

    // Getters
    string getId() const { return id; }
    string getName() const { return name; }
    vector<string> getBorrowedBooks() const { return borrowedBooks; }

    // Borrow and return books
    void borrowBook(const string &isbn) { borrowedBooks.push_back(isbn); }
    void returnBook(const string &isbn) {
        borrowedBooks.erase(remove(borrowedBooks.begin(), borrowedBooks.end(), isbn), borrowedBooks.end());
    }

    // Save user info to file
    string toString() const {
        string data = id + "|" + name + "|";
        for (size_t i = 0; i < borrowedBooks.size(); i++) {
            data += borrowedBooks[i];
            if (i < borrowedBooks.size() - 1)
                data += ",";
        }
        return data;
    }

    // Load user from string
    static User fromString(const string &data) {
        size_t pos1 = data.find("|");
        size_t pos2 = data.find("|", pos1 + 1);
        string id = data.substr(0, pos1);
        string name = data.substr(pos1 + 1, pos2 - pos1 - 1);
        User user(id, name);
        string borrowed = data.substr(pos2 + 1);
        size_t start = 0, end;
        while ((end = borrowed.find(",", start)) != string::npos) {
            user.borrowBook(borrowed.substr(start, end - start));
            start = end + 1;
        }
        if (!borrowed.empty())
            user.borrowBook(borrowed.substr(start));
        return user;
    }
};

// ====================== Library Class ======================
class Library {
private:
    vector<Book> books;
    vector<User> users;

    const string booksFile = "books.txt";
    const string usersFile = "users.txt";

public:
    Library() {
        loadData();
        resetDefaults(); // Reset to only Alice, Bob, and 2 books
    }

    // ================== Load Data ==================
    void loadData() {
        ifstream bookFile(booksFile);
        string line;
        while (getline(bookFile, line)) {
            books.push_back(Book::fromString(line));
        }
        bookFile.close();

        ifstream userFile(usersFile);
        while (getline(userFile, line)) {
            users.push_back(User::fromString(line));
        }
        userFile.close();
    }

    // ================== Save Data ==================
    void saveData() {
        ofstream bookFile(booksFile);
        for (const auto &b : books)
            bookFile << b.toString() << endl;
        bookFile.close();

        ofstream userFile(usersFile);
        for (const auto &u : users)
            userFile << u.toString() << endl;
        userFile.close();
    }

    // ================== Reset Defaults ==================
    void resetDefaults() {
        books.clear();
        users.clear();

        // Default books
        addBook("1984", "George Orwell", "978-0451524935");
        addBook("To Kill a Mockingbird", "Harper Lee", "978-0061120084");

        // Default users
        registerUser("001", "Alice");
        registerUser("002", "Bob");

        saveData();
    }

    // ================== Book Management ==================
    void addBook(const string &title, const string &author, const string &isbn) {
        for (const auto &b : books) {
            if (b.getISBN() == isbn) {
                cout << "Book already exists!\n";
                return;
            }
        }
        books.push_back(Book(title, author, isbn));
        saveData();
    }

    void removeBook(const string &isbn) {
        auto it = remove_if(books.begin(), books.end(),
                            [&](const Book &b) { return b.getISBN() == isbn; });
        if (it != books.end()) {
            books.erase(it, books.end());
            cout << "Book removed successfully!\n";
            saveData();
        } else {
            cout << "Book not found!\n";
        }
    }

    // ================== User Management ==================
    void registerUser(const string &id, const string &name) {
        for (const auto &u : users) {
            if (u.getId() == id) {
                cout << "User ID already exists!\n";
                return;
            }
        }
        users.push_back(User(id, name));
        saveData();
    }

    void removeUser(const string &id) {
        auto it = remove_if(users.begin(), users.end(),
                            [&](const User &u) { return u.getId() == id; });
        if (it != users.end()) {
            users.erase(it, users.end());
            cout << "User removed successfully!\n";
            saveData();
        } else {
            cout << "User not found!\n";
        }
    }

    // ================== Borrow & Return ==================
    void borrowBook(const string &userId, const string &isbn) {
        auto userIt = find_if(users.begin(), users.end(),
                              [&](const User &u) { return u.getId() == userId; });
        auto bookIt = find_if(books.begin(), books.end(),
                              [&](const Book &b) { return b.getISBN() == isbn; });

        if (userIt != users.end() && bookIt != books.end()) {
            if (bookIt->isAvailable()) {
                bookIt->borrowBook();
                userIt->borrowBook(isbn);
                cout << "Book borrowed successfully!\n";
                saveData();
            } else {
                cout << "Book is already borrowed!\n";
            }
        } else {
            cout << "Invalid user ID or ISBN!\n";
        }
    }

    void returnBook(const string &userId, const string &isbn) {
        auto userIt = find_if(users.begin(), users.end(),
                              [&](const User &u) { return u.getId() == userId; });
        auto bookIt = find_if(books.begin(), books.end(),
                              [&](const Book &b) { return b.getISBN() == isbn; });

        if (userIt != users.end() && bookIt != books.end()) {
            bookIt->returnBook();
            userIt->returnBook(isbn);
            cout << "Book returned successfully!\n";
            saveData();
        } else {
            cout << "Invalid user ID or ISBN!\n";
        }
    }

    // ================== Display ==================
    void displayBooks() const {
        cout << "\n=== Library Books ===\n";
        for (const auto &b : books) {
            cout << "Title: " << b.getTitle()
                 << " | Author: " << b.getAuthor()
                 << " | ISBN: " << b.getISBN()
                 << " | Status: " << (b.isAvailable() ? "Available" : "Borrowed") << endl;
        }
    }

    void displayUsers() const {
        cout << "\n=== Registered Users ===\n";
        for (const auto &u : users) {
            cout << "ID: " << u.getId()
                 << " | Name: " << u.getName() << endl;
        }
    }
};

// ====================== Main Function ======================
int main() {
    Library library;
    int choice;
    string id, name, isbn, title, author;

    while (true) {
        cout << "\n===== Library Management Menu =====\n";
        cout << "1. Display Books\n";
        cout << "2. Display Users\n";
        cout << "3. Add Book\n";
        cout << "4. Remove Book\n";
        cout << "5. Register User\n";
        cout << "6. Remove User\n";
        cout << "7. Borrow Book\n";
        cout << "8. Return Book\n";
        cout << "9. Exit\n";
        cout << "Enter choice: ";
        cin >> choice;

        switch (choice) {
        case 1:
            library.displayBooks();
            break;
        case 2:
            library.displayUsers();
            break;
        case 3:
            cout << "Enter title: "; cin.ignore(); getline(cin, title);
            cout << "Enter author: "; getline(cin, author);
            cout << "Enter ISBN: "; cin >> isbn;
            library.addBook(title, author, isbn);
            break;
        case 4:
            cout << "Enter ISBN to remove: "; cin >> isbn;
            library.removeBook(isbn);
            break;
        case 5:
            cout << "Enter user ID: "; cin >> id;
            cout << "Enter name: "; cin.ignore(); getline(cin, name);
            library.registerUser(id, name);
            break;
        case 6:
            cout << "Enter user ID to remove: "; cin >> id;
            library.removeUser(id);
            break;
        case 7:
            cout << "Enter user ID: "; cin >> id;
            cout << "Enter ISBN: "; cin >> isbn;
            library.borrowBook(id, isbn);
            break;
        case 8:
            cout << "Enter user ID: "; cin >> id;
            cout << "Enter ISBN: "; cin >> isbn;
            library.returnBook(id, isbn);
            break;
        case 9:
            cout << "Exiting...\n";
            return 0;
        default:
            cout << "Invalid choice!\n";
        }
    }
}