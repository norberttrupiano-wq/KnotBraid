#include "MainWindow.h"
#include "BraidBoardWidget.h"
#include "ui_MainWindow.h"

#include <QAbstractItemView>
#include <QAction>
#include <QApplication>
#include <QBrush>
#include <QComboBox>
#include <QColor>
#include <QCoreApplication>
#include <QCryptographicHash>
#include <QDate>
#include <QDesktopServices>
#include <QDir>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QFont>
#include <QFormLayout>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QKeySequence>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QMenu>
#include <QPushButton>
#include <QProcess>
#include <QRegularExpression>
#include <QSettings>
#include <QSignalBlocker>
#include <QShortcut>
#include <QSizePolicy>
#include <QStatusBar>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QTimer>
#include <QToolButton>
#include <QUrl>
#include <QUrlQuery>
#include <QVector>
#include <QVBoxLayout>
#include <QWidget>

namespace {

static constexpr int kTrialPeriodDays = 15;
static const char *kRegistrationInboxEmail = "CHANGE_ME@example.com";
static const char *kRegistrationSalt = "KeyGenerator::SidoineArifene::v1";

static QSettings appSettings()
{
    return QSettings(QSettings::NativeFormat,
                     QSettings::UserScope,
                     QStringLiteral("LogiBraiding"),
                     QStringLiteral("LogiBraiding"));
}


static QString referenceDrawingSettingKey(const QString &patternCode)
{
    return QStringLiteral("braids/%1/reference_drawing_path").arg(patternCode);
}
static QString uiLanguageSettingKey()
{
    return QStringLiteral("ui/language");
}

static QVector<QColor> paletteForPreset(const QString &presetCode)
{
    if (presetCode == QStringLiteral("fr")) {
        return {QColor("#0055A4"), QColor("#FFFFFF"), QColor("#EF4135")};
    }
    if (presetCode == QStringLiteral("it")) {
        return {QColor("#009246"), QColor("#FFFFFF"), QColor("#CE2B37")};
    }
    if (presetCode == QStringLiteral("de")) {
        return {QColor("#000000"), QColor("#DD0000"), QColor("#FFCE00")};
    }
    if (presetCode == QStringLiteral("be")) {
        return {QColor("#000000"), QColor("#FFD90C"), QColor("#EF3340")};
    }
    if (presetCode == QStringLiteral("ro")) {
        return {QColor("#002B7F"), QColor("#FCD116"), QColor("#CE1126")};
    }
    if (presetCode == QStringLiteral("pt")) {
        return {QColor("#006600"), QColor("#FF0000"), QColor("#FFCC00")};
    }

    return {};
}

static QString canonicalEmail(const QString &email)
{
    return email.trimmed().toLower();
}

static bool isValidEmail(const QString &email)
{
    static const QRegularExpression re(
        QStringLiteral(R"(^[A-Z0-9._%+\-]+@[A-Z0-9.\-]+\.[A-Z]{2,}$)"),
        QRegularExpression::CaseInsensitiveOption);

    return re.match(canonicalEmail(email)).hasMatch();
}

static QString authorIdFromEmail(const QString &email)
{
    const QString normalized = canonicalEmail(email);
    if (normalized.isEmpty()) {
        return QString();
    }

    const QByteArray digest =
        QCryptographicHash::hash(normalized.toUtf8(), QCryptographicHash::Sha256).toHex().toUpper();
    return QString::fromLatin1(digest.left(12));
}

static QString normalizeUnlockKey(const QString &key)
{
    QString out;
    out.reserve(key.size());
    for (const QChar ch : key.toUpper()) {
        if (ch.isDigit() || (ch >= QLatin1Char('A') && ch <= QLatin1Char('Z'))) {
            out.push_back(ch);
        }
    }
    return out;
}

static QString generateUnlockKeyForEmail(const QString &email)
{
    const QString normalizedEmail = canonicalEmail(email);
    const QByteArray data = (normalizedEmail + QString::fromLatin1(kRegistrationSalt)).toUtf8();
    const QByteArray hash = QCryptographicHash::hash(data, QCryptographicHash::Sha256).toHex();

    QString key;
    for (int i = 0; i < 16; ++i) {
        key.append(QChar::fromLatin1(hash[i]));
        if (((i + 1) % 4) == 0 && i != 15) {
            key.append(QLatin1Char('-'));
        }
    }

    return key.toUpper();
}

static bool verifyUnlockKey(const QString &email, const QString &key)
{
    if (!isValidEmail(email)) {
        return false;
    }

    const QString expected = normalizeUnlockKey(generateUnlockKeyForEmail(email));
    const QString provided = normalizeUnlockKey(key);
    return !provided.isEmpty() && (expected == provided);
}

static QDate ensureTrialStartDate(QSettings &s)
{
    const QString key = QStringLiteral("registration/trial_start_utc");
    QDate d = QDate::fromString(s.value(key).toString(), Qt::ISODate);
    const QDate todayUtc = QDate::currentDate();

    if (!d.isValid() || d > todayUtc) {
        d = todayUtc;
        s.setValue(key, d.toString(Qt::ISODate));
    }

    return d;
}

static int trialDaysRemaining(QSettings &s)
{
    const QDate start = ensureTrialStartDate(s);
    const int elapsedDays = start.daysTo(QDate::currentDate());
    return kTrialPeriodDays - elapsedDays;
}

static bool isRegistrationUnlocked(QSettings &s)
{
    const QString email = canonicalEmail(s.value(QStringLiteral("registration/email")).toString());
    const bool unlocked = s.value(QStringLiteral("registration/unlocked"), false).toBool();
    if (unlocked && isValidEmail(email)) {
        return true;
    }

    const QString legacyKey = s.value(QStringLiteral("registration/unlock_key")).toString().trimmed();
    if (isValidEmail(email) && !legacyKey.isEmpty() && verifyUnlockKey(email, legacyKey)) {
        s.setValue(QStringLiteral("registration/unlocked"), true);
        s.remove(QStringLiteral("registration/unlock_key"));
        s.sync();
        return true;
    }

    return false;
}

} // namespace

const QVector<MainWindow::Move3082> MainWindow::kMoves3082 = {
    {1, 1, 3, 34, 32, u'P', QString(), QStringLiteral(":/docs/3082-01.bmp")},
    {2, 2, 1, 31, 35, u'Q', QString(), QStringLiteral(":/docs/3082-02.bmp")},
    {3, 3, 2, 36, 30, u'P', QString(), QStringLiteral(":/docs/3082-03.bmp")},
    {4, 4, 1, 29, 37, u'Q', QString(), QStringLiteral(":/docs/3082-04.bmp")},
    {5, 5, 1, 1, 25, u'Q', QString(), QStringLiteral(":/docs/3082-05.bmp")},
    {6, 6, 1, 24, 2, u'P', QString(), QStringLiteral(":/docs/3082-06.bmp")},
    {7, 7, 2, 3, 23, u'Q', QString(), QStringLiteral(":/docs/3082-07.bmp")},
    {8, 8, 1, 22, 4, u'P', QString(), QStringLiteral(":/docs/3082-08.bmp")},
    {9, 9, 3, 2, 40, u'P', QString(), QStringLiteral(":/docs/3082-09.bmp")},
    {10, 10, 1, 39, 3, u'Q', QString(), QStringLiteral(":/docs/3082-10.bmp")},
    {11, 11, 2, 4, 38, u'P', QString(), QStringLiteral(":/docs/3082-11.bmp")},
    {12, 12, 1, 37, 5, u'Q', QString(), QStringLiteral(":/docs/3082-12.bmp")},
    {13, 13, 1, 9, 33, u'Q', QString(), QStringLiteral(":/docs/3082-13.bmp")},
    {14, 14, 1, 32, 10, u'P', QString(), QStringLiteral(":/docs/3082-14.bmp")},
    {15, 15, 2, 11, 31, u'Q', QString(), QStringLiteral(":/docs/3082-15.bmp")},
    {16, 16, 1, 30, 12, u'P', QStringLiteral("Erreur dans le Ashley"), QStringLiteral(":/docs/3082-16.bmp")},
    {17, 17, 3, 10, 8, u'P', QString(), QStringLiteral(":/docs/3082-17.bmp")},
    {18, 18, 1, 7, 11, u'Q', QString(), QStringLiteral(":/docs/3082-18.bmp")},
    {19, 19, 2, 12, 6, u'P', QString(), QStringLiteral(":/docs/3082-19.bmp")},
    {20, 20, 1, 5, 13, u'Q', QString(), QStringLiteral(":/docs/3082-20.bmp")},
    {21, 21, 1, 17, 1, u'Q', QString(), QStringLiteral(":/docs/3082-21.bmp")},
    {22, 22, 2, 40, 18, u'P', QString(), QStringLiteral(":/docs/3082-22.bmp")},
    {23, 23, 1, 19, 39, u'Q', QString(), QStringLiteral(":/docs/3082-23.bmp")},
    {24, 24, 2, 38, 20, u'P', QString(), QStringLiteral(":/docs/3082-24.bmp")},
    {25, 25, 2, 18, 16, u'P', QString(), QStringLiteral(":/docs/3082-25.bmp")},
    {26, 26, 1, 15, 19, u'Q', QString(), QStringLiteral(":/docs/3082-26.bmp")},
    {27, 27, 2, 20, 14, u'P', QString(), QStringLiteral(":/docs/3082-27.bmp")},
    {28, 28, 1, 13, 21, u'Q', QString(), QStringLiteral(":/docs/3082-28.bmp")},
    {29, 29, 2, 25, 9, u'Q', QString(), QStringLiteral(":/docs/3082-29.bmp")},
    {30, 30, 1, 8, 26, u'P', QString(), QStringLiteral(":/docs/3082-30.bmp")},
    {31, 31, 2, 27, 7, u'Q', QString(), QStringLiteral(":/docs/3082-31.bmp")},
    {32, 32, 1, 6, 28, u'P', QString(), QStringLiteral(":/docs/3082-32.bmp")},
    {33, 33, 2, 26, 24, u'P', QString(), QStringLiteral(":/docs/3082-33.bmp")},
    {34, 34, 2, 23, 27, u'Q', QString(), QStringLiteral(":/docs/3082-34.bmp")},
    {35, 35, 1, 28, 22, u'P', QString(), QStringLiteral(":/docs/3082-35.bmp")},
    {36, 36, 2, 21, 29, u'Q', QString(), QStringLiteral(":/docs/3082-36.bmp")},
    {37, 37, 1, 33, 17, u'Q', QString(), QStringLiteral(":/docs/3082-37.bmp")},
    {38, 38, 1, 16, 34, u'P', QString(), QStringLiteral(":/docs/3082-38.bmp")},
    {39, 39, 2, 35, 15, u'Q', QString(), QStringLiteral(":/docs/3082-39.bmp")},
    {40, 40, 1, 14, 36, u'P', QString(), QStringLiteral(":/docs/3082-40.bmp")}
};

const QVector<MainWindow::Move3082> MainWindow::kMovesPentalpha78 = {
    {1, 1, 3, 42, 40, u'P', QString(), QString()},
    {2, 2, 1, 39, 43, u'Q', QString(), QString()},
    {3, 3, 2, 44, 38, u'P', QString(), QString()},
    {4, 4, 1, 37, 45, u'Q', QString(), QString()},
    {5, 5, 2, 46, 36, u'P', QString(), QString()},
    {6, 6, 1, 1, 31, u'Q', QString(), QString()},
    {7, 7, 1, 30, 2, u'P', QString(), QString()},
    {8, 8, 2, 3, 29, u'Q', QString(), QString()},
    {9, 9, 1, 28, 4, u'P', QString(), QString()},
    {10, 10, 2, 5, 27, u'Q', QString(), QString()},
    {11, 11, 2, 32, 30, u'P', QString(), QString()},
    {12, 12, 2, 29, 33, u'Q', QString(), QString()},
    {13, 13, 1, 34, 28, u'P', QString(), QString()},
    {14, 14, 2, 27, 35, u'Q', QString(), QString()},
    {15, 15, 1, 36, 26, u'P', QString(), QString()},
    {16, 16, 1, 41, 21, u'Q', QString(), QString()},
    {17, 17, 1, 20, 42, u'P', QString(), QString()},
    {18, 18, 2, 43, 19, u'Q', QString(), QString()},
    {19, 19, 1, 18, 44, u'P', QString(), QString()},
    {20, 20, 2, 45, 17, u'Q', QString(), QString()},
    {21, 21, 2, 22, 20, u'P', QString(), QString()},
    {22, 22, 2, 19, 23, u'Q', QString(), QString()},
    {23, 23, 1, 24, 18, u'P', QString(), QString()},
    {24, 24, 2, 17, 25, u'Q', QString(), QString()},
    {25, 25, 1, 26, 16, u'P', QString(), QString()},
    {26, 26, 1, 31, 11, u'Q', QString(), QString()},
    {27, 27, 1, 10, 32, u'P', QString(), QString()},
    {28, 28, 2, 33, 9, u'Q', QString(), QString()},
    {29, 29, 1, 8, 34, u'P', QString(), QString()},
    {30, 30, 2, 35, 7, u'Q', QString(), QString()},
    {31, 31, 2, 12, 10, u'P', QString(), QString()},
    {32, 32, 2, 9, 13, u'Q', QString(), QString()},
    {33, 33, 1, 14, 8, u'P', QString(), QString()},
    {34, 34, 2, 7, 15, u'Q', QString(), QString()},
    {35, 35, 1, 16, 6, u'P', QString(), QString()},
    {36, 36, 1, 21, 1, u'Q', QString(), QString()},
    {37, 37, 2, 50, 22, u'P', QString(), QString()},
    {38, 38, 1, 23, 49, u'Q', QString(), QString()},
    {39, 39, 2, 48, 24, u'P', QString(), QString()},
    {40, 40, 1, 25, 47, u'Q', QString(), QString()},
    {41, 41, 3, 2, 50, u'P', QString(), QString()},
    {42, 42, 2, 49, 3, u'Q', QString(), QString()},
    {43, 43, 1, 4, 48, u'P', QString(), QString()},
    {44, 44, 2, 47, 5, u'Q', QString(), QString()},
    {45, 45, 1, 6, 46, u'P', QString(), QString()},
    {46, 46, 2, 11, 41, u'Q', QString(), QString()},
    {47, 47, 1, 40, 12, u'P', QString(), QString()},
    {48, 48, 2, 13, 39, u'Q', QString(), QString()},
    {49, 49, 1, 38, 14, u'P', QString(), QString()},
    {50, 50, 2, 15, 37, u'Q', QString(), QString()},
};

const QVector<MainWindow::Move3082> MainWindow::kMovesRoseDesVents97 = {
    {1, 1, 3, 58, 56, u'P', QString(), QString()},
    {2, 2, 1, 55, 59, u'Q', QString(), QString()},
    {3, 3, 2, 60, 54, u'P', QString(), QString()},
    {4, 4, 1, 53, 61, u'Q', QString(), QString()},
    {5, 5, 1, 1, 41, u'Q', QString(), QString()},
    {6, 6, 1, 40, 2, u'P', QString(), QString()},
    {7, 7, 2, 3, 39, u'Q', QString(), QString()},
    {8, 8, 1, 38, 4, u'P', QString(), QString()},
    {9, 9, 3, 2, 64, u'P', QString(), QString()},
    {10, 10, 1, 63, 3, u'Q', QString(), QString()},
    {11, 11, 2, 4, 62, u'P', QString(), QString()},
    {12, 12, 1, 61, 5, u'Q', QString(), QString()},
    {13, 13, 1, 9, 49, u'Q', QString(), QString()},
    {14, 14, 1, 48, 10, u'P', QString(), QString()},
    {15, 15, 2, 11, 47, u'Q', QString(), QString()},
    {16, 16, 1, 46, 12, u'P', QString(), QString()},
    {17, 17, 3, 10, 8, u'P', QString(), QString()},
    {18, 18, 1, 7, 11, u'Q', QString(), QString()},
    {19, 19, 2, 12, 6, u'P', QString(), QString()},
    {20, 20, 1, 5, 13, u'Q', QString(), QString()},
    {21, 21, 1, 17, 57, u'Q', QString(), QString()},
    {22, 22, 1, 56, 18, u'P', QString(), QString()},
    {23, 23, 2, 19, 55, u'Q', QString(), QString()},
    {24, 24, 1, 54, 20, u'P', QString(), QString()},
    {25, 25, 3, 18, 16, u'P', QString(), QString()},
    {26, 26, 1, 15, 19, u'Q', QString(), QString()},
    {27, 27, 2, 20, 14, u'P', QString(), QString()},
    {28, 28, 1, 13, 21, u'Q', QString(), QString()},
    {29, 29, 1, 25, 1, u'Q', QString(), QString()},
    {30, 30, 1, 64, 26, u'P', QString(), QString()},
    {31, 31, 2, 27, 63, u'Q', QString(), QString()},
    {32, 32, 1, 62, 28, u'P', QString(), QString()},
    {33, 33, 3, 26, 24, u'P', QString(), QString()},
    {34, 34, 1, 23, 27, u'Q', QString(), QString()},
    {35, 35, 2, 28, 22, u'P', QString(), QString()},
    {36, 36, 1, 21, 29, u'Q', QString(), QString()},
    {37, 37, 1, 33, 9, u'Q', QString(), QString()},
    {38, 38, 2, 8, 34, u'P', QString(), QString()},
    {39, 39, 1, 35, 7, u'Q', QString(), QString()},
    {40, 40, 2, 6, 36, u'P', QString(), QString()},
    {41, 41, 2, 34, 32, u'P', QString(), QString()},
    {42, 42, 1, 31, 35, u'Q', QString(), QString()},
    {43, 43, 2, 36, 30, u'P', QString(), QString()},
    {44, 44, 1, 29, 37, u'Q', QString(), QString()},
    {45, 45, 1, 41, 17, u'Q', QString(), QString()},
    {46, 46, 2, 16, 42, u'P', QString(), QString()},
    {47, 47, 1, 43, 15, u'Q', QString(), QString()},
    {48, 48, 2, 14, 44, u'P', QString(), QString()},
    {49, 49, 2, 42, 40, u'P', QString(), QString()},
    {50, 50, 1, 39, 43, u'Q', QString(), QString()},
    {51, 51, 2, 44, 38, u'P', QString(), QString()},
    {52, 52, 1, 37, 45, u'Q', QString(), QString()},
    {53, 53, 2, 49, 25, u'Q', QString(), QString()},
    {54, 54, 1, 24, 50, u'P', QString(), QString()},
    {55, 55, 2, 51, 23, u'Q', QString(), QString()},
    {56, 56, 1, 22, 52, u'P', QString(), QString()},
    {57, 57, 2, 50, 48, u'P', QString(), QString()},
    {58, 58, 2, 47, 51, u'Q', QString(), QString()},
    {59, 59, 1, 52, 46, u'P', QString(), QString()},
    {60, 60, 2, 45, 53, u'Q', QString(), QString()},
    {61, 61, 1, 57, 33, u'Q', QString(), QString()},
    {62, 62, 1, 32, 58, u'P', QString(), QString()},
    {63, 63, 2, 59, 31, u'Q', QString(), QString()},
    {64, 64, 1, 30, 60, u'P', QString(), QString()},
};

const QVector<MainWindow::Move3082> &MainWindow::currentMoveSet() const
{
    if (m_selectedBraidCode == QStringLiteral("3082_2_78") && !kMovesPentalpha78.isEmpty()) {
        return kMovesPentalpha78;
    }

    if (m_selectedBraidCode == QStringLiteral("3082_3_rose97") && !kMovesRoseDesVents97.isEmpty()) {
        return kMovesRoseDesVents97;
    }

    return kMoves3082;
}
QString MainWindow::currentBraidTitle() const
{
    if (m_selectedBraidCode == QStringLiteral("3082")) {
        return QStringLiteral("ABoK #3082 - Pentalpha a 61 brins");
    }

    if (m_selectedBraidCode == QStringLiteral("3082_2_78")) {
        return QStringLiteral("ABoK #3082-2 - Pentalpha a 78 brins");
    }

    if (m_selectedBraidCode == QStringLiteral("3082_3_rose97")) {
        return QStringLiteral("ABoK #3082-3 - Rose des vents a 97 brins");
    }

    return QStringLiteral("ABoK #%1").arg(m_selectedBraidCode);
}
void MainWindow::reloadCurrentBraid()
{
    const auto &moves = currentMoveSet();

    int maxCase = 0;
    for (const auto &move : moves) {
        maxCase = qMax(maxCase, move.boardCase);
        maxCase = qMax(maxCase, move.fromPeg);
        maxCase = qMax(maxCase, move.toPeg);
    }
    if (maxCase <= 0) {
        maxCase = 40;
    }

    QVector<int> capacities(maxCase, 0);
    QVector<int> routeNext(maxCase, -1);

    for (const auto &move : moves) {
        const int idx = move.boardCase - 1;
        if (idx >= 0 && idx < capacities.size()) {
            capacities[idx] = qMax(capacities[idx], move.strands);
        }

        const int fromIdx = move.fromPeg - 1;
        const int toIdx = move.toPeg - 1;
        if (fromIdx >= 0 && fromIdx < routeNext.size() && toIdx >= 0 && toIdx < routeNext.size()) {
            routeNext[fromIdx] = toIdx;
        }
    }

    m_currentMoveIndex = 0;
    if (m_boardWidget) {
        m_boardWidget->setColorProfileKey(m_selectedBraidCode);
        m_boardWidget->initializeCases(capacities);
        m_boardWidget->setCaseRouteMap(routeNext);
        m_boardWidget->setPendingMove(-1, -1);

        QSettings s = appSettings();
        const QString drawingPath = s.value(referenceDrawingSettingKey(m_selectedBraidCode)).toString();
        if (drawingPath.isEmpty()) {
            m_boardWidget->clearReferenceImage();
        } else {
            QString error;
            if (!m_boardWidget->loadReferenceImage(drawingPath, &error)) {
                m_boardWidget->clearReferenceImage();
            }
        }
    }

    setWindowTitle(QStringLiteral("LogiBraiding - %1").arg(currentBraidTitle()));
}
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    setupInterface();
    configureMovesTable();

    auto *registrationShortcut = new QShortcut(QKeySequence(QStringLiteral("Ctrl+I")), this);
    connect(registrationShortcut, &QShortcut::activated, this, [this]() {
        if (openRegistrationDialog(false)) {
            statusBar()->showMessage(
                QStringLiteral("Inscription active. AuthorID: %1").arg(currentAuthorId()),
                6000);
        }
        updateView();
    });

    if (!ensureTrialAccess()) {
        QTimer::singleShot(0, qApp, &QCoreApplication::quit);
        return;
    }

    updateView();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupInterface()
{
    ui->setupUi(this);

    m_titleLabel = ui->titleLabel;
    m_progressLabel = ui->progressLabel;
    m_moveLabel = ui->moveLabel;
    m_detailLabel = ui->detailLabel;
    m_noteLabel = ui->noteLabel;
    m_doneButton = ui->doneButton;
    m_restartButton = ui->restartButton;
    m_movesTable = ui->movesTable;

    auto *topWidget = new QWidget(this);
    auto *topLayout = new QHBoxLayout(topWidget);
    topLayout->setContentsMargins(8, 4, 8, 4);
    topLayout->setSpacing(8);

    m_braidSelector = new QComboBox(topWidget);
    m_braidSelector->setMinimumWidth(290);
    m_braidSelector->setToolTip(QStringLiteral("Selection de tresse ABoK"));
    m_braidSelector->addItem(QStringLiteral("ABoK #3082 - Pentalpha a 61 brins"), QStringLiteral("3082"));
    m_braidSelector->addItem(QStringLiteral("ABoK #3082-2 - Pentalpha a 78 brins"), QStringLiteral("3082_2_78"));
    m_braidSelector->addItem(QStringLiteral("ABoK #3082-3 - Rose des vents a 97 brins"), QStringLiteral("3082_3_rose97"));

    m_colorModeSelector = new QComboBox(topWidget);
    m_colorModeSelector->setToolTip(QStringLiteral("Mode de coloriage"));
    m_colorModeSelector->addItem(QStringLiteral("Parcours"), QStringLiteral("orbit"));
    m_colorModeSelector->addItem(QStringLiteral("Alternance"), QStringLiteral("origin_case"));
    m_colorModeSelector->setMinimumWidth(130);

    m_palettePresetSelector = new QComboBox(topWidget);
    m_palettePresetSelector->setToolTip(QStringLiteral("Palette rapide pour l'alternance"));
    m_palettePresetSelector->addItem(QStringLiteral("Palette: Libre"), QStringLiteral("none"));
    m_palettePresetSelector->addItem(QStringLiteral("France (Bleu Blanc Rouge)"), QStringLiteral("fr"));
    m_palettePresetSelector->addItem(QStringLiteral("Italie (Vert Blanc Rouge)"), QStringLiteral("it"));
    m_palettePresetSelector->addItem(QStringLiteral("Allemagne (Noir Rouge Jaune)"), QStringLiteral("de"));
    m_palettePresetSelector->addItem(QStringLiteral("Belgique (Noir Jaune Rouge)"), QStringLiteral("be"));
    m_palettePresetSelector->addItem(QStringLiteral("Roumanie (Bleu Jaune Rouge)"), QStringLiteral("ro"));
    m_palettePresetSelector->addItem(QStringLiteral("Portugal (Vert Rouge Jaune)"), QStringLiteral("pt"));
    m_palettePresetSelector->setMinimumWidth(220);

    m_compactLineLabel = new QLabel(topWidget);
    m_compactLineLabel->setTextFormat(Qt::RichText);
    m_compactLineLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    m_compactLineLabel->setToolTip(QStringLiteral("Ctrl+I : inscription / AuthorID"));
    QFont compactFont = m_compactLineLabel->font();
    compactFont.setPointSize(compactFont.pointSize() + 2);
    compactFont.setBold(true);
    m_compactLineLabel->setFont(compactFont);

    m_loadDrawingButton = new QToolButton(topWidget);
    m_loadDrawingButton->setText(QStringLiteral("Dessin"));
    m_loadDrawingButton->setToolTip(QStringLiteral("Charger un dessin de reference"));
    m_loadDrawingButton->setMinimumWidth(62);

    m_openDocButton = new QToolButton(topWidget);
    m_openDocButton->setText(QStringLiteral("Doc"));
    m_openDocButton->setToolTip(QStringLiteral("Ouvrir Solid Sinnet variations.pdf"));
    m_openDocButton->setMinimumWidth(44);

    m_mainMenuButton = new QToolButton(topWidget);
    m_mainMenuButton->setText(QStringLiteral("Menu"));
    m_mainMenuButton->setToolTip(QStringLiteral("Langues / Aide / A propos"));
    m_mainMenuButton->setPopupMode(QToolButton::InstantPopup);
    m_mainMenuButton->setMinimumWidth(60);

    auto *menu = new QMenu(m_mainMenuButton);
    auto *langMenu = menu->addMenu(QStringLiteral("Langues"));
    QAction *autoLang = langMenu->addAction(QStringLiteral("Systeme (auto)"));
    QAction *frLang = langMenu->addAction(QStringLiteral("Francais"));
    QAction *enLang = langMenu->addAction(QStringLiteral("English"));
    connect(autoLang, &QAction::triggered, this, [this]() { applyUiLanguage(QStringLiteral("auto")); });
    connect(frLang, &QAction::triggered, this, [this]() { applyUiLanguage(QStringLiteral("fr_FR")); });
    connect(enLang, &QAction::triggered, this, [this]() { applyUiLanguage(QStringLiteral("en_US")); });

    auto *helpMenu = menu->addMenu(QStringLiteral("Aide"));
    QAction *quickHelp = helpMenu->addAction(QStringLiteral("Aide rapide"));
    QAction *openGuide = helpMenu->addAction(QStringLiteral("Ouvrir le guide Solid Sinnet"));
    connect(quickHelp, &QAction::triggered, this, &MainWindow::openQuickHelp);
    connect(openGuide, &QAction::triggered, this, &MainWindow::openReferenceDocument);

    menu->addSeparator();
    QAction *aboutAction = menu->addAction(QStringLiteral("A propos"));
    connect(aboutAction, &QAction::triggered, this, &MainWindow::openAboutDialog);

    m_mainMenuButton->setMenu(menu);

    m_toggleDetailsButton = new QToolButton(topWidget);
    m_toggleDetailsButton->setText(QStringLiteral("↓"));
    m_toggleDetailsButton->setToolTip(QStringLiteral("Afficher/masquer les details"));
    m_toggleDetailsButton->setMinimumWidth(30);

    topLayout->addWidget(m_braidSelector, 0);
    topLayout->addWidget(m_colorModeSelector, 0);
    topLayout->addWidget(m_palettePresetSelector, 0);
    topLayout->addWidget(m_compactLineLabel, 1);
    topLayout->addWidget(m_loadDrawingButton);
    topLayout->addWidget(m_openDocButton);
    topLayout->addWidget(m_mainMenuButton);
    topLayout->addWidget(m_toggleDetailsButton);
    setMenuWidget(topWidget);

    QFont infoFont = m_progressLabel->font();
    infoFont.setPointSize(infoFont.pointSize() + 2);
    m_progressLabel->setFont(infoFont);
    m_moveLabel->setFont(infoFont);
    m_detailLabel->setFont(infoFont);

    QFont buttonFont = m_doneButton->font();
    buttonFont.setPointSize(buttonFont.pointSize() + 1);
    m_doneButton->setFont(buttonFont);
    m_restartButton->setFont(buttonFont);

    m_titleLabel->hide();
    m_noteLabel->hide();

    m_boardWidget = new BraidBoardWidget(this);
    m_boardWidget->setMinimumHeight(640);
    m_boardWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    ui->verticalLayout->replaceWidget(ui->imageLabel, m_boardWidget);
    ui->imageLabel->hide();

    if (m_braidSelector) {
        const int idx = m_braidSelector->findData(m_selectedBraidCode);
        m_braidSelector->setCurrentIndex(idx >= 0 ? idx : 0);
    }
    reloadCurrentBraid();

    if (m_colorModeSelector) {
        m_colorModeSelector->setCurrentIndex(0);
        handleColorModeChanged(m_colorModeSelector->currentIndex());
    }

    if (m_palettePresetSelector) {
        m_palettePresetSelector->setCurrentIndex(0);
        handlePalettePresetChanged(m_palettePresetSelector->currentIndex());
    }

    connect(m_doneButton, &QPushButton::clicked, this, &MainWindow::handleMoveDone);
    connect(m_restartButton, &QPushButton::clicked, this, &MainWindow::restartSequence);
    connect(m_toggleDetailsButton, &QToolButton::clicked, this, [this]() {
        setDetailsExpanded(!m_detailsExpanded);
    });
    connect(m_loadDrawingButton, &QToolButton::clicked, this, &MainWindow::loadReferenceDrawing);
    connect(m_openDocButton, &QToolButton::clicked, this, &MainWindow::openReferenceDocument);
    connect(m_colorModeSelector, qOverload<int>(&QComboBox::currentIndexChanged),
            this, &MainWindow::handleColorModeChanged);
    connect(m_palettePresetSelector, qOverload<int>(&QComboBox::currentIndexChanged),
            this, &MainWindow::handlePalettePresetChanged);
    connect(m_braidSelector, qOverload<int>(&QComboBox::currentIndexChanged),
            this, &MainWindow::handleBraidSelectionChanged);

    connect(m_boardWidget, &BraidBoardWidget::moveAnimationFinished, this,
            [this](bool success, const QString &errorMessage) {
                m_doneButton->setEnabled(true);
                m_restartButton->setEnabled(true);
                if (m_braidSelector) {
                    m_braidSelector->setEnabled(true);
                }

                if (!success) {
                    statusBar()->showMessage(errorMessage, 5000);
                    return;
                }

                ++m_currentMoveIndex;
                updateView();
            });

    setDetailsExpanded(false);
}
void MainWindow::handleBraidSelectionChanged(int index)
{
    if (!m_braidSelector || index < 0) {
        return;
    }

    if (m_boardWidget && m_boardWidget->isAnimating()) {
        QSignalBlocker blocker(m_braidSelector);
        const int currentIndex = m_braidSelector->findData(m_selectedBraidCode);
        if (currentIndex >= 0) {
            m_braidSelector->setCurrentIndex(currentIndex);
        }
        statusBar()->showMessage(QStringLiteral("Impossible de changer de tresse pendant une animation."), 3000);
        return;
    }

    const QString code = m_braidSelector->itemData(index).toString();
    if (code.isEmpty() || code == m_selectedBraidCode) {
        return;
    }

    m_selectedBraidCode = code;
    reloadCurrentBraid();
    updateView();
    statusBar()->showMessage(QStringLiteral("Tresse selectionnee: %1").arg(currentBraidTitle()), 3500);
}

void MainWindow::loadReferenceDrawing()
{
    if (!m_boardWidget) {
        return;
    }

    QSettings s = appSettings();
    const QString key = referenceDrawingSettingKey(m_selectedBraidCode);
    const QString previousPath = s.value(key).toString();

    QString initialDir;
    if (!previousPath.isEmpty()) {
        initialDir = QFileInfo(previousPath).absolutePath();
    }

    const QString selectedPath = QFileDialog::getOpenFileName(
        this,
        QStringLiteral("Choisir le dessin de reference"),
        initialDir,
        QStringLiteral("Images (*.bmp *.png *.jpg *.jpeg)"));

    if (selectedPath.isEmpty()) {
        return;
    }

    QString error;
    if (!m_boardWidget->loadReferenceImage(selectedPath, &error)) {
        QMessageBox::warning(this,
                             QStringLiteral("Dessin de reference"),
                             error.isEmpty() ? QStringLiteral("Chargement impossible.") : error);
        return;
    }

    s.setValue(key, selectedPath);
    s.sync();
    statusBar()->showMessage(QStringLiteral("Dessin charge: %1").arg(QFileInfo(selectedPath).fileName()), 3500);
}

void MainWindow::handleColorModeChanged(int index)
{
    if (!m_boardWidget || !m_colorModeSelector) {
        return;
    }

    const QString mode = m_colorModeSelector->itemData(index).toString();
    const bool originCaseMode = (mode == QStringLiteral("origin_case"));

    m_boardWidget->setColorApplyMode(
        originCaseMode ? BraidBoardWidget::ColorApplyMode::OriginCase
                       : BraidBoardWidget::ColorApplyMode::OrbitPath);

    if (m_palettePresetSelector) {
        const QString presetCode = m_palettePresetSelector->currentData().toString();
        const bool hasPalette = !paletteForPreset(presetCode).isEmpty();
        m_boardWidget->setQuickPaletteEnabled(originCaseMode && hasPalette);
        if (originCaseMode && hasPalette) {
            m_boardWidget->resetQuickPaletteIndex();
        }
    }

    statusBar()->showMessage(
        originCaseMode ? QStringLiteral("Mode couleur: alternance par tour.")
                       : QStringLiteral("Mode couleur: concordance des parcours."),
        3000);
}

void MainWindow::handlePalettePresetChanged(int index)
{
    Q_UNUSED(index);

    if (!m_boardWidget || !m_palettePresetSelector) {
        return;
    }

    const QString presetCode = m_palettePresetSelector->currentData().toString();
    const QVector<QColor> palette = paletteForPreset(presetCode);

    m_boardWidget->setQuickPalette(palette);

    const bool originCaseMode = m_colorModeSelector
                                && m_colorModeSelector->currentData().toString() == QStringLiteral("origin_case");
    const bool enableQuickPalette = originCaseMode && !palette.isEmpty();

    m_boardWidget->setQuickPaletteEnabled(enableQuickPalette);
    m_boardWidget->resetQuickPaletteIndex();

    if (palette.isEmpty()) {
        statusBar()->showMessage(QStringLiteral("Palette libre: selection manuelle sur les clous."), 3000);
        return;
    }

    statusBar()->showMessage(QStringLiteral("Palette appliquee: %1").arg(m_palettePresetSelector->currentText()), 3000);
}

void MainWindow::openQuickHelp()
{
    QMessageBox::information(
        this,
        QStringLiteral("Aide rapide"),
        QStringLiteral(
            "1) Choisir la tresse dans la liste.\n"
            "2) Regarder le vecteur rouge de previsualisation.\n"
            "3) Cliquer 'Animer le mouvement' pour deplacer le fil interieur vers le clou exterieur.\n"
            "4) Cliquer un clou occupe pour colorer un fil (ou un parcours complet selon le mode).\n"
            "5) En mode 'Alternance', choisir une palette drapeau pour alterner automatiquement les couleurs.\n"
            "6) Les couleurs sont sauvegardees automatiquement en .lbc (JSON)."));
}

void MainWindow::openAboutDialog()
{
    QMessageBox::about(
        this,
        QStringLiteral("A propos de LogiBraiding"),
        QStringLiteral("LogiBraiding\n"
                       "Assistant visuel de tressage ABoK\n\n"
                       "Version d'essai: 15 jours\n"
                       "Inscription: AuthorID (Ctrl+I)\n\n"
                       "Suite logicielle: LogiKnotting + LogiBraiding"));
}

void MainWindow::applyUiLanguage(const QString &languageCode)
{
    const QString normalized = languageCode.trimmed().isEmpty() ? QStringLiteral("auto") : languageCode.trimmed();

    QSettings s = appSettings();
    s.setValue(uiLanguageSettingKey(), normalized);
    s.sync();

    const QMessageBox::StandardButton answer = QMessageBox::question(
        this,
        QStringLiteral("Langue"),
        QStringLiteral("La langue sera appliquee au prochain demarrage.\nRedemarrer maintenant ?"),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::Yes);

    if (answer != QMessageBox::Yes) {
        statusBar()->showMessage(QStringLiteral("Langue enregistree: redemarrage requis."), 3500);
        return;
    }

    const QString appPath = QCoreApplication::applicationFilePath();
    const QStringList args = QCoreApplication::arguments().mid(1);
    const bool launched = QProcess::startDetached(appPath, args);

    if (!launched) {
        QMessageBox::warning(this,
                             QStringLiteral("Langue"),
                             QStringLiteral("Impossible de relancer automatiquement l'application."));
        return;
    }

    qApp->quit();
}

void MainWindow::openReferenceDocument()
{
    const QString fileName = QStringLiteral("Solid Sinnet variations.pdf");

    const QStringList candidates = {
        QDir(QCoreApplication::applicationDirPath()).filePath(QStringLiteral("../../Docs/%1").arg(fileName)),
        QDir(QCoreApplication::applicationDirPath()).filePath(QStringLiteral("../Docs/%1").arg(fileName)),
        QDir(QCoreApplication::applicationDirPath()).filePath(QStringLiteral("Docs/%1").arg(fileName)),
        QDir::current().filePath(QStringLiteral("Docs/%1").arg(fileName)),
        QDir(QStringLiteral("E:/LogiBrainding/Docs")).filePath(fileName)
    };

    QString docPath;
    for (const QString &candidate : candidates) {
        const QFileInfo info(candidate);
        if (info.exists() && info.isFile()) {
            docPath = info.absoluteFilePath();
            break;
        }
    }

    if (docPath.isEmpty()) {
        statusBar()->showMessage(QStringLiteral("Document introuvable: %1").arg(fileName), 5000);
        return;
    }

    if (!QDesktopServices::openUrl(QUrl::fromLocalFile(docPath))) {
        statusBar()->showMessage(QStringLiteral("Impossible d'ouvrir le document de reference."), 5000);
        return;
    }

    statusBar()->showMessage(QStringLiteral("Document ouvert: %1").arg(QFileInfo(docPath).fileName()), 3500);
}
void MainWindow::setDetailsExpanded(bool expanded)
{
    m_detailsExpanded = expanded;
    m_progressLabel->setVisible(expanded);
    m_moveLabel->setVisible(expanded);
    m_detailLabel->setVisible(expanded);
    m_movesTable->setVisible(expanded);

    if (m_toggleDetailsButton) {
        m_toggleDetailsButton->setText(expanded ? QStringLiteral("↑") : QStringLiteral("↓"));
    }
}

void MainWindow::configureMovesTable()
{
    m_movesTable->setColumnCount(6);
    m_movesTable->setHorizontalHeaderLabels({
        QStringLiteral("Etape"),
        QStringLiteral("Case"),
        QStringLiteral("Brins"),
        QStringLiteral("Depart"),
        QStringLiteral("Arrivee"),
        QStringLiteral("Rotation")
    });

    m_movesTable->setRowCount(1);
    m_movesTable->setVerticalHeaderLabels({QStringLiteral("Active")});
    m_movesTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_movesTable->setSelectionMode(QAbstractItemView::NoSelection);
    m_movesTable->setFocusPolicy(Qt::NoFocus);
    m_movesTable->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_movesTable->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    m_movesTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_movesTable->verticalHeader()->setDefaultSectionSize(56);

    QFont tableFont = m_movesTable->font();
    tableFont.setPointSize(tableFont.pointSize() + 3);
    m_movesTable->setFont(tableFont);

    const int frame = m_movesTable->frameWidth() * 2;
    const int header = m_movesTable->horizontalHeader()->height();
    const int rows = m_movesTable->verticalHeader()->sectionSize(0);
    const int tableHeight = frame + header + rows + 8;
    m_movesTable->setMinimumHeight(tableHeight);
    m_movesTable->setMaximumHeight(tableHeight);
    m_movesTable->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    ui->verticalLayout->setStretch(0, 0);
    ui->verticalLayout->setStretch(1, 0);
    ui->verticalLayout->setStretch(2, 0);
    ui->verticalLayout->setStretch(3, 0);
    ui->verticalLayout->setStretch(4, 0);
    ui->verticalLayout->setStretch(5, 14);
    ui->verticalLayout->setStretch(6, 0);
    ui->verticalLayout->setStretch(7, 0);
}

bool MainWindow::openRegistrationDialog(bool trialExpired)
{
    QSettings s = appSettings();

    QDialog dialog(this);
    dialog.setWindowTitle(QStringLiteral("Inscription"));
    dialog.setModal(true);

    auto *layout = new QVBoxLayout(&dialog);

    auto *intro = new QLabel(
        trialExpired
            ? QStringLiteral("La periode d'essai de 15 jours est expiree. Entrez une cle de debridage pour continuer.")
            : QStringLiteral("Entrez un email valide pour demander une inscription, ou collez directement votre cle de debridage."),
        &dialog);
    intro->setWordWrap(true);
    layout->addWidget(intro);

    auto *form = new QFormLayout();

    auto *emailEdit = new QLineEdit(&dialog);
    emailEdit->setPlaceholderText(QStringLiteral("name@example.com"));
    emailEdit->setText(canonicalEmail(s.value(QStringLiteral("registration/pending_email")).toString()));
    form->addRow(QStringLiteral("Email"), emailEdit);

    auto *keyEdit = new QLineEdit(&dialog);
    keyEdit->setPlaceholderText(QStringLiteral("XXXX-XXXX-XXXX-XXXX"));
    form->addRow(QStringLiteral("Cle de debridage"), keyEdit);

    layout->addLayout(form);

    auto *help = new QLabel(
        QStringLiteral("Validation avec email: ouvre votre client mail vers la boite d'inscription.\nValidation avec cle: debloque immediatement l'application et cree l'AuthorID."),
        &dialog);
    help->setWordWrap(true);
    layout->addWidget(help);

    auto *buttons = new QDialogButtonBox(&dialog);
    auto *validateBtn = buttons->addButton(QStringLiteral("Valider"), QDialogButtonBox::AcceptRole);
    buttons->addButton(trialExpired ? QStringLiteral("Quitter") : QStringLiteral("Fermer"), QDialogButtonBox::RejectRole);
    layout->addWidget(buttons);

    QObject::connect(validateBtn, &QPushButton::clicked, &dialog, [this, &dialog, emailEdit, keyEdit]() {
        QSettings sLocal = appSettings();

        const QString emailInput = canonicalEmail(emailEdit->text());
        const QString keyInput = keyEdit->text().trimmed();

        if (!keyInput.isEmpty()) {
            QString emailForKey = emailInput;
            if (emailForKey.isEmpty()) {
                emailForKey = canonicalEmail(sLocal.value(QStringLiteral("registration/pending_email")).toString());
            }
            if (emailForKey.isEmpty()) {
                emailForKey = canonicalEmail(sLocal.value(QStringLiteral("registration/email")).toString());
            }

            if (!isValidEmail(emailForKey)) {
                QMessageBox::warning(this,
                                     QStringLiteral("Inscription"),
                                     QStringLiteral("Entrez d'abord un email valide associe a cette cle."));
                return;
            }

            if (!verifyUnlockKey(emailForKey, keyInput)) {
                QMessageBox::warning(this,
                                     QStringLiteral("Inscription"),
                                     QStringLiteral("Cle invalide pour cet email."));
                return;
            }

            sLocal.setValue(QStringLiteral("registration/email"), emailForKey);
            sLocal.setValue(QStringLiteral("registration/unlocked"), true);
            sLocal.remove(QStringLiteral("registration/pending_email"));
            sLocal.sync();

            const QString authorId = authorIdFromEmail(emailForKey);
            QMessageBox::information(
                this,
                QStringLiteral("Inscription"),
                QStringLiteral("Inscription validee. Application debridee.\nAuthorID: %1").arg(authorId));
            dialog.accept();
            return;
        }

        if (!isValidEmail(emailInput)) {
            QMessageBox::warning(this,
                                 QStringLiteral("Inscription"),
                                 QStringLiteral("Entrez un email valide ou une cle."));
            return;
        }

        sLocal.setValue(QStringLiteral("registration/pending_email"), emailInput);
        sLocal.sync();

        const QString inbox = QString::fromLatin1(kRegistrationInboxEmail).trimmed();
        if (inbox.isEmpty() || inbox.startsWith(QStringLiteral("CHANGE_ME"), Qt::CaseInsensitive)) {
            QMessageBox::information(
                this,
                QStringLiteral("Inscription"),
                QStringLiteral("Email d'inscription non configure dans le code.\nRemplacez kRegistrationInboxEmail dans MainWindow.cpp puis recompilez."));
            return;
        }

        QUrl mailUrl;
        mailUrl.setScheme(QStringLiteral("mailto"));
        mailUrl.setPath(inbox);

        QUrlQuery query;
        query.addQueryItem(QStringLiteral("subject"), QStringLiteral("Demande d'inscription LogiBraiding"));
        query.addQueryItem(QStringLiteral("body"),
                           QStringLiteral("Bonjour,\n\nMerci de m'envoyer une cle de debridage pour cet email :\n%1\n\nCordialement,")
                               .arg(emailInput));
        mailUrl.setQuery(query);

        if (!QDesktopServices::openUrl(mailUrl)) {
            QMessageBox::warning(this,
                                 QStringLiteral("Inscription"),
                                 QStringLiteral("Impossible d'ouvrir le client email par defaut."));
            return;
        }

        QMessageBox::information(
            this,
            QStringLiteral("Inscription"),
            QStringLiteral("Demande envoyee via votre client email.\nQuand vous recevez la cle, revenez ici et collez-la dans le champ cle."));
    });

    if (dialog.exec() != QDialog::Accepted) {
        return false;
    }

    QSettings check = appSettings();
    return isRegistrationUnlocked(check);
}

bool MainWindow::ensureTrialAccess()
{
    QSettings s = appSettings();
    if (isRegistrationUnlocked(s)) {
        return true;
    }

    const int remaining = trialDaysRemaining(s);
    if (remaining > 0) {
        if (statusBar()) {
            statusBar()->showMessage(
                QStringLiteral("Version d'essai: %1 jour(s) restant(s). Ctrl+I pour inscription.").arg(remaining),
                6000);
        }
        return true;
    }

    QMessageBox::warning(this,
                         QStringLiteral("Periode d'essai expiree"),
                         QStringLiteral("La periode d'essai de 15 jours est terminee.\nUne inscription est necessaire pour continuer."));
    return openRegistrationDialog(true);
}

QString MainWindow::currentRegisteredEmail() const
{
    QSettings s = appSettings();
    const QString email = canonicalEmail(s.value(QStringLiteral("registration/email")).toString());
    return isValidEmail(email) ? email : QString();
}

QString MainWindow::currentAuthorId() const
{
    const QString email = currentRegisteredEmail();
    if (email.isEmpty()) {
        return QString();
    }

    return authorIdFromEmail(email);
}

QString MainWindow::registrationStatusText() const
{
    QSettings s = appSettings();
    if (isRegistrationUnlocked(s)) {
        const QString authorId = authorIdFromEmail(s.value(QStringLiteral("registration/email")).toString());
        return authorId.isEmpty() ? QStringLiteral("Inscription active")
                                  : QStringLiteral("AuthorID %1").arg(authorId);
    }

    const int remaining = trialDaysRemaining(s);
    const int shownRemaining = (remaining > 0) ? remaining : 0;
    return QStringLiteral("Essai %1j").arg(shownRemaining);
}

void MainWindow::updateCompactLine(const Move3082 *move, int totalMoves)
{
    if (!m_compactLineLabel) {
        return;
    }

    const QString registrationState = registrationStatusText();

    if (!move) {
        m_compactLineLabel->setText(
            QStringLiteral("Cycle termine: %1/%1 | %2").arg(totalMoves).arg(registrationState));
        return;
    }

    const QString rotationGlyph =
        QStringLiteral("<span style=\"font-family:'Wingdings 3';\">%1</span>").arg(move->rotationCode);

    m_compactLineLabel->setText(
        QStringLiteral("Etape %1/%2 | Case %3 (%4 brins) | %5 -> %6 | Rotation %7 | %8")
                        .arg(move->step)
            .arg(totalMoves)
            .arg(move->boardCase)
            .arg(move->strands)
            .arg(move->fromPeg)
            .arg(move->toPeg)
            .arg(rotationGlyph)
            .arg(registrationState));
}

void MainWindow::updateView()
{
    const auto &moves = currentMoveSet();
    const int totalMoves = static_cast<int>(moves.size());

    auto setCell = [this](int col, QTableWidgetItem *item, const QColor &background) {
        item->setBackground(QBrush(background));
        m_movesTable->setItem(0, col, item);
    };

    auto populateActiveRow = [&](const Move3082 &move, const QColor &background) {
        QFont textFont = m_movesTable->font();
        textFont.setBold(true);

        auto *stepItem = new QTableWidgetItem(QString::number(move.step));
        stepItem->setFont(textFont);
        setCell(0, stepItem, background);

        auto *caseItem = new QTableWidgetItem(QString::number(move.boardCase));
        caseItem->setFont(textFont);
        setCell(1, caseItem, background);

        auto *strandItem = new QTableWidgetItem(QString::number(move.strands));
        strandItem->setFont(textFont);
        setCell(2, strandItem, background);

        auto *fromItem = new QTableWidgetItem(QString::number(move.fromPeg));
        fromItem->setFont(textFont);
        setCell(3, fromItem, background);

        auto *toItem = new QTableWidgetItem(QString::number(move.toPeg));
        toItem->setFont(textFont);
        setCell(4, toItem, background);

        auto *rotationItem = new QTableWidgetItem(QString(move.rotationCode));
        QFont rotationFont = textFont;
        rotationFont.setFamily(QStringLiteral("Wingdings 3"));
        rotationFont.setPointSize(rotationFont.pointSize() + 2);
        rotationItem->setFont(rotationFont);
        rotationItem->setTextAlignment(Qt::AlignCenter);
        setCell(5, rotationItem, background);
    };

    if (m_currentMoveIndex >= totalMoves) {
        updateCompactLine(nullptr, totalMoves);
        m_progressLabel->setText(QStringLiteral("Progression: %1/%1 (cycle complet)").arg(totalMoves));
        m_moveLabel->setText(QStringLiteral("La sequence complete %1 est terminee.").arg(currentBraidTitle()));
        m_detailLabel->setText(QStringLiteral("Conformement a la logique append-only, toute correction demande un redemarrage complet."));

        m_boardWidget->setPendingMove(-1, -1);
        m_doneButton->setEnabled(false);
        m_doneButton->setText(QStringLiteral("Cycle termine"));

        populateActiveRow(moves.back(), QColor(226, 248, 229));

        statusBar()->showMessage(QStringLiteral("Cycle termine. Utilisez 'Recommencer' pour repartir de l'etape 1."));
        return;
    }

    const auto &move = moves[m_currentMoveIndex];
    updateCompactLine(&move, totalMoves);
    m_boardWidget->setPendingMove(move.fromPeg, move.toPeg);

    m_progressLabel->setText(
        QStringLiteral("Progression: %1/%2 - prochain mouvement: etape %3")
                        .arg(m_currentMoveIndex)
            .arg(totalMoves)
            .arg(move.step));

    m_moveLabel->setText(
        QStringLiteral("Mouvement %1: prendre le fil du clou interieur de la case %2 et l'amener au clou exterieur de la case %3.")
            .arg(move.step)
            .arg(move.fromPeg)
            .arg(move.toPeg));

    const QString currentRotationGlyph =
        QStringLiteral("<span style=\"font-family:'Wingdings 3';\">%1</span>").arg(move.rotationCode);
    m_detailLabel->setText(
        QStringLiteral("Case %1 | Brins %2 | Rotation %3 | Puis recentrer les fils restants vers l'interieur.")
            .arg(move.boardCase)
            .arg(move.strands)
            .arg(currentRotationGlyph));

    m_doneButton->setEnabled(true);
    m_doneButton->setText(QStringLiteral("Animer le mouvement %1").arg(move.step));

    populateActiveRow(move, QColor(224, 238, 255));

    statusBar()->showMessage(QStringLiteral("Mode didactique: animation du fil interieur vers le clou exterieur."));
}

void MainWindow::handleMoveDone()
{
    if (m_currentMoveIndex >= static_cast<int>(currentMoveSet().size()) || m_boardWidget->isAnimating()) {
        return;
    }

    const auto &move = currentMoveSet()[m_currentMoveIndex];
    m_doneButton->setEnabled(false);
    m_restartButton->setEnabled(false);
    if (m_braidSelector) {
        m_braidSelector->setEnabled(false);
    }
    statusBar()->showMessage(QStringLiteral("Animation du mouvement %1 en cours...").arg(move.step));

    m_boardWidget->animateMove(move.fromPeg, move.toPeg);
}

void MainWindow::restartSequence()
{
    if (m_boardWidget->isAnimating()) {
        return;
    }

    m_currentMoveIndex = 0;
    m_boardWidget->resetBoard();
    clearFocus();
    updateView();
}






























































