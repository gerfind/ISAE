#include "MainApp.h"

#include <QMouseEvent>  // 移动窗口所需
#include <QtCore/Qt>
#include <QFontDatabase>
#include <iostream>  // 输入输出, debug用

#include "DockManager.h"  // 高级停靠系统
#include "about.h"        // 关于页面
#include "ui_MainApp.h"   // ui文件
#include "monitor.h"
#include <string>
#include <iostream>

MainApp::MainApp(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);  // 先布局
    QFontDatabase::addApplicationFont(":/imgs/wqy");
    QFontDatabase::addApplicationFont(":/imgs/earthOrbiter");

    QImage logo;  // 准备logo
    logo.load(":/imgs/assets/ISAE.svg");
    this->setWindowIcon(QPixmap::fromImage(logo));

    /* 设置无边框与背景 */
    this->setWindowFlags(Qt::FramelessWindowHint);
    this->setAttribute(Qt::WA_StyledBackground);

    /* 初始化窗口标志 */
    this->mMoving = false;
    this->isMaximum = true;
    this->isSidebarShow = true;
    this->isAppAreaShow = false;
    this->isTeamAreaShow = false;
    this->isWorkspaceAreaShow = false;

    /* 初始化窗口可伸缩部件的大小 */
    this->ui->appsArea->setFixedHeight(0);
    this->ui->teamArea->setFixedHeight(0);
    this->ui->workspaceArea->setFixedHeight(0);
    this->ui->sidebarWidget->setFixedWidth(250);

    /* 初始化动画计时器 */
    this->startDuration = 1;
    this->sideBarAnimation.setInterval(10);
    this->appAreaAnimation.setInterval(10);
    this->teamAreaAnimation.setInterval(10);
    this->workspaceAreaAnimation.setInterval(10);

    /* 初始化窗口状态 */
    this->showMaximized();

    /* 初始化关于页面 */
    this->aboutWindow = new about(this);
    this->aboutWindow->setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    this->aboutWindow->setWindowModality(Qt::ApplicationModal);

    /* 信号与槽 */
    QObject::connect(ui->maxButton, SIGNAL(clicked()), this,
                     SLOT(changeWindowStatus()));
    QObject::connect(&(this->sideBarAnimation), SIGNAL(timeout()),
                     SLOT(animateSideBar()));
    QObject::connect(ui->welcomeButton, SIGNAL(clicked()), this,
                     SLOT(pushSideBar()));
    QObject::connect(&(this->appAreaAnimation), SIGNAL(timeout()),
                     SLOT(animateAppList()));
    QObject::connect(ui->appsButton, SIGNAL(clicked()), this,
                     SLOT(pushAppList()));
    QObject::connect(&(this->teamAreaAnimation), SIGNAL(timeout()),
                     SLOT(animateTeamList()));
    QObject::connect(ui->teamButton, SIGNAL(clicked()), this,
                     SLOT(pushTeamList()));
    QObject::connect(&(this->workspaceAreaAnimation), SIGNAL(timeout()),
                     SLOT(animateWorkspaceList()));
    QObject::connect(&(this->CPUMonitorTimer), SIGNAL(timeout()),
                     SLOT(upgradeCPUStatus()));
    QObject::connect(ui->workspaceButton, SIGNAL(clicked()), this,
                     SLOT(pushWorkspaceList()));

    QAction *act = this->ui->aboutButton->getmenu()->addAction(QString("关于"));
    QObject::connect(act, SIGNAL(triggered()), this, SLOT(showAbout()));

    /* 初始化默认字体 */
    this->defaultFont.setFamily("WenQuanYi Micro Hei Mono");

    /* 初始化状态栏 */
    this->statusBar()->setContentsMargins(8, 0, 0, 0);

    this->WootecStatusBox = new QPushButton(this);
    this->WootecStatusBox->setObjectName("WootecStatusBox");
    this->WootecStatusBox->setIcon(QIcon::fromTheme(":/imgs/assets/online.svg"));
    this->WootecStatusBox->setObjectName("WootecStatusBox");
    this->WootecStatusBox->setIconSize(QSize(24, 20));
    this->WootecStatusBox->setText(QString(" Wootec Cloud  "));
    this->statusBar()->addWidget(this->WootecStatusBox);
    this->WootecStatusBox->setFont(this->defaultFont);

    this->CPUStatusBox = new QPushButton(this);
    this->CPUStatusBox->setObjectName("CPUStatusBox");
    this->CPUStatusBox->setIcon(QIcon::fromTheme(":/imgs/assets/CPU.svg"));
    this->CPUStatusBox->setIconSize(QSize(22, 20));
    this->CPUStatusBox->setText(QString(" CPU 00.00% "));
    this->CPUStatusBox->setFont(this->defaultFont);
    this->statusBar()->addWidget(this->CPUStatusBox);

    // CPU Monitor
    this->CPUMonitorTimer.setInterval(1000);
    JQCPUMonitor::initialize();
    this->CPUMonitorTimer.start();
    QFile qssFile(":/imgs/defaultStyle");
    qssFile.open(QFile::ReadOnly);
    QString qss = QLatin1String(qssFile.readAll());
    this->setStyleSheet(qss);
    qssFile.close();
}

MainApp::~MainApp() { delete ui; }

/* 窗口移动函数 */
void MainApp::mousePressEvent(QMouseEvent *event) {
    if (this->isMaximum) return;
    this->mMoving = true;
    this->mMovePosition = event->globalPos() - pos();
    return QMainWindow::mousePressEvent(event);
}

void MainApp::mouseMoveEvent(QMouseEvent *event) {
    if (this->isMaximum) return;
    if (this->mMoving && (event->buttons() & Qt::LeftButton) &&
        (event->globalPos() - mMovePosition).manhattanLength() >
            QApplication::startDragDistance()) {
        move(event->globalPos() - mMovePosition);
        mMovePosition = event->globalPos() - pos();
    }
    return QMainWindow::mouseMoveEvent(event);
}

void MainApp::mouseReleaseEvent(QMouseEvent *event) {
    this->mMoving = false;
    event->accept();
}
void MainApp::changeWindowStatus() {
    if (this->isMaximum) {
        this->showNormal();
        this->isMaximum = false;
    } else {
        this->showMaximized();
        this->isMaximum = true;
    }
}

/* 侧边栏动画函数 */
void MainApp::pushSideBar() { this->sideBarAnimation.start(); }

void MainApp::animateSideBar() {
    if (this->isSidebarShow) {
        // std::cout << startDuration << ", " <<
        // this->ui->sidebarWidget->width() << std::endl;

        this->ui->sidebarWidget->setFixedWidth(
            this->ui->sidebarWidget->width() - startDuration);

        if (this->ui->sidebarWidget->width() - 42 > 104)
            startDuration += 1;
        else if (this->ui->sidebarWidget->width() == 145)
            ;
        else
            startDuration -= 1;

        if (startDuration <= 0) startDuration = 1;

        if (this->ui->sidebarWidget->width() <= 42) {
            this->ui->sidebarWidget->setFixedWidth(42);
            this->sideBarAnimation.stop();
            this->isSidebarShow = false;

            startDuration = 1;
        }
    } else {
        this->ui->sidebarWidget->setFixedWidth(
            this->ui->sidebarWidget->width() + startDuration);

        if (this->ui->sidebarWidget->width() - 42 < 104)
            startDuration += 1;
        else if (this->ui->sidebarWidget->width() == 147)
            ;
        else
            startDuration -= 1;

        if (startDuration <= 0) startDuration = 1;

        if (this->ui->sidebarWidget->width() >= 250) {
            this->ui->sidebarWidget->setFixedWidth(250);
            this->sideBarAnimation.stop();
            this->isSidebarShow = true;

            startDuration = 1;
        }
    }
}

/* 工作区/分区/团队区分栏展开动画 */
void MainApp::pushAppList() { this->appAreaAnimation.start(); }

void MainApp::animateAppList() {
    if (this->isAppAreaShow) {

        if (this->ui->appsArea->height() - 8 <= 0) {
            this->ui->appsArea->setFixedHeight(0);
            this->appAreaAnimation.stop();
            this->isAppAreaShow = false;
        }
        else this->ui->appsArea->setFixedHeight(this->ui->appsArea->height() - 8);
    } else {
        this->ui->appsArea->setFixedHeight(this->ui->appsArea->height() + 8);

        if (this->ui->appsArea->height() >= 150) {
            this->ui->appsArea->setFixedHeight(150);
            this->appAreaAnimation.stop();
            this->isAppAreaShow = true;
        }
    }
}
void MainApp::pushTeamList() { this->teamAreaAnimation.start(); }

void MainApp::animateTeamList() {
    if (this->isTeamAreaShow) {

        if (this->ui->teamArea->height() - 8 <= 0) {
            this->ui->teamArea->setFixedHeight(0);
            this->teamAreaAnimation.stop();
            this->isTeamAreaShow = false;
        }
        else this->ui->teamArea->setFixedHeight(this->ui->teamArea->height() - 8);

    } else {
        this->ui->teamArea->setFixedHeight(this->ui->teamArea->height() + 8);

        if (this->ui->teamArea->height() >= 150) {
            this->ui->teamArea->setFixedHeight(150);
            this->teamAreaAnimation.stop();
            this->isTeamAreaShow = true;
        }
    }
}
void MainApp::pushWorkspaceList() { this->workspaceAreaAnimation.start(); }

void MainApp::animateWorkspaceList() {
    if (this->isWorkspaceAreaShow) {

        if (this->ui->workspaceArea->height() - 8 <= 0) {
            this->ui->workspaceArea->setFixedHeight(0);
            this->workspaceAreaAnimation.stop();
            this->isWorkspaceAreaShow = false;
        }
        else this->ui->workspaceArea->setFixedHeight(this->ui->workspaceArea->height() - 8);
    } else {
        this->ui->workspaceArea->setFixedHeight(
            this->ui->workspaceArea->height() + 8);

        if (this->ui->workspaceArea->height() >= 150) {
            this->ui->workspaceArea->setFixedHeight(150);
            this->workspaceAreaAnimation.stop();
            this->isWorkspaceAreaShow = true;
        }
    }
}

/* 显示关于页 */
void MainApp::showAbout() { this->aboutWindow->show(); }

void MainApp::upgradeCPUStatus() {
    QString info = " CPU ";
    info += QString::number(JQCPUMonitor::cpuUsagePercentage() * 100, 'f', 2).rightJustified(5, '0') + "% ";
    this->CPUStatusBox->setText(info);
}
