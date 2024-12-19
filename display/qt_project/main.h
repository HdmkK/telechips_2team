#ifndef FULLSCREENWINDOW_H
#define FULLSCREENWINDOW_H

#include <QMainWindow>
#include <QKeyEvent>
#include <QLabel>
#include <QThread>
#include <QTcpServer>
#include <QTcpSocket>
#include <QGridLayout>

struct sensor_dto {
    int motor_speed;
    int precipitation;
    int tunnel_distance;
    int air_condition;
};

class TcpServerWorker : public QThread {
    Q_OBJECT

public:
    explicit TcpServerWorker(QObject *parent = nullptr);
    ~TcpServerWorker();

    void run() override;

signals:
    void dataReceived(sensor_dto data);

private slots:
    void handleNewConnection();
    void handleClientData();

private:
    QTcpServer *tcpServer;
    QTcpSocket *clientSocket;
};

class FullScreenWindow : public QMainWindow {
    Q_OBJECT

public:
    FullScreenWindow();

protected:
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void onDataReceived(sensor_dto data);

private:
    bool isFullScreenMode = false;
    QLabel *motorSpeedLabel;
    QLabel *precipitationLabel;
    QLabel *tunnelDistanceLabel;
    QLabel *airConditionLabel;
    TcpServerWorker *serverWorker;

    QLabel *createStyledLabel(const QString &title);
    void updateDisplay(const sensor_dto &data);
};

#endif // FULLSCREENWINDOW_H
