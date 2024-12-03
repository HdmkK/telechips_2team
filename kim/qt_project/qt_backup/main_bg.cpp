#include <QApplication>
#include <QMainWindow>
#include <QLabel>
#include <QKeyEvent>
#include <QPixmap>
#include <QFile>
#include <QDebug>

class DashboardWindow : public QMainWindow {
    Q_OBJECT

public:
    DashboardWindow(QWidget *parent = nullptr) : QMainWindow(parent), isFullScreenMode(false) {
        // QLabel을 사용해 배경 이미지 설정
        label = new QLabel(this);

        // 이미지 경로 설정
        QString imagePath = "./car_dashboard.jpg"; // 프로젝트 디렉터리에 이미지가 있을 경우

        // 이미지 로드 확인
        if (QFile::exists(imagePath)) {
            originalPixmap = QPixmap(imagePath);
            updateScaledImage();
        } else {
            qDebug() << "Image not found at:" << imagePath;
            label->setText("Image not found!");
        }

        // QLabel 배경 설정
        label->setAlignment(Qt::AlignCenter); // 이미지를 중앙 정렬
        label->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
        label->setScaledContents(true); // QLabel 크기와 이미지 크기를 동기화

        // 창 외곽선 제거 및 기본 크기 설정
        setCentralWidget(label);
        setContentsMargins(0, 0, 0, 0);
        resize(800, 600);
    }

protected:
    void keyPressEvent(QKeyEvent *event) override {
        if (event->key() == Qt::Key_Escape) {
            // ESC 키로 전체화면 ON/OFF 전환
            if (isFullScreenMode) {
                showNormal();
            } else {
                showFullScreen();
            }
            isFullScreenMode = !isFullScreenMode;
            updateScaledImage(); // 화면 상태에 따라 이미지 크기 재조정
        }
        QMainWindow::keyPressEvent(event);
    }

    void resizeEvent(QResizeEvent *event) override {
        QMainWindow::resizeEvent(event);
        updateScaledImage(); // 창 크기 변경 시 이미지 크기 재조정
    }

private:
    QLabel *label;
    QPixmap originalPixmap; // 원본 이미지 저장
    bool isFullScreenMode;

    void updateScaledImage() {
        if (!originalPixmap.isNull()) {
            QPixmap scaledPixmap;

            if (isFullScreenMode) {
                // 전체화면: 창 크기에 맞게 비율 유지하며 스케일링
                scaledPixmap = originalPixmap.scaled(size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
            } else {
                // 창 모드: 창 크기에 맞게 비율 유지하며 스케일링 (여백 없이 채움)
                scaledPixmap = originalPixmap.scaled(size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
            }

            label->setPixmap(scaledPixmap);
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
