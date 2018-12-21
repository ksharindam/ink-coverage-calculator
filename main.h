#ifndef MAIN_H
#define MAIN

#include <QDialog>
#include <QLabel>
#include <QString>
#include <QThread>
#include <poppler-qt4.h>
#include "ui_mainwindow.h"

QT_BEGIN_NAMESPACE

float roundOff(float num, int dec);

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
    float result=0;
    int numPages;
    QString filename;
    QThread *thread0, *thread1, *thread2;
    Calculator *calc0, *calc1, *calc2;
    QLabel *thumbnail;
    MainWindow();
    void setFilename(QString filepath);
public slots:
    void setPageRange();
    void selectFile();
    void calculate();
    void setInfo(int total_pages, QImage thumbnail);
    void onResultFound(int page_no, float value);
signals:
    void infoRequested(int);
    void loadDocRequested(QString);
    void calculateRequested(int, int);
};



QT_END_NAMESPACE

#endif //MAIN_H
