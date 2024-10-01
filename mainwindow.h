#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtWidgets>

class MainWindow : public QMainWindow
{
    Q_OBJECT

    QByteArray data;
    QVector<int> codeFrequencies;

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void loadButtonClicked();
    void encodeButtonClicked();
    void decodeButtonClicked();

public:
    QHBoxLayout *myButtonLayout;
    QPushButton *myLoadButton;
    QPushButton *myEncodeButton;
    QPushButton *myDecodeButton;
    QTableWidget *myTableWidget;
};
#endif // MAINWINDOW_H
