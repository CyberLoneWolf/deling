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
#ifndef MISCWIDGET_H
#define MISCWIDGET_H

#include <QtGui>
#include "widgets/PageWidget.h"
#include "files/MiscFile.h"
#include "files/WalkmeshFile.h"

class MiscWidget : public PageWidget
{
	Q_OBJECT
public:
	MiscWidget(QWidget *parent=0);
	void build();
	void clear();
	void setReadOnly(bool readOnly);
	void fill();
	inline QString tabName() const { return tr("Divers"); }
private slots:
	void editName(const QString &);
	void editPmp(const QString &);
	void editPmd(const QString &);
	void editPvp(const QString &);
	void editCaVector(int value);
	void editCaPos(double value);
private:
	void editCaVector(int id, int id2, int value);
	void editCaPos(int id, int value);

	QLineEdit *nameEdit, *pmpEdit, *pmdEdit, *pvpEdit;
	QListWidget *gateList;
	QSpinBox *caVectorX1Edit, *caVectorX2Edit, *caVectorX3Edit, *caVectorY1Edit, *caVectorY2Edit, *caVectorY3Edit, *caVectorZ1Edit, *caVectorZ2Edit, *caVectorZ3Edit;
	QDoubleSpinBox *caSpaceXEdit, *caSpaceYEdit, *caSpaceZEdit;
};

#endif // MISCWIDGET_H