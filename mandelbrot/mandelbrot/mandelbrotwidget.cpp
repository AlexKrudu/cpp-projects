#include "mandelbrotwidget.h"
//#include "ui_mainwindow.h"
#include <QWheelEvent>
#include <QPainter>
#include <complex>
#include <iostream>
#include <chrono>

MandelbrotWidget::MandelbrotWidget(QWidget *parent)
    : QWidget(parent)
{
    image = QImage(width(), height(), QImage::Format_RGB888);
    runThreads();
}

MandelbrotWidget::~MandelbrotWidget() {
    stop = true;
    tp.waitForDone();
}

void MandelbrotWidget::runThreads(){
    for (size_t tid = 0; tid < threadCount; tid++) {
        tp.start([tid, this]() {if(!stop){ renderImage(tid); }});
    }
}
void MandelbrotWidget::waitThreads(){
    if (!tp.waitForDone()) {
        std::cerr << "ERROR!\n";
    }
}

void MandelbrotWidget::waitFirstLayer(){
    while (firstLayerCount.load() != static_cast<int>(threadCount)){
        ;
    }
}

void MandelbrotWidget::wheelEvent(QWheelEvent *ev) {
    setUpdatesEnabled(false);
    stop = true;
    waitThreads();
    stop = false;
    const int numDegrees = ev->angleDelta().y() / 8;
    const double numSteps = numDegrees / double(15);
    scale *= (pow(0.8, numSteps));
    firstLayerCount.store(0);
    runThreads();
    waitFirstLayer();
    setUpdatesEnabled(true);
}

void MandelbrotWidget::mousePressEvent(QMouseEvent *ev) {
    if (ev->button() == Qt::LeftButton) {
        lastDragPos = ev->pos();
    }
}

void MandelbrotWidget::mouseMoveEvent(QMouseEvent *event) {
    if (event->buttons() & Qt::LeftButton) {
        setUpdatesEnabled(false);
        stop = true;
        waitThreads();
        double x = event->pos().x() - lastDragPos.x();
        double y = event->pos().y() - lastDragPos.y();
        offset -= std::complex<double>(x * scale, y * scale);
        lastDragPos = event->pos();

        stop = false;
        firstLayerCount.store(0);
        runThreads();
        waitFirstLayer();
        setUpdatesEnabled(true);
    }
}

void MandelbrotWidget::resizeEvent(QResizeEvent *ev) {
    QWidget::resizeEvent(ev);
    setUpdatesEnabled(false);
    stop = true;
    waitThreads();
    stop = false;
    image = QImage(width(), height(), QImage::Format_RGB888);
    firstLayerCount.store(0);
    runThreads();
    waitFirstLayer();
    setUpdatesEnabled(true);
}

void MandelbrotWidget::paintEvent(QPaintEvent *ev) {
    std::clog << "paintEvent is called ";
    QWidget::paintEvent(ev);
    QPainter painter(this);
    painter.drawImage(0, 0, image);
    std::stringstream ss;
    ss << "Current zoom : " << INIT_SCALE / scale << "x.";
    QString text( ss.str().c_str());
    QFontMetrics metrics = painter.fontMetrics();
    int textWidth = metrics.horizontalAdvance(text);
    painter.setPen(Qt::yellow);
    painter.drawText((width() - textWidth) / 2, metrics.leading() + metrics.ascent(), text);
    //update();
}

void MandelbrotWidget::renderImage(size_t id) {

    size_t gridSize = std::ceil(static_cast<double>(image.height()) / threadCount);
    size_t begin = id * gridSize;
    size_t end = std::min(static_cast<size_t>(image.height()), begin + gridSize);

    ssize_t stride = image.bytesPerLine();
    uchar *bits = image.bits();

    for (size_t j = 64; j >= 1; j /= 2) {
        for (size_t y = begin; y < end; y++) {
            uchar *p = bits + y * stride;
            for (size_t x = 0; x < static_cast<size_t>(image.width());) {
                auto clr = value(x, y);
                for (size_t i = 0; i < j && x < static_cast<size_t>(image.width()); i++) {
                    if (stop) {
                        return;
                    }
                    *p++ = clr.red();
                    *p++ = clr.green();
                    *p++ = clr.blue();
                    x++;
                }
            }
        }
        if (j == 64){
            firstLayerCount++;
        }
        update();
    }

}

QColor MandelbrotWidget::value(ssize_t x, ssize_t y) const {
    std::complex<double> c(x - width() / 2, y - height() / 2);
    c *= scale;
    c += offset;
    std::complex<double> z = 0;
    size_t it = 0;
    while (std::abs(z) <= 2. && it < MAX_ITER) {
        z = z * z + c;
        ++it;
    }
    auto hue = int(255 * it / MAX_ITER);
    auto saturation = 255;
    int value = it < MAX_ITER ? 255 : 0;
    auto clr = QColor::fromHsv(hue, saturation, value);
    return clr;
}
