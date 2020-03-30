#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QMouseEvent>
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QSizeGrip>
#include <QResizeEvent>
#include <QTextStream>
#include <QColorDialog>
#include <QPalette>
#include <QColor>
#include <QLayout>
#include <QLabel>

namespace Ui {
class Widget;
}
class Widget : public QWidget
{
    Q_OBJECT
private:
    QPoint z;                   //窗口坐标系中，鼠标点击的位置的坐标。
    QSizeGrip * grip;           //窗口左下角由于窗口缩放的控件
    QString currentFileName;    //当前打开的文件绝对路径
    bool resizeable;            //控制窗口是否可以移动
    bool controlPressed;        //control是否按下
    bool shiftPressed;          //shift是否按下
    bool isSaved;               //文件是否已经保存
    bool isCancel;              //是否返回（不执行后面的其他操作）
private slots:
    void on_currentFileChanged();
signals:
    void currentFileChanged();
public:
    explicit Widget(QWidget *parent = nullptr);
    ~Widget();

    //鼠标点击、移动、释放 事件
    void mousePressEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);
    void mousereleaseEvent(QMouseEvent *);

    //主窗口大小重置事件
    void resizeEvent(QResizeEvent *);

    //键盘按下、释放事件
    void keyPressEvent(QKeyEvent*);
    void keyReleaseEvent(QKeyEvent*);

    //窗口关闭事件
    void closeEvent(QCloseEvent*);

    //关闭、打开、新建、保存、另存为和帮助 函数
    void closeFile();
    void openFile(QString = "");
    void newFile();
    void save();
    void saveAs();
    void help();

    //更改背景颜色
    void alterColor();

    //根据不同状态以不同方式保存
    void checkSave();
private:
    Ui::Widget *ui;
};
#endif // WIDGET_H
