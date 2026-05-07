#ifndef SOFTINPUTDIALOG_H
#define SOFTINPUTDIALOG_H

#include <QDialog>
#include <QString>

class QLineEdit;
class QLabel;
class QPushButton;

class SoftInputDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SoftInputDialog(QWidget *parent = nullptr);

    void setTitle(const QString &title);
    void setLabelText(const QString &label);
    void setTextValue(const QString &text);
    QString textValue() const;

    static QString getText(QWidget *parent,
                           const QString &title,
                           const QString &label,
                           const QString &text = QString(),
                           bool *ok = nullptr);

private slots:
    void onAccept();
    void onReject();

private:
    void setupUi();

    QLabel *m_labelLabel;
    QLineEdit *m_lineEdit;
    QPushButton *m_okButton;
    QPushButton *m_cancelButton;
    bool m_accepted;
};

#endif // SOFTINPUTDIALOG_H
