#include <QtMath>
#include <QDesktopServices>
#include <QPainter>
//
#include "AppCore"
#include "FileSys"
#include "Convert"
#include "NetworkClient"
#include "Geo"
#include "Graph"
#include "SystemUtils"
#include "GuiUtils"
//
#include "canvas.h"
#include "monitorobject.h"
//==============================================================================

using namespace nayk;
using namespace file_sys;

//==============================================================================
Canvas::Canvas(QWidget *parent)
    : QGraphicsView(parent)
{
    setAttribute( Qt::WA_QuitOnClose );
    setAttribute( Qt::WA_TranslucentBackground );
    setWindowFlags( Qt::Tool | Qt::WindowStaysOnBottomHint | Qt::FramelessWindowHint );
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setRenderHint(QPainter::Antialiasing);

    m_log = new Log(this);
    connect(this, &Canvas::toLog, m_log, &Log::saveToLog, Qt::QueuedConnection);
    emit toLog(tr("Delete old log files"), Log::LogDbg);
    m_log->deleteOldLogFiles();

    m_settings = new Settings(this);
    connect(m_settings, &Settings::toLog, m_log, &Log::saveToLog, Qt::QueuedConnection);
    connect(m_settings, &Settings::startReading, this, &Canvas::on_settingsStartReading);
    connect(m_settings, &Settings::finishReading,
            this, &Canvas::on_settingsFinishReading, Qt::QueuedConnection);

    m_scene = new CustomScene(this);
    setScene(m_scene);
    connect(m_scene, &CustomScene::toLog, m_log, &Log::saveToLog, Qt::QueuedConnection);
    connect(m_scene, &CustomScene::rightClick, this, &Canvas::on_sceneRightClick);

    setupMenu();
    installEventFilter(this);
    m_settings->reloadSettings();
}
//==============================================================================
Canvas::~Canvas()
{
    emit toLog(tr("Destructor ~Canvas()"), Log::LogDbg);

    if(m_scene) {
        m_scene->clear();
        delete m_scene;
    }

    if(m_settings) delete m_settings;
    if(m_log) delete m_log;
}
//==============================================================================
void Canvas::setupMenu()
{
    m_menu.addAction( tr("Open Map"), this, &Canvas::on_actionOpenMapTriggered );
    m_menu.addSeparator();
    m_menu.addAction( tr("Reload settings"), m_settings, &Settings::reloadSettings );
    m_menu.addAction( tr("About"), this, &Canvas::on_actionAboutTriggered );
    m_menu.addSeparator();
    m_menu.addAction( tr("Exit"), this, &Canvas::close );
}
//==============================================================================
bool Canvas::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::GraphicsSceneWheel) {
        //QWheelEvent *wEvent = static_cast<QWheelEvent *>(event);
        emit toLog(tr("GraphicsSceneWheel event"), Log::LogDbg);
        return true;
    }
    else if (event->type() == QEvent::Scroll) {
        emit toLog(tr("Scroll event"), Log::LogDbg);
        return true;
    }
    else if (event->type() == QEvent::Wheel) {
        emit toLog(tr("Wheel event"), Log::LogDbg);
        return true;
    }
    else if(event->type() == QEvent::KeyPress) {
        //QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        emit toLog(tr("KeyPress event"), Log::LogDbg);
        return true;
    }
    else {
        return QObject::eventFilter(obj, event);
    }
}
//==============================================================================
void Canvas::on_settingsStartReading()
{
    emit toLog(tr("Canvas slot on_settingsStartReading()"), Log::LogDbg);

}
//==============================================================================
void Canvas::on_settingsFinishReading()
{
    emit toLog(tr("Canvas slot on_settingsFinishReading()"), Log::LogDbg);

    m_scene->clear();
    setFixedSize( m_settings->screenGeometry().width(),
                  m_settings->screenGeometry().height() );
    setGeometry( m_settings->screenGeometry() );
    m_scene->setSceneRect( 0, 0, geometry().width(), geometry().height() );

    MonitorObject *obj = new MonitorObject(m_settings);
    obj->setObjectName("monitorObject");
    obj->setPos( 0, static_cast<qreal>(geometry().height())
                 - m_settings->box().height - 3.0 * m_settings->box().border_width );
    m_scene->addItem(obj);
}
//==============================================================================
void Canvas::on_actionAboutTriggered()
{
    emit toLog(tr("Canvas slot on_actionAboutTriggered()"), Log::LogDbg);
    gui_utils::showAboutDialog(tr("Evgeniy Teterin"), this);
}
//==============================================================================
void Canvas::on_actionOpenMapTriggered()
{
    emit toLog(tr("Canvas slot on_actionOpenMapTriggered()"), Log::LogDbg);

}
//==============================================================================
void Canvas::on_sceneRightClick(const QPoint &screenPos, const QPointF &scenePos)
{
    Q_UNUSED(scenePos)
    emit toLog(tr("Canvas slot on_sceneRightClick()"), Log::LogDbg);
    m_menu.popup(screenPos);
}
//==============================================================================