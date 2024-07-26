#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QFile>
#include <QXmlStreamReader>
#include <QDebug>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QResizeEvent>
#include <thread>
#include <chrono>
#include <mutex>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    view = new QGraphicsView(this);
    scene = new QGraphicsScene(this);
    view->setScene(scene);
    setCentralWidget(view);
    view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view->fitInView(scene->sceneRect(), Qt::KeepAspectRatio);

    connect(this, &MainWindow::pointsReady, this, &MainWindow::drawPoints);

    // Запускаем поток чтения координат из файла
    std::thread readThread(&MainWindow::readCoordinates, this);
    readThread.detach();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::resizeEvent(QResizeEvent *event) {
    QMainWindow::resizeEvent(event);
    scene->setSceneRect(0, 0, this->width(), this->height());
    drawPoints();
}

void MainWindow::readCoordinates() {
    const QString filename = "xml/128.xml";
    QFile file(filename);
    QXmlStreamReader xml;
    int counter = 0;
    while (true) {
        if(counter >= 1000){
            counter = 0;
            emit pointsReady();
            //clearCoordinates();
        }
        if (!file.isOpen()) {
            if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                qDebug() << "Не удалось открыть файл";
                return;
            }
            xml.setDevice(&file);
            // Пропускаем заголовок XML
            xml.readNext();
            ++counter;
        }

        if (xml.readNextStartElement()) {
            if (xml.name() == "Point") {
                QXmlStreamAttributes attributes = xml.attributes();

                if (attributes.hasAttribute("X") && attributes.hasAttribute("Y")) {
                    QString x_str = attributes.value("X").toString();
                    QString y_str = attributes.value("Y").toString();

                    std::int16_t x = x_str.toDouble();
                    std::int16_t y = y_str.toDouble();

                    std::lock_guard<std::mutex> lock(mutex);
                    coordinates.push_back(std::make_pair(x, y));
                    ++counter;

                    //qDebug() << "X:" << x << ", Y:" << y;
                }
            }
        } else if (xml.atEnd() || xml.hasError()) {
            ++counter;
            // Если достигнут конец файла или произошла ошибка
            file.close();
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            continue;  // Начинаем чтение файла сначала
        }

        // Небольшая задержка для предотвращения чрезмерной нагрузки на CPU
        //std::this_thread::sleep_for(std::chrono::microseconds(1));
    }
}

void MainWindow::drawPoints() {
    std::lock_guard<std::mutex> lock(mutex);

    scene->clear();

    QRectF sceneRect = view->viewport()->rect();
    qreal sceneWidth = sceneRect.width();
    qreal sceneHeight = sceneRect.height();

    for (const auto& point : coordinates) {
        qreal x = sceneWidth/2 + (point.first / 50.0) * sceneWidth;
        qreal y = sceneHeight/2 + (point.second / 50.0) * sceneHeight;
        QPen pen;

        pen.setColor(Qt::black);
        pen.setWidth(5);
        pen.setBrush(Qt::black);

        scene->addEllipse(x, y, 5, 5, pen);
    }

    // Отрисовка осей координат
    for(int i = -20; i <= 21; i+=5){
        qreal x = sceneWidth/2 + (i / 50.0) * sceneWidth;
        qreal y = sceneHeight/2 + (0 / 50.0) * sceneHeight;
        qreal size = 4;
        QLineF line1(x - size/2, y, x + size/2, y);
        QLineF line2(x, y + size/2, x, y - size/2);

        QPen pen(Qt::red);
        pen.setWidth(1);

        scene->addLine(line1, pen);
        scene->addLine(line2, pen);
    }

    for(int i = -20; i <= 21; i+=5){
        qreal x = sceneWidth/2 + (0 / 50.0) * sceneWidth;
        qreal y = sceneHeight/2 + (i / 50.0) * sceneHeight;

        qreal size = 4;

        QLineF line1(x - size/2, y, x + size/2, y);
        QLineF line2(x, y + size/2, x, y - size/2);

        QPen pen(Qt::red);
        pen.setWidth(1);

        scene->addLine(line1, pen);
        scene->addLine(line2, pen);
    }
    coordinates.clear();
}
