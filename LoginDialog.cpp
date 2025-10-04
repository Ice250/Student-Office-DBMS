#include "LoginDialog.h"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

LoginDialog::LoginDialog(AuthenticateFunction authenticate, QWidget* parent)
    : QDialog(parent), m_authenticate(std::move(authenticate)), m_userTypeCombo(nullptr), m_idEdit(nullptr), m_passwordEdit(nullptr), m_loginButton(nullptr), m_cancelButton(nullptr) {
  setWindowTitle("Login");
  setModal(true);

  auto* layout = new QVBoxLayout(this);
  auto* formLayout = new QFormLayout();

  m_userTypeCombo = new QComboBox(this);
  m_userTypeCombo->addItem("Student");
  m_userTypeCombo->addItem("Admin");

  m_idEdit = new QLineEdit(this);
  m_idEdit->setPlaceholderText("Enter ID");

  m_passwordEdit = new QLineEdit(this);
  m_passwordEdit->setEchoMode(QLineEdit::Password);
  m_passwordEdit->setPlaceholderText("Enter password");

  formLayout->addRow("User Type:", m_userTypeCombo);
  formLayout->addRow("ID:", m_idEdit);
  formLayout->addRow("Password:", m_passwordEdit);

  layout->addLayout(formLayout);

  auto* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
  m_loginButton = buttonBox->button(QDialogButtonBox::Ok);
  m_cancelButton = buttonBox->button(QDialogButtonBox::Cancel);

  layout->addWidget(buttonBox);

  connect(m_loginButton, &QPushButton::clicked, this, &LoginDialog::onAccept);
  connect(m_cancelButton, &QPushButton::clicked, this, &LoginDialog::reject);
}

QString LoginDialog::userType() const { return m_selectedUserType; }
QString LoginDialog::userId() const { return m_enteredUserId; }
bool LoginDialog::isAdmin() const { return m_selectedUserType.compare("admin", Qt::CaseInsensitive) == 0; }

void LoginDialog::onAccept() {
  const QString selected = m_userTypeCombo->currentText();
  const QString type = (selected.compare("Admin", Qt::CaseInsensitive) == 0) ? "admin" : "student";
  const QString id = m_idEdit->text().trimmed();
  const QString password = m_passwordEdit->text();

  if (id.isEmpty() || password.isEmpty()) {
    QMessageBox::warning(this, "Missing information", "Please enter both ID and password.");
    return;
  }

  if (!m_authenticate) {
    QMessageBox::critical(this, "Internal error", "Authentication handler is not set.");
    return;
  }

  const bool ok = m_authenticate(type.toStdString(), id.toStdString(), password.toStdString());
  if (!ok) {
    QMessageBox::warning(this, "Login failed", "Invalid credentials. Please try again.");
    return;
  }

  m_selectedUserType = type;
  m_enteredUserId = id;
  accept();
}
