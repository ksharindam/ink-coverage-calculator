#include "main.h"
#include <QApplication>
#include <QFileInfo>
#include <QFileDialog>
#include <QPixmap>
#include <QSizePolicy>
#include <cmath>

MainWindow:: MainWindow()
{
    setupUi(this);
    thumbnail = new QLabel(this);
    thumbnail->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    verticalLayout->addWidget(thumbnail, 0, Qt::AlignHCenter);
    connect(pageNoBox, SIGNAL(valueChanged(int)), this, SLOT(setPageRange()));
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
    connect(calc0, SIGNAL(resultReady(int,float)), this, SLOT(onResultFound(int,float)));
    connect(this, SIGNAL(loadDocRequested(QString)), calc1, SLOT(loadDoc(QString)));
    connect(this, SIGNAL(calculateRequested(int,int)), calc1, SLOT(calculate(int,int)));
    connect(calc1, SIGNAL(resultReady(int,float)), this, SLOT(onResultFound(int,float)));
    connect(this, SIGNAL(loadDocRequested(QString)), calc2, SLOT(loadDoc(QString)));
    connect(this, SIGNAL(calculateRequested(int,int)), calc2, SLOT(calculate(int,int)));
    connect(calc2, SIGNAL(resultReady(int,float)), this, SLOT(onResultFound(int,float)));
    thread0->start();
    thread1->start();
    thread2->start();
}

void
MainWindow:: setPageRange()
{
    pageNoBox->setMaximum(numPages);
    pagesBox->setMaximum(numPages-pageNoBox->value()+1);
}

void
MainWindow:: selectFile()
{
    QString filepath = QFileDialog::getOpenFileName(this, "Select PDF file",
                        QString(), "PDF Files (*.pdf);;All Files (*)");
    if (!filepath.isEmpty())
        setFilename(filepath);
}

void
MainWindow:: setFilename(QString filepath)
{
    filename = filepath;
    QFileInfo fileinfo(filepath);
    filenameLabel->setText(fileinfo.fileName());
    emit loadDocRequested(filename);
    emit infoRequested(pageNoBox->value()-1);
}

void
MainWindow:: calculate()
{
    result = 0;
    int start_page_no = pageNoBox->value()-1;
    int pages = pagesBox->value();
    int dpi = dpiBox->value();
    for (int i=start_page_no;i<start_page_no+pages;++i){
        emit calculateRequested(i, dpi);
    }
}

void
MainWindow:: onResultFound(int /*page_no*/, float value)
{
    //qDebug("%i %f", page_no, value);
    result += value;
    value = result/pagesBox->value();
    resultLabel->setText(QString("Page Coverage is   %1%").arg(roundOff(value,1)));
}

void
MainWindow:: setInfo(int total_pages, QImage img)
{
    numPages = total_pages;
    setPageRange();
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
    double val = 0;
    for (int y=0;y<img.height();y++) {
        QRgb* line = ((QRgb*)img.constScanLine(y));
        for (int x=0;x<img.width();x++) {
            val += 1-qGray(line[x])/255;
        }
    }
    float coverage = val*100.0/(img.width()*img.height());
    emit resultReady(page_no, coverage);
}


float roundOff(float num, int dec)
{
    double m = (num < 0.0) ? -1.0 : 1.0;   // check if input is negative
    double pwr = pow(10, dec);
    return float(floor((double)num * m * pwr + 0.5) / pwr) * m;
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
