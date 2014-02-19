#include "simplepascalsyntaxanalizer.h"
#include "fpcanalizerplugin.h"

namespace FpcAnalizer {

SimplePascalSyntaxAnalizer* SimplePascalSyntaxAnalizer::create(QObject *parent)
{
    static const QStringList kwds = QStringList()
            << "and"
            << "array"
            << "asm"
            << "begin"
            << "break"
            << "case"
            << "const"
            << "constructor"
            << "continue"
            << "destructor"
            << "div"
            << "do"
            << "downto"
            << "else"
            << "end"
//            << "false"
            << "file"
            << "for"
            << "function"
            << "goto"
            << "if"
            << "implementation"
            << "in"
            << "inline"
            << "interface"
            << "label"
            << "mod"
//            << "nil"
            << "not"
            << "object"
            << "of"
            << "on"
            << "operator"
            << "or"
            << "packed"
            << "procedure"
            << "program"
            << "record"
            << "repeat"
            << "set"
            << "shl"
            << "shr"
            << "then"
            << "to"
//            << "true"
            << "type"
            << "unit"
            << "until"
            << "uses"
            << "var"
            << "while"
            << "with"
            << "xor"
            << "as"
            << "class"
            << "dispose"
            << "except"
            << "exit"
            << "exports"
            << "finalization"
            << "finally"
            << "inherited"
            << "initialization"
            << "is"
            << "library"
            << "new"
            << "on"
            << "out"
            << "property"
            << "raise"
            << "self"
            << "threadvar"
            << "try"
            << "absolute"
            << "abstract"
            << "alias"
            << "assembler"
            << "cdecl"
            << "cppdecl"
            << "default"
            << "export"
            << "external"
            << "forward"
            << "index"
            << "local"
            << "name"
            << "nostackframe"
            << "oldfpccall"
            << "override"
            << "pascal"
            << "private"
            << "protected"
            << "public"
            << "published"
            << "register"
            << "reintroduce"
            << "safecall"
            << "softfloat"
            << "stdcall"
            << "virtual"
    ;
    static const QStringList ops = QStringList()
            << "*" << "/" << "+" << "-" << ":="
            << "(" << ")" << "[" << "]" << "," << ";"
            << "." << "^" << "<" << ">" << "=" << "'";
    static const QStringList tps = QStringList()
            << "integer" << "real" << "char" << "string" << "pchar"
            << "boolean" << "word" << "longint" << "smallint" << "text" << "file";
    static const QString pattern = "\\b" + kwds.join("\\b|\\b") + "\\b" +
            "|\\*|//|/|\\+|\\-|:=|\\(|\\)|\\[|\\]|,|:|;|\\.|\\^|<|>|=|'|\\{|\\}|\\s+";
    QRegExp rx(pattern);
    rx.setMinimal(true);
    SimplePascalSyntaxAnalizer* self =
            new SimplePascalSyntaxAnalizer(parent, kwds, ops, tps, rx);
    return self;
}



SimplePascalSyntaxAnalizer::SimplePascalSyntaxAnalizer
(QObject *parent, const QStringList& kwds, const QStringList& ops,
 const QStringList & tps, const QRegExp & rx)
    : QObject(parent)
    , Keywords(kwds)
    , Operators(ops)
    , StandardTypes(tps)
    , Delimeters(rx)
{
    capitalizationHints_ = FpcAnalizerPlugin::self()->readCapitalizationHints();
}

void SimplePascalSyntaxAnalizer::reset()
{
    unitNames_.clear();
    functionNames_.clear();
    lineStartStates_.clear();
    typeNames_.clear();
    thisUnitName_.clear();
    addHelperUnitPosition_.clear();
}

void SimplePascalSyntaxAnalizer::processSyntaxAnalysis(
        const QStringList &lines
        , const QSet<QString> & unitNames
        , const QSet<QString> & functionNames
        , const QSet<QString> & typeNames
        , QList<LineProp> &lineProps
        , QList<QPoint> &lineRanks
        )
{
    reset();
    unitNames_ = unitNames;
    functionNames_ = functionNames;
    typeNames_ = typeNames;
    State currentState = Program;
    QString previousKeyword;
    for (int i=0; i<lines.size(); i++) {
        const QString line = lines[i].toLower();
        LineProp & lineProp = lineProps[i];
        QPoint lineRank(0,0);
        State nextState;
        lineStartKeywords_.append(previousKeyword);
        processLine(line, currentState, lineProp, lineRank, nextState, previousKeyword, true, i);
        lineStartStates_.append(currentState);
        lineRanks[i] = lineRank;
        currentState = nextState;
    }
    lineStartStates_.append(currentState);
    lineStartKeywords_.append(previousKeyword);
}

void SimplePascalSyntaxAnalizer::processLineProp(const QString& line,
                     const QSet<QString> & unitNames,
                     const QSet<QString> & functionNames,
                     const QSet<QString> & typeNames,
                     int lineNo,
                     LineProp & lineProp
                     )
{
    State initialState = lineNo < lineStartStates_.size() ? lineStartStates_.at(lineNo) : Program;
//    QString previousKeyword = lineNo < lineStartKeywords_.size() ? lineStartKeywords_.at(lineNo) : "";
    QString previousKeyword = "";
    QPoint dummy;
    State dummyState;
    processLine(line, initialState, lineProp, dummy, dummyState, previousKeyword, false, lineNo);
}

QString SimplePascalSyntaxAnalizer::makePreprocessedSourceText(const QStringList &lines) const
{
    QString result;
    for (uint i=0; i<lines.size(); i++) {
        QString line = lines[i];
        if (addHelperUnitPosition_.valid && addHelperUnitPosition_.line==i) {
            QString insertion =
                    addHelperUnitPosition_.prefix +
                    "KumirHelper" +
                    addHelperUnitPosition_.suffix;
            line.insert(addHelperUnitPosition_.col, insertion);
        }
        result += line;
        if (i < lines.size()-1) result += "\n";
    }
    return result;
}

SimplePascalSyntaxAnalizer::Lexem
SimplePascalSyntaxAnalizer::takeLexem
(const QString &line, int startPos, State startState) const
{
    Lexem result;
    if (LineComment == startState) {
        result.start = startPos;
        result.length = line.length() - startPos;
        result.stateAfter = Program;
    }
    else if (BlockComment == startState) {
        int end = line.indexOf('}', startPos);
        if (-1 == end) {
            result.start = startPos;
            result.length = line.length() - startPos;
            result.stateAfter = BlockComment;
        }
        else {
            result.start = startPos;
            result.length = end - startPos;
            result.stateAfter = Program;
        }
    }
    else if (String == startState) {
        int end = line.indexOf('\'', startPos);
        if (-1 == end) {
            result.start = startPos;
            result.length = line.length() - startPos;
        }
        else {
            result.start = startPos;
            result.length = 1 + end - startPos;
        }
        result.stateAfter = Program;
    }
    else if (Program == startState) {
        int start = Delimeters.indexIn(line, startPos);
        int end = start + 1;
        QString lxText;
        if (-1 == start) {
            start = startPos;
            end = Delimeters.indexIn(line, startPos+1);
            lxText = Delimeters.cap();
        }
        else if (start > startPos && line.mid(startPos, start-startPos).trimmed().length()>0) {
            end = start;
            start = startPos;
            lxText = Delimeters.cap();
        }
        else {
            end = start + Delimeters.matchedLength();
            lxText = Delimeters.cap();
        }
        if (-1 == end)
            end = line.length();
        result.start = start;
        while (result.start < end && line.at(result.start).isSpace()) {
            result.start ++;
        }
        result.length = end - result.start;
        if ("//" == lxText)
            result.stateAfter = LineComment;
        else if ("{" == lxText)
            result.stateAfter = BlockComment;
        else if ("'" == lxText)
            result.stateAfter = String;
        else
            result.stateAfter = Program;
    }
    else {
        qFatal("Unknown state");
    }
    return result;
}

void SimplePascalSyntaxAnalizer::processLine
(const QString &line, const State initialState,
 LineProp &lineProp, QPoint &rank, State &endState, QString & previousKeyword,
 bool fullText, uint lineNumber)
{
    State currentState = initialState;
    int currentPos = 0;
    Q_FOREVER {
        Lexem lx = takeLexem(line, currentPos, currentState);
        if (0 == lx.length) {
            currentPos ++;
            continue;
        }
        QString lxText = line.mid(lx.start, lx.length).trimmed();
        LexemType tp = LxTypeName;
        if (String == currentState) {
            tp = LxConstLiteral;
        }
        else if (LineComment == currentState || BlockComment == currentState) {
            tp = LxTypeComment;
        }
        else {
            if (Operators.contains(lxText)) {
                tp = LxTypeOperator;
                if (lxText=="." || lxText==";" || lxText=="(" || lxText == "[") {
                    if ("procedure" == previousKeyword
                            || "function" == previousKeyword
                            || "program" == previousKeyword
                            || "unit" == previousKeyword
                            || "uses" == previousKeyword
                            ) {
                        previousKeyword = "";
                    }
                }
                if ("'" == lxText) {
                    tp = LxConstLiteral;
                }
            }
            else if (Keywords.contains(lxText)) {
                tp = LxTypePrimaryKwd;
                rank += keywordRank(lxText);
                QString kwd = lxText.toLower();
                if ("procedure" == kwd || "function" == kwd || "type" == kwd ||
                        "program" == kwd || "unit" == kwd || "uses" == kwd ||
                        "record" == kwd || "object" == kwd || "class" == kwd)
                {
                    previousKeyword = kwd;
                }
                else if ("var" == kwd || "begin" == kwd || "interface" == kwd
                         || "implementation" == kwd)
                {
                    previousKeyword = "";
                }
                else if ("end" == kwd) {
                    if ("record" == previousKeyword || "object" == previousKeyword || "class" == previousKeyword) {
                        previousKeyword = "type";
                    }
                }
            }
            else if (StandardTypes.contains(lxText) || typeNames_.contains(lxText)) {
                tp = LxNameClass;
            }
            else if (functionNames_.contains(lxText)) {
                tp = LxNameAlg;
            }
            else if (unitNames_.contains(lxText)) {
                tp = LxNameModule;
            }
        }
        if (LxTypeName == tp && previousKeyword.length() > 0) {
            if ("type" == previousKeyword) {
                tp = LxNameClass;
                typeNames_.insert(lxText.toLower());
            }
            else if ("procedure" == previousKeyword || "function" == previousKeyword) {
                tp = LxNameAlg;
                functionNames_.insert(lxText.toLower());
                previousKeyword = "";
            }
            else if ("program" == previousKeyword || "unit" == previousKeyword) {
                tp = LxNameModule;
                unitNames_.insert(lxText.toLower());
                previousKeyword = "";
                if (fullText) {
                    thisUnitName_ = lxText.toLower();
                    int semicolonPos = line.indexOf(";", currentPos+lxText.length());
                    if (-1 != semicolonPos) {
                        addHelperUnitPosition_.valid = true;
                        addHelperUnitPosition_.line = lineNumber;
                        addHelperUnitPosition_.col = uint(semicolonPos);
                        addHelperUnitPosition_.prefix = "; uses ";
                        addHelperUnitPosition_.suffix = "";
                    }
                }
            }
            else if ("uses" == previousKeyword) {
                tp = LxNameModule;
                unitNames_.insert(lxText.toLower());
                if (fullText) {
                    addHelperUnitPosition_.valid = true;
                    addHelperUnitPosition_.line = lineNumber;
                    addHelperUnitPosition_.col = currentPos;
                    addHelperUnitPosition_.prefix = "";
                    addHelperUnitPosition_.suffix = ", ";
                }
            }
        }
        if (LxTypeName == tp && "true" == lxText.toLower()) {
            tp = LxConstBoolTrue;
        }
        if (LxTypeName == tp && "false" == lxText.toLower()) {
            tp = LxConstBoolFalse;
        }
        if (LxTypeName == tp && "nil" == lxText.toLower()) {
            tp = LxTypeConstant;
        }
        if (LxTypeName == tp) {
            bool ok;
            lxText.toLongLong(&ok);
            if (ok) {
                tp = LxConstInteger;
            }
            else {
                lxText.toDouble(&ok);
                if (ok) {
                    tp = LxConstReal;
                }
            }
        }
        for (int i=lx.start; i<lx.start+lx.length; i++) lineProp[i] = tp;
        currentPos = lx.start + lx.length;
        currentState = lx.stateAfter;
        if (currentPos >= line.length())
            break;
    }
    endState = currentState;
}

Analizer::TextAppend
SimplePascalSyntaxAnalizer::closingBracketSuggestion
(int lineNo, const QStringList & lines)
const
{
    State currentState = Program;
    QString currentKeyword;
    QString headKeyword;
    QStack<QString> headKeywordsStack;
    int beginCount = 0;
    int beginLineNo = -1;

    // Count begin/end before end of line
    for (int i=0; i<=lineNo; i++) {
        const QString & line = lines[i];
        int currentPos = 0;
        Q_FOREVER {
            Lexem lx = takeLexem(line, currentPos, currentState);
            if (0 == lx.length) {
                currentPos ++;
                continue;
            }
            QString lxText = line.mid(lx.start, lx.length).trimmed().toLower();
            if (Program == currentState && Keywords.contains(lxText)) {
                currentKeyword = lxText;
                if ("procedure" == currentKeyword
                        || "function" == currentKeyword
                        || "do" == currentKeyword
                        || "then" == currentKeyword
                        || "else" == currentKeyword
                        )
                    headKeyword = currentKeyword;
                if ("begin" == currentKeyword) {
                    beginCount ++;
                    beginLineNo = i;
                    headKeywordsStack.push_back(headKeyword);
                    headKeyword = "";
                }
                else if ("end" == currentKeyword) {
                    beginCount --;
                    if (headKeywordsStack.size() > 0)
                        headKeywordsStack.pop_back();
                    headKeyword = "";
                }
            }
            currentPos = lx.start + lx.length;
            currentState = lx.stateAfter;
            if (currentPos >= line.length())
                break;
        }
    }

    // Count begin/end after line
    if (beginCount > 0) {
        for (int i=lineNo+1; i<lines.size(); i++) {
            const QString & line = lines[i];
            int currentPos = 0;
            Q_FOREVER {
                Lexem lx = takeLexem(line, currentPos, currentState);
                if (0 == lx.length) {
                    currentPos ++;
                    continue;
                }
                QString lxText = line.mid(lx.start, lx.length).trimmed().toLower();
                if (Program == currentState && Keywords.contains(lxText)) {
                    currentKeyword = lxText;
                    if ("begin" == currentKeyword)
                        beginCount ++;
                    else if ("end" == currentKeyword)
                        beginCount --;
                }
                currentPos = lx.start + lx.length;
                currentState = lx.stateAfter;
                if (currentPos >= line.length())
                    break;
            }
        }
    }

    Analizer::TextAppend result;
    if (beginCount > 0) {
        // Insert pairing 'end'
        result.first += "\n";
        QString indent;
        const QString & begin = lines[beginLineNo];
        for (int i=0; i<begin.length(); i++) {
            if (begin[i].isSpace()) {
                result.first += ' ';
                indent += ' ';
            }
            else
                break;
        }
        indent += "  ";
        result.first += "end";
        if (headKeywordsStack.size() > 0) {
            if (headKeywordsStack.top().length() > 0) {
                result.first += ";";
            }
            else {
                result.first += ".";
            }
        }
        result.first.prepend(indent);
        result.first += "\n";
        result.second = 2u;
    }
    return result;
}

QPoint SimplePascalSyntaxAnalizer::keywordRank(const QString &keyword)
{
    if ("procedure" == keyword || "function" == keyword) {
        return QPoint(0, 0);
    }
    else if ("var" == keyword) {
        return QPoint(-1, 1);
    }
    else if ("type" == keyword) {
        return QPoint(-1, 1);
    }
    else if ("begin" == keyword) {
        return QPoint(-1, 1);
    }
    else if ("do" == keyword) {
        return QPoint(0, 1);
    }
    else if ("then" == keyword || "else" == keyword) {
        return QPoint(0, 1);
    }
    else if ("end" == keyword) {
        return QPoint(-1, 0);
    }
    else {
        return QPoint(0, 0);
    }
}

QString SimplePascalSyntaxAnalizer::correctCapitalization
(const QString &name, LexemType lxType) const
{
    if (capitalizationHints_.contains(name.toLower())) {
        return capitalizationHints_[name.toLower()];
    }
    QString result;
    if (lxType == LxTypePrimaryKwd || lxType == LxTypeSecondaryKwd) {
        result = name.toLower();
    }
    else if (lxType == LxNameClass) {
        if ((name.toLower().startsWith("t") || name.toLower().startsWith("p"))&& name.length() > 1) {
            result.append(name.at(0).toUpper());
            result.append(name.at(1).toUpper());
            result.append(name.mid(2));
        }
        else {
            result.append(name.at(0).toUpper());
            result.append(name.mid(1).toLower());
        }
    }
    else if (lxType == LxNameAlg) {
        if (name.toLower().startsWith("get") && name.length() > 3) {
            result = "Get" + QString(name.at(3).toUpper()) + name.mid(4);
        }
        else if (name.toLower().startsWith("set") && name.length() > 3) {
            result = "Set" + QString(name.at(3).toUpper()) + name.mid(4);
        }
        else if (name.toLower().startsWith("is") && name.length() > 2) {
            result = "Is" + QString(name.at(2).toUpper()) + name.mid(3);
        }
        else if (name.toLower().startsWith("do") && name.length() > 2) {
            result = "Do" + QString(name.at(2).toUpper()) + name.mid(3);
        }
        else {
            result = name.at(0).toUpper() + name.mid(1);
        }
    }
    else if (lxType == LxNameModule) {
        result.append(name.at(0).toUpper());
        result.append(name.mid(1));
    }
    else {
        result = name;
    }
    return result;
}

} // namespace FpcAnalizer