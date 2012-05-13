#ifndef TDWWIDGET_H
#define TDWWIDGET_H

#include <QtGui>
#include "widgets/PageWidget.h"
#include "widgets/TdwGrid.h"

class TdwWidget : public PageWidget
{
public:
	TdwWidget(QWidget *parent=0);
	void clear();
	void fill();
	inline QString tabName() const { return tr("Caract�res additionnels"); }
private:
	void build();
	TdwGrid *tdwGrid;
};

#endif // TDWWIDGET_H
