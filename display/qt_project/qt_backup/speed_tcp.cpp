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
QMutex mutex;

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
                qDebug() << "Client connected.";

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
                            qDebug() << "Invalid data size received.";
                        }
                    }
                }

                qDebug() << "Client disconnected.";
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
    QLabel *label; // motor_speed 값을 표시할 QLabel

public:
    explicit ShowValueThread(QLabel *outputLabel) : label(outputLabel) {}

    void run() override {
        while (true) {
            QMutexLocker locker(&mutex);
            int speed = motor_speed; // 전역 변수 읽기
            locker.unlock();

            QString displayText = QString("Motor Speed: %1").arg(speed);

            // QLabel을 업데이트 (UI 스레드에서 안전하게 실행)
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
        QString imagePath = "./car_dashboard.jpg"; // 프로젝트 디렉터리에 이미지가 있을 경우

        if (QFile::exists(imagePath)) {
            originalPixmap = QPixmap(imagePath);
            updateScaledImage();
        } else {
            qDebug() << "Image not found at:" << imagePath;
            backgroundLabel->setText("Image not found!");
        }

        backgroundLabel->setScaledContents(true);

        // motor_speed 값을 표시할 텍스트 QLabel 설정
        overlayLabel = new QLabel(this);
        overlayLabel->setAlignment(Qt::AlignLeft | Qt::AlignTop);
        overlayLabel->setStyleSheet("color: white; font-size: 20px; background: transparent; padding: 10px;");
        overlayLabel->setGeometry(10, 10, 400, 50); // 텍스트 위치 및 크기 설정
        overlayLabel->raise(); // 텍스트 QLabel을 배경 QLabel 위로 올림

        // 창 기본 설정
        setCentralWidget(backgroundLabel);
        overlayLabel->raise(); // 텍스트 QLabel을 항상 배경 위로 유지

        defaultSize = QSize(800, 600); // 기본 창 크기 저장
        resize(defaultSize);

        // 창 크기 고정
        setMinimumSize(defaultSize);
        setMaximumSize(defaultSize);

        // MotorSpeedReceiveThread 생성 및 시작
        motorSpeedReceiveThread = new MotorSpeedReceiveThread();
        motorSpeedReceiveThread->start();

        // ShowValueThread 생성 및 시작
        showValueThread = new ShowValueThread(overlayLabel);
        showValueThread->start();
    }

    ~DashboardWindow() {
        motorSpeedReceiveThread->terminate();
        motorSpeedReceiveThread->wait();

        showValueThread->terminate();
        showValueThread->wait();
    }

protected:
    void keyPressEvent(QKeyEvent *event) override {
        if (event->key() == Qt::Key_Escape) {
            if (isFullScreenMode) {
                showNormal();
                resize(defaultSize); // 기본 창 크기로 복원
                setMinimumSize(defaultSize); // 크기 고정
                setMaximumSize(defaultSize); // 크기 고정
            } else {
                setMinimumSize(QSize(0, 0)); // 전체화면 전환 전 고정 해제
                setMaximumSize(QSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX)); // 고정 해제
                showFullScreen();
            }
            isFullScreenMode = !isFullScreenMode;
            updateScaledImage(); // 화면 상태에 따라 이미지 크기 재조정
            overlayLabel->raise(); // 텍스트 QLabel을 항상 위로 유지
        }
        QMainWindow::keyPressEvent(event);
    }

    void resizeEvent(QResizeEvent *event) override {
        updateScaledImage(); // 창 크기 변경 시 배경 이미지 크기 조정
        overlayLabel->raise(); // 텍스트 QLabel을 항상 위로 유지
        QMainWindow::resizeEvent(event);
    }

private:
    QLabel *backgroundLabel;
    QLabel *overlayLabel; // 텍스트 QLabel
    QPixmap originalPixmap;
    bool isFullScreenMode;
    QSize defaultSize; // 기본 창 크기 저장

    MotorSpeedReceiveThread *motorSpeedReceiveThread;
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
