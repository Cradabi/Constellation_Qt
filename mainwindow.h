#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMainWindow>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsEllipseItem>
#include <QPainter>
#include <QRandomGenerator>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <cstdint>
#include <chrono>
#include <thread>
#include <QPair>
#include <QTimer>
#include <mutex>
#include <QResizeEvent>
#include <QFile>
#include <QXmlStreamReader>
#include <QDebug>
#include <QThread>
#include <QMutexLocker>
#include <QFile>
#include <QXmlStreamReader>
#include <QDebug>



QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();


private:
    Ui::MainWindow *ui;
    QGraphicsView *view;
    QGraphicsScene *scene;
    std::vector<std::pair<std::int16_t, std::int16_t>> coordinates;
    std::mutex mutex;


    void readCoordinates();
    void resizeEvent(QResizeEvent *event) override;


private slots:
    void drawPoints();


signals:
    void pointsReady();


};
#endif // MAINWINDOW_H
