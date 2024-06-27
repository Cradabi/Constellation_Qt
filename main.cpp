#include "mainwindow.h"

#include <QApplication>



std::atomic<bool> shouldExit(false);

void readPoints(std::ifstream& file, int pointsToRead) {
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

            std::cout << "X: " << x << ", Y: " << y << std::endl;
            pointsRead++;
        }
    }
}





int main(int argc, char *argv[]) {
//    std::thread oneThread(read);
//    while (true) {  // Выполняем 10 итераций для примера
//        std::cout << "2" << std::endl;
//        std::this_thread::sleep_for(std::chrono::seconds(1));
//    }

//    shouldExit = true;
//    oneThread.join();

    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
