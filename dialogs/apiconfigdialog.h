#ifndef APICONFIGDIALOG_H
#define APICONFIGDIALOG_H

#include <QDialog>
#include <QString>
#include <QStringList>

#include "models/apiconfig.h"

class QComboBox;
class QPlainTextEdit;
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

signals:
    void submitted(const QString &profileName, const ApiConfig &config);

private slots:
    void onSubmit();
    void onReject();
    void onProviderChanged(int index);

private:
    void setupUi();
    void loadProviderTemplate(int presetIndex);
    bool isCurrentTemplateModifiedFromPreset() const;

    SoftDialogTitleBar *m_titleBar;

    QLabel *m_nameLabel;
    QLineEdit *m_nameEdit;

    QLabel *m_providerLabel;
    QComboBox *m_providerCombo;

    QLabel *m_formatLabel;
    QComboBox *m_formatCombo;

    QLabel *m_templateLabel;
    QPlainTextEdit *m_templateEdit;

    QPushButton *m_okButton;
    QPushButton *m_cancelButton;

    QStringList m_existingNames;
    bool m_validateProfileName;
    int m_lastProviderIndex;
    bool m_updatingProviderCombo;
};

#endif // APICONFIGDIALOG_H
