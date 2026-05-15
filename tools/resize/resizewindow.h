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
#include <QColor>

enum class ResizeResult {
    Success,
    Skipped,
    Failed
};

struct ResizeToolTheme {
    QColor windowBg;
    QColor panelBg;
    QColor text;
    QColor mutedText;
    QColor border;
    QColor accent;
    QColor accentHover;
    QColor accentPressed;
    QColor buttonBg;
    QColor buttonText;
    QColor inputBg;
    QColor inputBorder;
    QColor inputFocusBorder;
    QColor logBg;
    QColor checkboxBorder;
    QColor checkboxCheckedBg;
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
    void autoFillBackupDir();
    void log(const QString &message);

    // Theme
    ResizeToolTheme loadAppTheme() const;
    void applyAppStyle();
    QString buildStyleSheet(const ResizeToolTheme &theme) const;

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
    bool m_backupDirUserCustom;
};

#endif // RESIZEWINDOW_H
