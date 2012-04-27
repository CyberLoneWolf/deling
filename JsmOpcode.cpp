#include "JsmOpcode.h"

JsmOpcode::JsmOpcode() :
	op(0)
{

}

JsmOpcode::JsmOpcode(quint32 op) :
	op(op)
{
}

JsmOpcode::JsmOpcode(unsigned int key, int param)
{
	setKey(key);
	setParam(param);
}

unsigned int JsmOpcode::key() const
{
	if(hasParam()) 	return op >> 24;
	else			return op;
}

int JsmOpcode::param() const
{
	if(!(op & 0x00800000))		return op & 0x00FFFFFF;
	else						return op | 0xFF000000;
}

quint32 JsmOpcode::opcode() const
{
	return op;
}

void JsmOpcode::setOpcode(quint32 op)
{
	this->op = op;
}

void JsmOpcode::setKey(unsigned int key)
{
	if(hasParam()) 	op = (key << 24) | (param() & 0x00FFFFFF);
	else			op = key;
}

void JsmOpcode::setParam(int param)
{
	op = (key() << 24) | (param & 0x00FFFFFF);
}

bool JsmOpcode::hasParam() const
{
	return (op & 0xFF000000) != 0;
}

int JsmOpcode::popCount() const
{
	return -1;
}

int JsmOpcode::pushCount() const
{
	return -1;
}

QString JsmOpcode::name() const
{
	unsigned int k = key();
	return k<358 ? opcodes[k] : QString("UNKNOWN%1").arg(k-358+1);
}

QString JsmOpcode::paramStr() const
{
	return QString::number(param());
}

QString JsmOpcode::desc() const
{
	return QString();
}

JsmOpcodeCal::JsmOpcodeCal(quint32 op) : JsmOpcode(op) {}
JsmOpcodeCal::JsmOpcodeCal(unsigned int key, int param) : JsmOpcode(key, param) {}
bool JsmOpcodeCal::hasParam() const {	return true; }
int JsmOpcodeCal::popCount() const {	return 2; }
int JsmOpcodeCal::pushCount() const {	return 1; }
QString JsmOpcodeCal::paramStr() const {
	int p = param();
	return p<15 ? cal_table[p] : JsmOpcode::paramStr();
}

const char *JsmOpcodeCal::cal_table[15] = {
	"ADD",
	"SUB",
	"MUL",
	"MOD",
	"DIV",
	"MIN",
	"EQ",
	"GT",
	"GE",
	"LS",
	"LE",
	"NT",
	"AND",
	"OR",
	"EOR",
};

JsmOpcodePsh::JsmOpcodePsh(quint32 op) : JsmOpcode(op) {}
JsmOpcodePsh::JsmOpcodePsh(unsigned int key, int param) : JsmOpcode(key, param) {}
bool JsmOpcodePsh::hasParam() const {	return true; }
int JsmOpcodePsh::popCount() const {	return 0; }
int JsmOpcodePsh::pushCount() const {	return 1; }

JsmOpcodePop::JsmOpcodePop(quint32 op) : JsmOpcode(op) {}
JsmOpcodePop::JsmOpcodePop(unsigned int key, int param) : JsmOpcode(key, param) {}
bool JsmOpcodePop::hasParam() const {	return true; }
int JsmOpcodePop::popCount() const {	return 1; }
int JsmOpcodePop::pushCount() const {	return 0; }

/*
CAL: Un param�tre
JMP: Un param�tre (Sign� ?)
JPF: Un param�tre (Sign� ?)
LBL: Un param�tre
RET: Un param�tre
PSHN_L: Un param�tre (Sign� ?)
PSHI_L: Un param�tre
POPI_L: Un param�tre
PSHM_B: Un param�tre
POPM_B: Un param�tre
PSHM_W: Un param�tre
POPM_W: Un param�tre
PSHM_L: Un param�tre
POPM_L: Un param�tre
PSHSM_B: Un param�tre
PSHSM_W: Un param�tre
PSHSM_L: Un param�tre
PSHAC: Un param�tre
REQ: Un param�tre
REQSW: Un param�tre
REQEW: Un param�tre
PREQ: Un param�tre
PREQSW: Un param�tre
PREQEW: Un param�tre
UNUSE: Un param�tre
HALT: Un param�tre
SET: Un param�tre
SET3: Un param�tre
IDLOCK: Un param�tre
IDUNLOCK: Un param�tre
EFFECTPLAY2: Un param�tre
FOOTSTEP: Un param�tre
JUMP: Un param�tre
JUMP3: Un param�tre
LADDERUP: Un param�tre
LADDERDOWN: Un param�tre
LADDERUP2: Un param�tre
LADDERDOWN2: Un param�tre
MAPJUMP: Un param�tre
MAPJUMP3: Un param�tre
SETMODEL: Un param�tre
BASEANIME: Un param�tre
ANIME: Un param�tre
ANIMEKEEP: Un param�tre
CANIME: Un param�tre
CANIMEKEEP: Un param�tre
RANIME: Un param�tre
RANIMEKEEP: Un param�tre
RCANIME: Un param�tre
RCANIMEKEEP: Un param�tre
RANIMELOOP: Un param�tre
RCANIMELOOP: Un param�tre
LADDERANIME: Un param�tre
DISCJUMP: Un param�tre
SETLINE: Aucun param�tre
LINEON: Aucun param�tre
LINEOFF: Aucun param�tre
WAIT: Aucun param�tre
MSPEED: Aucun param�tre
MOVE: Aucun param�tre
MOVEA: Aucun param�tre
PMOVEA: Aucun param�tre
CMOVE: Aucun param�tre
FMOVE: Aucun param�tre
PJUMPA: Aucun param�tre
ANIMESYNC: Aucun param�tre
ANIMESTOP: Aucun param�tre
MES: Aucun param�tre
MESSYNC: Aucun param�tre
MESVAR: Aucun param�tre
ASK: Aucun param�tre
WINSIZE: Aucun param�tre
WINCLOSE: Aucun param�tre
UCON: Aucun param�tre
UCOFF: Aucun param�tre
MOVIE: Aucun param�tre
MOVIESYNC: Aucun param�tre
SETPC: Aucun param�tre
DIR: Aucun param�tre
DIRP: Aucun param�tre
DIRA: Aucun param�tre
PDIRA: Aucun param�tre
SPUREADY: Aucun param�tre
TALKON: Aucun param�tre
TALKOFF: Aucun param�tre
PUSHON: Aucun param�tre
PUSHOFF: Aucun param�tre
ISTOUCH: Aucun param�tre
MAPJUMPO: Aucun param�tre
MAPJUMPON: Aucun param�tre
MAPJUMPOFF: Aucun param�tre
SETMESSPEED: Aucun param�tre
SHOW: Aucun param�tre
HIDE: Aucun param�tre
TALKRADIUS: Aucun param�tre
PUSHRADIUS: Aucun param�tre
AMESW: Aucun param�tre
AMES: Aucun param�tre
GETINFO: Aucun param�tre
THROUGHON: Aucun param�tre
THROUGHOFF: Aucun param�tre
BATTLE: Aucun param�tre
BATTLERESULT: Aucun param�tre
BATTLEON: Aucun param�tre
BATTLEOFF: Aucun param�tre
KEYSCAN: Aucun param�tre
KEYON: Aucun param�tre
AASK: Aucun param�tre
PGETINFO: Aucun param�tre
DSCROLL: Aucun param�tre
LSCROLL: Aucun param�tre
CSCROLL: Aucun param�tre
DSCROLLA: Aucun param�tre
LSCROLLA: Aucun param�tre
CSCROLLA: Aucun param�tre
SCROLLSYNC: Aucun param�tre
RMOVE: Aucun param�tre
RMOVEA: Aucun param�tre
RPMOVEA: Aucun param�tre
RCMOVE: Aucun param�tre
RFMOVE: Aucun param�tre
MOVESYNC: Aucun param�tre
CLEAR: Aucun param�tre
DSCROLLP: Aucun param�tre
LSCROLLP: Aucun param�tre
CSCROLLP: Aucun param�tre
LTURNR: Aucun param�tre
LTURNL: Aucun param�tre
CTURNR: Aucun param�tre
CTURNL: Aucun param�tre
ADDPARTY: Aucun param�tre
SUBPARTY: Aucun param�tre
CHANGEPARTY: Aucun param�tre
REFRESHPARTY: Aucun param�tre
SETPARTY: Aucun param�tre
ISPARTY: Aucun param�tre
ADDMEMBER: Aucun param�tre
SUBMEMBER: Aucun param�tre
LTURN: Aucun param�tre
CTURN: Aucun param�tre
PLTURN: Aucun param�tre
PCTURN: Aucun param�tre
JOIN: Aucun param�tre
BGANIME: Aucun param�tre
RBGANIME: Aucun param�tre
RBGANIMELOOP: Aucun param�tre
BGANIMESYNC: Aucun param�tre
BGDRAW: Aucun param�tre
BGOFF: Aucun param�tre
BGANIMESPEED: Aucun param�tre
SETTIMER: Aucun param�tre
DISPTIMER: Aucun param�tre
SETGETA: Aucun param�tre
SETROOTTRANS: Aucun param�tre
SETVIBRATE: Aucun param�tre
MOVIEREADY: Aucun param�tre
GETTIMER: Aucun param�tre
FADEIN: Aucun param�tre
FADEOUT: Aucun param�tre
FADESYNC: Aucun param�tre
SHAKE: Aucun param�tre
SHAKEOFF: Aucun param�tre
FADEBLACK: Aucun param�tre
FOLLOWOFF: Aucun param�tre
FOLLOWON: Aucun param�tre
GAMEOVER: Aucun param�tre
SHADELEVEL: Aucun param�tre
SHADEFORM: Aucun param�tre
FMOVEA: Aucun param�tre
FMOVEP: Aucun param�tre
SHADESET: Aucun param�tre
MUSICCHANGE: Aucun param�tre
MUSICLOAD: Aucun param�tre
FADENONE: Aucun param�tre
POLYCOLOR: Aucun param�tre
POLYCOLORALL: Aucun param�tre
KILLTIMER: Aucun param�tre
CROSSMUSIC: Aucun param�tre
DUALMUSIC: Aucun param�tre
EFFECTPLAY: Aucun param�tre
EFFECTLOAD: Aucun param�tre
LOADSYNC: Aucun param�tre
MUSICSTOP: Aucun param�tre
MUSICVOL: Aucun param�tre
MUSICVOLTRANS: Aucun param�tre
MUSICVOLFADE: Aucun param�tre
ALLSEVOL: Aucun param�tre
ALLSEVOLTRANS: Aucun param�tre
ALLSEPOSTRANS: Aucun param�tre
SEVOL: Aucun param�tre
SEVOLTRANS: Aucun param�tre
SEPOS: Aucun param�tre
SEPOSTRANS: Aucun param�tre
SETBATTLEMUSIC: Aucun param�tre
BATTLEMODE: Aucun param�tre
SESTOP: Aucun param�tre
INITSOUND: Aucun param�tre
BGSHADE: Aucun param�tre
BGSHADESTOP: Aucun param�tre
RBGSHADELOOP: Aucun param�tre
DSCROLL2: Aucun param�tre
LSCROLL2: Aucun param�tre
CSCROLL2: Aucun param�tre
DSCROLLA2: Aucun param�tre
CSCROLLA2: Aucun param�tre
SCROLLSYNC2: Aucun param�tre
SCROLLMODE2: Aucun param�tre
MENUENABLE: Aucun param�tre
MENUDISABLE: Aucun param�tre
FOOTSTEPON: Aucun param�tre
FOOTSTEPOFF: Aucun param�tre
FOOTSTEPOFFALL: Aucun param�tre
FOOTSTEPCUT: Aucun param�tre
PREMAPJUMP: Aucun param�tre
USE: Aucun param�tre
SPLIT: Aucun param�tre
ANIMESPEED: Aucun param�tre
RND: Aucun param�tre
DCOLADD: Aucun param�tre
DCOLSUB: Aucun param�tre
TCOLADD: Aucun param�tre
TCOLSUB: Aucun param�tre
FCOLADD: Aucun param�tre
FCOLSUB: Aucun param�tre
COLSYNC: Aucun param�tre
DOFFSET: Aucun param�tre
LOFFSETS: Aucun param�tre
COFFSETS: Aucun param�tre
LOFFSET: Aucun param�tre
COFFSET: Aucun param�tre
OFFSETSYNC: Aucun param�tre
RUNENABLE: Aucun param�tre
RUNDISABLE: Aucun param�tre
MAPFADEOFF: Aucun param�tre
MAPFADEON: Aucun param�tre
INITTRACE: Aucun param�tre
SETDRESS: Aucun param�tre
FACEDIR: Aucun param�tre
FACEDIRA: Aucun param�tre
FACEDIRP: Aucun param�tre
FACEDIRLIMIT: Aucun param�tre
FACEDIROFF: Aucun param�tre
SARALYOFF: Aucun param�tre
SARALYON: Aucun param�tre
SARALYDISPOFF: Aucun param�tre
SARALYDISPON: Aucun param�tre
MESMODE: Aucun param�tre
FACEDIRINIT: Aucun param�tre
FACEDIRI: Aucun param�tre
JUNCTION: Aucun param�tre
SETCAMERA: Aucun param�tre
BATTLECUT: Aucun param�tre
FOOTSTEPCOPY: Aucun param�tre
WORLDMAPJUMP: Aucun param�tre
RFACEDIR: Aucun param�tre
RFACEDIRA: Aucun param�tre
RFACEDIRP: Aucun param�tre
RFACEDIROFF: Aucun param�tre
FACEDIRSYNC: Aucun param�tre
COPYINFO: Aucun param�tre
RAMESW: Aucun param�tre
BGSHADEOFF: Aucun param�tre
AXIS: Aucun param�tre
MENUNORMAL: Aucun param�tre
MENUPHS: Aucun param�tre
BGCLEAR: Aucun param�tre
GETPARTY: Aucun param�tre
MENUSHOP: Aucun param�tre
DISC: Aucun param�tre
LSCROLL3: Aucun param�tre
CSCROLL3: Aucun param�tre
MACCEL: Aucun param�tre
MLIMIT: Aucun param�tre
ADDITEM: Aucun param�tre
SETWITCH: Aucun param�tre
SETODIN: Aucun param�tre
RESETGF: Aucun param�tre
MENUNAME: Aucun param�tre
REST: Aucun param�tre
MOVECANCEL: Aucun param�tre
ACTORMODE: Aucun param�tre
MENUSAVE: Aucun param�tre
SAVEENABLE: Aucun param�tre
PHSENABLE: Aucun param�tre
HOLD: Aucun param�tre
MOVIECUT: Aucun param�tre
SETPLACE: Aucun param�tre
SETDCAMERA: Aucun param�tre
CHOICEMUSIC: Aucun param�tre
GETCARD: Aucun param�tre
DRAWPOINT: Aucun param�tre
PHSPOWER: Aucun param�tre
KEY: Aucun param�tre
CARDGAME: Aucun param�tre
SETBAR: Aucun param�tre
DISPBAR: Aucun param�tre
KILLBAR: Aucun param�tre
SCROLLRATIO2: Aucun param�tre
WHOAMI: Aucun param�tre
MUSICSTATUS: Aucun param�tre
MUSICREPLAY: Aucun param�tre
DOORLINEOFF: Aucun param�tre
DOORLINEON: Aucun param�tre
MUSICSKIP: Aucun param�tre
DYING: Aucun param�tre
SETHP: Aucun param�tre
MOVEFLUSH: Aucun param�tre
MUSICVOLSYNC: Aucun param�tre
PUSHANIME: Aucun param�tre
POPANIME: Aucun param�tre
KEYSCAN2: Aucun param�tre
PARTICLEON: Aucun param�tre
PARTICLEOFF: Aucun param�tre
KEYSIGHNCHANGE: Aucun param�tre
ADDGIL: Aucun param�tre
ADDPASTGIL: Aucun param�tre
ADDSEEDLEVEL: Aucun param�tre
PARTICLESET: Aucun param�tre
SETDRAWPOINT: Aucun param�tre
MENUTIPS: Aucun param�tre
LASTIN: Aucun param�tre
LASTOUT: Aucun param�tre
SEALEDOFF: Aucun param�tre
MENUTUTO: Aucun param�tre
CLOSEEYES: Aucun param�tre
SETCARD: Aucun param�tre
HOWMANYCARD: Aucun param�tre
WHERECARD: Aucun param�tre
ADDMAGIC: Aucun param�tre
SWAP: Aucun param�tre
SPUSYNC: Aucun param�tre
BROKEN: Aucun param�tre
Unknown1: Aucun param�tre
Unknown2: Aucun param�tre
Unknown3: Aucun param�tre
Unknown4: Aucun param�tre
Unknown5: Aucun param�tre
Unknown6: Aucun param�tre
Unknown7: Aucun param�tre
Unknown8: Aucun param�tre
Unknown9: Aucun param�tre
Unknown10: Aucun param�tre
Unknown11: Aucun param�tre
Unknown12: Aucun param�tre
Unknown13: Aucun param�tre
Unknown14: Aucun param�tre
Unknown15: Aucun param�tre
Unknown16: Aucun param�tre
Unknown17: Aucun param�tre
Unknown18: Aucun param�tre
*/

const char *JsmOpcode::opcodes[376] = {
	"NOP",
	"CAL",
	"JMP",
	"JPF",
	"GJMP",
	"LBL",
	"RET",
	"PSHN_L",
	"PSHI_L",
	"POPI_L",
	"PSHM_B",
	"POPM_B",
	"PSHM_W",
	"POPM_W",
	"PSHM_L",
	"POPM_L",
	"PSHSM_B",
	"PSHSM_W",
	"PSHSM_L",
	"PSHAC",
	"REQ",
	"REQSW",
	"REQEW",
	"PREQ",
	"PREQSW",
	"PREQEW",
	"UNUSE",
	"DEBUG",
	"HALT",
	"SET",
	"SET3",
	"IDLOCK",
	"IDUNLOCK",
	"EFFECTPLAY2",
	"FOOTSTEP",
	"JUMP",
	"JUMP3",
	"LADDERUP",
	"LADDERDOWN",
	"LADDERUP2",
	"LADDERDOWN2",
	"MAPJUMP",
	"MAPJUMP3",
	"SETMODEL",
	"BASEANIME",
	"ANIME",
	"ANIMEKEEP",
	"CANIME",
	"CANIMEKEEP",
	"RANIME",
	"RANIMEKEEP",
	"RCANIME",
	"RCANIMEKEEP",
	"RANIMELOOP",
	"RCANIMELOOP",
	"LADDERANIME",
	"DISCJUMP",
	"SETLINE",
	"LINEON",
	"LINEOFF",
	"WAIT",
	"MSPEED",
	"MOVE",
	"MOVEA",
	"PMOVEA",
	"CMOVE",
	"FMOVE",
	"PJUMPA",
	"ANIMESYNC",
	"ANIMESTOP",
	"MESW",
	"MES",
	"MESSYNC",
	"MESVAR",
	"ASK",
	"WINSIZE",
	"WINCLOSE",
	"UCON",
	"UCOFF",
	"MOVIE",
	"MOVIESYNC",
	"SETPC",
	"DIR",
	"DIRP",
	"DIRA",
	"PDIRA",
	"SPUREADY",
	"TALKON",
	"TALKOFF",
	"PUSHON",
	"PUSHOFF",
	"ISTOUCH",
	"MAPJUMPO",
	"MAPJUMPON",
	"MAPJUMPOFF",
	"SETMESSPEED",
	"SHOW",
	"HIDE",
	"TALKRADIUS",
	"PUSHRADIUS",
	"AMESW",
	"AMES",
	"GETINFO",
	"THROUGHON",
	"THROUGHOFF",
	"BATTLE",
	"BATTLERESULT",
	"BATTLEON",
	"BATTLEOFF",
	"KEYSCAN",
	"KEYON",
	"AASK",
	"PGETINFO",
	"DSCROLL",
	"LSCROLL",
	"CSCROLL",
	"DSCROLLA",
	"LSCROLLA",
	"CSCROLLA",
	"SCROLLSYNC",
	"RMOVE",
	"RMOVEA",
	"RPMOVEA",
	"RCMOVE",
	"RFMOVE",
	"MOVESYNC",
	"CLEAR",
	"DSCROLLP",
	"LSCROLLP",
	"CSCROLLP",
	"LTURNR",
	"LTURNL",
	"CTURNR",
	"CTURNL",
	"ADDPARTY",
	"SUBPARTY",
	"CHANGEPARTY",
	"REFRESHPARTY",
	"SETPARTY",
	"ISPARTY",
	"ADDMEMBER",
	"SUBMEMBER",
	"ISMEMBER",
	"LTURN",
	"CTURN",
	"PLTURN",
	"PCTURN",
	"JOIN",
	"MESFORCUS",
	"BGANIME",
	"RBGANIME",
	"RBGANIMELOOP",
	"BGANIMESYNC",
	"BGDRAW",
	"BGOFF",
	"BGANIMESPEED",
	"SETTIMER",
	"DISPTIMER",
	"SHADETIMER",
	"SETGETA",
	"SETROOTTRANS",
	"SETVIBRATE",
	"STOPVIBRATE",
	"MOVIEREADY",
	"GETTIMER",
	"FADEIN",
	"FADEOUT",
	"FADESYNC",
	"SHAKE",
	"SHAKEOFF",
	"FADEBLACK",
	"FOLLOWOFF",
	"FOLLOWON",
	"GAMEOVER",
	"ENDING",
	"SHADELEVEL",
	"SHADEFORM",
	"FMOVEA",
	"FMOVEP",
	"SHADESET",
	"MUSICCHANGE",
	"MUSICLOAD",
	"FADENONE",
	"POLYCOLOR",
	"POLYCOLORALL",
	"KILLTIMER",
	"CROSSMUSIC",
	"DUALMUSIC",
	"EFFECTPLAY",
	"EFFECTLOAD",
	"LOADSYNC",
	"MUSICSTOP",
	"MUSICVOL",
	"MUSICVOLTRANS",
	"MUSICVOLFADE",
	"ALLSEVOL",
	"ALLSEVOLTRANS",
	"ALLSEPOS",
	"ALLSEPOSTRANS",
	"SEVOL",
	"SEVOLTRANS",
	"SEPOS",
	"SEPOSTRANS",
	"SETBATTLEMUSIC",
	"BATTLEMODE",
	"SESTOP",
	"BGANIMEFLAG",
	"INITSOUND",
	"BGSHADE",
	"BGSHADESTOP",
	"RBGSHADELOOP",
	"DSCROLL2",
	"LSCROLL2",
	"CSCROLL2",
	"DSCROLLA2",
	"LSCROLLA2",
	"CSCROLLA2",
	"DSCROLLP2",
	"LSCROLLP2",
	"CSCROLLP2",
	"SCROLLSYNC2",
	"SCROLLMODE2",
	"MENUENABLE",
	"MENUDISABLE",
	"FOOTSTEPON",
	"FOOTSTEPOFF",
	"FOOTSTEPOFFALL",
	"FOOTSTEPCUT",
	"PREMAPJUMP",
	"USE",
	"SPLIT",
	"ANIMESPEED",
	"RND",
	"DCOLADD",
	"DCOLSUB",
	"TCOLADD",
	"TCOLSUB",
	"FCOLADD",
	"FCOLSUB",
	"COLSYNC",
	"DOFFSET",
	"LOFFSETS",
	"COFFSETS",
	"LOFFSET",
	"COFFSET",
	"OFFSETSYNC",
	"RUNENABLE",
	"RUNDISABLE",
	"MAPFADEOFF",
	"MAPFADEON",
	"INITTRACE",
	"SETDRESS",
	"GETDRESS",
	"FACEDIR",
	"FACEDIRA",
	"FACEDIRP",
	"FACEDIRLIMIT",
	"FACEDIROFF",
	"SARALYOFF",
	"SARALYON",
	"SARALYDISPOFF",
	"SARALYDISPON",
	"MESMODE",
	"FACEDIRINIT",
	"FACEDIRI",
	"JUNCTION",
	"SETCAMERA",
	"BATTLECUT",
	"FOOTSTEPCOPY",
	"WORLDMAPJUMP",
	"RFACEDIRI",
	"RFACEDIR",
	"RFACEDIRA",
	"RFACEDIRP",
	"RFACEDIROFF",
	"FACEDIRSYNC",
	"COPYINFO",
	"PCOPYINFO",
	"RAMESW",
	"BGSHADEOFF",
	"AXIS",
	"AXISSYNC",
	"MENUNORMAL",
	"MENUPHS",
	"BGCLEAR",
	"GETPARTY",
	"MENUSHOP",
	"DISC",
	"DSCROLL3",
	"LSCROLL3",
	"CSCROLL3",
	"MACCEL",
	"MLIMIT",
	"ADDITEM",
	"SETWITCH",
	"SETODIN",
	"RESETGF",
	"MENUNAME",
	"REST",
	"MOVECANCEL",
	"PMOVECANCEL",
	"ACTORMODE",
	"MENUSAVE",
	"SAVEENABLE",
	"PHSENABLE",
	"HOLD",
	"MOVIECUT",
	"SETPLACE",
	"SETDCAMERA",
	"CHOICEMUSIC",
	"GETCARD",
	"DRAWPOINT",
	"PHSPOWER",
	"KEY",
	"CARDGAME",
	"SETBAR",
	"DISPBAR",
	"KILLBAR",
	"SCROLLRATIO2",
	"WHOAMI",
	"MUSICSTATUS",
	"MUSICREPLAY",
	"DOORLINEOFF",
	"DOORLINEON",
	"MUSICSKIP",
	"DYING",
	"SETHP",
	"GETHP",
	"MOVEFLUSH",
	"MUSICVOLSYNC",
	"PUSHANIME",
	"POPANIME",
	"KEYSCAN2",
	"KEYON2",
	"PARTICLEON",
	"PARTICLEOFF",
	"KEYSIGHNCHANGE",
	"ADDGIL",
	"ADDPASTGIL",
	"ADDSEEDLEVEL",
	"PARTICLESET",
	"SETDRAWPOINT",
	"MENUTIPS",
	"LASTIN",
	"LASTOUT",
	"SEALEDOFF",
	"MENUTUTO",
	"OPENEYES",
	"CLOSEEYES",
	"BLINKEYES",
	"SETCARD",
	"HOWMANYCARD",
	"WHERECARD",
	"ADDMAGIC",
	"SWAP",
	"SETPARTY2",
	"SPUSYNC",
	"BROKEN",
	"UNKNOWN1",
	"UNKNOWN2",
	"UNKNOWN3",
	"UNKNOWN4",
	"UNKNOWN5",
	"UNKNOWN6",
	"UNKNOWN7",
	"UNKNOWN8",
	"UNKNOWN9",
	"UNKNOWN10",
	"UNKNOWN11",
	"UNKNOWN12",
	"UNKNOWN13",
	"UNKNOWN14",
	"UNKNOWN15",
	"UNKNOWN16",
	"UNKNOWN17",
	"UNKNOWN18"
};
