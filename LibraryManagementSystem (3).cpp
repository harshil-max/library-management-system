// ============================================================
//  LIBRARY MANAGEMENT SYSTEM USING C++
//  Summer Internship Training - Object Oriented Programming
//
//  Compile : g++ -std=c++17 -Wall LibraryManagementSystem.cpp -o mylibrary
//  Run     : ./mylibrary        (Windows: mylibrary.exe)
//  Login   : username = admin   password = admin123
//
//  Data is saved in books.txt, students.txt and transactions.txt
//  (created automatically in the same folder on first run).
// ============================================================

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <iomanip>
#include <ctime>
#include <limits>
#include <cstdlib>

using namespace std;

// ============================================================
//  PART 1: CLASSES, LOGIN, MAIN MENU
// ============================================================

// Replaces the '|' character so it can never break the file format
string cleanText(string s) {
    for (size_t i = 0; i < s.size(); i++) {
        if (s[i] == '|') s[i] = '/';
    }
    return s;
}

// ---------------- Class: Book ----------------
class Book {
public:
    int bookID;
    string title;
    string author;
    int totalCopies;
    int availableCopies;

    Book() {
        bookID = 0;
        totalCopies = 0;
        availableCopies = 0;
    }

    Book(int id, string t, string a, int total, int avail) {
        bookID = id;
        title = t;
        author = a;
        totalCopies = total;
        availableCopies = avail;
    }

    void display() {
        cout << left << setw(6) << bookID << setw(28) << title
             << setw(20) << author << setw(8) << totalCopies
             << setw(10) << availableCopies << "\n";
    }

    // Convert object to one line of text for saving to file
    string toFileString() {
        ostringstream oss;
        oss << bookID << "|" << title << "|" << author << "|"
            << totalCopies << "|" << availableCopies;
        return oss.str();
    }

    // Rebuild object from one line of text read from file
    static Book fromFileString(string line) {
        stringstream ss(line);
        string idStr, t, a, totalStr, availStr;
        getline(ss, idStr, '|');
        getline(ss, t, '|');
        getline(ss, a, '|');
        getline(ss, totalStr, '|');
        getline(ss, availStr, '|');
        return Book(stoi(idStr), t, a, stoi(totalStr), stoi(availStr));
    }
};

// ---------------- Class: Student ----------------
class Student {
public:
    int studentID;
    string name;
    string department;
    vector<int> issuedBookIDs;

    Student() {
        studentID = 0;
    }

    Student(int id, string n, string dept) {
        studentID = id;
        name = n;
        department = dept;
    }

    void display() {
        cout << left << setw(6) << studentID << setw(20) << name
             << setw(15) << department << "Books issued: "
             << issuedBookIDs.size() << "\n";
    }

    string toFileString() {
        ostringstream oss;
        oss << studentID << "|" << name << "|" << department << "|";
        for (size_t i = 0; i < issuedBookIDs.size(); i++) {
            oss << issuedBookIDs[i];
            if (i != issuedBookIDs.size() - 1) oss << ",";
        }
        return oss.str();
    }

    static Student fromFileString(string line) {
        stringstream ss(line);
        string idStr, n, dept, booksStr;
        getline(ss, idStr, '|');
        getline(ss, n, '|');
        getline(ss, dept, '|');
        getline(ss, booksStr, '|');

        Student s(stoi(idStr), n, dept);
        if (!booksStr.empty()) {
            stringstream bs(booksStr);
            string part;
            while (getline(bs, part, ',')) {
                if (!part.empty()) s.issuedBookIDs.push_back(stoi(part));
            }
        }
        return s;
    }
};

// ---------------- Class: Transaction ----------------
class Transaction {
public:
    int transactionID;
    int bookID;
    int studentID;
    string issueDate;
    string dueDate;
    string returnDate;   // "PENDING" until the book is returned
    double fine;

    Transaction() {
        transactionID = 0;
        bookID = 0;
        studentID = 0;
        returnDate = "PENDING";
        fine = 0.0;
    }

    Transaction(int tid, int bID, int sID, string iDate, string dDate,
                string rDate, double f) {
        transactionID = tid;
        bookID = bID;
        studentID = sID;
        issueDate = iDate;
        dueDate = dDate;
        returnDate = rDate;
        fine = f;
    }

    bool isReturned() {
        return returnDate != "PENDING";
    }

    void display() {
        cout << left << setw(6) << transactionID << setw(8) << bookID
             << setw(10) << studentID << setw(12) << issueDate
             << setw(12) << dueDate << "\n";
    }

    string toFileString() {
        ostringstream oss;
        oss << transactionID << "|" << bookID << "|" << studentID << "|"
            << issueDate << "|" << dueDate << "|" << returnDate << "|" << fine;
        return oss.str();
    }

    static Transaction fromFileString(string line) {
        stringstream ss(line);
        string tidStr, bidStr, sidStr, iDate, dDate, rDate, fineStr;
        getline(ss, tidStr, '|');
        getline(ss, bidStr, '|');
        getline(ss, sidStr, '|');
        getline(ss, iDate, '|');
        getline(ss, dDate, '|');
        getline(ss, rDate, '|');
        getline(ss, fineStr, '|');
        return Transaction(stoi(tidStr), stoi(bidStr), stoi(sidStr),
                           iDate, dDate, rDate, stod(fineStr));
    }
};

// ---------------- Global data (kept in memory while running) ----------------
vector<Book> books;
vector<Student> students;
vector<Transaction> transactions;

const string BOOKS_FILE = "books.txt";
const string STUDENTS_FILE = "students.txt";
const string TRANSACTIONS_FILE = "transactions.txt";

void saveAll();   // defined in Part 5

// ---------------- Input helpers ----------------
// Reads a whole line and converts it to int. Keeps asking until valid.
int readInt(const string& prompt) {
    while (true) {
        cout << prompt;
        string line;
        if (!getline(cin, line)) {
            cout << "\nInput closed. Exiting.\n";
            exit(0);
        }
        try {
            size_t used = 0;
            int value = stoi(line, &used);
            if (used == line.size()) return value;
        } catch (...) {
        }
        cout << "Please enter a valid number.\n";
    }
}

string readLine(const string& prompt) {
    cout << prompt;
    string line;
    if (!getline(cin, line)) {
        cout << "\nInput closed. Exiting.\n";
        exit(0);
    }
    return cleanText(line);
}

// ---------------- Login ----------------
const string ADMIN_USERNAME = "admin";
const string ADMIN_PASSWORD = "admin123";

bool adminLogin() {
    string user, pass;
    cout << "============================================\n";
    cout << "       LIBRARY MANAGEMENT SYSTEM\n";
    cout << "============================================\n";
    user = readLine("Enter Username : ");
    pass = readLine("Enter Password : ");

    if (user == ADMIN_USERNAME && pass == ADMIN_PASSWORD) {
        cout << "\nLogin successful. Welcome, " << user << "!\n";
        return true;
    }
    cout << "\nInvalid username or password.\n";
    return false;
}

// ---------------- Main Menu ----------------
void printMenu() {
    cout << "\n===== LIBRARY MANAGEMENT SYSTEM =====\n";
    cout << " 1. Add Book\n";
    cout << " 2. View Books\n";
    cout << " 3. Search Book\n";
    cout << " 4. Update Book\n";
    cout << " 5. Delete Book\n";
    cout << " 6. Register Student\n";
    cout << " 7. View Students\n";
    cout << " 8. Issue Book\n";
    cout << " 9. Return Book\n";
    cout << "10. View Active Issues\n";
    cout << "11. Exit\n";
    cout << "=====================================\n";
}

// ============================================================
//  PART 2: BOOK MANAGEMENT (Add, Display, Search, Update, Delete)
// ============================================================

int getNextBookID() {
    int maxID = 0;
    for (int i = 0; i < (int)books.size(); i++) {
        if (books[i].bookID > maxID) maxID = books[i].bookID;
    }
    return maxID + 1;
}

Book* findBookByID(int id) {
    for (int i = 0; i < (int)books.size(); i++) {
        if (books[i].bookID == id) return &books[i];
    }
    return nullptr;
}

void printBookHeader() {
    cout << "\n" << left << setw(6) << "ID" << setw(28) << "Title"
         << setw(20) << "Author" << setw(8) << "Total" << setw(10) << "Available" << "\n";
    cout << string(72, '-') << "\n";
}

void addBook() {
    string title = readLine("Enter Book Title  : ");
    string author = readLine("Enter Book Author : ");
    int copies = readInt("Enter Number of Copies : ");

    if (title.empty() || author.empty()) {
        cout << "Title and author cannot be empty.\n";
        return;
    }
    if (copies <= 0) {
        cout << "Number of copies must be positive.\n";
        return;
    }

    Book newBook(getNextBookID(), title, author, copies, copies);
    books.push_back(newBook);
    saveAll();
    cout << "Book added successfully with ID " << newBook.bookID << "\n";
}

void displayBooks() {
    if (books.empty()) {
        cout << "No books found in the library.\n";
        return;
    }
    printBookHeader();
    for (int i = 0; i < (int)books.size(); i++) {
        books[i].display();
    }
}

// Converts text to lower case so searching ignores capital letters
string toLowerText(string s) {
    for (size_t i = 0; i < s.size(); i++) {
        if (s[i] >= 'A' && s[i] <= 'Z') s[i] = s[i] - 'A' + 'a';
    }
    return s;
}

void searchBook() {
    string keyword = toLowerText(readLine("Enter title to search : "));

    bool found = false;
    printBookHeader();
    for (int i = 0; i < (int)books.size(); i++) {
        if (toLowerText(books[i].title).find(keyword) != string::npos) {
            books[i].display();
            found = true;
        }
    }
    if (!found) cout << "No book found with that title.\n";
}

void updateBook() {
    int id = readInt("Enter Book ID to update : ");

    Book* b = findBookByID(id);
    if (b == nullptr) {
        cout << "Book not found.\n";
        return;
    }

    cout << "1. Update Title\n2. Update Author\n3. Add More Copies\n";
    int choice = readInt("Choice: ");

    if (choice == 1) {
        string t = readLine("Enter new title: ");
        if (t.empty()) {
            cout << "Title cannot be empty.\n";
            return;
        }
        b->title = t;
    } else if (choice == 2) {
        string a = readLine("Enter new author: ");
        if (a.empty()) {
            cout << "Author cannot be empty.\n";
            return;
        }
        b->author = a;
    } else if (choice == 3) {
        int extra = readInt("Enter copies to add: ");
        if (extra <= 0) {
            cout << "Copies to add must be positive.\n";
            return;
        }
        b->totalCopies += extra;
        b->availableCopies += extra;
    } else {
        cout << "Invalid choice.\n";
        return;
    }
    saveAll();
    cout << "Book updated successfully.\n";
}

void deleteBook() {
    int id = readInt("Enter Book ID to delete : ");

    for (int i = 0; i < (int)books.size(); i++) {
        if (books[i].bookID == id) {
            if (books[i].availableCopies != books[i].totalCopies) {
                cout << "Cannot remove: copies issued.\n";
                return;
            }
            books.erase(books.begin() + i);
            saveAll();
            cout << "Book deleted successfully.\n";
            return;
        }
    }
    cout << "Book not found.\n";
}

// ============================================================
//  PART 3: STUDENT MANAGEMENT
// ============================================================

int getNextStudentID() {
    int maxID = 0;
    for (int i = 0; i < (int)students.size(); i++) {
        if (students[i].studentID > maxID) maxID = students[i].studentID;
    }
    return maxID + 1;
}

Student* findStudentByID(int id) {
    for (int i = 0; i < (int)students.size(); i++) {
        if (students[i].studentID == id) return &students[i];
    }
    return nullptr;
}

void registerStudent() {
    string name = readLine("Enter Student Name       : ");
    string department = readLine("Enter Student Department : ");

    if (name.empty() || department.empty()) {
        cout << "Name and department cannot be empty.\n";
        return;
    }

    Student newStudent(getNextStudentID(), name, department);
    students.push_back(newStudent);
    saveAll();
    cout << "Student registered successfully with ID " << newStudent.studentID << "\n";
}

void displayStudents() {
    if (students.empty()) {
        cout << "No students registered yet.\n";
        return;
    }
    cout << "\n" << left << setw(6) << "ID" << setw(20) << "Name"
         << setw(15) << "Department" << "Books Issued\n";
    cout << string(60, '-') << "\n";
    for (int i = 0; i < (int)students.size(); i++) {
        students[i].display();
    }
}

// ============================================================
//  PART 4: ISSUE / RETURN BOOKS AND FINE CALCULATION
// ============================================================

const int LOAN_PERIOD_DAYS = 14;
const double FINE_PER_DAY = 5.0;

// Returns today's date as a string in YYYY-MM-DD format
string getTodayDate() {
    time_t t = time(nullptr);
    tm* now = localtime(&t);
    char buf[11];
    strftime(buf, sizeof(buf), "%Y-%m-%d", now);
    return string(buf);
}

// Converts "YYYY-MM-DD" to time_t (noon of that day, to avoid daylight-saving issues)
time_t dateToTime(const string& date) {
    tm tmVal = {};
    int y = 0, m = 0, d = 0;
    if (sscanf(date.c_str(), "%d-%d-%d", &y, &m, &d) != 3) return (time_t)-1;
    tmVal.tm_year = y - 1900;
    tmVal.tm_mon = m - 1;
    tmVal.tm_mday = d;
    tmVal.tm_hour = 12;
    tmVal.tm_isdst = -1;
    return mktime(&tmVal);
}

// Adds a given number of days to a date string
string addDaysToDate(string date, int days) {
    time_t t = dateToTime(date);
    t += (time_t)days * 24 * 60 * 60;
    tm* result = localtime(&t);
    char buf[11];
    strftime(buf, sizeof(buf), "%Y-%m-%d", result);
    return string(buf);
}

// Returns the number of days between two date strings (date2 - date1)
int daysBetween(string date1, string date2) {
    time_t t1 = dateToTime(date1);
    time_t t2 = dateToTime(date2);
    double diff = difftime(t2, t1) / (24.0 * 60.0 * 60.0);
    return (int)(diff + (diff >= 0 ? 0.5 : -0.5));
}

int getNextTransactionID() {
    int maxID = 0;
    for (int i = 0; i < (int)transactions.size(); i++) {
        if (transactions[i].transactionID > maxID) maxID = transactions[i].transactionID;
    }
    return maxID + 1;
}

void issueBook() {
    int bookID = readInt("Enter Book ID    : ");
    int studentID = readInt("Enter Student ID : ");

    Book* b = findBookByID(bookID);
    if (b == nullptr) {
        cout << "Book not found.\n";
        return;
    }
    Student* s = findStudentByID(studentID);
    if (s == nullptr) {
        cout << "Student not found.\n";
        return;
    }
    if (b->availableCopies <= 0) {
        cout << "No copies available.\n";
        return;
    }
    for (int i = 0; i < (int)s->issuedBookIDs.size(); i++) {
        if (s->issuedBookIDs[i] == bookID) {
            cout << "This student already has a copy of this book.\n";
            return;
        }
    }

    string today = getTodayDate();
    string due = addDaysToDate(today, LOAN_PERIOD_DAYS);

    Transaction t(getNextTransactionID(), bookID, studentID, today, due, "PENDING", 0.0);
    transactions.push_back(t);

    b->availableCopies = b->availableCopies - 1;
    s->issuedBookIDs.push_back(bookID);

    saveAll();
    cout << "Book issued successfully. Transaction ID: " << t.transactionID << "\n";
    cout << "Issue date: " << today << "   Due date: " << due << "\n";
}

void returnBook() {
    int transactionID = readInt("Enter Transaction ID : ");

    for (int i = 0; i < (int)transactions.size(); i++) {
        if (transactions[i].transactionID == transactionID && !transactions[i].isReturned()) {
            string returnDate = getTodayDate();
            int lateDays = daysBetween(transactions[i].dueDate, returnDate);
            double fine = 0.0;
            if (lateDays > 0) fine = lateDays * FINE_PER_DAY;

            transactions[i].returnDate = returnDate;
            transactions[i].fine = fine;

            Book* b = findBookByID(transactions[i].bookID);
            if (b != nullptr) b->availableCopies = b->availableCopies + 1;

            Student* s = findStudentByID(transactions[i].studentID);
            if (s != nullptr) {
                for (int j = 0; j < (int)s->issuedBookIDs.size(); j++) {
                    if (s->issuedBookIDs[j] == transactions[i].bookID) {
                        s->issuedBookIDs.erase(s->issuedBookIDs.begin() + j);
                        break;
                    }
                }
            }

            saveAll();
            cout << "Book returned successfully.\n";
            if (fine > 0)
                cout << "Late by " << lateDays << " day(s). Fine charged: Rs." << fine << "\n";
            else
                cout << "Returned on time. No fine.\n";
            return;
        }
    }
    cout << "Active transaction not found with that ID.\n";
}

void viewActiveIssues() {
    bool found = false;
    cout << "\n" << left << setw(6) << "TxnID" << setw(8) << "BookID"
         << setw(10) << "StudentID" << setw(12) << "IssueDate" << setw(12) << "DueDate" << "\n";
    cout << string(50, '-') << "\n";
    for (int i = 0; i < (int)transactions.size(); i++) {
        if (!transactions[i].isReturned()) {
            transactions[i].display();
            found = true;
        }
    }
    if (!found) cout << "No active issues.\n";
}

// ============================================================
//  PART 5: FILE HANDLING AND MAIN()
// ============================================================

void saveAll() {
    ofstream bf(BOOKS_FILE.c_str());
    for (int i = 0; i < (int)books.size(); i++) bf << books[i].toFileString() << "\n";
    bf.close();

    ofstream sf(STUDENTS_FILE.c_str());
    for (int i = 0; i < (int)students.size(); i++) sf << students[i].toFileString() << "\n";
    sf.close();

    ofstream tf(TRANSACTIONS_FILE.c_str());
    for (int i = 0; i < (int)transactions.size(); i++) tf << transactions[i].toFileString() << "\n";
    tf.close();
}

void loadAll() {
    string line;

    ifstream bf(BOOKS_FILE.c_str());
    while (getline(bf, line)) {
        if (line.empty()) continue;
        try { books.push_back(Book::fromFileString(line)); } catch (...) {}
    }
    bf.close();

    ifstream sf(STUDENTS_FILE.c_str());
    while (getline(sf, line)) {
        if (line.empty()) continue;
        try { students.push_back(Student::fromFileString(line)); } catch (...) {}
    }
    sf.close();

    ifstream tf(TRANSACTIONS_FILE.c_str());
    while (getline(tf, line)) {
        if (line.empty()) continue;
        try { transactions.push_back(Transaction::fromFileString(line)); } catch (...) {}
    }
    tf.close();
}

int main() {
    // Admin login: 3 attempts
    bool loggedIn = false;
    for (int attempt = 1; attempt <= 3; attempt++) {
        if (adminLogin()) {
            loggedIn = true;
            break;
        }
        if (attempt < 3) cout << "Attempts left: " << (3 - attempt) << "\n\n";
    }
    if (!loggedIn) {
        cout << "Too many failed attempts. Exiting.\n";
        return 0;
    }

    loadAll();

    int choice = 0;
    while (choice != 11) {
        printMenu();
        choice = readInt("Enter your choice: ");

        switch (choice) {
            case 1:  addBook();          break;
            case 2:  displayBooks();     break;
            case 3:  searchBook();       break;
            case 4:  updateBook();       break;
            case 5:  deleteBook();       break;
            case 6:  registerStudent();  break;
            case 7:  displayStudents();  break;
            case 8:  issueBook();        break;
            case 9:  returnBook();       break;
            case 10: viewActiveIssues(); break;
            case 11: cout << "Thank you for using the Library Management System. Goodbye!\n"; break;
            default: cout << "Invalid choice. Please enter a number from 1 to 11.\n";
        }
    }
    return 0;
}
