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
#ifndef FF8IMAGE_H
#define FF8IMAGE_H

#include <QtGui>
#include "LZS.h"

#define COEFF_COLOR	8.2258064516129032258064516129032 // 255/31

class FF8Image
{
public:
	static QPixmap lzs(const QByteArray &data);
	static QPixmap tex(const QByteArray &data, int palID=0);
	static QPixmap tim(const QByteArray &data, int palID=0);

	static int findFirstTim(const QByteArray &data);
	static int findTims(const QByteArray &data);

	static QPixmap errorPixmap();
};

#endif // FF8IMAGE_H
