#ifndef APICONFIGDIALOG_H
#define APICONFIGDIALOG_H

#include <QDialog>
#include <QString>
#include <QStringList>

#include "models/apiconfig.h"

class QLineEdit;
class QLabel;
class QPushButton;
class SoftDialogTitleBar;

class ApiConfigDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ApiConfigDialog(QWidget *parent = nullptr);

    void setTitle(const QString &title);

    void setProfileName(const QString &name);
    QString profileName() const;
    void setNameEditable(bool editable);

    void setApiConfig(const ApiConfig &config);
    ApiConfig apiConfig() const;

    void setExistingNames(const QStringList &names);
    void setValidateProfileName(bool validate);

    // New profile: name editable, validated in-dialog
    static bool getNewProfile(QWidget *parent,
                              const QStringList &existingNames,
                              QString *outName,
                              ApiConfig *outConfig);

    // Edit profile: name read-only, pre-filled with initialConfig
    static bool editProfile(QWidget *parent,
                            const QString &profileName,
                            const ApiConfig &initialConfig,
                            ApiConfig *outConfig);

private slots:
    void onAccept();
    void onReject();

private:
    void setupUi();

    SoftDialogTitleBar *m_titleBar;
    QLabel *m_nameLabel;
    QLineEdit *m_nameEdit;
    QLabel *m_apiKeyLabel;
    QLineEdit *m_apiKeyEdit;
    QLabel *m_baseUrlLabel;
    QLineEdit *m_baseUrlEdit;
    QLabel *m_modelLabel;
    QLineEdit *m_modelEdit;
    QPushButton *m_okButton;
    QPushButton *m_cancelButton;
    bool m_accepted;

    QStringList m_existingNames;
    bool m_validateProfileName;
};

#endif // APICONFIGDIALOG_H
