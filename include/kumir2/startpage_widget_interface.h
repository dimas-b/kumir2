#ifndef STARTPAGE_WIDGET_INTERFACE_H
#define STARTPAGE_WIDGET_INTERFACE_H

#include <QtPlugin>

class QWidget;
class QAction;
class QMenu;

namespace Shared
{

class StartpageWidgetInterface
{
public:
	virtual QWidget *startPageWidget() = 0;

	virtual QMenu *editMenuForStartPage()
	{
		return nullptr;
	}

	virtual QList<QAction *> startPageActions()
	{
		return QList<QAction *>();
	}

	virtual QString startPageTitle() const = 0;

	virtual void setStartPageTitleChangeHandler(
		const QObject *receiver,
		const char *method  /*(QString title, const QObject * sender)*/
	) = 0;

	virtual QString startPageTabStyle() const
	{
		return "";
	}
};

}

Q_DECLARE_INTERFACE(Shared::StartpageWidgetInterface, "kumir2.startpage")

#endif // STARTPAGE_WIDGET_INTERFACE_H
