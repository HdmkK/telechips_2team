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
int fine_dust = 0;
const int RAINING_THRESHOLD = 100; // RAINING_THRESHOLD 값 정의
QMutex mutex;

// Struct for vent data
struct VentDTO {
    int raining;
    int in_tunnel;
    int air_condition;
    int fine_dust;
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

                            //데이터 받는 부분
                            if (ventData.raining >= 0 && ventData.raining <= 255 &&
                                ventData.in_tunnel >= 0 && ventData.in_tunnel <= 255 &&
                                ventData.air_condition >= 0 && ventData.air_condition <= 255 &&
                                ventData.fine_dust >= 0 && ventData.fine_dust <= 255) {
                                QMutexLocker locker(&mutex);
                                raining = ventData.raining;
                                in_tunnel = ventData.in_tunnel;
                                air_condition = ventData.air_condition;
                                fine_dust = (ventData.fine_dust - 20) * 2;
                                fine_dust = (fine_dust < 0) ? 0 : fine_dust;

                                qDebug() << "Received vent data: Raining:" << raining
                                         << "In Tunnel:" << in_tunnel
                                         << "Air Condition:" << air_condition
                                         << "Fine Dust:" << fine_dust;
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

// RedBallImageThread 클래스
class RedBallImageThread : public QThread {
    Q_OBJECT

private:
    QLabel *redBallLabel;

public:
    explicit RedBallImageThread(QLabel *label) : redBallLabel(label) {}

    void run() override {
        while (true) {
            QMutexLocker locker(&mutex);
            int tunnel = in_tunnel;
            locker.unlock();

            QMetaObject::invokeMethod(redBallLabel, [=]() {
                if (tunnel <= 50) {
                    redBallLabel->setVisible(true);
                    redBallLabel->raise(); // 이미지를 다른 요소들 위로 표시
                    redBallLabel->update(); // 강제로 업데이트
                } else {
                    redBallLabel->setVisible(false);
                }
            });

            QThread::msleep(500); // 0.5초마다 갱신
        }
    }
};

// BlueBallImageThread 클래스
class BlueBallImageThread : public QThread {
    Q_OBJECT

private:
    QLabel *blueBallLabel;

public:
    explicit BlueBallImageThread(QLabel *label) : blueBallLabel(label) {}

    void run() override {
        while (true) {
            QMutexLocker locker(&mutex);
            int rain = raining;
            locker.unlock();

            QMetaObject::invokeMethod(blueBallLabel, [=]() {
                if (rain <= RAINING_THRESHOLD) {
                    blueBallLabel->setVisible(true);
                    blueBallLabel->raise(); // 이미지를 다른 요소들 위로 표시
                    blueBallLabel->update(); // 강제로 업데이트
                } else {
                    blueBallLabel->setVisible(false);
                }
            });

            QThread::msleep(500); // 0.5초마다 갱신
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
    QLabel *fineDustLabel;

public:
    explicit ShowValueThread(QLabel *motorLabel, QLabel *rainLabel, QLabel *tunnelLabel, QLabel *airLabel, QLabel *dustLabel)
        : motorSpeedLabel(motorLabel), rainingLabel(rainLabel), inTunnelLabel(tunnelLabel), airConditionLabel(airLabel), fineDustLabel(dustLabel) {}

    void run() override {
        while (true) {
            QMutexLocker locker(&mutex);
            int speed = motor_speed;
            int rain = raining;
            int tunnel = in_tunnel;
            int air = air_condition;
            int dust = fine_dust;
            locker.unlock();

            QMetaObject::invokeMethod(motorSpeedLabel, [=]() {
                motorSpeedLabel->setText(QString::number(speed));
                motorSpeedLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter); // 오른쪽 정렬 설정
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
            QMetaObject::invokeMethod(fineDustLabel, [=]() {
                fineDustLabel->setText(QString("%1 µg/m³").arg(dust));
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
        motorSpeedLabel->setGeometry(10, 260, 180, 100); // 초기 크기와 위치 설정
        motorSpeedLabel->setStyleSheet("color: white; font-size: 48px; background: transparent;");
        motorSpeedFontSize = 48; // 초기 폰트 크기 저장
        motorSpeedLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter); // 오른쪽 정렬 설정

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

        fineDustLabel = new QLabel(this);
        fineDustLabel->setGeometry(10, 270, 200, 30);
        fineDustLabel->setStyleSheet("color: white; font-size: 16px; background: transparent;");
        fineDustFontSize = 16;

        redBallLabel = new QLabel(this);
        QString redBallImagePath = "./red_ball.jpg";
        if (QFile::exists(redBallImagePath)) {
            QPixmap redBallPixmap(redBallImagePath);
            redBallLabel->setPixmap(redBallPixmap);
            redBallLabel->setScaledContents(true); // 이미지 크기 조정 설정
            redBallLabel->setGeometry(400, 300, 100, 100);
            redBallLabel->setVisible(false);
        } else {
            qDebug() << "Red ball image not found at:" << redBallImagePath;
        }

        blueBallLabel = new QLabel(this);
        QString blueBallImagePath = "./blue_ball.jpg";
        if (QFile::exists(blueBallImagePath)) {
            QPixmap blueBallPixmap(blueBallImagePath);
            blueBallLabel->setPixmap(blueBallPixmap);
            blueBallLabel->setScaledContents(true); // 이미지 크기 조정 설정
            blueBallLabel->setGeometry(200, 300, 100, 100);
            blueBallLabel->setVisible(false);
        } else {
            qDebug() << "Blue ball image not found at:" << blueBallImagePath;
        }

        initialSizes = {motorSpeedLabel->geometry(), rainingLabel->geometry(), inTunnelLabel->geometry(), airConditionLabel->geometry(), fineDustLabel->geometry()};

        motorSpeedReceiveThread = new MotorSpeedReceiveThread();
        motorSpeedReceiveThread->start();

        ventDataReceiveThread = new VentDataReceiveThread();
        ventDataReceiveThread->start();

        showValueThread = new ShowValueThread(motorSpeedLabel, rainingLabel, inTunnelLabel, airConditionLabel, fineDustLabel);
        showValueThread->start();

        redBallImageThread = new RedBallImageThread(redBallLabel);
        redBallImageThread->start();

        blueBallImageThread = new BlueBallImageThread(blueBallLabel);
        blueBallImageThread->start();

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

        redBallImageThread->terminate();
        redBallImageThread->wait();

        blueBallImageThread->terminate();
        blueBallImageThread->wait();
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
    QLabel *fineDustLabel;
    QLabel *redBallLabel;
    QLabel *blueBallLabel;
    QPixmap originalPixmap;
    QSize defaultSize;
    bool isFullScreenMode;
    QList<QRect> initialSizes;
    int motorSpeedFontSize, rainingFontSize, inTunnelFontSize, airConditionFontSize, fineDustFontSize;

    MotorSpeedReceiveThread *motorSpeedReceiveThread;
    VentDataReceiveThread *ventDataReceiveThread;
    ShowValueThread *showValueThread;
    RedBallImageThread *redBallImageThread;
    BlueBallImageThread *blueBallImageThread;

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
            } else if (i == 4) {
                label = fineDustLabel;
                fontSize = fineDustFontSize;
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
