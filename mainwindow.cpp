/*
 * popcorn (c) 2016 Michael Franzl
 * 
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */


#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    m_version = QString::number(VERSION_MAJOR) + "." + QString::number(VERSION_MINOR) + "." + QString::number(VERSION_PATCH);
}

MainWindow::~MainWindow() {
    qDebug() << "Level0 [MainWindow::~MainWindow] Called";
    if (windowState().testFlag(Qt::WindowMinimized) == true) {
        settings->setValue("minified_state", "true");
    } else {
        settings->setValue("minified_state", "false");
    }
    settings->setValue("exit", "true");
    settings->sync();

    qDebug() << "Level0 [MainWindow::~MainWindow] Done";
}

void MainWindow::init(QString index_file_cmdline) {
    //qApp->installEventFilter(this);
  
    setWindowTitle(settings->value("window_title").toString());

    if (settings->value("minified_state").toString() == "true") {
        setWindowState(Qt::WindowMinimized);
    }

    this->setMinimumWidth(430);
    this->setMinimumHeight(430);

    webView = new QWebView(this);
    if (settings->value("context_menu").toString() == "false") {
        webView->setContextMenuPolicy(Qt::NoContextMenu);
    }
    this->setCentralWidget(webView);

    m_network_manager = new QNetworkAccessManager(this);

    QWebPage *webPage = webView->page();
    webPage->setNetworkAccessManager(m_network_manager);
    webPage->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);
    connect(webPage, &QWebPage::linkClicked, this, &MainWindow::onLinkClicked);
    connect(webPage->mainFrame(), &QWebFrame::javaScriptWindowObjectCleared, this, &MainWindow::attachJsApi);

    webView->show();

    bootstrap(index_file_cmdline);

    // restore saved window geometry from .ini file
    QVariant size = settings->value("main_window_geometry");
    if ( size.isNull() ) {
        this->setGeometry(QRect(500, 100, 700, 500));
    } else {
        this->setGeometry(size.toRect());
    }

    // restore saved zoom factor from .ini file
    qreal z = settings->value("zoom_factor").toReal();
    if ( z ) webView->page()->mainFrame()->setZoomFactor(z);

    m_jsApi = new JsApi(this);

    optionsDialog = new OptionsDialog(this);
    optionsDialog->m_version = m_version;
    connect(optionsDialog, SIGNAL(accepting()), m_jsApi, SLOT(onOptionsDialogAccepted()));

    QShortcut *zoomin = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Plus), this);
    connect(zoomin, &QShortcut::activated, this, &MainWindow::onZoomIn);

    QShortcut *zoomout = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Minus), this);
    connect(zoomout, &QShortcut::activated, this, &MainWindow::onZoomOut);

    QShortcut *shutdown = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Q), this);
    connect(shutdown, &QShortcut::activated, m_jsApi, &JsApi::shutdown);

    QWebSettings::setObjectCacheCapacities(0, 0, 0);

    if (settings->value("webinspector").toString() == "true") {
        QWebSettings::globalSettings()->setAttribute(QWebSettings::DeveloperExtrasEnabled, true);

        QWebInspector *inspector;
        inspector = new QWebInspector();
        inspector->setPage(webView->page());
        inspector->setGeometry(QRect(500, 10, 1000, 700));
        //inspector->show();
    }

    QIcon icon = QIcon(application_path + "/popcorn.png");
    m_trayIcon = new QSystemTrayIcon(this);
    m_trayIcon->setIcon(icon);
    m_trayIcon->show();
    connect(m_trayIcon, &QSystemTrayIcon::messageClicked, m_jsApi, &JsApi::onTrayMessageClicked);
    connect(m_trayIcon, &QSystemTrayIcon::activated, m_jsApi, &JsApi::onTrayIconActivated);
}



void MainWindow::bootstrap(QString index_file_cmdline) {
    QString index_file_ini = settings->value("index_file").toString();
    QString index_file_chosen;

    if ( index_file_cmdline != "" ) {
        // given on command line has highest priority
        index_file_chosen = "file:///" + index_file_cmdline;

    } else if ( index_file_ini != "" ) {
        // given in ini file
        index_file_chosen = index_file_ini;

    } else {
        index_file_chosen = "file:///" + application_path + "/boot.html";
    }

    qDebug() << "Level0 [MainWindow::bootstrap] Navigating to" << index_file_chosen;
    webView->load(QUrl(index_file_chosen));
}


void MainWindow::attachJsApi() {
    qDebug() << "Level1 [MainWindow::attachJsApi] Exposing the jsApi class to Javascript.";
    webView->page()->mainFrame()->addToJavaScriptWindowObject("API", m_jsApi);
}

void MainWindow::onLinkClicked(QUrl url) {
    QDesktopServices::openUrl( url );
}


void MainWindow::resizeEvent(QResizeEvent *event) {
    qDebug() << "Level1 [MainWindow::resizeEvent] Saving MainWindow geometry" << this->geometry();
    QVariant geometry = this->geometry();
    settings->setValue("main_window_geometry", geometry);
    QWidget::resizeEvent(event);
}

void MainWindow::moveEvent(QMoveEvent *event) {
    qDebug() << "Level1 [MainWindow::moveEvent] Saving MainWindow geometry" << this->geometry();
    QVariant geometry = this->geometry();
    settings->setValue("main_window_geometry", geometry);
    QWidget::moveEvent(event);
}

void MainWindow::onZoomIn() {
    qDebug() << "Level1 [MainWindow::onZoomIn]";
    qreal z = webView->page()->mainFrame()->zoomFactor();
    z = z + 0.05;
    webView->page()->mainFrame()->setZoomFactor(z);
    settings->setValue("zoom_factor", z);
}

void MainWindow::onZoomOut() {
    qDebug() << "Level1 [MainWindow::onZoomOut]";
    qreal z = webView->page()->mainFrame()->zoomFactor();
    z = z - 0.05;
    webView->page()->mainFrame()->setZoomFactor(z);
    settings->setValue("zoom_factor", z);
}
