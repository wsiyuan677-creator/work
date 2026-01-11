#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "strut.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    //初始化图表
    setupCustomPlot();
    //初始化交互功能
    setupInteractions();

    freq.resize(1024);
    double step = 1000.0 / 1023.0;
    for (int i = 0; i < 1024; i++)
            freq[i] = -500.0 + i * step;

    // 初始化网络
    m_tcpSocket = new QTcpSocket(this);
    ip = ui->lineEdit->text();
    port = ui->lineEdit_2->text().toUShort();
    m_tcpSocket->connectToHost(ip, port);
    connect(m_tcpSocket, &QTcpSocket::readyRead,this, &MainWindow::onReadyRead);
    connect(this,&MainWindow::spectrumDataReady,this,&MainWindow::ononspectrumDataReady);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onReadyRead()
{

    buffer.append(m_tcpSocket->readAll());

      const int frameSize = sizeof(TcpComHeadT) + 2048 * sizeof(ushort);

      while (buffer.size() >= frameSize)
      {

          TcpComHeadT head;
          memcpy(&head, buffer.data(), sizeof(head));

          if (memcmp(head.head1, HEAD1, 4) != 0 ||
              head.len != 2048 * sizeof(ushort))
          {

              buffer.remove(0, 1);
              continue;
          }
          qDebug() << 1;
          buffer.remove(0, sizeof(TcpComHeadT));

          QVector<double> chA(1024), chB(1024);

          for (int i = 0; i < 2048; ++i)
          {
              ushort v;
              memcpy(&v, buffer.data() + i * 2, 2);

              double db = (v > 0) ? 20.0 * log10(v) : -120.0;

              if ((i & 1) == 0)
                  chA[i / 2] = db;
              else
                  chB[i / 2] = db;
          }

          buffer.remove(0, 2048 * 2);
          emit spectrumDataReady(chA, chB);
      }
}

void MainWindow::ononspectrumDataReady(QVector<double> chA, QVector<double> chB)
{
     qDebug() << 2;
    customPlot->graph(0)->setData(freq, chA);
    customPlot->graph(1)->setData(freq, chB);
    customPlot->replot(QCustomPlot::rpQueuedReplot);
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == ui->widget && event->type() == QEvent::Wheel)
    {
        QWheelEvent *wheelEvent = static_cast<QWheelEvent*>(event);
        if (QApplication::keyboardModifiers() & Qt::ControlModifier)
        {
            double factor = wheelEvent->angleDelta().y() > 0 ? 0.85 : 1.15; // 缩放因子

            ui->widget->xAxis->scaleRange(factor, ui->widget->xAxis->range().center());
            ui->widget->yAxis->scaleRange(factor, ui->widget->yAxis->range().center());
            ui->widget->replot();
            return true;
        }
    }
    return QMainWindow::eventFilter(obj, event);
}

void MainWindow::setupCustomPlot()
{
    customPlot = ui->widget;

    // ============ 1. 图表基本设置 ============
    // 设置背景色
    customPlot->setBackground(QBrush(QColor(245, 245, 245)));

    // 添加标题
    customPlot->plotLayout()->insertRow(0);
    QCPTextElement *title = new QCPTextElement(customPlot, "频谱图", QFont("微软雅黑", 12, QFont::Bold));
    customPlot->plotLayout()->addElement(0, 0, title);

    // ============ 2. 创建多条曲线 ============
    customPlot->addGraph(); // 通道1
    customPlot->addGraph(); // 通道2

    // ============ 3. 设置曲线样式 ============
    // 通道1：红色实线，带圆形数据点
    customPlot->graph(0)->setPen(QPen(QColor(255, 50, 50), 1.5));
    customPlot->graph(0)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 5));
    customPlot->graph(0)->setName("通道1");

    // 通道2：绿色虚线，带方形数据点
    QPen greenPen(QColor(50, 180, 50), 1.5);
    greenPen.setStyle(Qt::DashLine);
    customPlot->graph(1)->setPen(greenPen);
    customPlot->graph(1)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssSquare, 5));
    customPlot->graph(1)->setName("通道2");

    // ============ 4. 生成模拟生物信号数据 ============
//    QVector<double> x(500), y0(500), y1(500), y2(500);
//    for (int i = 0; i < 500; ++i)
//    {
//        x[i] = i * 0.1; // 20ms采样间隔

//        // ECG信号 (模拟心电)
//        y0[i] = 1.5 * qSin(i * 0.1) + 0.3 * qSin(i * 0.5) + (rand() % 100) * 0.01;

//        // EMG信号 (模拟肌电)
//        y1[i] = 0.8 * qCos(i * 0.15) + (rand() % 100) * 0.05 - 2;

//        // EEG信号 (模拟脑电)
//        y2[i] = 0.5 * qSin(i * 0.05) + 0.2 * qCos(i * 0.3) + (rand() % 100) * 0.02 + 2;
//    }

//    // 设置数据
//    customPlot->graph(0)->setData(x, y0);
//    customPlot->graph(1)->setData(x, y1);

    // ============ 5. 坐标轴设置 ============
    // X轴
    customPlot->xAxis->setLabel("频率 (MHz)");
    customPlot->xAxis->setRange(-500, 500); // 10秒数据
    customPlot->xAxis->grid()->setSubGridVisible(true);

    // Y轴
    customPlot->yAxis->setLabel("幅值 (dB)");
    customPlot->yAxis->setRange(-100, 100);
    customPlot->yAxis->grid()->setSubGridVisible(true);

    // 网格样式
    customPlot->xAxis->grid()->setPen(QPen(QColor(200, 200, 200), 1, Qt::DotLine));
    customPlot->yAxis->grid()->setPen(QPen(QColor(200, 200, 200), 1, Qt::DotLine));
    customPlot->xAxis->grid()->setSubGridPen(QPen(QColor(220, 220, 220), 1, Qt::DotLine));
    customPlot->yAxis->grid()->setSubGridPen(QPen(QColor(220, 220, 220), 1, Qt::DotLine));

    // ============ 6. 图例设置 ============
    customPlot->legend->setVisible(true);
    customPlot->legend->setFont(QFont("Arial", 9));
    customPlot->legend->setBrush(QBrush(QColor(255, 255, 255, 200))); // 半透明背景
    customPlot->legend->setBorderPen(QPen(QColor(180, 180, 180), 1));

    // 图例位置
    customPlot->axisRect()->insetLayout()->setInsetAlignment(0, Qt::AlignTop|Qt::AlignRight);

    // ============ 7. 十字线初始化 ============
    // 水平线
    crosshairH = new QCPItemStraightLine(customPlot);
    crosshairH->setPen(QPen(QColor(150, 150, 150), 1, Qt::DashLine));
    crosshairH->setVisible(false);

    // 垂直线
    crosshairV = new QCPItemStraightLine(customPlot);
    crosshairV->setPen(QPen(QColor(150, 150, 150), 1, Qt::DashLine));
    crosshairV->setVisible(false);

    // 坐标标签
    coordText = new QCPItemText(customPlot);
    coordText->setPositionAlignment(Qt::AlignLeft|Qt::AlignTop);
    coordText->position->setType(QCPItemPosition::ptAxisRectRatio);
    coordText->position->setCoords(0.02, 0.02); // 左上角
    coordText->setText("频率: -, 幅度: -");
    coordText->setTextAlignment(Qt::AlignLeft);
    coordText->setFont(QFont(font().family(), 9));
    coordText->setPadding(QMargins(5, 5, 5, 5));
    coordText->setBrush(QBrush(QColor(255, 255, 255, 200)));
    coordText->setPen(QPen(QColor(100, 100, 100)));

    // ============ 8. 重绘图表 ============
    customPlot->replot();
}

void MainWindow::setupInteractions()
{
    QCustomPlot *customPlot = ui->widget;

    // 启用鼠标拖拽和缩放
    customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);

    // 鼠标移动事件 - 显示十字线和坐标
    connect(customPlot, &QCustomPlot::mouseMove, this, [=](QMouseEvent *event)
            {
                double x = customPlot->xAxis->pixelToCoord(event->pos().x());
                double y = customPlot->yAxis->pixelToCoord(event->pos().y());

                // 更新坐标显示
                coordText->setText(QString("X: %1 Mhz\nY: %2 dB").arg(x, 0, 'f', 2).arg(y, 0, 'f', 2));

                // 显示十字线
                crosshairV->point1->setCoords(x, customPlot->yAxis->range().lower);
                crosshairV->point2->setCoords(x, customPlot->yAxis->range().upper);
                crosshairH->point1->setCoords(customPlot->xAxis->range().lower, y);
                crosshairH->point2->setCoords(customPlot->xAxis->range().upper, y);

                crosshairV->setVisible(true);
                crosshairH->setVisible(true);

                customPlot->replot();
            });

    // 鼠标离开图表区域时隐藏十字线
    connect(customPlot, &QCustomPlot::mouseRelease, this, [=]()
            {
                crosshairV->setVisible(false);
                crosshairH->setVisible(false);
                customPlot->replot();
            });

    // 旧版兼容的缩放控制
    customPlot->setInteraction(QCP::iRangeZoom, true);  // 启用缩放

    // 实现Ctrl+滚轮缩放 (手动实现)
    customPlot->installEventFilter(this);  // 需要添加eventFilter函数

    // 双击重置视图
    connect(customPlot, &QCustomPlot::mouseDoubleClick, this, [=]()
            {
                customPlot->rescaleAxes();
                customPlot->replot();
            });

    // 右键菜单
    customPlot->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(customPlot, &QCustomPlot::customContextMenuRequested, this, &MainWindow::showContextMenu);
}

void MainWindow::showContextMenu(const QPoint &pos)
{
    QCustomPlot *customPlot = ui->widget;

    QMenu *menu = new QMenu(this);
    menu->setAttribute(Qt::WA_DeleteOnClose);

    // 添加菜单项
    menu->addAction("保存图像", this, [=]()
                    {
                        QString fileName = QFileDialog::getSaveFileName(this, "保存图像", "", "PNG (*.png);;JPG (*.jpg);;PDF (*.pdf)");
                        if (!fileName.isEmpty())
                        {
                            if (fileName.endsWith(".png"))
                                customPlot->savePng(fileName);
                            else if (fileName.endsWith(".jpg"))
                                customPlot->saveJpg(fileName);
                            else if (fileName.endsWith(".pdf"))
                                customPlot->savePdf(fileName);
                        }
                    });

    menu->addSeparator();

    menu->addAction("重置视图", this, [=]()
                    {
                        customPlot->rescaleAxes();
                        customPlot->replot();
                    });

    menu->addAction("显示/隐藏图例", this, [=]()
                    {
                        customPlot->legend->setVisible(!customPlot->legend->visible());
                        customPlot->replot();
                    });

    menu->popup(customPlot->mapToGlobal(pos));
}
