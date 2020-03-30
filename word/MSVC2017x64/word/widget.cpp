#include "widget.h"
#include "ui_widget.h"
#include <QtDebug>

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);
    this->setWindowFlag(Qt::FramelessWindowHint);

    //布局（ui.label 要动态布局）
    QVBoxLayout * vLayout = new QVBoxLayout(this);
    vLayout->addWidget(ui->plainTextEdit);
    this->setLayout(vLayout);

    //设置QPlainTextEdit的字体
    QFont font = ui->plainTextEdit->font();
    font.setFamily(tr("楷体"));
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
        QMessageBox::critical(this,"error","file style.qss open fail!");

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
        checkSave();
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
    QMessageBox::StandardButton b = QMessageBox::question(this,"Question",
                                                          "Do you want save this file?",
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
    checkSave();
    if(isCancel)
    {
        isCancel = false;
        return;
    }
    bool b = fName.isEmpty();
    QString fileName;
    if(b)
        fileName = QFileDialog::getOpenFileName(this,"open a text file",
                                                    QDir::currentPath(),".text file(*.txt)");
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
    ui->plainTextEdit->clear();
    ui->plainTextEdit->setPlainText(file.readAll());
    currentFileName = fileName;
    emit currentFileChanged();
    file.close();
    isSaved = true;
}
void Widget::newFile()
{
    checkSave();
    if(isCancel)
    {
        isCancel = false;
        return;
    }
    QString fileName = QFileDialog::getSaveFileName(this,"create a new file",
                                                    QDir::currentPath(),"text file(*.txt)");
    if(fileName.isEmpty())
    {
        //QMessageBox::warning(this,"waring","The file name is empty!");
        return ;
    }
    QFile f(fileName);
    f.open(QFile::WriteOnly);
    f.write("");
    f.close();
    openFile(fileName);
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
    QString fileName = QFileDialog::getSaveFileName(this,"Save As",
                                                    QDir::currentPath(),"text file(*.txt)");
    if(fileName.isEmpty())
        return;
    QFile f(fileName);
    if(!f.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QMessageBox::warning(this,"warning","The file save as fail!");
        return;
    }
    QString text = ui->plainTextEdit->toPlainText();
    f.write(text.toUtf8());
    f.close();
    isSaved = true;             //保存后将保存状态置为true

}
void Widget::help()
{
    QString str = "Alt + F4 : Quit\n"
                  "Control + H : Help\n"
                  "Control + O : Open file\n"
                  "Control + N : New file\n"
                  "Control + S :  Save file\n"
                  "Control + Shift + S : Save as file\n"
                  "Control + Shift + C : Choose background color\n"
                  "Control + Shift + O :Close current file\n";
    QMessageBox::information(this,"Help",str);
}
void Widget::alterColor()
{
    QColor oldColor = ui->plainTextEdit->palette().color(QPalette::Background);
    QColor newColor = QColorDialog::getColor(oldColor,this,"Choose Background Color");
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
    if(isSaved)
        return;
    if(ui->plainTextEdit->toPlainText().isEmpty())
        return;
    QMessageBox::StandardButton b = QMessageBox::question(this,"Question",
                                                          "Do you want save this file?",
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
