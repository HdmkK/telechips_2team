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
                        if (data.size() == sizeof(int)) {
                            int speed;
                            memcpy(&speed, data.data(), sizeof(int));

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
                        if (data.size() == sizeof(VentDTO)) {
                            VentDTO ventData;
                            memcpy(&ventData, data.data(), sizeof(VentDTO));

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
    QLabel *motorSpeedLabel;
    QLabel *rainingLabel;
    QLabel *inTunnelLabel;
    QLabel *airConditionLabel;

public:
    explicit ShowValueThread(QLabel *motorLabel, QLabel *rainLabel, QLabel *tunnelLabel, QLabel *airLabel)
        : motorSpeedLabel(motorLabel), rainingLabel(rainLabel), inTunnelLabel(tunnelLabel), airConditionLabel(airLabel) {}

    void run() override {
        while (true) {
            QMutexLocker locker(&mutex);
            int speed = motor_speed;
            int rain = raining;
            int tunnel = in_tunnel;
            int air = air_condition;
            locker.unlock();

            QMetaObject::invokeMethod(motorSpeedLabel, [=]() {
                motorSpeedLabel->setText(QString::number(speed)); // 값만 출력
            });
            QMetaObject::invokeMethod(rainingLabel, [=]() {
                rainingLabel->setText(QString("Raining: %1").arg(rain));
            });
            QMetaObject::invokeMethod(inTunnelLabel, [=]() {
                inTunnelLabel->setText(QString("In Tunnel: %1").arg(tunnel));
            });
            QMetaObject::invokeMethod(airConditionLabel, [=]() {
                airConditionLabel->setText(QString("Air Condition: %1").arg(air));
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

        motorSpeedLabel = new QLabel(this);
        motorSpeedLabel->setGeometry(155, 260, 300, 100); // 초기 크기와 위치 설정
        motorSpeedLabel->setStyleSheet("color: white; font-size: 48px; background: transparent;");
        motorSpeedFontSize = 48; // 초기 폰트 크기 저장

        rainingLabel = new QLabel(this);
        rainingLabel->setGeometry(10, 150, 200, 30);
        rainingLabel->setStyleSheet("color: white; font-size: 16px; background: transparent;");
        rainingFontSize = 16;

        inTunnelLabel = new QLabel(this);
        inTunnelLabel->setGeometry(10, 190, 200, 30);
        inTunnelLabel->setStyleSheet("color: white; font-size: 16px; background: transparent;");
        inTunnelFontSize = 16;

        airConditionLabel = new QLabel(this);
        airConditionLabel->setGeometry(10, 230, 200, 30);
        airConditionLabel->setStyleSheet("color: white; font-size: 16px; background: transparent;");
        airConditionFontSize = 16;

        initialSizes = {motorSpeedLabel->geometry(), rainingLabel->geometry(), inTunnelLabel->geometry(), airConditionLabel->geometry()};

        motorSpeedReceiveThread = new MotorSpeedReceiveThread();
        motorSpeedReceiveThread->start();

        ventDataReceiveThread = new VentDataReceiveThread();
        ventDataReceiveThread->start();

        showValueThread = new ShowValueThread(motorSpeedLabel, rainingLabel, inTunnelLabel, airConditionLabel);
        showValueThread->start();

        setCentralWidget(backgroundLabel);

        defaultSize = QSize(800, 600);
        resize(defaultSize);
        setFixedSize(defaultSize); // 초기 창 크기 고정
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
                setFixedSize(defaultSize); // 초기 크기 복원
            } else {
                setFixedSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX); // 전체화면 크기 제한 제거
                showFullScreen();
            }
            isFullScreenMode = !isFullScreenMode;
            updateScaledImage();
            adjustLabelPositions(); // 전체화면 상태에 따라 텍스트 위치와 크기 조정
        }
        QMainWindow::keyPressEvent(event);
    }

    void resizeEvent(QResizeEvent *event) override {
        updateScaledImage();
        adjustLabelPositions(); // 창 크기 변경 시 텍스트 위치와 크기 조정
        QMainWindow::resizeEvent(event);
    }

private:
    QLabel *backgroundLabel;
    QLabel *motorSpeedLabel;
    QLabel *rainingLabel;
    QLabel *inTunnelLabel;
    QLabel *airConditionLabel;
    QPixmap originalPixmap;
    QSize defaultSize;
    bool isFullScreenMode;
    QList<QRect> initialSizes;
    int motorSpeedFontSize, rainingFontSize, inTunnelFontSize, airConditionFontSize;

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

    void adjustLabelPositions() {
        for (int i = 0; i < initialSizes.size(); ++i) {
            QRect initial = initialSizes[i];
            QLabel *label = nullptr;
            int fontSize = 0;

            if (i == 0) {
                label = motorSpeedLabel;
                fontSize = motorSpeedFontSize;
            } else if (i == 1) {
                label = rainingLabel;
                fontSize = rainingFontSize;
            } else if (i == 2) {
                label = inTunnelLabel;
                fontSize = inTunnelFontSize;
            } else if (i == 3) {
                label = airConditionLabel;
                fontSize = airConditionFontSize;
            }

            if (label) {
                QRect adjusted = QRect(
                    initial.x() * width() / defaultSize.width(),
                    initial.y() * height() / defaultSize.height(),
                    initial.width() * width() / defaultSize.width(),
                    initial.height() * height() / defaultSize.height()
                );
                label->setGeometry(adjusted);

                int adjustedFontSize = fontSize * width() / defaultSize.width();
                label->setStyleSheet(QString("color: white; font-size: %1px; background: transparent;").arg(adjustedFontSize));
            }
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
