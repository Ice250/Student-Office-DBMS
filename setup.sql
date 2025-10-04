-- =============================================
-- College Student Office DBMS Setup Script
-- Database: bvp_student_office
-- Tables: Students, Admins, Marksheets, FeeReceipts
-- Sample Data Included for Testing
-- =============================================

-- Create Database (if not exists)
CREATE DATABASE IF NOT EXISTS bvp_student_office;
USE bvp_student_office;

-- Drop tables if they exist (for clean setup; comment out if you want to preserve data)
DROP TABLE IF EXISTS FeeReceipts;
DROP TABLE IF EXISTS Marksheets;
DROP TABLE IF EXISTS Students;
DROP TABLE IF EXISTS Admins;

-- Create Admins Table
-- Stores admin credentials and basic info
CREATE TABLE Admins (
    AdminID VARCHAR(20) PRIMARY KEY,
    Name VARCHAR(100) NOT NULL,
    Department VARCHAR(50),
    Contact VARCHAR(100),
    Password VARCHAR(255) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- Create Students Table
-- Main table for student profiles
CREATE TABLE Students (
    StudentID VARCHAR(20) PRIMARY KEY,
    Name VARCHAR(100) NOT NULL,
    Department VARCHAR(50) NOT NULL,
    Year INT NOT NULL CHECK (Year >= 1 AND Year <= 4),
    Contact VARCHAR(100),
    AcademicRecord TEXT,
    FeeStatus ENUM('Paid', 'Pending', 'Overdue') DEFAULT 'Pending',
    Password VARCHAR(255) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- Create Marksheets Table
-- Stores student marks and grades (one row per subject per student)
CREATE TABLE Marksheets (
    StudentID VARCHAR(20),
    Subject VARCHAR(50) NOT NULL,
    Marks INT NOT NULL CHECK (Marks >= 0 AND Marks <= 100),
    Grade VARCHAR(5) NOT NULL,
    PRIMARY KEY (StudentID, Subject),
    FOREIGN KEY (StudentID) REFERENCES Students(StudentID) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- Create FeeReceipts Table
-- Stores fee payment receipts for students
CREATE TABLE FeeReceipts (
    ReceiptID VARCHAR(20) PRIMARY KEY,
    StudentID VARCHAR(20) NOT NULL,
    Amount DECIMAL(10, 2) NOT NULL CHECK (Amount > 0),
    PaidOn DATE NOT NULL,
    TransactionDetails TEXT,
    Status ENUM('Paid', 'Pending') DEFAULT 'Pending',
    FOREIGN KEY (StudentID) REFERENCES Students(StudentID) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- =============================================
-- Insert Sample Data for Testing
-- =============================================

-- Sample Admin (Login: ADMIN001 / adminpass)
INSERT INTO Admins (AdminID, Name, Department, Contact, Password) VALUES
('ADMIN001', 'Admin User', 'Information Technology', 'admin@college.edu', 'adminpass');

-- Sample Students (Login: STU001 / studpass; STU002 / studpass2)
INSERT INTO Students (StudentID, Name, Department, Year, Contact, AcademicRecord, FeeStatus, Password) VALUES
('STU001', 'John Doe', 'Computer Science', 2, 'john.doe@email.com', 'Excellent academic performance, no disciplinary issues.', 'Paid', 'studpass'),
('STU002', 'Jane Smith', 'Electronics Engineering', 3, 'jane.smith@email.com', 'Good standing, participated in tech fests.', 'Pending', 'studpass2');

-- Sample Marks for STU001 (viewable in marksheet)
INSERT INTO Marksheets (StudentID, Subject, Marks, Grade) VALUES
('STU001', 'Mathematics', 85, 'B'),
('STU001', 'Physics', 92, 'A'),
('STU001', 'Programming', 78, 'A');

INSERT INTO Marksheets (StudentID, Subject, Marks, Grade) VALUES
('STU002', 'Mathematics', 90, 'B'),
('STU002', 'Physics', 92, 'A'),
('STU002', 'Programming', 98, 'C');

-- Sample Fee Receipt for STU001 (viewable in fee receipts)
INSERT INTO FeeReceipts (ReceiptID, StudentID, Amount, PaidOn, TransactionDetails, Status) VALUES
('REC001', 'STU001', 5000.00, '2023-09-01', 'Annual Tuition Fee Payment via Online Banking', 'Paid');

-- =============================================
-- Verification Queries (Run these to test)
-- =============================================
-- SELECT * FROM Admins;
-- SELECT * FROM Students;
-- SELECT * FROM Marksheets WHERE StudentID = 'STU001';
-- SELECT * FROM FeeReceipts WHERE StudentID = 'STU001';

-- Success Message
SELECT 'Database setup completed successfully! Tables created with sample data.' AS Status;
