#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QVBoxLayout>
#include <QNetworkAccessManager>
#include <QTimer>
#include <QJsonObject>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void requestData();
    void onDataReceived(QNetworkReply *reply);
    void updateDisplay(const QJsonObject &data);

private:
    void setupUI();
    QLabel *m_arduinoLabel;
    QLabel *m_elbearLabel;
    QLabel *m_lastUpdateLabel;
    QNetworkAccessManager *m_networkManager;
    QTimer *m_timer;
    QString m_serverUrl;
};

#endif // MAINWINDOW_H
