#include "painterwindow.h"
#include "ui_painterwindow.h"
#include "painternewimagedialog.h"
#include "paintertools.h"
#include "paintermodule.h"


namespace ActorPainter {

PainterWindow::PainterWindow(PainterModule * module, QWidget *parent) :
    QWidget(),
    ui(new Ui::PainterWindow),
    m_module(module)
{
    setParent(parent);
    setMinimumSize(500, 300);
    ui->setupUi(this);
    ui->horizontalRuler->setScrollBar(ui->scrollArea->horizontalScrollBar());
    ui->verticalRuler->setScrollBar(ui->scrollArea->verticalScrollBar());
    ui->horizontalRuler->setView(ui->view);
    ui->verticalRuler->setView(ui->view);
    connect (ui->scrollArea->horizontalScrollBar(), SIGNAL(valueChanged(int)),
             ui->horizontalRuler, SLOT(update()));
    connect (ui->scrollArea->verticalScrollBar(), SIGNAL(valueChanged(int)),
             ui->verticalRuler, SLOT(update()));
    connect (ui->view, SIGNAL(cursorOver(int,int,QColor)), this, SLOT(handleCursorMoved(int,int,QColor)));
    m_newImageDialog = new PainterNewImageDialog(this, m_module);
    connect (m_module->m_actionPainterNewPage, SIGNAL(triggered()), this, SLOT(newImage()));
    connect (m_module->m_actionPainterLoadPage, SIGNAL(triggered()), this, SLOT(loadImage()));
    connect (m_module->m_actionPainterSaveACopyAs, SIGNAL(triggered()), this, SLOT(saveImageAs()));
    connect (m_module->m_actionPainterRevertPage, SIGNAL(triggered()), this, SLOT(reset()));

    connect (m_module->m_actionPainterColorInStatusBarCMYK, SIGNAL(triggered()), this, SLOT(handleColorTextModeChanged()));
    connect (m_module->m_actionPainterColorInStatusBarRGB, SIGNAL(triggered()), this, SLOT(handleColorTextModeChanged()));
    connect (m_module->m_actionPainterColorInStatusBarHSL, SIGNAL(triggered()), this, SLOT(handleColorTextModeChanged()));
    connect (m_module->m_actionPainterColorInStatusBarHSV, SIGNAL(triggered()), this, SLOT(handleColorTextModeChanged()));
    connect (m_module->m_actionPainterColorInStatusBarRRGGBB, SIGNAL(triggered()), this, SLOT(handleColorTextModeChanged()));

    QSettings * s = m_module->mySettings();
    restoreGeometry(s->value("WindowGeometry").toByteArray());
    qreal initialZoom = 1.0;
    QRect screenRect = qApp->desktop()->availableGeometry();
    if (screenRect.height()<700)
        initialZoom = 0.5;
    initialZoom = s->value("ViewZoom",initialZoom).toReal();
    e_showColorMode = s->value("ShowColorMode","HTML").toString();
    if (e_showColorMode=="HTML")
        m_module->m_actionPainterColorInStatusBarRRGGBB->setChecked(true);
    if (e_showColorMode=="RGB")
        m_module->m_actionPainterColorInStatusBarRGB->setChecked(true);
    if (e_showColorMode=="CMYK")
        m_module->m_actionPainterColorInStatusBarCMYK->setChecked(true);
    if (e_showColorMode=="HSL")
        m_module->m_actionPainterColorInStatusBarHSL->setChecked(true);
    if (e_showColorMode=="HSV")
        m_module->m_actionPainterColorInStatusBarHSV->setChecked(true);
    ui->view->setZoom(initialZoom);
    ui->horizontalRuler->setZoom(initialZoom);
    ui->verticalRuler->setZoom(initialZoom);


    connect(m_module->m_actionPainterScale100, SIGNAL(triggered()),
            this, SLOT(handleScale()));
    connect(m_module->m_actionPainterFitWidth, SIGNAL(triggered()),
            this, SLOT(handleScale()));
    connect(m_module->m_actionPainterFitHeight, SIGNAL(triggered()),
            this, SLOT(handleScale()));
    connect(m_module->m_actionPainterFitWindow, SIGNAL(triggered()),
            this, SLOT(handleScale()));

    handleCursorMoved(-1,-1,"");
    ui->zoomLabel->setText(QString::number(int(ui->view->zoom()*100))+"%");
    static const QString resourcesRoot = QApplication::instance()->property("sharePath").toString()+
            "/actors/painter/";
    QSettings templates(QDir(resourcesRoot).absoluteFilePath("templates.ini"), QSettings::IniFormat);
    templates.setIniCodec("UTF-8");
    const QString defName = templates.value("Names/default.png", "new page").toString();
    setWindowTitle(tr("Painter [%1]").arg(defName));


}

void PainterWindow::setCanvas(QImage * canvas, QMutex *locker)
{
    ui->view->setCanvas(canvas, locker);
    ui->horizontalRuler->setCanvas(canvas);
    ui->verticalRuler->setCanvas(canvas);
}

QWidget * PainterWindow::view()
{
    return ui->view;
}

void PainterWindow::handleCursorMoved(int x, int y, const QColor &color)
{
    const QString colorRgb = QString("rgb(%1,%2,%3)")
                             .arg(color.red())
                             .arg(color.green())
                             .arg(color.blue());
    const QString colorHumanName = CSS_RGB_toString(colorRgb);
    QString colorName;
    if (e_showColorMode=="RGB") {
        colorName = QString("%1, %2, %3")
                    .arg(color.red())
                    .arg(color.green())
                    .arg(color.blue());
    }
    else if (e_showColorMode=="CMYK") {
        colorName = QString("%1, %2, %3, %4")
                    .arg(color.cyan())
                    .arg(color.magenta())
                    .arg(color.yellow())
                    .arg(color.black());
    }
    else if (e_showColorMode=="HSL") {
        colorName = QString("%1, %2, %3")
                    .arg(color.hue())
                    .arg(color.saturation())
                    .arg(color.lightness());
    }
    else if (e_showColorMode=="HSV") {
        colorName = QString("%1, %2, %3")
                    .arg(color.hue())
                    .arg(color.saturation())
                    .arg(color.value());
    }
    else {
        colorName = color.name().toUpper();
    }
    if (!colorHumanName.isEmpty()) {
        colorName += " ("+colorHumanName+")";
    }
    if (x==-1 || y==-1) {
        ui->horizontalRuler->highlightValue(-1);
        ui->verticalRuler->highlightValue(-1);
        ui->statusBar->setText("");
    }
    else {
        ui->horizontalRuler->highlightValue(x);
        ui->verticalRuler->highlightValue(y);
        ui->statusBar->setText(tr("Coordinates: X = %1, Y = %2; Colour: %3").arg(x).arg(y).arg(colorName));
    }
}

void PainterWindow::handleColorTextModeChanged()
{
    QAction *a = qobject_cast<QAction*>(sender());
    QList<QAction*> as;
    as << m_module->m_actionPainterColorInStatusBarCMYK;
    as << m_module->m_actionPainterColorInStatusBarHSL;
    as << m_module->m_actionPainterColorInStatusBarHSV;
    as << m_module->m_actionPainterColorInStatusBarRGB;
    as << m_module->m_actionPainterColorInStatusBarRRGGBB;
    as.removeAll(a);
    a->setChecked(true);
    foreach (QAction *aa, as) {
        aa->setChecked(false);
    }
    if (a==m_module->m_actionPainterColorInStatusBarCMYK)
        e_showColorMode = "CMYK";
    if (a==m_module->m_actionPainterColorInStatusBarRGB)
        e_showColorMode = "RGB";
    if (a==m_module->m_actionPainterColorInStatusBarHSL)
        e_showColorMode = "HSL";
    if (a==m_module->m_actionPainterColorInStatusBarHSV)
        e_showColorMode = "HSV";
    if (a==m_module->m_actionPainterColorInStatusBarRRGGBB)
        e_showColorMode = "HTML";
}

void PainterWindow::newImage()
{
    if (m_newImageDialog->exec()==QDialog::Accepted) {
        int w = m_newImageDialog->w();
        int h = m_newImageDialog->h();
        s_fileName = "";
        s_templateName = m_newImageDialog->templateFileName();
        static const QString resourcesRoot = QApplication::instance()->property("sharePath").toString()+
                "/actors/painter/";
        if (m_newImageDialog->isTemplate()) {
            m_module->runLoadPage(resourcesRoot+s_templateName);
        }
        else {
            m_module->runNewPage(w, h, m_newImageDialog->color());
        }
        if (m_newImageDialog->isTemplate()) {
            setWindowTitle(tr("Painter [%1]").arg(m_newImageDialog->templateName()));
        }
        else {
            setWindowTitle(tr("Painter [new page]"));
        }
    }
}

void PainterWindow::loadImage()
{
    QString searchDir = QDir::homePath();
    QSettings * s = m_module->mySettings();
    searchDir = s->value("LastDir", searchDir).toString();
    QString fn = QFileDialog::getOpenFileName(this,
                                              tr("Load image..."),
                                              searchDir,
                                              tr("Images (*.png)"));
    if (!fn.isEmpty() && QFile::exists(fn)) {
        m_module->runLoadPage(fn);
        s_templateName = fn;
        s->setValue("LastDir", QFileInfo(fn).dir().absolutePath());
        setWindowTitle(tr("Painter [%1]").arg(QFileInfo(fn).fileName()));
    }
}



void PainterWindow::saveImageAs()
{
    QString searchDir = QDir::homePath();
    QSettings * s = m_module->mySettings();
    QString suggestName = s_templateName.isEmpty()?
                          QString::fromUtf8("picture.png") :
                          QString::fromUtf8("copy of ")+QFileInfo(s_templateName).fileName();
    searchDir = s->value("LastDir", searchDir).toString();
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save image..."),
                                searchDir+"/"+suggestName,
                                tr("Images (*.png)"));
    if (!fileName.isEmpty()) {
        saveImageToFile(fileName);
        s_fileName = fileName;
        if (s_templateName.isEmpty()) {
            setWindowTitle(tr("%1 - Painter").arg(QFileInfo(fileName).fileName()));
        }
        else {
            setWindowTitle(tr("%2 - Painter [%1]").arg(QFileInfo(s_templateName).fileName()).arg(QFileInfo(fileName).fileName()));
        }
        s->setValue("LastDir", QFileInfo(fileName).dir().absolutePath());
    }
}

void PainterWindow::saveImageToFile(const QString &fileName)
{
    ui->view->canvas()->save(fileName, "PNG");
}

void PainterWindow::reset()
{
    m_module->reset();
}


void PainterWindow::handleScale()
{
    qreal scale = 1.0;
    QAction *act = qobject_cast<QAction*>(sender());
    if (act==m_module->m_actionPainterFitWidth) {
        qreal imW = ui->view->canvas()->width()+60;
        qreal wW = ui->scrollArea->maximumViewportSize().width();
        scale = wW/imW;
    }
    else if (act==m_module->m_actionPainterFitHeight) {
        qreal imH = ui->view->canvas()->height()+60;
        qreal wH = ui->scrollArea->maximumViewportSize().height();
        scale = wH/imH;
    }
    else if (act==m_module->m_actionPainterFitWindow) {
        qreal imH = ui->view->canvas()->height()+60;
        qreal wH = ui->scrollArea->maximumViewportSize().height();
        qreal scaleH = wH/imH;
        qreal imW = ui->view->canvas()->width()+60;
        qreal wW = ui->scrollArea->maximumViewportSize().width();
        qreal scaleW = wW/imW;
        scale = qMin(scaleW, scaleH);
    }
    ui->view->setZoom(scale);
    ui->verticalRuler->setZoom(scale);
    ui->horizontalRuler->setZoom(scale);
    ui->zoomLabel->setText(QString::number(int(ui->view->zoom()*100))+"%");
}


void PainterWindow::handleFullScreen()
{
//    if (qApp->desktop()->screenCount()>1) {

//    }
//    ui->view->showFullScreen();
}

PainterWindow::~PainterWindow()
{
    delete ui;
}



}
