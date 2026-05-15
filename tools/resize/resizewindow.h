#ifndef RESIZEWINDOW_H
#define RESIZEWINDOW_H

#include <QMainWindow>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QCheckBox>
#include <QRadioButton>
#include <QTextEdit>
#include <QLabel>
#include <QGroupBox>

enum class ResizeResult {
    Success,
    Skipped,
    Failed
};

class ResizeWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit ResizeWindow(QWidget *parent = nullptr);
    ~ResizeWindow() = default;

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;

private slots:
    void onSelectPetJson();
    void onSelectActionDir();
    void onSelectBackupDir();
    void onStartProcessing();
    void onBackupToggled(bool checked);

private:
    void setupUi();
    void loadPetJson(const QString &path);
    void processImages();
    bool backupActionDir(const QString &sourceDir, const QString &backupDir);
    ResizeResult resizeImage(const QString &inputPath, int targetWidth, int targetHeight, bool keepAspectRatio);
    QString inferBackupDir(const QString &actionDir) const;
    void updateBackupDirPlaceholder();
    void log(const QString &message);

    // UI elements
    QLineEdit *m_petJsonPathEdit;
    QLineEdit *m_actionDirPathEdit;
    QLineEdit *m_backupDirPathEdit;
    QSpinBox *m_widthSpinBox;
    QSpinBox *m_heightSpinBox;
    QCheckBox *m_backupCheckBox;
    QRadioButton *m_keepAspectRadio;
    QRadioButton *m_stretchRadio;
    QPushButton *m_startButton;
    QPushButton *m_selectBackupDirButton;
    QTextEdit *m_logTextEdit;

    // Data
    int m_targetWidth;
    int m_targetHeight;
    QString m_actionId;
};

#endif // RESIZEWINDOW_H
