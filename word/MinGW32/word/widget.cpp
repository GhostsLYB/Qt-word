#include "widget.h"
#include "ui_widget.h"
#include <QtDebug>

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);
    this->resize(230,350);
    this->setWindowFlag(Qt::FramelessWindowHint);

    //布局（ui.label 要动态布局）
    QVBoxLayout * vLayout = new QVBoxLayout(this);
    vLayout->addWidget(ui->plainTextEdit);
    this->setLayout(vLayout);
    ui->label->move(-100,-100);//将label移出视口

    //设置QPlainTextEdit的字体
    QFont font = ui->plainTextEdit->font();
    font.setFamily("楷体");
    font.setPointSize(12);
    ui->plainTextEdit->setFont(font);

    //设置控件QSizeGrip
    grip = new QSizeGrip(this);
    grip->resize(10,10);
    grip->move(this->width()-grip->width(),this->height()-grip->height());

    //加载qss文件
    QFile fQss(":/style.qss");
    if(fQss.open(QFile::ReadOnly))
        qApp->setStyleSheet(fQss.readAll());
    else
        QMessageBox::critical(this,"非致命错误","配置文件style.qss打开失败");

    //文内容改变后将保存状态置为false
    connect(ui->plainTextEdit,&QPlainTextEdit::textChanged,[=](){
        isSaved = false;
    });

    //打开文件变化了
    connect(this,&Widget::currentFileChanged,this,&Widget::on_currentFileChanged);

    //初始化各状态
    resizeable = true;
    controlPressed = false;
    shiftPressed = false;
    isSaved = true;
    isCancel = false;
}

void Widget::mousePressEvent(QMouseEvent *e)
{
    resizeable = true;
    QPoint y = e->globalPos();
    QPoint x = this->geometry().topLeft();
    z = y - x;
    QWidget::mousePressEvent(e);
}
void Widget::mouseMoveEvent(QMouseEvent *e)
{
    QPoint y = e->globalPos();
    QPoint x = y - z;
    if(resizeable)
        this->move(x);
    QWidget::mouseMoveEvent(e);
}
void Widget::mousereleaseEvent(QMouseEvent *e)
{
    z = QPoint();
    resizeable = true;
    QWidget::mouseReleaseEvent(e);
}
void Widget::resizeEvent(QResizeEvent * e)
{
    resizeable = false;
    grip->move(this->width()-grip->width(),
               this->height()-grip->height());
    QWidget::resizeEvent(e);
}
void Widget::keyPressEvent(QKeyEvent* e)
{
    if(e->key() == Qt::Key_Control)
        controlPressed = true;
    if(e->key() == Qt::Key_Shift)
        shiftPressed = true;
    if(e->key() == Qt::Key_O && shiftPressed && controlPressed)
        closeFile();
    if(e->key() == Qt::Key_S && shiftPressed && controlPressed)
    {
        saveAs();
        shiftPressed = false;
    }
    if(e->key() == Qt::Key_C && shiftPressed && controlPressed)
    {
        alterColor();
        shiftPressed = false;
    }
    if(e->key() == Qt::Key_S && controlPressed)
    {
        if(currentFileName.isEmpty())
            saveAs();
        else
            save();
    }
    if(e->key() == Qt::Key_O && controlPressed)
        openFile();
    if(e->key() == Qt::Key_N && controlPressed)
        newFile();
    if(e->key() == Qt::Key_H && controlPressed)
        help();
    QWidget::keyPressEvent(e);
}
void Widget::closeEvent(QCloseEvent* e)
{
    Q_UNUSED(e);
    if(isSaved)
        return;
    if(ui->plainTextEdit->toPlainText().isEmpty())
        return;
    QMessageBox::StandardButton b = QMessageBox::question(this,"提示",
                                                          "当前文本未保存，是否保存？",
                                                          QMessageBox::Yes|QMessageBox::No,
                                                          QMessageBox::Yes);
    if(b == QMessageBox::Yes)
    {
        if(currentFileName.isEmpty())
            saveAs();
        else
            save();
    }
}
void Widget::keyReleaseEvent(QKeyEvent * e)
{
    if(e->key() == Qt::Key_Shift)
        shiftPressed = false;
    if(e->key() == Qt::Key_Control)
        controlPressed = false;
    QWidget::keyReleaseEvent(e);

}
void Widget::closeFile()
{
    checkSave();
    if(isCancel)
    {
        isCancel = false;
        return;
    }
    ui->plainTextEdit->clear();
    currentFileName = "";
    emit currentFileChanged();
    shiftPressed = false;
    controlPressed = false;
}
void Widget::openFile(QString fName)
{
    if(isCancel)
    {
        isCancel = false;
        return;
    }
    bool b = fName.isEmpty();
    QString fileName;
    if(b)
        fileName = QFileDialog::getOpenFileName(this,"打开文本文件",
                                                    QDir::currentPath(),".txt文件(*.txt)");
    else
        fileName = fName;
    if(fileName.isEmpty())
    {
        //QMessageBox::warning(this,"waring","The file name is empty!");
        return ;
    }

    QFile file(fileName);
    //qDebug()<<fileName;
    if(!file.open(QFile::ReadOnly))
    {
        QMessageBox::warning(this,"warning","file open fail!");
        return;
    }
    checkSave();//一定要在currentFileName改变之前检查，（可能会用到改变前的文件名保存内容）
    ui->plainTextEdit->clear();
    ui->plainTextEdit->setPlainText(file.readAll());
    currentFileName = fileName;
    emit currentFileChanged();
    file.close();
    isSaved = true;
}
void Widget::newFile()
{
    if(isCancel)
    {
        isCancel = false;
        return;
    }
    QString fileName = QFileDialog::getSaveFileName(this,"新建文本文件",
                                                    QDir::currentPath(),"txt(*.txt)");
    if(fileName.isEmpty())
    {
        //QMessageBox::warning(this,"waring","The file name is empty!");
        return ;
    }
    QFile f(fileName);
    f.open(QFile::WriteOnly);
    f.write("");
    f.close();
    openFile(fileName);//检查保存 在openFile中执行
}
void Widget::save()
{
    QString text = ui->plainTextEdit->toPlainText();
    //qDebug()<<text;
    QFile f(currentFileName);
    f.open(QIODevice::WriteOnly | QIODevice::Text);
    if(!f.isOpen())
    {
        //qDebug()<<currentFileName;
        //QMessageBox::warning(this,"warning","file"+currentFileName+" open fail!");
        return;
    }
    f.write(text.toUtf8());
    f.close();
    isSaved = true;             //保存后将保存状态置为true
    shiftPressed = false;
    controlPressed = false;
}
void Widget::saveAs()
{
    QString fileName = QFileDialog::getSaveFileName(this,"另存为",
                                                    QDir::currentPath(),"txt(*.txt)");
    if(fileName.isEmpty())
        return;
    QFile f(fileName);
    if(!f.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QMessageBox::warning(this,"警告","另存文件打开失败！");
        return;
    }
    QString text = ui->plainTextEdit->toPlainText();
    f.write(text.toUtf8());
    f.close();
    isSaved = true;             //保存后将保存状态置为true
    if(currentFileName.isEmpty())
    {
        currentFileName = fileName;
        emit currentFileChanged();
    }
}
void Widget::help()
{
    QString str = "Alt + F4 : 退出\n"
                  "Control + H : 帮助\n"
                  "Control + O : 打开文件\n"
                  "Control + N : 新建文件\n"
                  "Control + S :  保存\n"
                  "Control + Shift + S : 另存\n"
                  "Control + Shift + C : 选择背景颜色\n"
                  "Control + Shift + O :关闭当前文件\n";
    QMessageBox::information(this,"帮助",str);
}
void Widget::alterColor()
{
    QColor oldColor = ui->plainTextEdit->palette().color(QPalette::Background);
    QColor newColor = QColorDialog::getColor(oldColor,this,"选择背景颜色");
    if(newColor.isValid())
    {
        this->setStyleSheet(QString("background-color:rgb(%1,%2,%3);")
                            .arg(newColor.red()).arg(newColor.green()).arg(newColor.blue()));
    }
}
void Widget::on_currentFileChanged()
{
    if(ui->label->text().isEmpty())//路径标签文本为空
    {
        if(!currentFileName.isEmpty())
        {
            QVBoxLayout * layout = static_cast<QVBoxLayout*>(this->layout());
            layout->addWidget(ui->label);
            ui->label->setText(currentFileName);
            ui->label->setHidden(false);
        }
    }
    else                           //路径标签中有路径
    {
        QVBoxLayout * layout = static_cast<QVBoxLayout*>(this->layout());
        if(currentFileName.isEmpty())
        {
            layout->takeAt(1);
            ui->label->setHidden(true);
        }
        ui->label->setText(currentFileName);
    }

}
void Widget::checkSave()
{
    if(isSaved)//当前内容已经被保存
        return;
    if(ui->plainTextEdit->toPlainText().isEmpty())//当前内容为空
        return;
    QMessageBox::StandardButton b = QMessageBox::Yes;
    b = QMessageBox::question(this,"提示",
                             "当前文本未保存，是否保存？",
                             QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel,
                             QMessageBox::Yes);
    if(b == QMessageBox::Cancel)
    {
        isCancel = true;
        return;
    }
    if(b == QMessageBox::Yes)
    {
        if(currentFileName.isEmpty())
            saveAs();
        else
            save();
    }
    else
        return;
}
Widget::~Widget()
{
    delete ui;
}
