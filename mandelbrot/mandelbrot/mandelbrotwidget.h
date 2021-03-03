#pragma once

#include <QMainWindow>
#include <QThreadPool>
#include <QImage>
#include <QThread>
#include <complex>
#include <memory>
#include <mutex>
#include <vector>
#include <queue>
#include <atomic>
#include <iostream>
#include <QAbstractScrollArea>


class MandelbrotWidget : public QWidget
{
    Q_OBJECT

public:
    MandelbrotWidget(QWidget *parent = nullptr);
    void wheelEvent(QWheelEvent*) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void resizeEvent(QResizeEvent* ) override;
    void paintEvent(QPaintEvent*) override;
    void renderImage(size_t);
    void waitThreads();
    void waitFirstLayer();
    QColor value(ssize_t x, ssize_t y) const;
    ~MandelbrotWidget();

private:
    void runThreads();
    std::complex<double> offset = {-1., 0.};
    const double INIT_SCALE = 0.005;
    double scale = 0.005;
    const size_t MAX_ITER = 200;
    QPoint lastDragPos;
    std::atomic_bool stop;
    std::atomic_int firstLayerCount = 0;
    size_t threadCount = QThread::idealThreadCount();
    QImage image;
    QThreadPool tp;
};

