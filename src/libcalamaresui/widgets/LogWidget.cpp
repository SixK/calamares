#include "LogWidget.h"
#include <QStackedLayout>
#include "utils/Logger.h"
#include <QTextStream>
#include <QThread>
#include <QFile>

namespace Calamares
{

LogThread::LogThread(QObject *parent)
    : QThread(parent) {

}

void LogThread::run()
{
    auto filePath = Logger::logFile();

    qint64 lastPosition = 0;

    while (!QThread::currentThread()->isInterruptionRequested()) {
        auto filePath = Logger::logFile();
        QFile file(filePath);

        qint64 fileSize = file.size();
        // Check whether the file size has changed since last time
        // we read the file.
        if (lastPosition != fileSize && file.open(QFile::ReadOnly | QFile::Text)) {

            // Start reading at the position we ended up last time we read the file.
            file.seek(lastPosition);

            QTextStream in(&file);
            auto chunk = in.readAll();
            qint64 newPosition = in.pos();

            lastPosition = newPosition;

            onLogChunk(chunk);
        }
        QThread::msleep(100);
    }
}

LogWidget::LogWidget(QWidget *parent)
    : QWidget(parent)
    , m_text( new QPlainTextEdit )
    , m_log_thread( this )
{
    auto layout = new QStackedLayout(this);
    setLayout(layout);

    m_text->setReadOnly(true);

    QFont monospaceFont("monospace");
    monospaceFont.setStyleHint(QFont::Monospace);
    m_text->setFont(monospaceFont);

    layout->addWidget(m_text);

    connect(&m_log_thread, &LogThread::onLogChunk, this, &LogWidget::handleLogChunk);

    m_log_thread.start();
}

void
LogWidget::handleLogChunk(const QString &logChunk)
{
    m_text->appendPlainText(logChunk);
    m_text->moveCursor(QTextCursor::End);
    m_text->ensureCursorVisible();
}

}
