#include "mainwindow.h"
#include "./ui_mainwindow.h"



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


void MainWindow::resizeEvent(QResizeEvent *event){
        QMainWindow::resizeEvent(event);
        scene->setSceneRect(0, 0, this->width(), this->height());
        drawPoints();
    }


void MainWindow::readPoints(QXmlStreamReader& xml, int pointsToRead) {
    int pointsRead = 0;

    while (!xml.atEnd() && pointsRead < pointsToRead) {
        xml.readNext();

        if (xml.isStartElement() && xml.name() == "Point") {
            QXmlStreamAttributes attributes = xml.attributes();

            if (attributes.hasAttribute("X") && attributes.hasAttribute("Y")) {
                qint16 x = attributes.value("X").toShort();
                qint16 y = attributes.value("Y").toShort();

                QMutexLocker locker(&mutex);
                coordinates.push_back(qMakePair(x, y));

                qDebug() << "X:" << x << ", Y:" << y;
                pointsRead++;
            }
        }
    }
}

void MainWindow::readCoordinates() {
    const QString filename = "xml/256.xml"; // Замените на имя вашего файла
    const int pointsPerRead = 1000;

    while (true) {
        QFile file(filename);

        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qDebug() << "Не удалось открыть файл";
            return;
        }

        QXmlStreamReader xml(&file);

        // Пропускаем заголовок XML
        xml.readNext();

        while (!xml.atEnd()) {
            readPoints(xml, pointsPerRead);

            if (xml.atEnd()) {
                qDebug() << "Файл закончился. Начинаем сначала.";
                emit pointsReady();
                clearCoordinates();
                break;
            }

            QThread::msleep(10);
        }

        file.close();
    }
}


void MainWindow::drawPoints() {
    std::lock_guard<std::mutex> lock(mutex);

    scene->clear();

    QRectF sceneRect = view->viewport()->rect();
    qreal sceneWidth = sceneRect.width();
    qreal sceneHeight = sceneRect.height();

    for (const auto& point : coordinates) {
        qreal x = sceneWidth/2 +(point.first / 50.0) * sceneWidth;
        qreal y = sceneHeight/2 + (point.second / 50.0) * sceneHeight;
        QPen pen;

        pen.setColor(Qt::black);
        pen.setWidth(5);
        pen.setBrush(Qt::black);

        scene->addEllipse(x, y, 5, 5, pen);
    }
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

}


void MainWindow::clearCoordinates() {
    std::lock_guard<std::mutex> lock(mutex);
    coordinates.clear();
}





