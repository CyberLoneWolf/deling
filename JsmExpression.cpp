#include "JsmExpression.h"
#include "Field.h"
#include "Config.h"

QString JsmInstruction::toString(const Field *field, int indent) const
{
	switch(type()) {
	case Opcode:
		return QString("%1%2")
		        .arg(QString(indent, QChar('\t')),
		             instruction().opcode->toString());
	case Expression:
		return QString("%1%2")
		        .arg(QString(indent, QChar('\t')),
		             instruction().expression->toString(field));
	case Control:
		return instruction().control->toString(field, indent);
	case Application:
		return QString("%1%2")
		        .arg(QString(indent, QChar('\t')),
		             instruction().application->toString(field));
	}
	return QString(); // Never happen
}

QStringList JsmProgram::toStringList(const Field *field, int indent) const
{
	QStringList ret;

	foreach(const JsmInstruction &instr, *this) {
		QString instrStr = instr.toString(field, indent);
		if(!instrStr.isEmpty()) {
			ret.append(instrStr);
		}
	}

	return ret;
}

int JsmExpression::eval(bool *ok) const
{
	if(ok) {
		*ok = false;
	}
	return 0;
}

QString JsmExpression::stripParenthesis(const QString &exprStr)
{
	if (exprStr.startsWith('(')
	        && exprStr.endsWith(')')) {
		return exprStr.mid(1, exprStr.size() - 2);
	}
	return exprStr;
}

JsmExpression *JsmExpression::factory(const JsmOpcode *op,
                                      QStack<JsmExpression *> &stack)
{
	JsmExpression *ret = 0;

	switch(op->key()) {
	case JsmOpcode::CAL:
		if(op->param() == JsmExpressionUnary::Min
		        || op->param() == JsmExpressionUnary::Not) {
			if(!stack.isEmpty()) {
				ret = new JsmExpressionUnary(
				            JsmExpressionUnary::Operation(op->param()),
				            stack.pop());
			}
		} else if(op->param() <= JsmExpressionBinary::Eor) {
			if(stack.size() >= 2) {
				ret = new JsmExpressionBinary(
				            JsmExpressionBinary::Operation(op->param()),
				            stack.pop(),
				            stack.pop());
			}
		}
		break;
	case JsmOpcode::PSHN_L:
		ret = new JsmExpressionVal(op->param());
		break;
	case JsmOpcode::PSHI_L:
		ret = new JsmExpressionTemp(op->param());
		break;
	case JsmOpcode::PSHM_B:
		ret = new JsmExpressionVarUByte(op->param());
		break;
	case JsmOpcode::PSHM_W:
		ret = new JsmExpressionVarUWord(op->param());
		break;
	case JsmOpcode::PSHM_L:
		ret = new JsmExpressionVarULong(op->param());
		break;
	case JsmOpcode::PSHSM_B:
		ret = new JsmExpressionVarSByte(op->param());
		break;
	case JsmOpcode::PSHSM_W:
		ret = new JsmExpressionVarSWord(op->param());
		break;
	case JsmOpcode::PSHSM_L:
		ret = new JsmExpressionVarSLong(op->param());
		break;
	case JsmOpcode::PSHAC:
		ret = new JsmExpressionChar(op->param());
		break;
	default:
		break;
	}

	if(ret) {
		stack.push(ret);
	}

	return ret;
}

QString JsmExpressionVal::toString(const Field *field, int base) const
{
	Q_UNUSED(field)
	switch(base) {
	case 2:
		return QString("b%1").arg(quint32(_val), 0, base);
	case 16:
		return QString("0x%1").arg(QString::number(quint32(_val), base)
		                           .toUpper());
	default:
		return QString::number(_val, base);
	}
}

int JsmExpressionVal::eval(bool *ok) const
{
	if(ok) {
		*ok = true;
	}
	return _val;
}

QString JsmExpressionVar::varName() const
{
	QString name = Config::value(QString("var%1").arg(_var)).toString();
	if(!name.isEmpty() && _var < 1024) {
		return QString("%1_%2").arg(_var).arg(name
		                                      .replace(QRegExp("\\W"), "_")
		                                      .replace(QRegExp("_+"), "_"));
	}
	return QString::number(_var);
}

QString JsmExpressionVarUByte::toString(const Field *field, int base) const
{
	Q_UNUSED(field)
	Q_UNUSED(base)
	return QString("%1_ubyte").arg(varName());
}

QString JsmExpressionVarUWord::toString(const Field *field, int base) const
{
	Q_UNUSED(field)
	Q_UNUSED(base)
	return QString("%1_uword").arg(varName());
}

QString JsmExpressionVarULong::toString(const Field *field, int base) const
{
	Q_UNUSED(field)
	Q_UNUSED(base)
	return QString("%1_ulong").arg(varName());
}

QString JsmExpressionVarSByte::toString(const Field *field, int base) const
{
	Q_UNUSED(field)
	Q_UNUSED(base)
	return QString("%1_sbyte").arg(varName());
}

QString JsmExpressionVarSWord::toString(const Field *field, int base) const
{
	Q_UNUSED(field)
	Q_UNUSED(base)
	return QString("%1_sword").arg(varName());
}

QString JsmExpressionVarSLong::toString(const Field *field, int base) const
{
	Q_UNUSED(field)
	Q_UNUSED(base)
	return QString("%1_slong").arg(varName());
}

QString JsmExpressionChar::toString(const Field *field, int base) const
{
	Q_UNUSED(field)
	Q_UNUSED(base)
	return QString("char_%1").arg(_char);
}

QString JsmExpressionTemp::toString(const Field *field, int base) const
{
	Q_UNUSED(field)
	Q_UNUSED(base)
	return QString("temp_%1").arg(_temp);
}

QString JsmExpressionUnary::toString(const Field *field, int base) const
{
	if(_op == Not) {
		base = 16;
	}
	return QString("%1%2").arg(operationToString(),
	                           _first->toString(field, base));
}

int JsmExpressionUnary::opcodeCount() const
{
	return 1 + _first->opcodeCount();
}

int JsmExpressionUnary::eval(bool *ok) const
{
	int v = _first->eval(ok);
	if(ok && *ok) {
		switch (_op) {
		case Min:
			return -v;
		case Not:
			return ~v;
		}
	}
	return JsmExpression::eval(ok);
}

QString JsmExpressionUnary::operationToString(Operation op)
{
	switch (op) {
	case Min:
		return "-";
	case Not:
		return "~";
	}
	return QString();
}

QString JsmExpressionBinary::toString(const Field *field, int base) const
{
	if(_op == And
	        || _op == Or
	        || _op == Eor) {
		base = 16;
	}
	return QString("(%1 %2 %3)")
	        .arg(_first->toString(field, base),
	             operationToString(),
	             _second->toString(field, base));
}

int JsmExpressionBinary::eval(bool *ok) const
{
	int v1 = _first->eval(ok);
	if(ok && *ok) {
		int v2 = _second->eval(ok);
		if(ok && *ok) {
			switch (_op) {
			case Add:
				return v1 + v2;
			case Sub:
				return v1 - v2;
			case Mul:
				return v1 * v2;
			case Mod:
				return v1 % v2;
			case Div:
				return v1 / v2;
			case Eq:
				return v1 == v2;
			case Gt:
				return v1 > v2;
			case Ge:
				return v1 >= v2;
			case Ls:
				return v1 < v2;
			case Le:
				return v1 <= v2;
			case Nt:
				return v1 != v2;
			case And:
				return v1 & v2;
			case Or:
				return v1 | v2;
			case Eor:
				return v1 ^ v2;
			}
		}
	}
	return JsmExpression::eval(ok);
}

QString JsmExpressionBinary::operationToString(Operation op)
{
	switch (op) {
	case Add:
		return "+";
	case Sub:
		return "-";
	case Mul:
		return "*";
	case Mod:
		return "%";
	case Div:
		return "/";
	case Eq:
		return "==";
	case Gt:
		return ">";
	case Ge:
		return ">=";
	case Ls:
		return "<";
	case Le:
		return "<=";
	case Nt:
		return "!=";
	case And:
		return "&";
	case Or:
		return "|";
	case Eor:
		return "^";
	}
	return QString();
}

int JsmExpressionBinary::opcodeCount() const
{
	return 1 + _first->opcodeCount() + _second->opcodeCount();
}

QString &JsmControl::indentString(QString &str, int indent)
{
	return str.prepend(QString(indent, QChar('\t')));
}

QString JsmControl::indentString(const char *str, int indent)
{
	QString s(str);
	return indentString(s, indent);
}

QString JsmControlIfElse::toString(const Field *field, int indent,
                                   bool elseIf) const
{
	if(_block.isEmpty() && _blockElse.isEmpty()) {
		return QString();
	}
	// TODO: if block is empty, then blockElse will be executed unless condition
	QString condStr = JsmExpression::stripParenthesis(
	                      _condition->toString(field));
	QStringList lines(indentString("if %1 begin", elseIf ? 0 : indent)
	                  .arg(condStr));

	lines.append(_block.toStringList(field, indent + 1));

	if(!_blockElse.isEmpty()) {
		if(_blockElse.size() == 1) {
			const JsmInstruction &instr = _blockElse.first();
			if (instr.type() == JsmInstruction::Control
			        && instr.instruction().control->type() ==
			        JsmControl::IfElse) {
				JsmControlIfElse *ifElse =
				        static_cast<JsmControlIfElse *>(instr.instruction()
				                                        .control);
				lines.append(indentString("else %1", indent)
				             .arg(ifElse->toString(field, indent, true)));
				goto JsmControlIfElse_toString_end;
			}
		}

		lines.append(indentString("else", indent));
		lines.append(_blockElse.toStringList(field, indent + 1));
	}

JsmControlIfElse_toString_end:
	if(!elseIf) {
		lines.append(indentString("end", indent));
	}

	return lines.join("\n");
}

QString JsmControlWhile::toString(const Field *field, int indent) const
{
	QString prefix, firstLine;
	bool ok, noBlock = _block.isEmpty();

	if(noBlock) {
		prefix = "wait ";
	}

	int value = condition()->eval(&ok);
	if(ok && value == 1) {
		// Forever
		firstLine = "forever";
	} else {
		// While
		QString condStr = JsmExpression::stripParenthesis(
		                      _condition->toString(field));
		firstLine = QString("while %1").arg(condStr);
		if(!noBlock) {
			firstLine.append(" begin");
		}
	}
	QStringList lines(indentString(prefix.append(firstLine), indent));

	if(!noBlock) {
		lines.append(_block.toStringList(field, indent + 1));
		lines.append(indentString("end", indent));
	}

	return lines.join("\n");
}

QString JsmControlRepeatUntil::toString(const Field *field, int indent) const
{
	QStringList lines(indentString("repeat", indent));
	lines.append(_block.toStringList(field, indent + 1));

	QString condStr = JsmExpression::stripParenthesis(
	                      _condition->toString(field));
	lines.append(indentString("until %1 end", indent)
	             .arg(condStr));

	return lines.join("\n");
}

QStringList JsmApplication::stackToStringList(const Field *field) const
{
	QStringList params;

	QStack<JsmExpression *> stackCpy = _stack;
	while(!stackCpy.isEmpty()) {
		params.append(stackCpy.pop()->toString(field));
	}

	return params;
}

QString JsmApplication::paramsToString(const Field *field) const
{
	QStringList params;

	if(_opcode->hasParam()) {
		params.append(_opcode->paramStr());
	}

	params.append(stackToStringList(field));

	return params.join(", ");
}

QString JsmApplication::toString(const Field *field) const
{
	return QString("%1(%2)").arg(_opcode->name().toLower(),
	                             paramsToString(field));
}

JsmExpression *JsmApplicationAssignment::opcodeExpression() const
{
	switch(_opcode->key()) {
	case JsmOpcode::POPI_L:
		return new JsmExpressionTemp(_opcode->param());
	case JsmOpcode::POPM_B:
		return new JsmExpressionVarUByte(_opcode->param());
	case JsmOpcode::POPM_W:
		return new JsmExpressionVarUWord(_opcode->param());
	case JsmOpcode::POPM_L:
		return new JsmExpressionVarULong(_opcode->param());
	default:
		break;
	}
	return 0;
}

QString JsmApplicationAssignment::toString(const Field *field) const
{
	QString ret;
	JsmExpression *opExpr = opcodeExpression(), // New instance
	        *expr = _stack.top();
	if(expr->type() == JsmExpression::Binary) {
		JsmExpressionBinary *binaryExpr
		        = static_cast<JsmExpressionBinary *>(expr);
		if(opExpr->toString(field) ==
		        binaryExpr->leftOperand()->toString(field)) {
			int base = 10;
			if(binaryExpr->operation() == JsmExpressionBinary::And
			        || binaryExpr->operation() == JsmExpressionBinary::Or
			        || binaryExpr->operation() == JsmExpressionBinary::Eor) {
				base = 16;
			}
			ret = QString("%1 %2= %3")
			      .arg(opExpr->toString(field),
			           binaryExpr->operationToString(),
			           JsmExpression::stripParenthesis(
			               binaryExpr->rightOperand()->toString(field, base)));
		}
	}
	if(ret.isEmpty()) {
		ret = QString("%1 = %2").arg(opExpr->toString(field),
		                             JsmExpression::stripParenthesis(
		                                 expr->toString(field)));
	}
	delete opExpr;
	return ret;
}

QString JsmApplicationExec::execType() const
{
	switch(_opcode->key()) {
	case JsmOpcode::REQ:
		return QString();
	case JsmOpcode::REQSW:
		return "SW";
	case JsmOpcode::REQEW:
		return "EW";
	default:
		break;
	}
	return QString();
}

QString JsmApplicationExec::toString(const Field *field) const
{
	QString groupName, methodName;
	QStack<JsmExpression *> stackCpy = _stack;
	bool ok;
	int groupId = _opcode->param(),
	    methodId = stackCpy.pop()->eval(&ok);

	if(ok && field && groupId >= 0 && methodId >= 0 && field->hasJsmFile()) {
		JsmFile *jsm = field->getJsmFile();
		const JsmScripts &scripts = jsm->getScripts();
		if(groupId < scripts.nbGroup()
		        && methodId < scripts.nbScript()) {
			groupName = scripts.group(groupId).name();
			methodName = scripts.script(methodId).name();
		}
	}

	if(groupName.isEmpty() || methodName.isEmpty()) {
		return JsmApplication::toString(field);
	}

	QStringList params(stackCpy.pop()->toString(field));
	QString eType = execType();
	if(!eType.isEmpty()) {
		params.append(eType);
	}

	return QString("%1.%2(%3)").arg(groupName,
	                                methodName,
	                                params.join(", "));
}
