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


void MainWindow::readPoints(std::ifstream& file, int pointsToRead) {
    std::string line;
    int pointsRead = 0;

    while (std::getline(file, line) && pointsRead < pointsToRead) {
        if (line.find("<Point") != std::string::npos) {
            std::size_t x_pos = line.find("X=\"") + 3;
            std::size_t y_pos = line.find("Y=\"") + 3;
            std::size_t x_end = line.find("\"", x_pos);
            std::size_t y_end = line.find("\"", y_pos);

            std::string x_str = line.substr(x_pos, x_end - x_pos);
            std::string y_str = line.substr(y_pos, y_end - y_pos);

            std::int16_t x, y;
            std::stringstream(x_str) >> x;
            std::stringstream(y_str) >> y;

            std::lock_guard<std::mutex> lock(mutex);
            coordinates.push_back(std::make_pair(x, y));

            std::cout << "X: " << x << ", Y: " << y << std::endl;
            pointsRead++;
        }
    }
}



void MainWindow::readCoordinates() {
    const char* filename = "xml/256.xml";
    const int pointsPerRead = 1000;

    while (true) {
        std::ifstream file(filename);

        if (!file.is_open()) {
            std::cout << "Не удалось открыть файл" << std::endl;
            return;
        }

        std::string header;
        std::getline(file, header);

        while (file.good()) {
            readPoints(file, pointsPerRead);

            if (file.eof()) {
                std::cout << "Файл закончился. Начинаем сначала." << std::endl;
                emit pointsReady();
                clearCoordinates();
                break;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(10));
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














