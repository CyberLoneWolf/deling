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
#include "files/JsmFile.h"

QStringList JsmFile::opcodeNameCalc;
QStringList JsmFile::opcodeName;
QString JsmFile::lastError = QString();

JsmFile::JsmFile()
	: _hasSym(false), modified(false), needUpdate(true), groupItem(0)
{
	if(opcodeNameCalc.isEmpty()) {
		for(int i=0 ; i<15 ; ++i)
			opcodeNameCalc.append(JsmOpcodeCal::cal_table[i]);

		for(int i=0 ; i<376 ; ++i)
			opcodeName.append(JsmOpcode::opcodes[i]);
	}
}

JsmFile::~JsmFile()
{	
}

QString JsmFile::printCount()
{
	return QString("count0 = %1, count1 = %2, count2 = %3, count3 = %4").arg(scripts.countDoors()).arg(scripts.countLines()).arg(scripts.countBackgrounds()).arg(scripts.countOthers());
}

bool JsmFile::open(QString path)
{
	QFile jsm_file(path);
	if(!jsm_file.open(QIODevice::ReadOnly)) {
		lastError = jsm_file.errorString();
		return false;
	}
	//	this->path = jsm_file.fileName();
	path.chop(3);
	if(QFile::exists(path%"sym")) {
		QFile sym_file(path%"sym");
		if(!sym_file.open(QIODevice::ReadOnly)) {
			lastError = sym_file.errorString();
			return false;
		}
		return open(jsm_file.readAll(), sym_file.readAll());
	}

	return open(jsm_file.readAll());
}

bool JsmFile::open(const QByteArray &jsm, const QByteArray &sym_data)
{
	const char *jsm_data = jsm.constData();
	int jsm_data_size = jsm.size();
	QMultiMap<quint16, JsmGroup> linkGroupList;
	quint16 group, label, pos;
	int oldPos=-1;
	quint8 count;
	JsmHeader jsm_header;

	if(jsm_data_size<8) {
		qWarning() << "JsmFile::open error 1" << jsm_data_size;
		return false;
	}

	memcpy(&jsm_header, jsm_data, 8);

	int groupCount = (jsm_header.section1 - 8)/2;
	int scriptCount = (jsm_header.section2-jsm_header.section1)/2;

	if(jsm_data_size <= jsm_header.section1 || jsm_data_size <= jsm_header.section2) {
		qWarning() << "JsmFile::open error 2" << jsm_data_size << jsm_header.section1 << jsm_header.section2;
		return false;
	}

	if(groupCount != jsm_header.count0 + jsm_header.count1 + jsm_header.count2 + jsm_header.count3) {
		qWarning() << "JsmFile::open Nombre de groupes invalide !" << groupCount << jsm_header.count0 << jsm_header.count1 << jsm_header.count2 << jsm_header.count3;
		return false;
	}

	for(int i=0 ; i<groupCount ; ++i) {
		memcpy(&group, &jsm_data[8+i*2], 2);
		label = group >> 7;
		count = group & 0x7F;

		if(label+count+1 >= scriptCount) {
			qWarning() << "JsmFile::open error 3" << label << count << scriptCount;
			return false;
		}

		linkGroupList.insert(label, JsmGroup(i, label, count));
	}
	QList<JsmGroup> groupList = linkGroupList.values();
	QList<JsmScript> scriptList;

	// verification no-paddings between groups
	for(int i=0 ; i<groupCount-1 ; ++i) {
		const JsmGroup &grp = groupList.at(i);
		if(grp.label() + grp.scriptCount() > groupList.at(i+1).label()) {
			qWarning() << "JsmFile::open incorrect group pos+size" << grp.label() << grp.scriptCount() << groupList.at(i+1).label();
			return false;
		}
	}

	section1_padding = 0;
	for(int i=0 ; i<scriptCount ; ++i) {
		memcpy(&pos, &jsm_data[jsm_header.section1+i*2], 2);
		if(i!=0 && pos==0) {
			section1_padding = jsm_header.section2 - jsm_header.section1 - i*2;
			break;
		}
		bool flag = pos >> 15;
		pos &= 0x7FFF;
		if((jsm_header.section2 + pos*4) > jsm_data_size || (int)pos < oldPos) {
			qWarning() << "JsmFile::open error 4" << (jsm_header.section2 + pos*4) << jsm_data_size;
			return false;
		}
		oldPos = pos;

		scriptList.append(JsmScript(pos, flag));
	}

	scripts = JsmScripts(groupList, scriptList,
						 QByteArray((char *)&jsm_data[jsm_header.section2], jsm_data_size-jsm_header.section2),
						 jsm_header.count0, jsm_header.count1, jsm_header.count2, jsm_header.count3);

	searchWindows();
	searchMapID();
	searchGroupTypes();

	_hasSym = openSym(sym_data);

	return true;
}

bool JsmFile::openSym(const QByteArray &sym_data)
{
	QStringList sym_strings;
	int nbScripts, nbGroup, nbLines, nbEntities = scripts.nbEntitiesForSym();

	sym_strings = QString(sym_data).split('\n', QString::SkipEmptyParts);
	nbLines = sym_strings.size();

	if(nbLines<=0) return false;

	nbScripts = scripts.nbScript()-1;
	nbGroup = scripts.nbGroup();

	if(nbScripts>0 && nbLines==nbEntities+nbScripts) {
		QString line;
		QRegExp validName("^[\\w]+$");
		int groupID=-1, methodID=1, index;

		for(int i=0 ; i<nbScripts ; ++i) {
			if(groupID >= nbGroup) {
				qWarning() << "JsmFile::openSym trop de groupes" << groupID;
				break;
			}

			line = sym_strings.at(nbEntities+i).simplified();
			if((index = line.indexOf("::")) == -1) {
				if(validName.exactMatch(line)) {
					scripts.setGroupName(groupID+1, line);
				} else {
					qWarning() << "JsmFile::openSym erreur nom groupe invalide" << line;
				}

				++groupID;
				methodID = 1;
			}
			else if(validName.exactMatch(line.mid(index+2))) {
				if(groupID == -1) {
					qWarning() << "JsmFile::openSym constructeur ind�fini" << line;
					break;
				}

				if(!line.startsWith(scripts.group(groupID).name())) {
					qWarning() << "JsmFile::openSym la m�thode ne commence pas par le nom du groupe" << line << scripts.group(groupID).name();
					break;
				}

				if(methodID < scripts.nbScript(groupID)) {
					scripts.setScriptName(groupID, methodID, line.mid(index+2));
				}
				else {
					qWarning() << "JsmFile::openSym erreur 2" << methodID << (scripts.nbScript(groupID));
					break;
				}
				++methodID;
//				scriptList.at(i)->setName(line.mid(index+2));
			}
			else {
				qWarning() << "JsmFile::openSym erreur nom script invalide" << line;
				++methodID;
			}
		}
	}
	else {
		qWarning() << "JsmFile::openSym erreur 3" << nbLines << (nbEntities+nbScripts);
		return false;
	}

	return true;
}

bool JsmFile::save(const QString &path)
{
	QFile jsm_file(path);
	if(!jsm_file.open(QIODevice::WriteOnly)) {
		lastError = jsm_file.errorString();
		return false;
	}
	QByteArray sym;
	jsm_file.write(save(sym));
	jsm_file.close();

	if(!sym.isEmpty()) {
		QFile sym_file(path.left(path.lastIndexOf('.')) + ".sym");
		if(!sym_file.open(QIODevice::WriteOnly)) {
			lastError = sym_file.errorString();
			return false;
		}
		sym_file.write(sym);
		sym_file.close();
	}

	return true;
}

QByteArray JsmFile::save(QByteArray &sym)
{
	QByteArray ret;
	QByteArray section0(scripts.nbGroup()*2, '\x00'), section1;
	quint16 data;
	JsmHeader jsm_header;
	quint8 nbDoors=0, nbLines=0, nbBackground=0, nbOther=0;

	foreach(const JsmGroup &group, scripts.getGroupList()) {
		switch(group.type()) {
		case JsmGroup::Door:
			nbDoors++;
			break;
		case JsmGroup::Location:
			nbLines++;
			break;
		case JsmGroup::Background:
			nbBackground++;
			break;
		default:
			nbOther++;
			break;
		}

		data = (group.label() << 7) | ((group.scriptCount()-1) & 0x7F);
		section0.replace(group.execOrder()*2, 2, (char *)&data, 2);
	}

	if(nbDoors != scripts.countDoors()) {
		qWarning() << "Count Doors different" << nbDoors << scripts.countDoors();
	}

	if(nbLines != scripts.countLines()) {
		qWarning() << "Count Lines different" << nbLines << scripts.countLines();
	}

	if(nbBackground != scripts.countBackgrounds()) {
		qWarning() << "Count Background different" << nbBackground << scripts.countBackgrounds();
	}

	if(nbOther != scripts.countOthers()) {
		qWarning() << "Count Other different" << nbOther << scripts.countOthers();
	}

	foreach(const JsmScript &script, scripts.getScriptList()) {
		data = (script.flag() << 15) | (script.pos() & 0x7FFF);
		section1.append((char *)&data, 2);
	}
	section1.append(QByteArray(section1_padding, '\x00'));

	jsm_header.count0 = scripts.countDoors();
	jsm_header.count1 = scripts.countLines();
	jsm_header.count2 = scripts.countBackgrounds();
	jsm_header.count3 = scripts.countOthers();
	jsm_header.section1 = 8 + section0.size();
	jsm_header.section2 = jsm_header.section1 + section1.size();

	ret.append((char *)&jsm_header, 8);
	ret.append(section0);
	ret.append(section1);
	ret.append(scripts.data().constData());// Section 2

//	QFile debug("debug.jsm");
//	debug.open(QIODevice::WriteOnly);
//	debug.write(ret);
//	debug.close();

	sym = saveSym().toLatin1();

	modified = false;

	return ret;
}

QString JsmFile::saveSym()
{
	QString ret;

	foreach(const JsmGroup &group, scripts.getGroupList()) {
		if(group.name().isEmpty())	break;

		if(group.type() != JsmGroup::Door) {
			ret.append(group.name().leftJustified(31));
			ret.append('\n');
		}
	}

	if(ret.isEmpty()) {
		qWarning() << "noms manquants";
		return QString();
	}

	int nbGroups = scripts.nbGroup();
	for(int groupID=0 ; groupID<nbGroups ; ++groupID) {
		int nbScripts = scripts.nbScript(groupID);
		const QString &groupName = scripts.group(groupID).name();
		for(int scriptID=0 ; scriptID<nbScripts ; ++scriptID) {
			QString name;
			if(scriptID!=0)		name = groupName + "::" + scripts.script(groupID, scriptID).name();
			else				name = groupName;// constructor

			ret.append(name.leftJustified(31));
			ret.append('\n');
		}
	}

//	QFile debug("debug.sym");
//	debug.open(QIODevice::WriteOnly);
//	debug.write(ret.toLatin1());
//	debug.close();

	modified = false;

	return ret;
}

void JsmFile::searchWindows()
{
//	qDebug() << "JsmFile::searchWindows";
	int nbOpcode = scripts.data().nbOpcode();
	quint32 key;
	FF8Window window;
	window.ask_last = window.ask_first = window.ask_last2 = window.ask_first2 = 0;
	ff8Windows.clear();

	for(int i=4 ; i<nbOpcode ; ++i) {
		JsmOpcode op = scripts.opcode(i);
		key = op.key();

		switch(key) {
		case JsmOpcode::AMESW:
		case JsmOpcode::AMES:
		case JsmOpcode::RAMESW:
		{
			JsmOpcode psh1=scripts.opcode(i-4);
			JsmOpcode psh2=scripts.opcode(i-3);
			JsmOpcode psh3=scripts.opcode(i-2);
			JsmOpcode psh4=scripts.opcode(i-1);
			if(psh1.key() == JsmOpcode::PSHN_L
					&& psh2.key() == JsmOpcode::PSHN_L
					&& psh3.key() == JsmOpcode::PSHN_L
					&& psh4.key() == JsmOpcode::PSHN_L) {
				window.type = key;
				window.x = psh3.param();
				window.y = psh4.param();
				window.u1 = psh1.param();
				window.script_pos = i;
				ff8Windows.insert(psh2.param(), window);
			}
			break;
		}
		case JsmOpcode::AASK:
			if(i>=8) {
				JsmOpcode psh1=scripts.opcode(i-8);
				JsmOpcode psh2=scripts.opcode(i-7);
				JsmOpcode psh3=scripts.opcode(i-6);
				JsmOpcode psh4=scripts.opcode(i-5);
				JsmOpcode psh5=scripts.opcode(i-4);
				JsmOpcode psh6=scripts.opcode(i-3);
				JsmOpcode psh7=scripts.opcode(i-2);
				JsmOpcode psh8=scripts.opcode(i-1);
				if(i>=8
						&& psh1.key() == JsmOpcode::PSHN_L
						&& psh2.key() == JsmOpcode::PSHN_L
						&& psh3.key() == JsmOpcode::PSHN_L
						&& psh4.key() == JsmOpcode::PSHN_L
						&& psh5.key() == JsmOpcode::PSHN_L
						&& psh6.key() == JsmOpcode::PSHN_L
						&& psh7.key() == JsmOpcode::PSHN_L
						&& psh8.key() == JsmOpcode::PSHN_L) {
					window.type = key;
					window.x = psh7.param();
					window.y = psh8.param();
					window.ask_first = psh3.param();
					window.ask_last = psh4.param();
					window.ask_first2 = psh5.param();
					window.ask_last2 = psh6.param();
					window.u1 = psh1.param();
					window.script_pos = i;
					ff8Windows.insert(psh2.param(), window);
				}
			}
			break;
		}
	}
}

QList<qint32> JsmFile::searchJumps(int position, int nbOpcode) const
{
	quint32 key;
	qint32 param;
	QList<qint32> labels;

	for(int i=0 ; i<nbOpcode ; ++i)
	{
		JsmOpcode op = scripts.opcode(position+i);
		key = op.key();
		param = op.param();

		if(key>1 && key<5 && i + param >= 0 && i + param < nbOpcode) {
			labels.append(i + param);
		}
	}
	qSort(labels);

	return labels;
}

void JsmFile::searchMapID()
{
//	qDebug() << "JsmFile::searchMapID";
	int nbOpcode = scripts.data().nbOpcode();

	for(int i=1 ; i<nbOpcode ; ++i) {
		switch(scripts.key(i)) {
		case JsmOpcode::SETPLACE:
			if(scripts.key(i-1) == JsmOpcode::PSHN_L)
				_mapID = scripts.param(i-1);
			return;
		}
	}

	_mapID = -1;
}

void JsmFile::searchGroupTypes()
{
//	qDebug() << "JsmFile::searchGroupTypes";
	unsigned int key;
	int param;
	int nbGroup=scripts.nbGroup(), nbOpcode, methodCount, pos, character, model_id;
	bool main_type, location_type, door_type, bg_type;

	for(int groupID=0; groupID < nbGroup ; ++groupID) {
		const JsmGroup &jsmGroup = scripts.group(groupID);
		methodCount = (int)jsmGroup.scriptCount();
		main_type = location_type = door_type = bg_type = false;
		character = model_id = -1;

		for(int methodID=0 ; methodID < methodCount && !location_type && !door_type && !bg_type ; ++methodID) {
			pos = scripts.posScript(groupID, methodID, &nbOpcode);

			for(int opcodeID=0 ; opcodeID < nbOpcode && !location_type && !door_type && !bg_type ; ++opcodeID) {
				JsmOpcode op = scripts.opcode(pos + opcodeID);
				key = op.key();
				param = op.param();

				switch(key) {
				case JsmOpcode::SETMODEL:
					model_id = param;
					break;
				case JsmOpcode::SETPC:
					if(opcodeID>0 && scripts.key(pos + opcodeID - 1) == JsmOpcode::PSHN_L) {
						param = scripts.param(pos + opcodeID - 1);
						// if setpc has multiple definition
						if(character!=-1 && param!=character) {
							character = UNKNOWN_CHARACTER;
						}
						else {
							character = param;
						}
					}
					break;
				case JsmOpcode::SETDRAWPOINT:
					character = DRAWPOINT_CHARACTER;
					break;
				case JsmOpcode::FADEIN:
				case JsmOpcode::SETPLACE:
					main_type = true;
					break;
				case JsmOpcode::SETLINE:
					location_type = true;
					break;
				case JsmOpcode::DOORLINEON:
				case JsmOpcode::DOORLINEOFF:
					door_type = true;
					break;
				case JsmOpcode::BGDRAW:
				case JsmOpcode::BGANIMESPEED:
				case JsmOpcode::BGANIME:
				case JsmOpcode::BGANIMESYNC:
				case JsmOpcode::BGCLEAR:
				case JsmOpcode::BGOFF:
				case JsmOpcode::RBGANIME:
				case JsmOpcode::RBGANIMELOOP:
				case JsmOpcode::RBGSHADELOOP:
					bg_type = true;
					break;
				}
			}
		}

		if(character != -1) {
			scripts.setGroupCharacter(groupID, character);
		}
		if(model_id != -1) {
			scripts.setGroupType(groupID, JsmGroup::Model);
			scripts.setGroupModelId(groupID, model_id);
		}
		else if(location_type) {
			scripts.setGroupType(groupID, JsmGroup::Location);
		}
		else if(main_type) {
			scripts.setGroupType(groupID, JsmGroup::Main);
		}
		else if(door_type) {
			scripts.setGroupType(groupID, JsmGroup::Door);
		}
		else if(bg_type) {
			scripts.setGroupType(groupID, JsmGroup::Background);
		}
	}
}

int JsmFile::mapID() const
{
	return _mapID;
}

int JsmFile::nbWindows(quint8 textID) const
{
	return ff8Windows.count(textID);
}

QList<FF8Window> JsmFile::windows(quint8 textID) const
{
	return ff8Windows.values(textID);
}

void JsmFile::setWindow(quint8 textID, int winID, const FF8Window &window)
{
//	qDebug() << "JsmFile::setWindow" << window.x << window.y;
	quint32 key;

	if(window.script_pos >= scripts.data().nbOpcode()) {
		qWarning() << "JsmFile::setWindow error 1" << window.script_pos << scripts.data().nbOpcode() << textID << winID;
		return;
	}

	key = scripts.key(window.script_pos);
	if(window.type != key) {
		qWarning() << "JsmFile::setWindow error 2" << window.type << key << textID << winID;
		return;
	}

	switch(window.type) {
	case JsmOpcode::AMESW:
	case JsmOpcode::AMES:
	case JsmOpcode::RAMESW:
		scripts.setParam(window.script_pos-4, window.u1);
		scripts.setParam(window.script_pos-3, textID);
		scripts.setParam(window.script_pos-2, window.x);
		scripts.setParam(window.script_pos-1, window.y);
		break;
	case JsmOpcode::AASK:
		scripts.setParam(window.script_pos-8, window.u1);
		scripts.setParam(window.script_pos-7, textID);
		scripts.setParam(window.script_pos-6, window.ask_first);
		scripts.setParam(window.script_pos-5, window.ask_last);
		scripts.setParam(window.script_pos-4, window.ask_first2);
		scripts.setParam(window.script_pos-3, window.ask_last2);
		scripts.setParam(window.script_pos-2, window.x);
		scripts.setParam(window.script_pos-1, window.y);
		break;
	default:
		qWarning() << "JsmFile::setWindow error 3" << window.type << textID << winID;
		return;
	}

	needUpdate = modified = true;

	searchWindows();
}

QString JsmFile::toString(int groupID, int methodID)
{
	if(!scripts.script(groupID, methodID).decompiledScript().isEmpty() && !needUpdate) {
		return scripts.script(groupID, methodID).decompiledScript();
	}
	int position, nbOpcode;
	position = scripts.posScript(groupID, methodID, &nbOpcode);
	needUpdate = false;
	return _toString(position, nbOpcode);
}

QString JsmFile::_toString(int position, int nbOpcode) const
{
	QString ret;
	int lbl;
	unsigned int key;
	int param;
	QList<qint32> labels = searchJumps(position, nbOpcode);

	for(int i=0 ; i<nbOpcode ; ++i) {
		JsmOpcode *op = scripts.opcodep(position + i);
		key = op->key();

		if((lbl = labels.indexOf(i)) != -1) {
			ret.append(QString("LABEL%1\n").arg(lbl));
		}

		if(!op->hasParam())
			ret.append(op->name());
		else {
			param = op->param();
			ret.append(QString("%1 ").arg(op->name(), -11));

			if(key>=JsmOpcode::JMP && key<=JsmOpcode::GJMP) {
				if((lbl = labels.indexOf(i + param)) != -1) {
					ret.append(QString("LABEL%1").arg(lbl));
				} else {
					ret.append(QString("%1").arg(param));
				}
			} else {
				ret.append(op->paramStr());
			}
		}

		delete op;

		ret.append("\n");
	}

	return ret;
}

int JsmFile::opcodePositionInText(int groupID, int methodID, int opcodeID) const
{
	int line=opcodeID, nbOpcode, position=scripts.posScript(groupID, methodID, &nbOpcode);
	QList<qint32> labels = searchJumps(position, nbOpcode);

	for(int i=0 ; i<=opcodeID ; ++i) {
		if(labels.contains(i))	++line;
	}

	return line;
}

bool JsmFile::compileAll(int &errorGroupID, int &errorMethodID, int &errorLine, QString &errorStr)
{
	errorLine = 0;

	for(errorGroupID=0 ; errorGroupID < scripts.nbGroup() ; ++errorGroupID) {
		const JsmGroup &group = scripts.group(errorGroupID);
		for(errorMethodID=0 ; errorMethodID < group.scriptCount() ; ++errorMethodID) {
			const JsmScript &script = scripts.script(errorGroupID, errorMethodID);
			if(!script.decompiledScript().isEmpty()) {
				qDebug() << "compile" << group.name() << script.name();
				if(0 != (errorLine = fromString(errorGroupID, errorMethodID, script.decompiledScript(), errorStr))) {
					return false;
				}
			}
		}
	}

	return true;
}

int JsmFile::fromString(int groupID, int methodID, const QString &text, QString &errorStr)
{
//	qDebug() << "JsmFile::fromString";
	QRegExp endLine("(\\r\\n|\\n|\\r)");
	QRegExp spaces("[\\t ]+");
	QStringList lines = text.split(endLine);
	JsmData res;
	int key, param, posLbl, lbl;
	bool ok;
	QMap<int, int> labelsPos;
	QMultiMap<int, int> gotosPos;

	errorStr = "";

	int l=1;
	foreach(const QString &line, lines) {
		QStringList rows = line.split(spaces, QString::SkipEmptyParts);
		int rowsSize = rows.size();
		if(rowsSize < 1) {
			++l;
			continue;
		}

		if(rowsSize > 2) {
			errorStr = QObject::tr("Trop d'arguments");
			return l;
		}

		const QString &first = rows.first();

		if(first.startsWith("LABEL", Qt::CaseInsensitive)) {
			lbl = first.mid(5).toInt(&ok);
			if(!ok) {
				errorStr = QObject::tr("Conversion en entier impossible apr�s 'LABEL' : %1").arg(first.mid(5));
				return l;
			}
			if(labelsPos.contains(lbl)) {
				errorStr = QObject::tr("'LABEL%1' d�j� d�clar� pr�c�demment.").arg(lbl);
				return l;
			}
			labelsPos.insert(lbl, res.nbOpcode());
			++l;
			continue;
		}

		if((key = opcodeName.indexOf(first.toUpper())) == -1) {
			errorStr = QObject::tr("Opcode non reconnu : %1").arg(first);
			return l;
		}

		if(rowsSize == 2) {
			const QString &second = rows.at(1);

			if(key > 0x38) {
				errorStr = QObject::tr("Cet opcode ne peut pas avoir de param�tre : %1").arg(first);
				return l;
			}

			if(key == JsmOpcode::CAL) {
				if((param = opcodeNameCalc.indexOf(second.toUpper())) == -1) {
					errorStr = QObject::tr("Op�ration non reconnue : %1").arg(second);
					return l;
				}
			} else if(key>=JsmOpcode::JMP && key<=JsmOpcode::GJMP) {
				if(second.startsWith("LABEL", Qt::CaseInsensitive)) {
					lbl = second.mid(5).toInt(&ok);
					if(!ok) {
						errorStr = QObject::tr("Conversion en entier impossible apr�s 'LABEL' : %1").arg(second.mid(5));
						return l;
					}
					if((posLbl = labelsPos.value(lbl, -1)) != -1) {
						param = posLbl - res.nbOpcode();
//						qDebug() << "jump - LABEL" << lbl << "param" << param << "posLbl" << posLbl << "posCurrent" << res.nbOpcode();
					} else {
						param = -1;
						gotosPos.insert(lbl, res.nbOpcode());
					}
				} else {
					param = second.toInt(&ok);
					if(!ok) {
						errorStr = QObject::tr("Conversion en entier impossible : %1").arg(second);
						return l;
					}
				}
			} else {
				param = second.toInt(&ok);
				if(!ok) {
					errorStr = QObject::tr("Conversion en entier impossible : %1").arg(second);
					return l;
				}
			}

			res.append(JsmOpcode(key, param));
		} else {
			res.append(JsmOpcode(key));
		}

		++l;
	}

	QMapIterator<int, int> it(gotosPos);
	while(it.hasNext()) {
		it.next();
		lbl = it.key();
		if((posLbl = labelsPos.value(lbl, -1)) != -1) {
			param = posLbl - it.value();
//			qDebug() << "jump - LABEL" << lbl << "param" << param << "posLbl" << posLbl << "posCurrent" << it.value();
			JsmOpcode op = res.opcode(it.value());
			op.setParam(param);
			res.setOpcode(it.value(), op);
		} else {
			errorStr = QObject::tr("'LABEL%1' ind�fini.").arg(lbl);
			return -1;
		}
	}

//	int position, nbOpcode;
//	position = scripts.posScript(groupID, methodID, &nbOpcode);
//	if(res != scripts.data().mid(position, nbOpcode)) {
//		qDebug() << scripts.data().mid(position, nbOpcode).constData().toHex();
//		qDebug() << res.constData().toHex();
//		qDebug() << nbOpcode << res.nbOpcode();
//	} else {
//		qDebug() << "ok";
//	}

	scripts.replaceScript(groupID, methodID, res);

	modified = true;

	return 0;
}

const JsmScripts &JsmFile::getScripts() const
{
	return scripts;
}

bool JsmFile::search(int type, quint64 value, int &groupID, int &methodID, int &opcodeID) const
{
	int groupListSize = scripts.nbGroup(), nbOpcode, methodCount, pos;

	if(groupID < 0)		groupID = 0;
	if(methodID < 0)	methodID = 0;
	if(opcodeID < 0)	opcodeID = 0;

	while(groupID < groupListSize) {
		methodCount = scripts.nbScript(groupID);

		while(methodID < methodCount) {
			pos = scripts.posScript(groupID, methodID, &nbOpcode);

			while(opcodeID < nbOpcode) {
				if(search(type, value, pos, opcodeID))	return true;

				++opcodeID;
			}
			++methodID;
			opcodeID = 0;
		}
		++groupID;
		methodID = 0;
	}

	return false;
}

bool JsmFile::searchReverse(int type, quint64 value, int &groupID, int &methodID, int &opcodeID) const
{
	int nbOpcode;
	quint16 pos;

	if(groupID < 0 || groupID >= scripts.nbGroup())
		groupID = scripts.nbGroup() - 1;

	while(groupID >= 0) {
		if(methodID < 0 || methodID >= scripts.nbScript(groupID))
			methodID = scripts.nbScript(groupID) - 1;

		while(methodID >= 0) {
			pos = scripts.posScript(groupID, methodID, &nbOpcode);
			if(opcodeID < 0 || opcodeID >= nbOpcode)
				opcodeID = nbOpcode - 1;

			while(opcodeID >= 0) {
				if(search(type, value, pos, opcodeID))	return true;

				--opcodeID;
			}
			--methodID;
		}
		--groupID;
	}

	return false;
}

bool JsmFile::search(int type, quint64 value, quint16 pos, int opcodeID) const
{
	JsmOpcode op = scripts.opcode(pos + opcodeID);
	quint32 key = op.key();
	qint32 param = op.param();

	switch(type) {
	case 0:// Text
		switch(key) {
		case JsmOpcode::AMESW:
		case JsmOpcode::AMES:
		case JsmOpcode::RAMESW:
			return opcodeID>=3 && scripts.key(pos + opcodeID - 3) == JsmOpcode::PSHN_L
					&& value == (quint32)scripts.param(pos + opcodeID - 3);
		case JsmOpcode::MES:
			return opcodeID>=1 && scripts.key(pos + opcodeID - 1) == JsmOpcode::PSHN_L
					&& value == (quint32)scripts.param(pos + opcodeID - 1);
		case JsmOpcode::AASK:
			return opcodeID>=7 && scripts.key(pos + opcodeID - 7) == JsmOpcode::PSHN_L
					&& value == (quint32)scripts.param(pos + opcodeID - 7);
		case JsmOpcode::ASK:
			return opcodeID>=5 && scripts.key(pos + opcodeID - 5) == JsmOpcode::PSHN_L
					&& value == (quint32)scripts.param(pos + opcodeID - 5);
		}
		return false;
	case 1:// opcode
		return key == (value & 0xFFFF) && ((int)(value >> 16) == -1 || param == (int)(value >> 16));
	case 2:// Var
		return (((value & 0x80000000) && (key==JsmOpcode::POPM_B || key == JsmOpcode::POPM_W || key == JsmOpcode::POPM_L))
				|| (!(value & 0x80000000) && key >= 10 && key <= 18))
				&& (value & 0x7FFFFFFF) == (quint32)param;
	case 3:// Exec
		if(opcodeID < 1 || key < 20 || key > 22) return false;

		return ((value >> 16) & 0xFFFF) == (quint32)scripts.param(pos + opcodeID - 1) // label
				&& (value & 0xFFFF) == (quint32)param; // group
	}

	return false;
}

QList<int> JsmFile::searchAllVars() const
{
	int nbOpcode = scripts.data().nbOpcode();
	quint32 key;
	QList<int> vars;

	for(int i=0 ; i<nbOpcode ; ++i)
	{
		JsmOpcode op = scripts.opcode(i);
		key = op.key();

		if(key >= 10 && key <= 18 && !vars.contains(op.opcode())) {
			vars.append(op.opcode());
		}
	}

	return vars;
}

QList<int> JsmFile::searchAllSpells(const QString &fieldName) const
{
	int nbOpcode = scripts.data().nbOpcode();
	unsigned int key;
	int param;
	QList<int> ret;
	bool setDrawPoint = false;
//	QString ret;

	for(int i=0 ; i<nbOpcode ; ++i)
	{
		JsmOpcode op = scripts.opcode(i);
		key = op.key();
		param = op.param();

		if(key == JsmOpcode::SETDRAWPOINT) {
			setDrawPoint = true;
		}
		else if(key == JsmOpcode::DRAWPOINT && i>0) {
			if(!setDrawPoint) {
				qDebug() << fieldName << "magie ?";
			} else if(scripts.key(i-1) == JsmOpcode::PSHN_L) {
				param = scripts.param(i-1);
				/*QStringList curList = ret.value(param, QStringList());
				curList.append(fieldName);
				ret.insert(param, curList);*/
				ret.append(param);
//				ret.append(" - ").append(QString::number(param)).append("\n");
			}
		}
	}

	return ret;
}

QList<int> JsmFile::searchAllMoments() const
{
	int nbOpcode = scripts.data().nbOpcode();
	quint32 key;
	qint32 param;
	QList<int> ret;

	for(int i=0 ; i<nbOpcode ; ++i)
	{
		JsmOpcode op = scripts.opcode(i);
		key = op.key();
		param = op.param();

		if((key==JsmOpcode::POPM_B || key == JsmOpcode::POPM_W || key == JsmOpcode::POPM_L) && i>0 && param==256) {
			op = scripts.opcode(i-1);
			param = op.param();

			if(op.key()==JsmOpcode::PSHN_L && !ret.contains(param))
				ret.append(param);
		}
	}

	return ret;
}

void JsmFile::searchAllOpcodeTypes(QMap<int, int> &ret/*, QMap<int, QString> &stackState*/) const
{
	int nbOpcode = scripts.data().nbOpcode();
	quint32 key;
	qint32 param;
	QStack<QString> stack;

	for(int i=0 ; i<nbOpcode ; ++i)
	{
		JsmOpcode op = scripts.opcode(i);
		key = op.key();
		param = op.param();
		int type = !op.hasParam() ? JUST_KEY : (param < 0 ? KEY_AND_SPARAM : KEY_AND_UPARAM);

		switch(key) {
		case JsmOpcode::PSHN_L:
			stack.push("numerical value");
			break;
		case JsmOpcode::PSHI_L:
			stack.push("temp value");
			break;
		case JsmOpcode::PSHM_B:
			stack.push("var value (byte)");
			break;
		case JsmOpcode::PSHM_W:
			stack.push("var value (word)");
			break;
		case JsmOpcode::PSHM_L:
			stack.push("var value (long)");
			break;
		case JsmOpcode::PSHSM_B:
			stack.push("var value (signed byte)");
			break;
		case JsmOpcode::PSHSM_W:
			stack.push("var value (signed word)");
			break;
		case JsmOpcode::PSHSM_L:
			stack.push("var value (signed long)");
			break;
		case JsmOpcode::PSHAC:
			stack.push("AC value");
			break;
		case JsmOpcode::POPI_L:
		case JsmOpcode::POPM_B:
		case JsmOpcode::POPM_W:
		case JsmOpcode::POPM_L:
			stack.pop();
			break;
		default:
			stack.clear();
			break;
		}

//		if(ret.contains(key)) {
//			if(ret.value(key) != type) {
//				if(ret.value(key) == JUST_KEY || type == JUST_KEY) {
//					qWarning() << "[Type !=]" << "key" << QString::number(key,16) << "referencedType" << ret.value(key) << "currentType" << type;
//				}
//				else if(type == KEY_AND_SPARAM) {
//					ret.insert(key, KEY_AND_SPARAM);
//				}
//			}
//		} else {
			ret.insert(key, type);
//			stackState.insert(key,
//		}
	}
}

bool JsmFile::hasSym() const
{
	return _hasSym;
}

bool JsmFile::isModified() const
{
	return modified;
}

void JsmFile::setDecompiledScript(int groupID, int methodID, const QString &text)
{
//	qDebug() << "JsmFile::setDecompiledScript" << groupID << methodID;
	if(groupID == -1 || methodID == -1)		return;

	scripts.setDecompiledScript(groupID, methodID, text);
}

void JsmFile::setCurrentOpcodeScroll(int groupID, int methodID, int scrollValue, const QTextCursor &textCursor)
{
//	qDebug() << "JsmFile::setCurrentOpcodeScroll";
	if(groupID == -1)		return;
//	qDebug() << groupID << methodID << scrollValue << "mapid" << _mapID;
	groupItem = groupID;
	if(methodID == -1)		return;
	methodItem.insert(groupID, methodID);
	if(scrollValue == -1)		return;
	quint64 key = ((qint64)groupID << 32) | (qint64)methodID;
	opcodeScroll.insert(key, scrollValue);
	textCursors.insert(key, ((qint64)textCursor.position() << 32) | (qint64)textCursor.anchor());
}

int JsmFile::currentGroupItem() const
{
	return groupItem;
}

int JsmFile::currentMethodItem(int groupID) const
{
	return methodItem.value(groupID, -1);
}

int JsmFile::currentOpcodeScroll(int groupID, int methodID) const
{
	return opcodeScroll.value(((qint64)groupID << 32) | (qint64)methodID, -1);
}

int JsmFile::currentTextCursor(int groupID, int methodID, int &anchor) const
{
	quint64 val = textCursors.value(((qint64)groupID << 32) | (qint64)methodID, -1);
	anchor = val & 0xFFFFFFFF;
	return val >> 32;
}