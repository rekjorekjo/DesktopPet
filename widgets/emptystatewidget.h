#ifndef EMPTYSTATEWIDGET_H
#define EMPTYSTATEWIDGET_H

#include <QWidget>

class QLabel;

class EmptyStateWidget : public QWidget
{
    Q_OBJECT

public:
    explicit EmptyStateWidget(QWidget *parent = nullptr);

    void setTitle(const QString &title);
    void setDescription(const QString &description);
    void setContent(const QString &content);

    void clear();

public slots:
    void applyTheme();

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    void setupUi();

    QLabel *m_titleLabel;
    QLabel *m_descriptionLabel;
    QLabel *m_contentLabel;
};

#endif // EMPTYSTATEWIDGET_H
