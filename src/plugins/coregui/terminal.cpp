#include "terminal.h"
#include "terminal_plane.h"
#include "terminal_onesession.h"

#include "extensionsystem/pluginmanager.h"

namespace Terminal {


Term::Term(QWidget *parent) :
    QWidget(parent)
{
    setWindowTitle(tr("Input/Output"));
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);
//    setMinimumWidth(450);
//    setMinimumHeight(200);
    QGridLayout * l = m_layout = new QGridLayout();
    l->setContentsMargins(0,0,0,0);
    l->setSpacing(0);
    setLayout(l);
    m_plane = new Plane(this);
    m_plane->installEventFilter(this);
    l->addWidget(m_plane, 1, 1, 1, 1);
    sb_vertical = new QScrollBar(Qt::Vertical, this);
//    sb_vertical->setFixedWidth(qMin(10, sb_vertical->width()));
//    static const char * ScrollBarCSS = ""
//            "QScrollBar {"
//            "   width: 12px;"
//            "   background-color: transparent;"
//            "   padding-right: 4px;"
//            "   border: 0;"
//            "}"
//            "QScrollBar:handle {"
//            "   background-color: gray;"
//            "   border-radius: 4px;"
//            "}"
//            "QScrollBar:add-line {"
//            "   height: 0;"
//            "}"
//            "QScrollBar:sub-line {"
//            "   height: 0;"
//            "}"
//            ;
    l->addWidget(sb_vertical, 1, 2, 1, 1);
//    sb_vertical->setStyleSheet(ScrollBarCSS);
    sb_vertical->installEventFilter(this);
    sb_horizontal = new QScrollBar(Qt::Horizontal, this);
    l->addWidget(sb_horizontal, 2, 1, 1, 1);


    a_saveLast = new QAction(tr("Save last output"), this);
    a_saveLast->setEnabled(false);
    connect(a_saveLast, SIGNAL(triggered()), this, SLOT(saveLast()));

    a_copyLast = new QAction(tr("Copy last output"), this);
    a_copyLast->setEnabled(false);
    connect(a_copyLast, SIGNAL(triggered()), this, SLOT(copyLast()));

    a_copyAll = new QAction(tr("Copy all output"), this);
    a_copyAll->setEnabled(false);
    connect(a_copyAll, SIGNAL(triggered()), this, SLOT(copyAll()));

    a_editLast = new QAction(tr("Open last output in editor"), this);
    a_editLast->setIcon(QIcon::fromTheme("document-edit", QIcon(ExtensionSystem::PluginManager::instance()->sharePath()+"/icons/document-edit.png")));
    a_editLast->setEnabled(false);
    connect(a_editLast, SIGNAL(triggered()), this, SLOT(editLast()));

    a_saveAll = new QAction(tr("Save all output"), this);
    a_saveAll->setEnabled(false);
    connect(a_saveAll, SIGNAL(triggered()), this, SLOT(saveAll()));

    a_clear = new QAction(tr("Clear output"), this);
    a_clear->setEnabled(false);
    connect(a_clear, SIGNAL(triggered()), this, SLOT(clear()));

    m_plane->updateScrollBars();

    connect(sb_vertical,SIGNAL(valueChanged(int)),m_plane, SLOT(update()));
    connect(sb_horizontal,SIGNAL(valueChanged(int)),m_plane, SLOT(update()));


    connect(m_plane, SIGNAL(inputTextChanged(QString)),
            this, SLOT(handleInputTextChanged(QString)));

    connect(m_plane, SIGNAL(inputCursorPositionChanged(quint16)),
            this, SLOT(handleInputCursorPositionChanged(quint16)));

    connect(m_plane, SIGNAL(inputFinishRequest()),
            this, SLOT(handleInputFinishRequested()));

}

bool Term::isEmpty() const
{
    return sessions_.isEmpty();
}

QSize Term::minimumSizeHint() const
{
    QSize result = m_plane->minimumSizeHint();
    result.rwidth() = qMax(result.width(), 400);
    if (sb_vertical->isVisible()) {
        result.rwidth() += sb_vertical->width();
        result.rheight() = qMax(result.rheight(), sb_vertical->minimumHeight());
    }
    if (sb_horizontal->isVisible()) {
        result.rheight() += sb_horizontal->height();
        result.rwidth() = qMax(result.width(), sb_horizontal->minimumWidth());
    }
    return result;
}

void Term::resizeEvent(QResizeEvent *e)
{
//    const QSize sz = e->size();
//    if (sz.width()>sz.height()) {
//        m_toolBar->setOrientation(Qt::Vertical);
//        m_layout->addWidget(m_toolBar, 1, 0, 1, 1);
//    }
//    else {
//        m_toolBar->setOrientation(Qt::Horizontal);
//        m_layout->addWidget(m_toolBar, 0, 1, 1, 1);
//    }
    e->accept();
}

bool Term::isActiveComponent() const
{
    return m_plane->hasFocus();
}

void Term::paintEvent(QPaintEvent *e)
{
    QPainter p(this);
    p.setPen(Qt::NoPen);
    p.setBrush(palette().brush(QPalette::Base));
    p.drawRect(0,0,width(), height());
    p.end();
    QWidget::paintEvent(e);
    p.begin(this);
    const QBrush br = m_plane->hasFocus()
            ? palette().brush(QPalette::Highlight)
            : palette().brush(QPalette::Window);
    p.setPen(QPen(br, 3));
    p.setBrush(Qt::NoBrush);
    p.drawLine(width()-1, 0, width()-1, height()-1);
    p.end();
    e->accept();
}

bool Term::eventFilter(QObject *obj, QEvent *evt)
{
    if (obj == sb_vertical && evt->type() == QEvent::Paint) {
        QPainter p(sb_vertical);
        const QBrush br = m_plane->hasFocus()
                ? palette().brush(QPalette::Highlight)
                : palette().brush(QPalette::Window);
        p.setPen(QPen(br, 3));
        p.drawLine(0, 0,
                   sb_vertical->width()-1, 0);
        p.drawLine(0, sb_vertical->height()-1,
                   sb_vertical->width()-1, sb_vertical->height()-1);
        p.end();
    }
    else if (obj == m_plane) {
        if (evt->type() == QEvent::FocusIn || evt->type() == QEvent::FocusOut) {
            sb_vertical->repaint();
        }
    }
    return false;
}


void Term::changeGlobalState(ExtensionSystem::GlobalState , ExtensionSystem::GlobalState current)
{
    using Shared::PluginInterface;
    if (current==PluginInterface::GS_Unlocked || current==PluginInterface::GS_Observe) {
        a_saveAll->setEnabled(sessions_.size()>0);
        a_saveLast->setEnabled(sessions_.size()>0);
        a_copyAll->setEnabled(sessions_.size()>0);
        a_copyLast->setEnabled(sessions_.size()>0);
        a_editLast->setEnabled(sessions_.size()>0);
        a_clear->setEnabled(sessions_.size()>0);
    }
    else {
        a_saveAll->setEnabled(false);
        a_saveLast->setEnabled(false);
        a_copyAll->setEnabled(false);
        a_copyLast->setEnabled(false);
        a_editLast->setEnabled(false);
        a_clear->setEnabled(false);
    }
}

void Term::handleInputTextChanged(const QString &text)
{
    if (sessions_.isEmpty())
        return;
    OneSession * last = sessions_.last();
    last->changeInputText(text);
}

void Term::handleInputCursorPositionChanged(quint16 pos)
{
    if (sessions_.isEmpty())
        return;
    OneSession * last = sessions_.last();
    last->changeCursorPosition(pos);
}

void Term::handleInputFinishRequested()
{
    if (sessions_.isEmpty())
        return;
    OneSession * last = sessions_.last();
    last->tryFinishInput();
}

void Term::focusInEvent(QFocusEvent *e)
{
    QWidget::focusInEvent(e);
    m_plane->setFocus();
}

void Term::focusOutEvent(QFocusEvent *e)
{
    QWidget::focusOutEvent(e);
    m_plane->clearFocus();
}

void Term::clear()
{
    for (int i=0; i<sessions_.size(); i++) {
        sessions_[i]->deleteLater();
    }
    sessions_.clear();
    m_plane->update();
    a_saveAll->setEnabled(false);
    a_saveLast->setEnabled(false);
    a_editLast->setEnabled(false);
    a_clear->setEnabled(false);
}

void Term::start(const QString & fileName)
{
    int fixedWidth = -1;
    OneSession * session = new OneSession(
                fixedWidth,
                fileName.isEmpty() ? tr("New Program") : QFileInfo(fileName).fileName(),
                m_plane
                );
    session->relayout(m_plane->width(), 0, true);
    connect(session, SIGNAL(updateRequest()), m_plane, SLOT(update()));
    sessions_ << session;
    connect (sessions_.last(), SIGNAL(message(QString)),
             this, SIGNAL(message(QString)), Qt::DirectConnection);
    connect (sessions_.last(), SIGNAL(inputDone(QVariantList)),
             this, SLOT(handleInputDone(QVariantList)));
    m_plane->updateScrollBars();
    if (sb_vertical->isEnabled())
        sb_vertical->setValue(sb_vertical->maximum());
    m_plane->update();
}

void Term::finish()
{
    if (sessions_.isEmpty())
        sessions_ << new OneSession(-1,"unknown", m_plane);

    sessions_.last()->finish();
    m_plane->updateScrollBars();
    if (sb_vertical->isEnabled())
        sb_vertical->setValue(sb_vertical->maximum());
}

void Term::terminate()
{
    if (sessions_.isEmpty())
        sessions_ << new OneSession(-1,"unknown", m_plane);
    sessions_.last()->terminate();
    m_plane->updateScrollBars();
    if (sb_vertical->isEnabled())
        sb_vertical->setValue(sb_vertical->maximum());
    m_plane->setInputMode(false);
}

void Term::output(const QString & text)
{
    emit showWindowRequest();
    if (sessions_.isEmpty())
        sessions_ << new OneSession(-1,"unknown", m_plane);
    sessions_.last()->output(text, CS_Output);
    m_plane->updateScrollBars();
    if (sb_vertical->isEnabled())
        sb_vertical->setValue(sb_vertical->maximum());
}

void Term::outputErrorStream(const QString & text)
{
    emit showWindowRequest();
    if (sessions_.isEmpty())
        sessions_ << new OneSession(-1,"unknown", m_plane);
    sessions_.last()->output(text, CS_Error);
    m_plane->updateScrollBars();
    if (sb_vertical->isEnabled())
        sb_vertical->setValue(sb_vertical->maximum());
}

void Term::input(const QString & format)
{
    emit showWindowRequest();
    if (sessions_.isEmpty()) {
        sessions_ << new OneSession(-1,"unknown", m_plane);
        connect (sessions_.last(), SIGNAL(inputDone(QVariantList)),
                 this, SIGNAL(inputFinished(QVariantList)));
        connect (sessions_.last(), SIGNAL(message(QString)),
                 this, SIGNAL(message(QString)));
        connect (sessions_.last(), SIGNAL(inputDone(QVariantList)),
                 this, SLOT(handleInputDone()));
    }
    OneSession * lastSession = sessions_.last();

    inputFormats_ = format.split(";", QString::SkipEmptyParts);
    inputValues_.clear();

    lastSession->input(format);
    m_plane->updateScrollBars();
    if (sb_vertical->isEnabled())
        sb_vertical->setValue(sb_vertical->maximum());
    m_plane->setInputMode(true);

    m_plane->setFocus();
    m_plane->update();
}

void Term::handleInputDone(const QVariantList & values)
{
    m_plane->setInputMode(false);
    inputValues_ += values;
    if (inputValues_.size() < inputFormats_.size()) {
        QStringList formats = inputFormats_;
        for (int i=0; i<inputValues_.size(); i++) {
            formats.pop_front();
        }
        const QString format = formats.join(";");
        OneSession * lastSession = sessions_.last();
        lastSession->input(format);
        m_plane->updateScrollBars();
        if (sb_vertical->isEnabled())
            sb_vertical->setValue(sb_vertical->maximum());
        m_plane->setInputMode(true);
        m_plane->setFocus();
    }
    else {
        emit inputFinished(inputValues_);
    }
}

void Term::error(const QString & message)
{
    emit showWindowRequest();
    if (sessions_.isEmpty())
        sessions_ << new OneSession(-1,"unknown", m_plane);
    sessions_.last()->error(message);
    m_plane->updateScrollBars();
    if (sb_vertical->isEnabled())
        sb_vertical->setValue(sb_vertical->maximum());
}


void Term::saveAll()
{
    const QString suggestedFileName = QDir::current().absoluteFilePath("output-all.txt");
    QString allText;
    for (int i=0; i<sessions_.size(); i++) {
        allText += sessions_[i]->plainText(true);
    }
    saveText(suggestedFileName, allText);
}

void Term::saveLast()
{
    QString suggestedFileName = QDir::current().absoluteFilePath(sessions_.last()->fileName());
    suggestedFileName = suggestedFileName.left(suggestedFileName.length()-4)+"-out.txt";
    saveText(suggestedFileName, sessions_.last()->plainText(false));
}

void Term::copyAll()
{
    QString allText;
    for (int i=0; i<sessions_.size(); i++) {
        allText += sessions_[i]->plainText(true);
    }
    QClipboard * clipboard = QApplication::clipboard();
    clipboard->setText(allText);
}

void Term::copyLast()
{
    QClipboard * clipboard = QApplication::clipboard();
    clipboard->setText(sessions_.last()->plainText(false));
}

void Term::saveText(const QString &suggestedFileName, const QString &text)
{
    QString fileName = QFileDialog::getSaveFileName(
                this,
                tr("Save output..."),
                suggestedFileName,
                tr("Text files (*.txt);;All files (*)"));
    if (!fileName.isEmpty()) {
        QFile f(fileName);
        if (f.open(QIODevice::WriteOnly)) {
            QTextStream ts(&f);
            ts.setCodec("UTF-8");
            ts.setGenerateByteOrderMark(true);
            ts << text;
            f.close();
        }
        else {
            QMessageBox::critical(this,
                                  tr("Can't save output"),
                                  tr("The file you selected can not be written"));
        }
    }
}

void Term::editLast()
{
    Q_ASSERT(!sessions_.isEmpty());
    const QString fileName = sessions_.last()->fileName();
    const QString suggestedFileName = fileName.isEmpty()
            ? QString()
            : QDir::current().absoluteFilePath(sessions_.last()->fileName()) + "out.txt";
    const QString plainText = sessions_.last()->plainText(false);
    emit openTextEditor(suggestedFileName, plainText);
}

} // namespace Terminal
