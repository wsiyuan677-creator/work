#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "qcustomplot.h"
#include <QtGlobal>
#include <QTcpSocket>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

     void onReadyRead();

     void ononspectrumDataReady(QVector<double> chA,QVector<double> chB);
protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

signals:
    void spectrumDataReady(QVector<double> chA,QVector<double> chB);

private:
    Ui::MainWindow *ui;

    // 十字线成员变量
    QCPItemStraightLine *crosshairH; // 水平十字线
    QCPItemStraightLine *crosshairV; // 垂直十字线
    QCPItemText *coordText;          // 坐标显示文本

    // 初始化函数
    void setupCustomPlot();          // 初始化图表
    void setupInteractions();        // 初始化交互功能

    // 右键菜单函数
    void showContextMenu(const QPoint &pos);

    QString     ip;
    quint16     port;
    QTcpSocket  *m_tcpSocket;
    QByteArray buffer;

    QCustomPlot *customPlot = nullptr;
    QVector<double> freq;

};
#endif // MAINWINDOW_H
