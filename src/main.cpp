#include <QByteArray>
#include <QDataStream>
#include <QDebug>
#include <QFile>
#include <QIODevice>
#include <QImage>
#include <QList>
#include <QMap>
#include <QMimeData>
#include <QStandardPaths>
#include <QString>
#include <QStringLiteral>
#include <QUrl>
#include <QCoreApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QUuid>

static const char failed_load_warning[] = "Failed to load history resource. Clipboard history cannot be read.\n";

int cnt = 0;

bool create(QDataStream &data_stream, QTextStream &text_stream)
{
    if (data_stream.atEnd()) {
        return false;
    }

    QString type;
    data_stream >> type;

    if (type == QLatin1String("string")) {
        QString text;
        data_stream >> text;

        text_stream << text << "\n";
        return true;

    } else if (type == QLatin1String("image")) {
        QImage image;
        data_stream >> image;
        if (Q_LIKELY(!image.isNull())) {
            QString id = QUuid::createUuid().toString().remove("{").remove("}").remove("-");
            image.save(id + ".png");
        }
        
        return true;
    } else if (Q_UNLIKELY(type == QLatin1String("url"))) {
        QList<QUrl> urls;
        QMap<QString, QString> meta_data;
        int cut;

        data_stream >> urls;
        data_stream >> meta_data;
        data_stream >> cut;

        for (auto &url : urls) {
            text_stream << url.toString() << "\n";
        }
        return true;

    } 

    return false;
}

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("kde-klipper-history-file-extractor");
    QCoreApplication::setApplicationVersion("1.0.0");
    
    QCommandLineParser parser;
    parser.setApplicationDescription("A extractor for kde plasma klipper history file.");
    parser.addHelpOption();
    parser.addVersionOption();
    
    QCommandLineOption option_history_file("f");
    option_history_file.setValueName("path");
    option_history_file.setDescription("History file path, if this option does not set, program will use default path.");
    parser.addOption(option_history_file);

    QCommandLineOption option_output_file("o");
    option_history_file.setValueName("name");
    option_history_file.setDescription("Output file name, if this option does not set, program will use output.txt.");
    parser.addOption(option_output_file);

    parser.process(app);
    
    QFile history_file;  
    if (!parser.isSet(option_history_file)) {
        history_file.setFileName(
                QStandardPaths::locate(QStandardPaths::GenericDataLocation, QStringLiteral("klipper/history2.lst")));
        
    } else {
        history_file.setFileName(parser.value(option_history_file));
    }

    if (!history_file.exists()) {
        qWarning() << QString("%1History file (%2) did not exist.").arg(failed_load_warning).arg(history_file.fileName());
        history_file.close();
        exit(1);
    }

    if (!history_file.open(QIODevice::ReadOnly)) {
        qWarning() << failed_load_warning << history_file.errorString();
        history_file.close();
        exit(1);
    }

    QDataStream file_stream(&history_file);
    if (file_stream.atEnd()) {
        qWarning() << failed_load_warning << "Error in reading data.";
        history_file.close();
        exit(1);
    }
    
    QFile output_file;
    if (!parser.isSet(option_output_file)) {
        output_file.setFileName("output.txt");
    } else {
        output_file.setFileName(parser.value(option_output_file));
    }
    
    if (!output_file.open(QIODevice::ReadWrite)) {
        qWarning() << "failed to open output file" << output_file.errorString();
        history_file.close();
        output_file.close();
        exit(1);
    }
    
    QTextStream stream(&output_file);

    QByteArray data;
    quint32 crc;
    file_stream >> crc >> data;
    // if (crc32(0, reinterpret_cast<unsigned char *>(data.data(), data.size())) != crc) {
    //     qWarning() << failed_load_warning << ": " << "CRC checksum does not
    //     match";
    // }

    QDataStream history_stream(&data, QIODevice::ReadOnly);

    char *version;
    history_stream >> version;
    delete[] version;

    for (bool has_next = create(history_stream, stream); has_next; has_next = create(history_stream, stream)) {}

    history_file.close();
    return 0;
}
