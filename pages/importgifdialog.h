#ifndef IMPORTGIFDIALOG_H
#define IMPORTGIFDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QSpinBox>
#include <QPushButton>
#include <QLabel>
#include <QComboBox>

#include "pages/addactiondialog.h"

class ImportGifDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ImportGifDialog(const QString &petDirPath, QWidget *parent = nullptr);

    QString gifPath() const;
    QString actionId() const;
    QString actionName() const;
    int fps() const;

    TargetCategory targetCategory() const;
    int timedIntervalSeconds() const;
    QString emotionName() const;

    void clearForm();

private slots:
    void onBrowseGif();
    void onConfirm();
    void onCategoryChanged(int index);

private:
    void setupUi();
    void connectSignals();
    bool validateInput();
    void autoFillFromGifFileName(const QString &gifPath);
    void updateExtraConfigVisibility();

    QString m_petDirPath;

    QLineEdit *m_gifPathEdit;
    QPushButton *m_browseButton;
    QLineEdit *m_idEdit;
    QLineEdit *m_nameEdit;
    QSpinBox *m_fpsSpinBox;
    QLabel *m_frameCountLabel;

    QComboBox *m_categoryComboBox;
    QLabel *m_timedIntervalLabel;
    QSpinBox *m_timedIntervalSpinBox;
    QLabel *m_emotionLabel;
    QComboBox *m_emotionComboBox;

    QPushButton *m_confirmButton;
    QPushButton *m_cancelButton;

    int m_detectedFrameCount;
    int m_detectedFps;
};

#endif // IMPORTGIFDIALOG_H
