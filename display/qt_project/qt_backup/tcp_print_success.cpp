#include <QApplication>
#include <QMainWindow>
#include <QLabel>
#include <QThread>
#include <QTcpServer>
#include <QTcpSocket>
#include <QMutex>
#include <QKeyEvent>
#include <QPixmap>
#include <QFile>
#include <QDebug>

// 전역 변수 및 상호배제용 뮤텍스
int motor_speed = 0;
int raining = 0;
int in_tunnel = 0;
int air_condition = 0;
QMutex mutex;

// Struct for vent data
struct VentDTO {
    int raining;
    int in_tunnel;
    int air_condition;
};

// MotorSpeedReceiveThread 클래스
class MotorSpeedReceiveThread : public QThread {
    Q_OBJECT

public:
    void run() override {
        QTcpServer server;
        if (!server.listen(QHostAddress::Any, 8080)) {
            qDebug() << "MotorSpeedReceiveThread: Unable to start server.";
            return;
        }
        qDebug() << "MotorSpeedReceiveThread: Listening on port 8080";

        while (true) {
            if (server.waitForNewConnection(-1)) {
                QTcpSocket *client = server.nextPendingConnection();
                qDebug() << "Client connected to motor_speed_receive_thread.";

                while (client->state() == QAbstractSocket::ConnectedState) {
                    if (client->waitForReadyRead(100)) {
                        QByteArray data = client->readAll();
                        if (data.size() == sizeof(int)) { // 정확한 크기의 데이터인지 확인
                            int speed;
                            memcpy(&speed, data.data(), sizeof(int));

                            // 전역 변수 업데이트 및 콘솔 출력
                            if (speed >= 0 && speed <= 255) {
                                QMutexLocker locker(&mutex);
                                motor_speed = speed;
                                qDebug() << "Received motor_speed:" << motor_speed;
                            } else {
                                qDebug() << "Invalid motor_speed received:" << speed;
                            }
                        } else {
                            qDebug() << "Invalid data size received in motor_speed_receive_thread.";
                        }
                    }
                }

                qDebug() << "Client disconnected from motor_speed_receive_thread.";
                client->close();
                client->deleteLater();
            }
        }
    }
};

// VentDataReceiveThread 클래스
class VentDataReceiveThread : public QThread {
    Q_OBJECT

public:
    void run() override {
        QTcpServer server;
        if (!server.listen(QHostAddress::Any, 8081)) {
            qDebug() << "VentDataReceiveThread: Unable to start server.";
            return;
        }
        qDebug() << "VentDataReceiveThread: Listening on port 8081";

        while (true) {
            if (server.waitForNewConnection(-1)) {
                QTcpSocket *client = server.nextPendingConnection();
                qDebug() << "Client connected to vent_data_receive_thread.";

                while (client->state() == QAbstractSocket::ConnectedState) {
                    if (client->waitForReadyRead(100)) {
                        QByteArray data = client->readAll();
                        if (data.size() == sizeof(VentDTO)) { // 정확한 크기의 데이터인지 확인
                            VentDTO ventData;
                            memcpy(&ventData, data.data(), sizeof(VentDTO));

                            // 전역 변수 업데이트 및 콘솔 출력
                            if (ventData.raining >= 0 && ventData.raining <= 255 &&
                                ventData.in_tunnel >= 0 && ventData.in_tunnel <= 255 &&
                                ventData.air_condition >= 0 && ventData.air_condition <= 255) {
                                QMutexLocker locker(&mutex);
                                raining = ventData.raining;
                                in_tunnel = ventData.in_tunnel;
                                air_condition = ventData.air_condition;

                                qDebug() << "Received vent data: Raining:" << raining
                                         << "In Tunnel:" << in_tunnel
                                         << "Air Condition:" << air_condition;
                            } else {
                                qDebug() << "Invalid vent data received.";
                            }
                        } else {
                            qDebug() << "Invalid data size received in vent_data_receive_thread.";
                        }
                    }
                }

                qDebug() << "Client disconnected from vent_data_receive_thread.";
                client->close();
                client->deleteLater();
            }
        }
    }
};

// ShowValueThread 클래스
class ShowValueThread : public QThread {
    Q_OBJECT

private:
    QLabel *label;

public:
    explicit ShowValueThread(QLabel *outputLabel) : label(outputLabel) {}

    void run() override {
        while (true) {
            QMutexLocker locker(&mutex);
            int speed = motor_speed;
            int rain = raining;
            int tunnel = in_tunnel;
            int air = air_condition;
            locker.unlock();

            QString displayText = QString("Motor Speed: %1\nRaining: %2\nIn Tunnel: %3\nAir Condition: %4")
                                      .arg(speed)
                                      .arg(rain)
                                      .arg(tunnel)
                                      .arg(air);

            QMetaObject::invokeMethod(label, [=]() {
                label->setText(displayText);
            });

            QThread::msleep(500); // 0.5초마다 갱신
        }
    }
};

// DashboardWindow 클래스
class DashboardWindow : public QMainWindow {
    Q_OBJECT

public:
    DashboardWindow(QWidget *parent = nullptr) : QMainWindow(parent), isFullScreenMode(false) {
        // 배경 QLabel 설정
        backgroundLabel = new QLabel(this);
        QString imagePath = "./car_dashboard.jpg";

        if (QFile::exists(imagePath)) {
            originalPixmap = QPixmap(imagePath);
            updateScaledImage();
        } else {
            qDebug() << "Image not found at:" << imagePath;
            backgroundLabel->setText("Image not found!");
        }

        backgroundLabel->setScaledContents(true);

        overlayLabel = new QLabel(this);
        overlayLabel->setAlignment(Qt::AlignLeft | Qt::AlignTop);
        overlayLabel->setStyleSheet("color: white; font-size: 16px; padding: 10px; background: transparent;");
        overlayLabel->setGeometry(10, 10, 400, 100);
        overlayLabel->raise();

        setCentralWidget(backgroundLabel);
        overlayLabel->raise();

        defaultSize = QSize(800, 600);
        resize(defaultSize);

        setMinimumSize(defaultSize);
        setMaximumSize(defaultSize);

        motorSpeedReceiveThread = new MotorSpeedReceiveThread();
        motorSpeedReceiveThread->start();

        ventDataReceiveThread = new VentDataReceiveThread();
        ventDataReceiveThread->start();

        showValueThread = new ShowValueThread(overlayLabel);
        showValueThread->start();
    }

    ~DashboardWindow() {
        motorSpeedReceiveThread->terminate();
        motorSpeedReceiveThread->wait();

        ventDataReceiveThread->terminate();
        ventDataReceiveThread->wait();

        showValueThread->terminate();
        showValueThread->wait();
    }

protected:
    void keyPressEvent(QKeyEvent *event) override {
        if (event->key() == Qt::Key_Escape) {
            if (isFullScreenMode) {
                showNormal();
                resize(defaultSize);
                setMinimumSize(defaultSize);
                setMaximumSize(defaultSize);
            } else {
                setMinimumSize(QSize(0, 0));
                setMaximumSize(QSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX));
                showFullScreen();
            }
            isFullScreenMode = !isFullScreenMode;
            updateScaledImage();
            overlayLabel->raise();
        }
        QMainWindow::keyPressEvent(event);
    }

    void resizeEvent(QResizeEvent *event) override {
        updateScaledImage();
        overlayLabel->raise();
        QMainWindow::resizeEvent(event);
    }

private:
    QLabel *backgroundLabel;
    QLabel *overlayLabel;
    QPixmap originalPixmap;
    bool isFullScreenMode;
    QSize defaultSize;

    MotorSpeedReceiveThread *motorSpeedReceiveThread;
    VentDataReceiveThread *ventDataReceiveThread;
    ShowValueThread *showValueThread;

    void updateScaledImage() {
        if (!originalPixmap.isNull()) {
            QPixmap scaledPixmap = originalPixmap.scaled(size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
            backgroundLabel->setPixmap(scaledPixmap);
            backgroundLabel->setGeometry(0, 0, width(), height());
        }
    }
};

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    DashboardWindow window;
    window.show();

    return app.exec();
}

#include "main.moc"
