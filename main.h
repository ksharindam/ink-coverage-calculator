#ifndef MAIN_H
#define MAIN_H

#include <QDialog>
#include <QLabel>
#include <QString>
#include <QThread>
#include <QTimer>
#include <poppler-qt5.h>
#include "ui_mainwindow.h"

QT_BEGIN_NAMESPACE

float roundOff(float num, int dec);
void setElidedText(QLabel *label, QString text);

class Calculator : public QObject
{
    Q_OBJECT
public:
    int remainder;
    Poppler::Document *doc;
    Calculator(int remainder):remainder(remainder){};
public slots:
    void loadDoc(QString filename);
    void getInfo(int);
    void calculate(int page_no, int dpi);
signals:
    void infoReady(int total_pages, QImage);
    void resultReady(int page_no, float value);
};

class MainWindow : public QDialog, Ui_MainWindow
{
    Q_OBJECT
public:
    float result=0, count_result=0;
    int numPages;
    QString filename = QString();
    QThread *thread0, *thread1, *thread2;
    Calculator *calc0, *calc1, *calc2;
    QTimer *timer;
    QLabel *thumbnail;
    MainWindow();
    void setFilename(QString filepath);
public slots:
    void onStartPageChange();
    void selectFile();
    void calculate();
    void requestInfo();
    void setInfo(int total_pages, QImage thumbnail);
    void onResultFound(int page_no, float value);
signals:
    void infoRequested(int);
    void loadDocRequested(QString);
    void calculateRequested(int, int);
};



QT_END_NAMESPACE

#endif //MAIN_H
