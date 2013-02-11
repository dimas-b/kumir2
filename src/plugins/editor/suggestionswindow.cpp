#include "suggestionswindow.h"
#include "ui_suggestionswindow.h"
#include "settingspage.h"

namespace Editor {

void desaturate(QImage & img) {
    for (int y=0; y<img.height(); y++) {
        for (int x=0; x<img.width(); x++) {
            const QColor source = QColor(img.pixel(x,y));
            QColor target;
            target.setHsv(source.hsvHue(), source.hsvSaturation()/2, source.value()/4*3);
            target.setAlpha(qAlpha(img.pixel(x,y)));
            img.setPixel(x,y,target.rgba());
        }
    }
}

SuggestionsWindow::SuggestionsWindow(QWidget *editorWidget) :
    QWidget(0, Qt::Popup),
    ui(new Ui::SuggestionsWindow)
{
    this->editorWidget = editorWidget;
    QPalette pal = palette();
    const QString bgColor = pal.brush(QPalette::Normal, QPalette::ToolTipBase).color().name();
    ui->setupUi(this);
    static const QString css = QString::fromAscii(""
            "QWidget {"
            "  background-color: %1;"
            "  border: 1px solid black;"
            "}"
            "QWidget#widget { border-left: 0; }"
            "QListWidget { border-bottom: 0; }"
            "QToolButton { border: 0; }"
            "")
            .arg(bgColor);
//    qDebug() << css;
    setStyleSheet(css);
    ui->alist->installEventFilter(this);
    connect(ui->alist, SIGNAL(currentRowChanged(int)),
            this, SLOT(handleCurrentItemChanged(int)));
    connect(ui->alist, SIGNAL(itemActivated(QListWidgetItem*)),
            this, SLOT(handleItemActivated(QListWidgetItem*)));
    connect(ui->btnAccept, SIGNAL(clicked()), this, SLOT(acceptItem()));
    connect(ui->btnClose, SIGNAL(clicked()), this, SLOT(hide()));
    setCursor(Qt::ArrowCursor);
    ui->alist->setCursor(Qt::PointingHandCursor);

    QPixmap pxOkActive = ui->btnAccept->icon().pixmap(ui->btnAccept->iconSize());
    QPixmap pxCloseActive = ui->btnClose->icon().pixmap(ui->btnClose->iconSize());

    QImage imgOk = pxOkActive.toImage();
    QImage imgClose = pxCloseActive.toImage();

    desaturate(imgOk);
    desaturate(imgClose);

    QPixmap pxOkNormal = QPixmap::fromImage(imgOk);
    QPixmap pxCloseNormal = QPixmap::fromImage(imgClose);

    QIcon ok, close;
    ok.addPixmap(pxOkActive, QIcon::Active);
    ok.addPixmap(pxOkNormal, QIcon::Normal);

    close.addPixmap(pxCloseActive, QIcon::Active);
    close.addPixmap(pxCloseNormal, QIcon::Normal);

    ui->btnAccept->setIcon(ok);
    ui->btnClose->setIcon(close);

}

void SuggestionsWindow::updateSettings(const QSettings *settings)
{
    createIcons(settings);
    QFont f;
    f.setFamily(settings->value(
                    SettingsPage::KeyFontName,
                    SettingsPage::defaultFontFamily()
                    ).toString());
    f.setPointSize(settings->value(
                       SettingsPage::KeyFontSize,
                       SettingsPage::defaultFontSize
                       ).toInt());
    ui->alist->setFont(f);
}

void SuggestionsWindow::createIcons(const QSettings *settings)
{
    QPoint points[4] = { QPoint(4,0), QPoint(0,4), QPoint(4,8), QPoint(8,4) };
    QImage local(8,8,QImage::Format_ARGB32);
    local.fill(0);
    QPainter pl(&local);
    pl.setRenderHint(QPainter::Antialiasing, true);
    pl.setPen(QPen(QColor("black")));
    pl.setBrush(QColor(settings->value(
                           SettingsPage::KeyColorType,
                           SettingsPage::DefaultColorType
                           ).toString()));
    pl.drawEllipse(QRect(0,0,8,8));

    icon_local = QIcon(QPixmap::fromImage(local));

    QImage global(8,8,QImage::Format_ARGB32);
    global.fill(0);
    QPainter pg(&global);
    pg.setRenderHint(QPainter::Antialiasing, true);
    pg.setPen(QPen(QColor("black")));
    pg.setBrush(QColor(settings->value(
                           SettingsPage::KeyColorType,
                           SettingsPage::DefaultColorType
                           ).toString()));
    pg.drawPolygon(points, 4);

    icon_global = QIcon(QPixmap::fromImage(global));

    QImage algorithm(8,8,QImage::Format_ARGB32);
    algorithm.fill(0);
    QPainter pa(&algorithm);
    pa.setRenderHint(QPainter::Antialiasing, true);
    pa.setPen(QPen(QColor("black")));
    pa.setBrush(QColor(settings->value(
                           SettingsPage::KeyColorAlg,
                           SettingsPage::DefaultColorAlg
                           ).toString()));
    pa.drawPolygon(points, 4);

    icon_algorithm = QIcon(QPixmap::fromImage(algorithm));

    QImage module(8,8,QImage::Format_ARGB32);
    module.fill(0);
    QPainter pm(&module);
    pm.setRenderHint(QPainter::Antialiasing, true);
    pm.setPen(QPen(QColor("black")));
    pm.setBrush(QColor(settings->value(
                           SettingsPage::KeyColorMod,
                           SettingsPage::DefaultColorMod
                           ).toString()));
    pm.drawRect(QRect(1,1,6,6));

    icon_module = QIcon(QPixmap::fromImage(module));

    QImage kumfile(8,8,QImage::Format_ARGB32);
    kumfile.fill(0);
    QPainter pf(&kumfile);
    pf.setRenderHint(QPainter::Antialiasing, true);
    pf.setPen(QPen(QColor("black")));
    pf.setBrush(QColor(settings->value(
                           SettingsPage::KeyColorLiteral,
                           SettingsPage::DefaultColorLiteral
                           ).toString()));
    pf.drawRect(QRect(1,1,6,6));

    icon_kumfile = QIcon(QPixmap::fromImage(kumfile));

    QImage keyword(8,8,QImage::Format_ARGB32);
    keyword.fill(0);
    QPainter pk(&keyword);
    pk.setRenderHint(QPainter::Antialiasing, true);
    pk.setPen(QPen(QColor("black")));
    pk.setBrush(QColor(settings->value(
                           SettingsPage::KeyColorKw,
                           SettingsPage::DefaultColorKw
                           ).toString()));
    pk.drawRect(QRect(1,1,6,6));

    icon_keyword = QIcon(QPixmap::fromImage(keyword));

    QImage other(8,8,QImage::Format_ARGB32);
    other.fill(0);
    icon_other = QIcon(QPixmap::fromImage(other));

}

void SuggestionsWindow::handleCurrentItemChanged(int currentRow)
{
    if (currentRow>=0 && currentRow<l_suggestions.size()) {
        ui->descriptionView->setText(l_suggestions.at(currentRow).description);
    }
    else {
        ui->descriptionView->clear();
    }
}

void SuggestionsWindow::handleItemActivated(QListWidgetItem *)
{
    acceptItem();
}

void SuggestionsWindow::acceptItem()
{
    int row = ui->alist->currentRow();
    if (row>=0 && row<ui->alist->count()) {
        const QString value = l_suggestions.at(row).value;
        emit acceptedSuggestion(value);
    }
    hide();
}

void SuggestionsWindow::init(
        const QString &,
        const QList<Shared::Suggestion> &suggestions)
{
    b_keyPressed = false;
    ui->alist->clear();
    ui->descriptionView->clear();
    int prefWidth = 100;
    const QFontMetrics fm (ui->alist->font());
    int prefHeight = fm.height()*(5+suggestions.size());
    for (int index = 0; index<suggestions.size(); index++) {
        const Shared::Suggestion & s = suggestions.at(index);
        QListWidgetItem * item = new QListWidgetItem(ui->alist);
        item->setText(s.value);
        prefWidth = qMax(prefWidth, 100+fm.width(s.value));
        if (s.kind==Shared::Suggestion::Kind::Local) {
            item->setIcon(icon_local);
        }
        else if (s.kind==Shared::Suggestion::Kind::Global) {
            item->setIcon(icon_global);
        }
        else if (s.kind==Shared::Suggestion::Kind::Algorithm) {
            item->setIcon(icon_algorithm);
        }
        else if (s.kind==Shared::Suggestion::Kind::BuiltinModule) {
            item->setIcon(icon_module);
        }
        else if (s.kind==Shared::Suggestion::Kind::KumirModule) {
            item->setIcon(icon_kumfile);
        }
        else if (s.kind==Shared::Suggestion::Kind::SecondaryKeyword) {
            item->setIcon(icon_keyword);
        }
        else {
            item->setIcon(icon_other);
        }
    }
    int width = qMax(qMin(400, prefWidth), 150);
    int height = qMin(prefHeight, 400);
    setFixedWidth(width);
    setFixedHeight(height);
    if (l_suggestions.size()==0) {
        ui->descriptionView->setText(tr("No suggestions"));
    }
    l_suggestions = suggestions;
}

void SuggestionsWindow::focusInEvent(QFocusEvent *event)
{
    ui->alist->setFocus(event->reason());
    event->accept();
    if (ui->alist->count()>0) {
        ui->alist->setCurrentRow(0);
    }
}

bool SuggestionsWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (obj==ui->alist && event->type()==QEvent::KeyPress) {
        QKeyEvent * e = static_cast<QKeyEvent*>(event);
        if (e->key()==Qt::Key_Tab || e->key()==Qt::Key_Escape) {
            hide();
            return true;
        }
    }
    return false;
}

void SuggestionsWindow::keyPressEvent(QKeyEvent *event)
{
    b_keyPressed = true;
    if (event->key()==Qt::Key_Tab || event->key()==Qt::Key_Escape) {
        hide();
        event->accept();
    }
    else if (event->key()==Qt::Key_Enter || event->key()==Qt::Key_Return) {
        hide();
        event->accept();
    }
    else {
        event->ignore();
    }
}

void SuggestionsWindow::keyReleaseEvent(QKeyEvent *event)
{
    if (b_keyPressed || event->key()==Qt::Key_Escape) {
        b_keyPressed = false;
        if (event->key()==Qt::Key_Tab || event->key()==Qt::Key_Escape) {
            hide();
            event->accept();
        }
        else if (event->key()==Qt::Key_Enter || event->key()==Qt::Key_Return) {
            hide();
            event->accept();
        }
    }
    else {
        event->ignore();
    }
}

void SuggestionsWindow::hideEvent(QHideEvent *event)
{
    Q_CHECK_PTR(editorWidget);
    b_keyPressed = false;
    QWidget::hideEvent(event);
    editorWidget->setFocus();
    emit hidden();
}

SuggestionsWindow::~SuggestionsWindow()
{
    delete ui;
}

} // namespace Editor
