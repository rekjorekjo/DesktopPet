#ifndef NEWPETDIALOG_H
#define NEWPETDIALOG_H

#include <QDialog>
#include <QSize>

class QLineEdit;
class QSpinBox;

class NewPetDialog : public QDialog
{
    Q_OBJECT

public:
    explicit NewPetDialog(QWidget *parent = nullptr);

    QString petId() const;
    QString petName() const;
    QSize canvasSize() const;
    QSize displaySize() const;

private slots:
    void accept() override;

private:
    void setupUi();
    bool validateInput();

    QLineEdit *m_petIdEdit;
    QLineEdit *m_petNameEdit;
    QSpinBox *m_canvasWidthSpinBox;
    QSpinBox *m_canvasHeightSpinBox;
    QSpinBox *m_displayWidthSpinBox;
    QSpinBox *m_displayHeightSpinBox;
};

#endif // NEWPETDIALOG_H
