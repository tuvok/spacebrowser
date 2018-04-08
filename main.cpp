#include <BasicDownloader.h>
#include <ContentFilter.h>
#include <ViewHandler.h>
#include <PrintHandler.h>

#include <QApplication>
#include <QObject>
#include <QQuickItem>
#include <QQuickView>
//#include <QWebEngineView>
#include <QtWebEngine>
#include <QQuickWebEngineProfile>
#include <QShortcut>
#include <QTextEdit>

#include <memory>


int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("Space Browser v1");
    //FIXME: test this
    //QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QtWebEngine::initialize();

    ContentFilter cf;
    QQuickWebEngineProfile* profile = QQuickWebEngineProfile::defaultProfile();
    profile->setRequestInterceptor(&cf);

    PrintHandler ph;

    std::shared_ptr<QQuickView> view(new QQuickView);

//    qmlRegisterType<QTextEdit>("org.qt.qtextedit", 1, 0, "QTextEdit");
//    qmlRegisterType<QWebEngineView>("org.qt.qwebengineview", 1, 0, "QWebEngineView");

    view->setSource(QUrl("qrc:/ui/MainWindow.qml"));
    view->setResizeMode(QQuickView::SizeRootObjectToView);

    view->show();

    QObject::connect(view->rootObject(), SIGNAL(printRequest(QVariant)),
            &ph, SLOT(printRequested(QVariant)));

    QQuickItem* webViewContainer = qobject_cast<QQuickItem*>(
            view->rootObject()->findChild<QObject*>("webViewContainer"));
    if (!webViewContainer)
        throw std::runtime_error("No webViewContainer object found");

    QQuickItem* scriptBlockingView = qobject_cast<QQuickItem*>(
            view->rootObject()->findChild<QObject*>("scriptBlockingView"));
    if (!scriptBlockingView)
        throw std::runtime_error("No scriptBlockingView object found");

    QQuickItem* tabSelector = qobject_cast<QQuickItem*>(
            view->rootObject()->findChild<QObject*>("tabSelector"));

    ViewHandler vh(webViewContainer, tabSelector, scriptBlockingView, cf, view);
    view->engine()->rootContext()->setContextProperty("viewHandler", &vh);
    vh.loadTabs();

    /// Set-up downloader module
    ///
    QQuickItem* downloaderProgressBar = qobject_cast<QQuickItem*>(
            view->rootObject()->findChild<QObject*>("downloadProgressBar"));
    if (!downloaderProgressBar)
        throw std::runtime_error("No downloaderProgressBar object found");
    BasicDownloader bd;
    QObject::connect(profile, SIGNAL(downloadRequested(QQuickWebEngineDownloadItem*)),
            &bd, SLOT(downloadRequested(QQuickWebEngineDownloadItem*)));
    QObject::connect(profile, SIGNAL(downloadFinished(QQuickWebEngineDownloadItem*)),
                &bd, SLOT(downloadFinished(QQuickWebEngineDownloadItem*)));
    QObject::connect(&bd, SIGNAL(progressUpdated(QVariant)),
            downloaderProgressBar, SLOT(updateProgress(QVariant)));

    QQuickItem* downloadHistoryButton = qobject_cast<QQuickItem*>(
            view->rootObject()->findChild<QObject*>("downloadHistoryButton"));
    if (!downloadHistoryButton)
        throw std::runtime_error("No downloadHistoryButton object found");
    QObject::connect(&bd, &BasicDownloader::historyChanged,
            downloadHistoryButton, &QQuickItem::setVisible);

    QQuickItem* downloadHistoryView = qobject_cast<QQuickItem*>(
            view->rootObject()->findChild<QObject*>("downloadHistoryView"));
    if (!downloadHistoryView)
        throw std::runtime_error("No downloadHistoryView object found");
    QObject::connect(&bd, SIGNAL(downloadFinished(QVariant)),
            downloadHistoryView, SLOT(downloadFinished(QVariant)));
    QObject::connect(&bd, SIGNAL(newHistoryEntry(QVariant)),
            downloadHistoryView, SLOT(addEntry(QVariant)));
    QObject::connect(&bd, SIGNAL(progressUpdated(QVariant, QVariant, QVariant)),
            downloadHistoryView, SLOT(updateProgress(QVariant, QVariant, QVariant)));
    QObject::connect(&bd, SIGNAL(downloadPaused(QVariant)),
            downloadHistoryView, SLOT(downloadPaused(QVariant)));
    QObject::connect(&bd, SIGNAL(downloadResumed(QVariant)),
                downloadHistoryView, SLOT(downloadResumed(QVariant)));
    QObject::connect(&bd, SIGNAL(downloadCanceled(QVariant)),
                downloadHistoryView, SLOT(downloadCanceled(QVariant)));
    QObject::connect(downloadHistoryView, SIGNAL(openUrl(QString)),
            &bd, SLOT(openUrl(QString)));
    QObject::connect(downloadHistoryView, SIGNAL(pause(int)),
                &bd, SLOT(pause(int)));
    QObject::connect(downloadHistoryView, SIGNAL(resume(int)),
                &bd, SLOT(resume(int)));
    QObject::connect(downloadHistoryView, SIGNAL(cancel(int)),
                &bd, SLOT(cancel(int)));


    if (tabSelector)
    {
        QObject::connect(tabSelector, SIGNAL(viewSelected(int)), &vh, SLOT(viewSelected(int)));
        QObject::connect(tabSelector, SIGNAL(closeTab(int)), &vh, SLOT(closeTab(int)));
        QObject::connect(tabSelector, SIGNAL(openScriptBlockingView(int)), &vh, SLOT(openScriptBlockingView(int)));
    }

    QObject::connect(scriptBlockingView, SIGNAL(whitelistLocal(QString, QString)),
            &cf, SLOT(whitelistLocal(QString, QString)));
    QObject::connect(scriptBlockingView, SIGNAL(whitelistGlobal(QString)),
            &cf, SLOT(whitelistGlobal(QString)));
    QObject::connect(scriptBlockingView, SIGNAL(removeLocal(QString, QString)),
            &cf, SLOT(removeLocal(QString, QString)));
    QObject::connect(scriptBlockingView, SIGNAL(removeGlobal(QString)),
            &cf, SLOT(removeGlobal(QString)));

    return app.exec();
}
