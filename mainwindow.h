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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QApplication>
#include <QMainWindow>
#include <QtGui>
#include <QWebView>
#include <QWebFrame>
#include <QWebPage>
#include <QtWebKit>
#include <QSettings>
#include <QDebug>

#include <QShortcut>
#include <QWebInspector>

extern QSettings *settings;
extern QString application_path;
extern QString home_path;


#include "jsapi.h"
#include "optionsdialog.h"

#define APPNAME "popcorn"
#define VERSION_MAJOR 0
#define VERSION_MINOR 1
#define VERSION_PATCH 0

class JsApi;

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

    // methods
    void init(bool is_development);
    void resizeEvent(QResizeEvent *event);
    void moveEvent(QMoveEvent *event);
    void bootstrap(bool is_development);

    //variables
    QWebView * webView;
    QSystemTrayIcon * m_trayIcon;
    OptionsDialog * optionsDialog;
    QString m_version;
    QNetworkAccessManager * m_network_manager;


private:
    // class member variables
    JsApi *m_jsApi;


public slots:
    void attachJsApi();
    void onZoomIn();
    void onZoomOut();
    void onLinkClicked(QUrl url);
};

#endif // MAINWINDOW_H

