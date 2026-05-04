#ifndef NEWACTIONDIALOG_H
#define NEWACTIONDIALOG_H

#include <QComboBox>
#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>

#include "core/actioncategory.h"

class NewActionDialog : public QDialog
{
    Q_OBJECT

public:
    explicit NewActionDialog(const QString &petDirPath, QWidget *parent = nullptr);

    QString gifPath() const;
    QString actionId() const;
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

#endif // NEWACTIONDIALOG_H
