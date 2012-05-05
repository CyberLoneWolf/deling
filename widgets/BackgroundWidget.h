/****************************************************************************
 ** Deling Final Fantasy VIII Field Editor
 ** Copyright (C) 2009-2012 Arzel J�r�me <myst6re@gmail.com>
 **
 ** This program is free software: you can redistribute it and/or modify
 ** it under the terms of the GNU General Public License as published by
 ** the Free Software Foundation, either version 3 of the License, or
 ** (at your option) any later version.
 **
 ** This program is distributed in the hope that it will be useful,
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 ** GNU General Public License for more details.
 **
 ** You should have received a copy of the GNU General Public License
 ** along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ****************************************************************************/
#ifndef BACKGROUNDWIDGET_H
#define BACKGROUNDWIDGET_H

#include <QtGui>
#include "widgets/PageWidget.h"
#include "BGPreview.h"
#include "BGPreview2.h"
#include "FF8Image.h"
#include "files/BackgroundFile.h"

class BackgroundWidget : public PageWidget
{
	Q_OBJECT
public:
	BackgroundWidget(QWidget *parent=0);
	void clear();
	void setData(Field *field);
	void cleanData();
	void fill();
	inline QString tabName() const { return tr("D�cors"); }
private slots:
	void parameterChanged(int index);
	void enableState(QListWidgetItem *item);
	void enableLayer(QListWidgetItem *item);
//	void switchItem(QListWidgetItem *item);
private:
	void build();

	BackgroundFile *backgroundFile;
	BGPreview2 *image;
	QComboBox *parametersWidget;
	QListWidget *statesWidget, *layersWidget;
};

#endif // BACKGROUNDWIDGET_H
