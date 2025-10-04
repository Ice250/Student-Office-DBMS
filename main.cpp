#include <iostream>
#include <string>
#include <vector>
#include <iomanip>
#include <sstream>
#include <tuple>
#include <limits>
#include <cstdlib>

#ifndef USE_MOCK_DB
#include <mysql/mysql.h>
#else
// Forward-declare MYSQL when building without the MySQL client library
struct MYSQL;
#endif

using namespace std;

// Database connection configuration via environment variables with sensible defaults
static const char* DB_HOST = std::getenv("DB_HOST") && *std::getenv("DB_HOST") ? std::getenv("DB_HOST") : "localhost";
static const char* DB_USER = std::getenv("DB_USER") && *std::getenv("DB_USER") ? std::getenv("DB_USER") : "root";
static const char* DB_PASS = std::getenv("DB_PASS") && *std::getenv("DB_PASS") ? std::getenv("DB_PASS") : "rajal_mysql";
static const char* DB_NAME = std::getenv("DB_NAME") && *std::getenv("DB_NAME") ? std::getenv("DB_NAME") : "bvp_student_office";

// Forward declarations
class DBManager;
class Student;
class Admin;

// Escape input to prevent SQL injection (standalone function)
string escapeString(MYSQL* conn, const string& str) {
#ifndef USE_MOCK_DB
    char* escaped = new char[str.length() * 2 + 1];
    unsigned long escaped_len = mysql_real_escape_string(conn, escaped, str.c_str(), str.length());
    string result(escaped, escaped_len);
    delete[] escaped;
    return result;
#else
    (void)conn; // unused in mock mode
    return str;
#endif
}

// Student class (Extended with marks and receipts)
class Student {
public:
    string studentID, name, department, contact, feeStatus, academicRecord, password;
    int year;
    vector<pair<string, pair<int, string>>> marks;  // Subject -> (Marks, Grade)
    vector<tuple<string, double, string, string, string>> receipts;  // (ReceiptID, Amount, PaidOn, Details, Status)

    void viewProfile();
    void viewMarksheet(DBManager& db);
    void viewFeeReceipts(DBManager& db);
};

// Admin class (Full implementations)
class Admin {
public:
    void viewAllStudents(DBManager& db);
    void searchStudents(DBManager& db, string key, string value);
    void addStudent(DBManager& db);
    void updateStudent(DBManager& db, string studentID);
    void deleteStudent(DBManager& db, string studentID);
    void updateMarks(DBManager& db, string studentID);
    void addFeeReceipt(DBManager& db, string studentID);
};

// Database Manager (Improved with escaping and new methods)
class DBManager {
public:
    MYSQL* conn;
    DBManager();
    ~DBManager();
    bool login(string userType, string id, string password);
    Student getStudent(string studentID);
    vector<Student> getAllStudents();
    bool executeQuery(const string& query);  // For INSERT/UPDATE/DELETE
    vector<pair<string, pair<int, string>>> getMarksheet(string studentID);
    vector<tuple<string, double, string, string, string>> getFeeReceipts(string studentID);
public:
#ifdef USE_MOCK_DB
    // In-memory storage for mock mode
    vector<Student> mockStudents;
#endif
};

DBManager::DBManager() {
#ifndef USE_MOCK_DB
    conn = mysql_init(0);
    if (!conn) {
        cout << "MySQL Init Failed!" << endl;
        exit(1);
    }
    conn = mysql_real_connect(conn, DB_HOST, DB_USER, DB_PASS, DB_NAME, 3306, NULL, 0);
    if (!conn) {
        cout << "Database Connection Failed: " << mysql_error(conn) << endl;
        exit(1);
    }
    cout << "Database Connected Successfully!" << endl;
#else
    conn = nullptr;
    // Seed sample data mirroring setup.sql
    Student s1; s1.studentID = "STU001"; s1.name = "John Doe"; s1.department = "Computer Science"; s1.year = 2; s1.contact = "john.doe@email.com";
    s1.academicRecord = "Excellent academic performance, no disciplinary issues."; s1.feeStatus = "Paid"; s1.password = "studpass";
    s1.marks = {
        {"Mathematics", {85, "B"}},
        {"Physics", {92, "A"}},
        {"Programming", {78, "A"}}
    };
    s1.receipts = { make_tuple(string("REC001"), 5000.00, string("2023-09-01"), string("Annual Tuition Fee Payment via Online Banking"), string("Paid")) };

    Student s2; s2.studentID = "STU002"; s2.name = "Jane Smith"; s2.department = "Electronics Engineering"; s2.year = 3; s2.contact = "jane.smith@email.com";
    s2.academicRecord = "Good standing, participated in tech fests."; s2.feeStatus = "Pending"; s2.password = "studpass2";
    s2.marks = {
        {"Mathematics", {90, "B"}},
        {"Physics", {92, "A"}},
        {"Programming", {98, "C"}}
    };
    // No receipts initially for s2

    mockStudents.clear();
    mockStudents.push_back(s1);
    mockStudents.push_back(s2);
    cout << "Running in MOCK DB mode (no MySQL client required)." << endl;
#endif
}

DBManager::~DBManager() {
#ifndef USE_MOCK_DB
    if (conn) mysql_close(conn);
#endif
}

bool DBManager::login(string userType, string id, string password) {
#ifndef USE_MOCK_DB
    string table = (userType == "admin") ? "Admins" : "Students";
    string idField = (userType == "admin") ? "AdminID" : "StudentID";
    string passField = "Password";  // Both tables have it now
    string escapedId = escapeString(conn, id);
    string escapedPass = escapeString(conn, password);
    string query = "SELECT * FROM " + table + " WHERE " + idField + "='" + escapedId + 
                   "' AND " + passField + "='" + escapedPass + "'";
    if (mysql_query(conn, query.c_str()) != 0) {
        cout << "Query Error: " << mysql_error(conn) << endl;
        return false;
    }
    MYSQL_RES* res = mysql_store_result(conn);
    bool success = (res && mysql_num_rows(res) > 0);
    if (res) mysql_free_result(res);
    return success;
#else
    if (userType == "admin") {
        return id == "ADMIN001" && password == "adminpass";
    }
    for (const auto& s : mockStudents) {
        if (s.studentID == id && s.password == password) return true;
    }
    return false;
#endif
}

Student DBManager::getStudent(string studentID) {
    Student s;
#ifndef USE_MOCK_DB
    string escapedID = escapeString(conn, studentID);
    string query = "SELECT * FROM Students WHERE StudentID='" + escapedID + "'";
    if (mysql_query(conn, query.c_str()) != 0) {
        cout << "Query Error: " << mysql_error(conn) << endl;
        return s;
    }
    MYSQL_RES* res = mysql_store_result(conn);
    MYSQL_ROW row = mysql_fetch_row(res);
    if (row) {
        s.studentID = row[0] ? row[0] : "";
        s.name = row[1] ? row[1] : "";
        s.department = row[2] ? row[2] : "";
        s.year = row[3] ? atoi(row[3]) : 0;
        s.contact = row[4] ? row[4] : "";
        s.academicRecord = row[5] ? row[5] : "";
        s.feeStatus = row[6] ? row[6] : "";
        s.password = row[7] ? row[7] : "";  // Password column (index 7)
    }
    if (res) mysql_free_result(res);
#else
    for (const auto& st : mockStudents) {
        if (st.studentID == studentID) {
            s = st;
            break;
        }
    }
#endif
    // Fetch marks and receipts
    s.marks = getMarksheet(studentID);
    s.receipts = getFeeReceipts(studentID);
    return s;
}

vector<Student> DBManager::getAllStudents() {
    vector<Student> students;
#ifndef USE_MOCK_DB
    string query = "SELECT * FROM Students";
    if (mysql_query(conn, query.c_str()) != 0) {
        cout << "Query Error: " << mysql_error(conn) << endl;
        return students;
    }
    MYSQL_RES* res = mysql_store_result(conn);
    MYSQL_ROW row;
    while ((row = mysql_fetch_row(res))) {
        Student s;
        s.studentID = row[0] ? row[0] : "";
        s.name = row[1] ? row[1] : "";
        s.department = row[2] ? row[2] : "";
        s.year = row[3] ? atoi(row[3]) : 0;
        s.contact = row[4] ? row[4] : "";
        s.academicRecord = row[5] ? row[5] : "";
        s.feeStatus = row[6] ? row[6] : "";
        s.marks = getMarksheet(s.studentID);
        s.receipts = getFeeReceipts(s.studentID);
        students.push_back(s);
    }
    if (res) mysql_free_result(res);
    return students;
#else
    return mockStudents;
#endif
}

bool DBManager::executeQuery(const string& query) {
#ifndef USE_MOCK_DB
    if (mysql_query(conn, query.c_str()) != 0) {
        cout << "Query Error: " << mysql_error(conn) << endl;
        return false;
    }
    return true;
#else
    cout << "[MOCK] executeQuery: " << query << endl;
    // No-op success in mock mode
    return true;
#endif
}

vector<pair<string, pair<int, string>>> DBManager::getMarksheet(string studentID) {
    vector<pair<string, pair<int, string>>> marks;
#ifndef USE_MOCK_DB
    string escapedID = escapeString(conn, studentID);
    string query = "SELECT Subject, Marks, Grade FROM Marksheets WHERE StudentID='" + escapedID + "'";
    if (mysql_query(conn, query.c_str()) != 0) return marks;
    MYSQL_RES* res = mysql_store_result(conn);
    MYSQL_ROW row;
    while ((row = mysql_fetch_row(res))) {
        string subject = row[0] ? row[0] : "";
        int marksInt = row[1] ? atoi(row[1]) : 0;
        string grade = row[2] ? row[2] : "";
        marks.push_back({subject, {marksInt, grade}});
    }
    if (res) mysql_free_result(res);
    return marks;
#else
    for (const auto& st : mockStudents) {
        if (st.studentID == studentID) {
            return st.marks;
        }
    }
    return marks;
#endif
}

vector<tuple<string, double, string, string, string>> DBManager::getFeeReceipts(string studentID) {
    vector<tuple<string, double, string, string, string>> receipts;
#ifndef USE_MOCK_DB
    string escapedID = escapeString(conn, studentID);
    string query = "SELECT ReceiptID, Amount, PaidOn, TransactionDetails, Status FROM FeeReceipts WHERE StudentID='" + escapedID + "'";
    if (mysql_query(conn, query.c_str()) != 0) return receipts;
    MYSQL_RES* res = mysql_store_result(conn);
    MYSQL_ROW row;
    while ((row = mysql_fetch_row(res))) {
        string id = row[0] ? row[0] : "";
        double amount = row[1] ? atof(row[1]) : 0.0;
        string date = row[2] ? row[2] : "";
        string details = row[3] ? row[3] : "";
        string status = row[4] ? row[4] : "";
        receipts.push_back(make_tuple(id, amount, date, details, status));
    }
    if (res) mysql_free_result(res);
    return receipts;
#else
    for (const auto& st : mockStudents) {
        if (st.studentID == studentID) {
            return st.receipts;
        }
    }
    return receipts;
#endif
}

// Student Methods
void Student::viewProfile() {
    cout << "\n=== Student Profile ===" << endl;
    cout << "StudentID: " << studentID << "\nName: " << name
         << "\nDepartment: " << department << "\nYear: " << year
         << "\nContact: " << contact << "\nAcademic Record: " << academicRecord
         << "\nFee Status: " << feeStatus << endl;
}

void Student::viewMarksheet(DBManager& db) {
    marks = db.getMarksheet(studentID);  // Refresh
    cout << "\n=== Marksheet ===" << endl;
    if (marks.empty()) {
        cout << "No marks recorded." << endl;
        return;
    }
    cout << left << setw(20) << "Subject" << setw(10) << "Marks" << "Grade" << endl;
    for (const auto& m : marks) {
        cout << left << setw(20) << m.first << setw(10) << m.second.first << m.second.second << endl;
    }
}

void Student::viewFeeReceipts(DBManager& db) {
    receipts = db.getFeeReceipts(studentID);  // Refresh
    cout << "\n=== Fee Receipts ===" << endl;
    if (receipts.empty()) {
        cout << "No receipts found." << endl;
        return;
    }
    cout << left << setw(10) << "ReceiptID" << setw(12) << "Amount" << setw(12) << "PaidOn" 
         << setw(20) << "Details" << "Status" << endl;
    for (const auto& r : receipts) {
        string id, date, details, status;
        double amount;
        tie(id, amount, date, details, status) = r;
        cout << left << setw(10) << id << setw(12) << fixed << setprecision(2) << amount 
             << setw(12) << date << setw(20) << details << status << endl;
    }
}

// Admin Methods
void Admin::viewAllStudents(DBManager& db) {
    vector<Student> students = db.getAllStudents();
    cout << "\n=== All Students ===" << endl;
    if (students.empty()) {
        cout << "No students found." << endl;
        return;
    }
    cout << left << setw(12) << "StudentID" << setw(20) << "Name" << setw(15) << "Department"
         << setw(6) << "Year" << setw(15) << "Contact" << setw(15) << "FeeStatus" << endl;
    for (const auto& s : students) {
        cout << left << setw(12) << s.studentID << setw(20) << s.name << setw(15) << s.department
             << setw(6) << s.year << setw(15) << s.contact << setw(15) << s.feeStatus << endl;
    }
}

void Admin::searchStudents(DBManager& db, string key, string value) {
    vector<Student> students = db.getAllStudents();
    cout << "\n=== Search Results (" << key << " = " << value << ") ===" << endl;
    bool found = false;
    for (const auto& s : students) {
        if ((key == "department" && s.department == value) ||
            (key == "year" && to_string(s.year) == value) ||
            (key == "name" && s.name.find(value) != string::npos)) {
            cout << s.studentID << " - " << s.name << " (" << s.department << ", Year " << s.year << ")\n";
            found = true;
        }
    }
    if (!found) cout << "No matches found." << endl;
}

void Admin::addStudent(DBManager& db) {
    Student s;
    cout << "\n=== Add New Student ===" << endl;
    cout << "StudentID: "; cin >> s.studentID;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');  // Clear input buffer
    cout << "Name: "; getline(cin, s.name);
    cout << "Department: "; getline(cin, s.department);
    cout << "Year (1-4): ";
    string yearStr;
    while (true) {
        getline(cin, yearStr);
        stringstream ss(yearStr);
        int y;
        if (ss >> y && y >= 1 && y <= 4 && ss.eof()) {
            s.year = y;
            break;
        }
        cout << "Invalid year. Enter 1-4: ";
    }
    cout << "Contact: "; getline(cin, s.contact);
    cout << "Academic Record: "; getline(cin, s.academicRecord);
    cout << "Fee Status (Paid/Pending/Overdue): "; getline(cin, s.feeStatus);
    cout << "Password: "; getline(cin, s.password);

    string escapedID = escapeString(db.conn, s.studentID);
    string escapedName = escapeString(db.conn, s.name);
    string escapedDept = escapeString(db.conn, s.department);
    string escapedContact = escapeString(db.conn, s.contact);
    string escapedRecord = escapeString(db.conn, s.academicRecord);
    string escapedStatus = escapeString(db.conn, s.feeStatus);
    string escapedPass = escapeString(db.conn, s.password);

    string query = "INSERT INTO Students (StudentID, Name, Department, Year, Contact, AcademicRecord, FeeStatus, Password) VALUES ('" +
                   escapedID + "', '" + escapedName + "', '" + escapedDept + "', " + to_string(s.year) + 
                   ", '" + escapedContact + "', '" + escapedRecord + "', '" + escapedStatus + "', '" + escapedPass + "')";
#ifndef USE_MOCK_DB
    if (db.executeQuery(query)) {
        cout << "Student added successfully!" << endl;
    } else {
        cout << "Failed to add student (ID may already exist)." << endl;
    }
#else
    for (const auto& st : db.mockStudents) {
        if (st.studentID == s.studentID) {
            cout << "Failed to add student (ID may already exist)." << endl;
            return;
        }
    }
    db.mockStudents.push_back(s);
    cout << "Student added successfully!" << endl;
#endif
}

void Admin::updateStudent(DBManager& db, string studentID) {
    Student s = db.getStudent(studentID);
    if (s.studentID.empty()) {
        cout << "Student not found." << endl;
        return;
    }
    cout << "\n=== Update Student (Current: " << s.name << ") ===" << endl;
    cout << "Leave blank to keep current value.\n";
    string input;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');  // Clear buffer
    cout << "Name (" << s.name << "): "; getline(cin, input); if (!input.empty()) s.name = input;
    cout << "Department (" << s.department << "): "; getline(cin, input); if (!input.empty()) s.department = input;
    cout << "Year (" << s.year << ", 1-4): "; getline(cin, input);
    if (!input.empty()) {
        stringstream ss(input);
        int y;
        if (ss >> y && y >= 1 && y <= 4 && ss.eof()) s.year = y;
        else cout << "Invalid year; keeping current." << endl;
    }
    cout << "Contact (" << s.contact << "): "; getline(cin, input); if (!input.empty()) s.contact = input;
    cout << "Academic Record (" << s.academicRecord << "): "; getline(cin, input); if (!input.empty()) s.academicRecord = input;
    cout << "Fee Status (" << s.feeStatus << "): "; getline(cin, input); if (!input.empty()) s.feeStatus = input;

    string escapedID = escapeString(db.conn, s.studentID);
    string escapedName = escapeString(db.conn, s.name);
    string escapedDept = escapeString(db.conn, s.department);
    string escapedContact = escapeString(db.conn, s.contact);
    string escapedRecord = escapeString(db.conn, s.academicRecord);
    string escapedStatus = escapeString(db.conn, s.feeStatus);

    string query = "UPDATE Students SET Name='" + escapedName + "', Department='" + escapedDept + 
                   "', Year=" + to_string(s.year) + ", Contact='" + escapedContact + 
                   "', AcademicRecord='" + escapedRecord + "', FeeStatus='" + escapedStatus + 
                   "' WHERE StudentID='" + escapedID + "'";
#ifndef USE_MOCK_DB
    if (db.executeQuery(query)) {
        cout << "Student updated successfully!" << endl;
    } else {
        cout << "Failed to update student." << endl;
    }
#else
    bool updated = false;
    for (auto& st : db.mockStudents) {
        if (st.studentID == s.studentID) {
            st = s;
            updated = true;
            break;
        }
    }
    cout << (updated ? "Student updated successfully!" : "Failed to update student.") << endl;
#endif
}

void Admin::deleteStudent(DBManager& db, string studentID) {
    Student s = db.getStudent(studentID);
    if (s.studentID.empty()) {
        cout << "Student not found." << endl;
        return;
    }
    cout << "\n=== Delete Student Confirmation ===" << endl;
    cout << "Are you sure you want to delete " << s.name << " (ID: " << studentID << ")? (y/n): ";
    char confirm;
    cin >> confirm;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');  // Clear buffer
    if (confirm == 'y' || confirm == 'Y') {
        string escapedID = escapeString(db.conn, studentID);
#ifndef USE_MOCK_DB
        // Delete related marks and receipts first (cascade)
        string delMarks = "DELETE FROM Marksheets WHERE StudentID='" + escapedID + "'";
        string delReceipts = "DELETE FROM FeeReceipts WHERE StudentID='" + escapedID + "'";
        string delStudent = "DELETE FROM Students WHERE StudentID='" + escapedID + "'";
        if (db.executeQuery(delMarks) && db.executeQuery(delReceipts) && db.executeQuery(delStudent)) {
            cout << "Student deleted successfully!" << endl;
        } else {
            cout << "Failed to delete student." << endl;
        }
#else
        bool erased = false;
        for (size_t i = 0; i < db.mockStudents.size(); ++i) {
            if (db.mockStudents[i].studentID == studentID) {
                db.mockStudents.erase(db.mockStudents.begin() + static_cast<long>(i));
                erased = true;
                break;
            }
        }
        cout << (erased ? "Student deleted successfully!" : "Failed to delete student.") << endl;
#endif
    } else {
        cout << "Deletion cancelled." << endl;
    }
}

void Admin::updateMarks(DBManager& db, string studentID) {
    Student s = db.getStudent(studentID);
    if (s.studentID.empty()) {
        cout << "Student not found." << endl;
        return;
    }
    cout << "\n=== Update Marks for " << s.name << " (ID: " << studentID << ") ===" << endl;
    s.viewMarksheet(db);  // Show current marks
    cout << "Enter subject name: ";
    string subject;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    getline(cin, subject);
    if (subject.empty()) {
        cout << "No subject entered. Cancelled." << endl;
        return;
    }
    cout << "Enter marks (0-100): ";
    string marksStr;
    while (true) {
        getline(cin, marksStr);
        stringstream ss(marksStr);
        int m;
        if (ss >> m && m >= 0 && m <= 100 && ss.eof()) {
            break;
        }
        cout << "Invalid marks. Enter 0-100: ";
    }
    int marks = stoi(marksStr);
    // Simple grade calculation (A:90+, B:80-89, C:70-79, D:60-69, F:<60)
    string grade;
    if (marks >= 90) grade = "A";
    else if (marks >= 80) grade = "B";
    else if (marks >= 70) grade = "C";
    else if (marks >= 60) grade = "D";
    else grade = "F";

    string escapedID = escapeString(db.conn, studentID);
    string escapedSubject = escapeString(db.conn, subject);
    string escapedGrade = escapeString(db.conn, grade);

#ifndef USE_MOCK_DB
    // Check if subject exists; update or insert
    string checkQuery = "SELECT * FROM Marksheets WHERE StudentID='" + escapedID + "' AND Subject='" + escapedSubject + "'";
    if (mysql_query(db.conn, checkQuery.c_str()) != 0) {
        cout << "Query Error: " << mysql_error(db.conn) << endl;
        return;
    }
    MYSQL_RES* res = mysql_store_result(db.conn);
    bool exists = (mysql_num_rows(res) > 0);
    mysql_free_result(res);

    string query;
    if (exists) {
        query = "UPDATE Marksheets SET Marks=" + to_string(marks) + ", Grade='" + escapedGrade + 
                "' WHERE StudentID='" + escapedID + "' AND Subject='" + escapedSubject + "'";
        cout << "Marks updated for " << subject << "." << endl;
    } else {
        query = "INSERT INTO Marksheets (StudentID, Subject, Marks, Grade) VALUES ('" + escapedID + 
                "', '" + escapedSubject + "', " + to_string(marks) + ", '" + escapedGrade + "')";
        cout << "Marks added for " << subject << "." << endl;
    }
    if (db.executeQuery(query)) {
        cout << "Operation successful! Grade: " << grade << endl;
    } else {
        cout << "Failed to update/add marks." << endl;
    }
#else
    // Update in-memory mock storage
    bool updated = false;
    for (auto& st : db.mockStudents) {
        if (st.studentID == studentID) {
            for (auto& entry : st.marks) {
                if (entry.first == subject) {
                    entry.second.first = marks;
                    entry.second.second = grade;
                    updated = true;
                    break;
                }
            }
            if (!updated) {
                st.marks.push_back({subject, {marks, grade}});
            }
            cout << (updated ? "Marks updated for " : "Marks added for ") << subject << "." << endl;
            cout << "Operation successful! Grade: " << grade << endl;
            return;
        }
    }
    cout << "Failed to update/add marks in mock storage." << endl;
#endif
}

void Admin::addFeeReceipt(DBManager& db, string studentID) {
    Student s = db.getStudent(studentID);
    if (s.studentID.empty()) {
        cout << "Student not found." << endl;
        return;
    }
    cout << "\n=== Add Fee Receipt for " << s.name << " (ID: " << studentID << ") ===" << endl;
    s.viewFeeReceipts(db);  // Show current receipts
    cout << "Receipt ID: "; 
    string receiptID;
    cin >> receiptID;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    cout << "Amount: ";
    string amountStr;
    while (true) {
        getline(cin, amountStr);
        stringstream ss(amountStr);
        double a;
        if (ss >> a && a > 0 && ss.eof()) {
            break;
        }
        cout << "Invalid amount. Enter positive number: ";
    }
    double amount = stod(amountStr);
    cout << "Paid On (YYYY-MM-DD): "; 
    string paidOn;
    getline(cin, paidOn);
    cout << "Transaction Details: "; 
    string details;
    getline(cin, details);
    cout << "Status (Paid/Pending): "; 
    string status;
    getline(cin, status);

    string escapedID = escapeString(db.conn, studentID);
    string escapedReceiptID = escapeString(db.conn, receiptID);
    string escapedPaidOn = escapeString(db.conn, paidOn);
    string escapedDetails = escapeString(db.conn, details);
    string escapedStatus = escapeString(db.conn, status);

    string query = "INSERT INTO FeeReceipts (ReceiptID, StudentID, Amount, PaidOn, TransactionDetails, Status) VALUES ('" +
                   escapedReceiptID + "', '" + escapedID + "', " + to_string(amount) + 
                   ", '" + escapedPaidOn + "', '" + escapedDetails + "', '" + escapedStatus + "')";
#ifndef USE_MOCK_DB
    if (db.executeQuery(query)) {
        cout << "Fee receipt added successfully!" << endl;
        // Update fee status to Paid if this is a full payment (simple logic)
        if (status == "Paid") {
            string updateStatus = "UPDATE Students SET FeeStatus='Paid' WHERE StudentID='" + escapedID + "'";
            db.executeQuery(updateStatus);
            cout << "Student fee status updated to Paid." << endl;
        }
    } else {
        cout << "Failed to add fee receipt (ID may already exist)." << endl;
    }
#else
    // Ensure uniqueness by receipt ID and then append
    for (auto& st : db.mockStudents) {
        if (st.studentID == studentID) {
            for (const auto& r : st.receipts) {
                if (std::get<0>(r) == receiptID) {
                    cout << "Failed to add fee receipt (ID may already exist)." << endl;
                    return;
                }
            }
            st.receipts.push_back(make_tuple(receiptID, amount, paidOn, details, status));
            if (status == "Paid") {
                st.feeStatus = "Paid";
            }
            cout << "Fee receipt added successfully!" << endl;
            if (status == "Paid") {
                cout << "Student fee status updated to Paid." << endl;
            }
            return;
        }
    }
    cout << "Failed to add fee receipt (student missing)." << endl;
#endif
}

// Main function with login and menu loops
int main() {
    DBManager db;
    Admin admin;
    Student currentStudent;
    string userType, id, password;
    bool loggedIn = false;
    bool isAdmin = false;

    cout << "=== College Student Office DBMS ===" << endl;
    cout << "1. Student Login\n2. Admin Login\n3. Exit\nChoice: ";
    int choice;
    cin >> choice;
    if (choice == 3) return 0;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');  // Clear buffer

    if (choice == 1) userType = "student";
    else if (choice == 2) userType = "admin";
    else {
        cout << "Invalid choice. Exiting." << endl;
        return 1;
    }

    cout << "Enter " << userType << " ID: ";
    getline(cin, id);
    cout << "Enter Password: ";
    getline(cin, password);

    if (db.login(userType, id, password)) {
        loggedIn = true;
        isAdmin = (userType == "admin");
        if (isAdmin) {
            cout << "Admin login successful!" << endl;
        } else {
            currentStudent = db.getStudent(id);
            cout << "Student login successful!" << endl;
        }
    } else {
        cout << "Login failed. Invalid credentials." << endl;
        return 1;
    }

    // Session loop
    while (loggedIn) {
        if (isAdmin) {
            // Admin Menu
            cout << "\n=== Admin Menu ===" << endl;
            cout << "1. View All Students\n2. Search Students\n3. Add Student\n4. Update Student\n5. Delete Student\n6. Update Marks\n7. Add Fee Receipt\n8. Logout\nChoice: ";
            int adminChoice;
            cin >> adminChoice;
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            string tempID;
            switch (adminChoice) {
                case 1: admin.viewAllStudents(db); break;
                case 2: {
                    cout << "Search by (department/year/name): ";
                    string key; getline(cin, key);
                    cout << "Value: "; string value; getline(cin, value);
                    admin.searchStudents(db, key, value);
                    break;
                }
                case 3: admin.addStudent(db); break;
                case 4: {
                    cout << "Enter Student ID: "; getline(cin, tempID);
                    admin.updateStudent(db, tempID);
                    break;
                }
                case 5: {
                    cout << "Enter Student ID: "; getline(cin, tempID);
                    admin.deleteStudent(db, tempID);
                    break;
                }
                case 6: {
                    cout << "Enter Student ID: "; getline(cin, tempID);
                    admin.updateMarks(db, tempID);
                    break;
                }
                case 7: {
                    cout << "Enter Student ID: "; getline(cin, tempID);
                    admin.addFeeReceipt(db, tempID);
                    break;
                }
                case 8: {
                    loggedIn = false;
                    cout << "Logged out." << endl;
                    break;
                }
                default: cout << "Invalid choice." << endl;
            }
        } else {
            // Student Menu
            cout << "\n=== Student Menu ===" << endl;
            cout << "1. View Profile\n2. View Marksheet\n3. View Fee Receipts\n4. Logout\nChoice: ";
            int studentChoice;
            cin >> studentChoice;
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            switch (studentChoice) {
                case 1: currentStudent.viewProfile(); break;
                case 2: currentStudent.viewMarksheet(db); break;
                case 3: currentStudent.viewFeeReceipts(db); break;
                case 4: {
                    loggedIn = false;
                    cout << "Logged out." << endl;
                    break;
                }
                default: cout << "Invalid choice." << endl;
            }
        }
    }

    return 0;
}
