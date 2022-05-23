#include "main.h"
#include <QApplication>
#include <QFileInfo>
#include <QFileDialog>
#include <QPixmap>
#include <QSizePolicy>
#include <cmath>
#include <QFontMetrics>

#define MAX(a,b) ({ __typeof__ (a) _a = (a); \
                    __typeof__ (b) _b = (b); \
                    _a > _b ? _a : _b; })


MainWindow:: MainWindow()
{
    setupUi(this);
    thumbnail = new QLabel(this);
    thumbnail->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    verticalLayout->addWidget(thumbnail, 0, Qt::AlignHCenter);
    QPixmap pm(170, 254);
    pm.fill();
    thumbnail->setPixmap(pm);
    connect(pageNoBox, SIGNAL(valueChanged(int)), this, SLOT(onStartPageChange()));
    connect(calculateBtn, SIGNAL(clicked()), this, SLOT(calculate()));
    connect(chooseFileBtn, SIGNAL(clicked()), this, SLOT(selectFile()));
    thread0 = new QThread(this);
    thread1 = new QThread(this);
    thread2 = new QThread(this);
    calc0 = new Calculator(0);
    calc1 = new Calculator(1);
    calc2 = new Calculator(2);
    calc0->moveToThread(thread0);
    calc1->moveToThread(thread1);
    calc2->moveToThread(thread2);
    connect(this, SIGNAL(infoRequested(int)), calc0, SLOT(getInfo(int)));
    connect(calc0, SIGNAL(infoReady(int, QImage)), this, SLOT(setInfo(int, QImage)));
    connect(this, SIGNAL(loadDocRequested(QString)), calc0, SLOT(loadDoc(QString)));
    connect(this, SIGNAL(calculateRequested(int,int)), calc0, SLOT(calculate(int,int)));
    connect(calc0, SIGNAL(resultReady(int,float,float,float,float,float)), this,
                    SLOT(onResultFound(int,float,float,float,float,float)));
    connect(this, SIGNAL(loadDocRequested(QString)), calc1, SLOT(loadDoc(QString)));
    connect(this, SIGNAL(calculateRequested(int,int)), calc1, SLOT(calculate(int,int)));
    connect(calc1, SIGNAL(resultReady(int,float,float,float,float,float)), this,
                    SLOT(onResultFound(int,float,float,float,float,float)));
    connect(this, SIGNAL(loadDocRequested(QString)), calc2, SLOT(loadDoc(QString)));
    connect(this, SIGNAL(calculateRequested(int,int)), calc2, SLOT(calculate(int,int)));
    connect(calc2, SIGNAL(resultReady(int,float,float,float,float,float)), this,
                    SLOT(onResultFound(int,float,float,float,float,float)));
    connect(closeBtn, SIGNAL(clicked()), this, SLOT(close()));
    thread0->start();
    thread1->start();
    thread2->start();
    timer = new QTimer(this);
    timer->setSingleShot(true);
    connect(timer, SIGNAL(timeout()), this, SLOT(requestInfo()));
}

void
MainWindow:: onStartPageChange()
{
    pageNoBox->setMaximum(numPages);
    pagesBox->setMaximum(numPages-pageNoBox->value()+1);
    timer->start(1000);
}

void
MainWindow:: requestInfo()
{
    emit infoRequested(pageNoBox->value()-1);
}

void
MainWindow:: selectFile()
{
    QString filepath = QFileDialog::getOpenFileName(this, "Select PDF file",
                        filename, "PDF Files (*.pdf);;All Files (*)");
    if (!filepath.isEmpty())
        setFilename(filepath);
}

void
MainWindow:: setFilename(QString filepath)
{
    filename = filepath;
    QFileInfo fileinfo(filepath);
    setElidedText(filenameLabel, fileinfo.fileName());
    emit loadDocRequested(filename);
    emit infoRequested(pageNoBox->value()-1);
    colorC->setText("0%");
    colorM->setText("0%");
    colorY->setText("0%");
    colorK->setText("0%");
    monoK->setText("0%");
}

void
MainWindow:: calculate()
{
    if (filename.isEmpty())
        return;
    resC = 0;
    resM = 0;
    resY = 0;
    resK = 0;
    resMonoK = 0;
    count_result = 0;
    int start_page_no = pageNoBox->value()-1;
    int pages = pagesBox->value();
    int dpi = dpiBox->value();
    for (int i=start_page_no;i<start_page_no+pages;++i){
        emit calculateRequested(i, dpi);
    }
    statusbar->setText("Calculating Page Coverage ...");
}

void
MainWindow:: onResultFound(int /*page_no*/, float C, float M, float Y, float K, float grayK)
{
    //qDebug("%i %f", page_no, value);
    count_result++;
    resC += C;
    resM += M;
    resY += Y;
    resK += K;
    resMonoK += grayK;
    if (count_result == pagesBox->value()) {
        colorC->setText(QString("%1%").arg(resC/count_result));
        colorM->setText(QString("%1%").arg(resM/count_result));
        colorY->setText(QString("%1%").arg(resY/count_result));
        colorK->setText(QString("%1%").arg(resK/count_result));
        monoK->setText(QString("%1%").arg(resMonoK/count_result));
        statusbar->setText("Done !");
    }
}

void
MainWindow:: setInfo(int total_pages, QImage img)
{
    numPages = total_pages;
    pageNoBox->setMaximum(numPages);
    pagesBox->setMaximum(numPages-pageNoBox->value()+1);
    thumbnail->setPixmap(QPixmap::fromImage(img));
}

/* Calculator object calculates in separage thread*/
void
Calculator :: loadDoc(QString filename)
{
    doc = Poppler::Document::load(filename);
}

void
Calculator :: getInfo(int page_no)
{
    int numPages = doc->numPages();
    if (page_no>=numPages) page_no=0;
    QImage thumbnail = doc->page(page_no)->renderToImage(75, 75);
    thumbnail = thumbnail.scaledToWidth(180, Qt::SmoothTransformation);
    emit infoReady(numPages, thumbnail);
}

void
Calculator :: calculate(int page_no, int dpi)
{
    if (page_no%3 != remainder) return;
    // This is a slot that connects with MainWindow
    Poppler::Page *page = doc->page(page_no);
    if (! page) return;
    QImage img = page->renderToImage(dpi, dpi);
    double sumC=0, sumM=0, sumY=0, sumK=0;
    double sumMonoK = 0;
    for (int y=0; y<img.height(); y++) {
        QRgb* line = ((QRgb*)img.constScanLine(y));
        float C,M,Y,K;
        for (int x=0; x<img.width(); x++) {
            rgb2cmyk(qRed(line[x])/255.0, qGreen(line[x])/255.0, qBlue(line[x])/255.0, C,M,Y,K);
            sumC += C;
            sumM += M;
            sumY += Y;
            sumK += K;
            sumMonoK += 1 - qGray(line[x])/255.0;
        }
    }
    int pixels_count = img.width()*img.height();
    float resultC = 100.0*sumC/pixels_count;
    float resultM = 100.0*sumM/pixels_count;
    float resultY = 100.0*sumY/pixels_count;
    float resultK = 100.0*sumK/pixels_count;
    float resultMonoK = 100.0*sumMonoK/pixels_count;
    emit resultReady(page_no, resultC, resultM, resultY, resultK, resultMonoK);
}

/* Conversion Formula
K = 1.0 - max(R, G, B)
C = (1-R-K)/(1-K)
M = (1-G-K)/(1-K)
Y = (1-B-K)/(1-K)
*/
// r,g,b input and c,m,y,k output is in 0-1.0 range
void rgb2cmyk(float r, float g, float b, float &c, float &m, float &y, float &k)
{
    float rgbMax = MAX(r, MAX(g, b));
    if (rgbMax==0){
        c = 0;
        m = 0;
        y = 0;
        k = 1.0;
        return;
    }
    c = 1.0 - r/rgbMax;
    m = 1.0 - g/rgbMax;
    y = 1.0 - b/rgbMax;
    k = 1.0 - rgbMax;
}


float roundOff(float num, int dec)
{
    double m = (num < 0.0) ? -1.0 : 1.0;   // check if input is negative
    double pwr = pow(10, dec);
    return float(floor((double)num * m * pwr + 0.5) / pwr) * m;
}

void setElidedText(QLabel *label, QString text)
{
    QString elidedText(QFontMetrics(label->font()).elidedText(text, Qt::ElideMiddle, label->width()-2));
    label->setText(elidedText);
}

// The main function
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    MainWindow *win = new MainWindow();
    win->show();
    if (argc > 1) {
        QString path = QString::fromUtf8(argv[1]);
        QFileInfo fileinfo(path);
        if (fileinfo.exists())
            win->setFilename(path);
    }
    return app.exec();
}
