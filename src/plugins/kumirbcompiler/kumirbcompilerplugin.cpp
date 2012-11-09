#include "kumirbcompilerplugin.h"

#include <QtCore>
#include "kumiranalizer/analizer.h"
#include "kumiranalizer/kumiranalizerplugin.h"
#include "interfaces/generatorinterface.h"
#include "dataformats/bytecode.h"
#include "dataformats/kumfile.h"

#include <iostream>

using namespace KumirBytecodeCompiler;
using namespace KumirAnalizer;

typedef Shared::GeneratorInterface::DebugLevel DebugLevel;

KumirBytecodeCompilerPlugin::KumirBytecodeCompilerPlugin()
{


}


KumirBytecodeCompilerPlugin::~KumirBytecodeCompilerPlugin()
{

}


QString KumirBytecodeCompilerPlugin::initialize(const QStringList &arguments)
{
    Q_UNUSED(arguments)
    m_analizer = qobject_cast<Shared::AnalizerInterface*>(myDependency("KumirAnalizer"));
    if (m_analizer)
        return "";
    else
        return "Could not find KumirAnalizer interface";
}

#include <iostream>


void KumirBytecodeCompilerPlugin::start()
{
    QString filename;
    QString encoding = "";
    DebugLevel debugLevel = Shared::GeneratorInterface::LinesOnly;
    for (int i=1; i<qApp->argc(); i++) {
        const QString arg = qApp->arguments()[i];
        if ( !arg.startsWith("-") && !arg.startsWith("[") && arg.endsWith(".kum")) {
            filename = arg;
        }
        if ( arg.startsWith("--encoding=") ) {
            encoding = arg.mid(11).toUpper();
        }
        if ( arg.toLower().startsWith("--debuglevel=") ) {
            int level = arg.mid(13).toInt();
            if (level==0)
                debugLevel = Shared::GeneratorInterface::NoDebug;
            else if (level==1)
                debugLevel = Shared::GeneratorInterface::LinesOnly;
            else
                debugLevel = Shared::GeneratorInterface::LinesAndVariables;
        }
    }
    if (!filename.isEmpty() && !qApp->arguments().contains("-h") && !qApp->arguments().contains("-help") && !qApp->arguments().contains("--help") && !qApp->arguments().contains("/?")) {
        filename = QFileInfo(filename).absoluteFilePath();
        QFile f(filename);
        if (f.open(QIODevice::ReadOnly)) {            
            QDataStream ts(&f);
            KumFile::Data kumFile;
            kumFile.sourceEncoding = encoding;
            ts >> kumFile;
            f.close();

            int id = m_analizer->newDocument();
            m_analizer->setSourceText(id, kumFile.visibleText);
            if (kumFile.hasHiddenText) {
                m_analizer->setHiddenText(id, kumFile.hiddenText, -1);
            }
            QList<Shared::Error> errors = m_analizer->errors(id);
            const AST::Data * ast = m_analizer->abstractSyntaxTree(id);
            const QString baseName = QFileInfo(filename).baseName();
            const QString outFileName = QFileInfo(filename).dir().absoluteFilePath(
                        baseName+".dump.json");
            QFile ff(outFileName);
            if (qApp->arguments().contains("-J") && ff.open(QIODevice::WriteOnly|QIODevice::Text)) {
                QTextStream ots(&ff);
                ots.setCodec("UTF-8");
                ots.setGenerateByteOrderMark(true);
                QString modJs = ast->dump();
                modJs.replace("\t", "  ");
                ots << modJs;
                ff.close();;
            }
            for (int i=0; i<errors.size(); i++) {
                Shared::Error e = errors[i];
                std::cerr << "Error: " <<
                             QFileInfo(filename).fileName().toLocal8Bit().data() <<
                             ":" << e.line+1 <<
                             ":" << e.start+1 << "-" << e.start+e.len <<
                             ": " << e.code.toLocal8Bit().data() << std::endl;
            }
            Shared::GeneratorInterface * generator =
                    qobject_cast<Shared::GeneratorInterface*>(myDependency("Generator"));
            Q_CHECK_PTR(generator);

            QString outKodFileName = QFileInfo(filename).dir().absoluteFilePath(baseName+".kod");
            foreach (QString arg, qApp->arguments()) {
                if (arg.startsWith("-o=")) {
                    outKodFileName = arg.mid(3);
                }
            }

            if (!outKodFileName.endsWith(".kod"))
                outKodFileName += ".kod";

            QFile binOut(outKodFileName);
            QPair<QString,QString> res = generator->generateExecuable(ast, &binOut, debugLevel);
            if (!res.first.isEmpty()) {
                std::cerr << "Error generating execuable: " << res.first.toStdString() << std::endl;
            }
            if (res.second==MIME_BYTECODE_BINARY && QFile::exists(outKodFileName)) {
                QFile::Permissions ps = binOut.permissions();
                ps |= QFile::ExeGroup | QFile::ExeOwner | QFile::ExeOther;
                QFile::setPermissions(outKodFileName, ps);
                if (qApp->arguments().contains("-S")) {
                    binOut.open(QIODevice::ReadOnly);
                    QDataStream ds(&binOut);
                    Bytecode::Data data;
                    ds >> data;
                    binOut.close();
                    const QString SfileName = outKodFileName.mid(0,outKodFileName.length()-4)+".ks";
                    QFile Sfile(SfileName);
                    if (Sfile.open(QIODevice::WriteOnly)) {
                        QTextStream ts(&Sfile);
                        ts << data;
                        Sfile.close();
                    }
                }
            }
            qApp->setProperty("returnCode", errors.isEmpty() && res.first.isEmpty()? 0 : 1);
        }
    }
    else {
        std::cerr << "Usage:" << std::endl;
        std::cerr << "\t" << qApp->argv()[0] << " [-J] [-S] [-o=OUTFILE.kod] FILENAME.kum" << std::endl;
        std::cerr << std::endl;
        std::cerr << "\t-J\t\tIf present, generates internal AST in JSON file" << std::endl;
        std::cerr << "\t-S\t\tIf present, generates human-readable form of code" << std::endl;
        std::cerr << "\t-o OUTFILE.kod\tOutput file name (default: FILENAME.kod)" << std::endl;
        std::cerr << "\tFILENAME.kum\tKumir program input file name" << std::endl;
        qApp->setProperty("returnCode", 127);
    }
}

void KumirBytecodeCompilerPlugin::stop()
{

}


Q_EXPORT_PLUGIN(KumirBytecodeCompilerPlugin)
