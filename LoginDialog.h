#pragma once

#include <QDialog>
#include <functional>
#include <string>

class QComboBox;
class QLineEdit;
class QPushButton;

class LoginDialog : public QDialog {
  Q_OBJECT
public:
  using AuthenticateFunction = std::function<bool(const std::string&, const std::string&, const std::string&)>;

  explicit LoginDialog(AuthenticateFunction authenticate, QWidget* parent = nullptr);

  QString userType() const;
  QString userId() const;
  bool isAdmin() const;

private slots:
  void onAccept();

private:
  AuthenticateFunction m_authenticate;
  QComboBox* m_userTypeCombo;
  QLineEdit* m_idEdit;
  QLineEdit* m_passwordEdit;
  QPushButton* m_loginButton;
  QPushButton* m_cancelButton;

  QString m_selectedUserType;
  QString m_enteredUserId;
};
