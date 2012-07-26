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
#ifndef CAFILE_H
#define CAFILE_H

#include "files/File.h"

typedef struct {
	qint16 x, y, z;
} Vertex_s;

typedef struct {
	Vertex_s camera_axis[3];
	qint16 camera_axis2z;// copy (padding)
	qint32 camera_position[3];
	qint32 blank;
	quint16 camera_zoom;
	quint16 camera_zoom2;// copy (padding)
} CaStruct;

class CaFile : public File
{
public:
	CaFile();
	bool open(const QByteArray &ca);
	bool save(QByteArray &ca);
	int cameraCount() const;
	const CaStruct &camera(int camID) const;
	void setCamera(int camID, const CaStruct &cam);
	void insertCamera(int camID, const CaStruct &cam);
	void removeCamera(int camID);
private:
	QList<CaStruct> cameras;
};

#endif // CAFILE_H
