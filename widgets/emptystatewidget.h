#ifndef EMPTYSTATEWIDGET_H
#define EMPTYSTATEWIDGET_H

#include <QWidget>

class QLabel;

// 空状态提示组件，用于显示列表为空时的提示信息
//
// 职责：
// - 在列表、表格等区域为空时显示友好的提示
// - 支持标题、描述和内容的自定义
// - 支持主题切换
//
// 使用场景：
// - 宠物列表为空时显示"尚未创建宠物"
// - 动作库为空时显示"暂无动作"
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
