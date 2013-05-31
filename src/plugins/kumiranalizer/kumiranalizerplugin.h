#ifndef KUMIRANALIZERPLUGIN_H
#define KUMIRANALIZERPLUGIN_H

#include <extensionsystem/kplugin.h>
#include <interfaces/analizerinterface.h>

#include <QtCore/QLocale>
#include <QtCore/QStringList>

namespace KumirAnalizer {

class Analizer;

class KumirAnalizerPlugin
        : public ExtensionSystem::KPlugin
        , public Shared::AnalizerInterface

{
    Q_OBJECT
    Q_INTERFACES(Shared::AnalizerInterface)
    friend struct AnalizerPrivate;
public:
    KumirAnalizerPlugin();
    ~KumirAnalizerPlugin();

    inline bool primaryAlphabetIsLatin() const { return false; }
    inline bool caseInsensitiveGrammatic() const { return false; }
    inline bool supportPartialCompiling() const { return true; }

    int newDocument();
    void dropDocument(int documentId);
    void setSourceDirName(int documentId, const QString &dirPath);
    void setSourceText(int documentId, const QString &text);
    void setHiddenText(int documentId, const QString &text, int baseLine);
    void setHiddenTextBaseLine(int documentId, int baseLine);
    void changeSourceText(int documentId, const QList<Shared::ChangeTextTransaction> & changes);

    QList<Shared::Suggestion> suggestAutoComplete(int documentId, int lineNo, const QString & before, const QString & after) const;
    Shared::TextAppend closingBracketSuggestion(int documentId, int lineNo) const;
    QStringList importModuleSuggestion(int documentId, int lineNo) const;

    std::string rawSourceData(int documentId) const;
    Shared::LineProp lineProp(int documentId, int lineNo, const QString & text) const;
    QList<Shared::Error> errors(int documentId) const;
    QList<Shared::LineProp> lineProperties(int documentId) const;
    QList<QPoint> lineRanks(int documentId) const;
    QStringList imports(int documentId) const;
    const AST::Data * abstractSyntaxTree(int documentId) const;
    QStringList algorhitmsAvailableFor(int documentId, int lineNo) const;
    QStringList globalsAvailableFor(int documentId, int lineNo) const;
    QStringList localsAvailableFor(int documentId, int lineNo) const;
    inline virtual QList<QRegExp> supportedFileNamePattern() const {
        return QList<QRegExp>()
                << QRegExp("*.kum",Qt::CaseInsensitive,QRegExp::Wildcard);
    }
    QString createImportStatementLine(const QString &importName) const;

protected:
    QString initialize(const QStringList &arguments);
    void start();
    void stop();
private:
    QVector<Analizer*> analizers_;
    bool teacherMode_;
};

}

#endif
