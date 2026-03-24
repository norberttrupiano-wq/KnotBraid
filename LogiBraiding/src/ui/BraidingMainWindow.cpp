#include "BraidingMainWindow.h"
#include "BraidBoardWidget.h"
#include "ui_BraidingMainWindow.h"

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
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFont>
#include <QFormLayout>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QKeySequence>
#include <QLabel>
#include <QLineEdit>
#include <QLocale>
#include <QMessageBox>
#include <QMenu>
#include <QPushButton>
#include <QProcess>
#include <QRegularExpression>
#include <QSettings>
#include <QSignalBlocker>
#include <QShortcut>
#include <QSaveFile>
#include <QSizePolicy>
#include <QStandardPaths>
#include <QStatusBar>
#include <QSet>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QTimer>
#include <QToolButton>
#include <QUrl>
#include <QUrlQuery>
#include <QVector>
#include <QVBoxLayout>
#include <QWidget>

#include <algorithm>
#include <limits>

namespace {

static constexpr int kTrialPeriodDays = 15;
static constexpr int kDefaultAnimationDurationMs = 5000;
static constexpr int kMinAnimationDurationMs = 5000;
static constexpr int kMaxAnimationDurationMs = 60000;
static const char *kRegistrationInboxEmail = "CHANGE_ME@example.com";
static const char *kRegistrationSalt = "KeyGenerator::SidoineArifene::v1";
static constexpr int kNoAbokOrder = std::numeric_limits<int>::max();

static int extractAbokFromName(const QString &name)
{
    static const QRegularExpression re(QStringLiteral(R"(ABoK\s*#\s*(\d+))"),
                                       QRegularExpression::CaseInsensitiveOption);
    const QRegularExpressionMatch m = re.match(name);
    if (!m.hasMatch()) {
        return -1;
    }

    bool ok = false;
    const int value = m.captured(1).toInt(&ok);
    return ok ? value : -1;
}

static int extractLeadingDigits(const QString &code)
{
    static const QRegularExpression re(QStringLiteral(R"(^\s*(\d+))"));
    const QRegularExpressionMatch m = re.match(code);
    if (!m.hasMatch()) {
        return -1;
    }

    bool ok = false;
    const int value = m.captured(1).toInt(&ok);
    return ok ? value : -1;
}

static int abokOrderKey(const QString &name, const QString &code)
{
    const int fromName = extractAbokFromName(name);
    if (fromName > 0) {
        return fromName;
    }

    const int fromCode = extractLeadingDigits(code);
    if (fromCode > 0) {
        return fromCode;
    }

    return kNoAbokOrder;
}

template <typename TBraid>
static void sortBraidsByAbok(QVector<TBraid> *braids)
{
    if (!braids) {
        return;
    }

    std::stable_sort(braids->begin(), braids->end(), [](const TBraid &a, const TBraid &b) {
        const int keyA = abokOrderKey(a.name, a.code);
        const int keyB = abokOrderKey(b.name, b.code);
        if (keyA != keyB) {
            return keyA < keyB;
        }

        const int byCode = QString::compare(a.code, b.code, Qt::CaseInsensitive);
        if (byCode != 0) {
            return byCode < 0;
        }

        return QString::compare(a.name, b.name, Qt::CaseInsensitive) < 0;
    });
}

static QSettings appSettings()
{
    return QSettings(QSettings::NativeFormat,
                     QSettings::UserScope,
                     QStringLiteral("LogiBraiding"),
                     QStringLiteral("LogiBraiding"));
}

static QSettings shellSettings()
{
    return QSettings(QSettings::NativeFormat,
                     QSettings::UserScope,
                     QStringLiteral("KnotBraid"),
                     QStringLiteral("Shell"));
}

static QString currentAppDataBraidsCatalogPath()
{
    QString appDataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (appDataDir.trimmed().isEmpty()) {
        appDataDir = QDir::currentPath();
    }

    QDir().mkpath(appDataDir);
    return QDir(appDataDir).filePath(QStringLiteral("braids_catalog.json"));
}

static QString sharedBraidsCatalogPath()
{
    QString appDataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (appDataDir.trimmed().isEmpty()) {
        return currentAppDataBraidsCatalogPath();
    }

    QDir siblingsRoot(appDataDir);
    if (!siblingsRoot.cdUp()) {
        return currentAppDataBraidsCatalogPath();
    }

    const QString sharedDir = siblingsRoot.filePath(QStringLiteral("LogiBraiding"));

    QDir().mkpath(sharedDir);
    return QDir(sharedDir).filePath(QStringLiteral("braids_catalog.json"));
}


static QString referenceDrawingSettingKey(const QString &patternCode)
{
    return QStringLiteral("braids/%1/reference_drawing_path").arg(patternCode);
}
static QString uiLanguageSettingKey()
{
    return QStringLiteral("ui/language");
}

static QString canonicalUiLanguageCode(const QString &languageCode)
{
    const QString trimmed = languageCode.trimmed();
    if (trimmed.isEmpty()) {
        return QStringLiteral("auto");
    }

    if (trimmed.compare(QStringLiteral("auto"), Qt::CaseInsensitive) == 0) {
        return QStringLiteral("auto");
    }

    QString normalized = trimmed;
    normalized.replace(QLatin1Char('-'), QLatin1Char('_'));

    const QString localeName = QLocale(normalized).name();
    const QString baseLanguage = localeName.section(QLatin1Char('_'), 0, 0);
    return baseLanguage.isEmpty() ? normalized : baseLanguage;
}

static QString effectiveUiLanguageCode()
{
    QString configured = shellSettings().value(uiLanguageSettingKey()).toString().trimmed();
    if (configured.isEmpty()) {
        configured = appSettings().value(uiLanguageSettingKey(), QStringLiteral("auto")).toString().trimmed();
    }

    const QString normalized = canonicalUiLanguageCode(configured);
    if (normalized == QStringLiteral("auto")) {
        const QStringList systemLanguages = QLocale::system().uiLanguages();
        for (const QString &locale : systemLanguages) {
            const QString base = locale.section(QLatin1Char('-'), 0, 0).section(QLatin1Char('_'), 0, 0);
            if (base == QStringLiteral("en") || base == QStringLiteral("de") || base == QStringLiteral("it")) {
                return base;
            }
        }
        return QStringLiteral("fr");
    }

    if (normalized == QStringLiteral("en") || normalized == QStringLiteral("de") ||
        normalized == QStringLiteral("it")) {
        return normalized;
    }

    return QStringLiteral("fr");
}

static QString uiText(const QString &languageCode,
                      const char *fr,
                      const char *en,
                      const char *de,
                      const char *it)
{
    if (languageCode == QStringLiteral("en")) {
        return QString::fromUtf8(en);
    }
    if (languageCode == QStringLiteral("de")) {
        return QString::fromUtf8(de);
    }
    if (languageCode == QStringLiteral("it")) {
        return QString::fromUtf8(it);
    }
    return QString::fromUtf8(fr);
}

static QString selectedBraidSettingKey()
{
    return QStringLiteral("ui/last_braid_code");
}

static QString animationDurationSettingKey()
{
    return QStringLiteral("ui/animation_duration_ms");
}

static QVector<int> sequentialIntervalOptionsMs()
{
    return {5000, 10000, 15000, 20000, 30000, 40000, 50000, 60000};
}

static int nearestSequentialIntervalMs(int durationMs)
{
    const QVector<int> options = sequentialIntervalOptionsMs();
    if (options.isEmpty()) {
        return durationMs;
    }

    int best = options.front();
    int bestDistance = qAbs(durationMs - best);
    for (const int option : options) {
        const int distance = qAbs(durationMs - option);
        if (distance < bestDistance) {
            best = option;
            bestDistance = distance;
        }
    }

    return best;
}

static QString sequentialIntervalLabel(int durationMs)
{
    return QStringLiteral("%1 s").arg(durationMs / 1000);
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

static QString rotationHintSymbol(QChar rotationCode)
{
    return rotationCode.toUpper() == QLatin1Char('Q')
               ? QStringLiteral("\u21AA")
               : QStringLiteral("\u21A9");
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

namespace LogiBraidingApp {

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

QVector<MainWindow::BraidDefinition> MainWindow::legacyBraidsCatalog() const
{
    QVector<BraidDefinition> out;

    out.push_back(BraidDefinition{
        QStringLiteral("3082"),
        QStringLiteral("ABoK #3082 - Pentalpha a 61 brins"),
        kMoves3082
    });

    out.push_back(BraidDefinition{
        QStringLiteral("3082_2_78"),
        QStringLiteral("ABoK #3082-2 - Pentalpha a 78 brins"),
        kMovesPentalpha78
    });

    out.push_back(BraidDefinition{
        QStringLiteral("3082_3_rose97"),
        QStringLiteral("ABoK #3082-3 - Rose des vents a 97 brins"),
        kMovesRoseDesVents97
    });

    return out;
}

bool MainWindow::parseBraidsCatalog(const QByteArray &jsonData,
                                    QVector<BraidDefinition> *outBraids,
                                    QString *errorMessage) const
{
    if (!outBraids) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Sortie invalide");
        }
        return false;
    }

    outBraids->clear();

    QJsonParseError parseError;
    const QJsonDocument doc = QJsonDocument::fromJson(jsonData, &parseError);
    if (doc.isNull()) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("JSON invalide: %1").arg(parseError.errorString());
        }
        return false;
    }

    QJsonArray braidsArray;
    if (doc.isObject()) {
        braidsArray = doc.object().value(QStringLiteral("braids")).toArray();
    } else if (doc.isArray()) {
        braidsArray = doc.array();
    }

    if (braidsArray.isEmpty()) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Aucune tresse trouvee dans le JSON");
        }
        return false;
    }

    auto readInt = [](const QJsonObject &obj,
                      const QStringList &keys,
                      int defaultValue,
                      bool *ok) -> int {
        if (ok) {
            *ok = false;
        }

        for (const QString &key : keys) {
            const QJsonValue v = obj.value(key);
            if (v.isUndefined() || v.isNull()) {
                continue;
            }

            if (v.isDouble()) {
                if (ok) {
                    *ok = true;
                }
                return v.toInt(defaultValue);
            }

            if (v.isString()) {
                bool localOk = false;
                const int n = v.toString().trimmed().toInt(&localOk);
                if (localOk) {
                    if (ok) {
                        *ok = true;
                    }
                    return n;
                }
            }
        }

        return defaultValue;
    };

    for (const QJsonValue &braidValue : braidsArray) {
        if (!braidValue.isObject()) {
            continue;
        }

        const QJsonObject braidObj = braidValue.toObject();
        const QString code = braidObj.value(QStringLiteral("code")).toString().trimmed();
        QString name = braidObj.value(QStringLiteral("name")).toString().trimmed();

        if (code.isEmpty()) {
            continue;
        }

        if (name.isEmpty()) {
            name = code;
        }

        const QJsonArray movesArray = braidObj.value(QStringLiteral("moves")).toArray();
        QVector<Move3082> moves;
        moves.reserve(movesArray.size());

        int rowIndex = 0;
        for (const QJsonValue &moveValue : movesArray) {
            ++rowIndex;
            if (!moveValue.isObject()) {
                continue;
            }

            const QJsonObject moveObj = moveValue.toObject();

            bool okStep = false;
            bool okCase = false;
            bool okStrands = false;
            bool okFrom = false;
            bool okTo = false;

            int step = readInt(moveObj,
                               QStringList{QStringLiteral("sequence"), QStringLiteral("step"), QStringLiteral("numero_sequence"), QStringLiteral("numero"), QStringLiteral("n_sequence")},
                               rowIndex,
                               &okStep);
            int boardCase = readInt(moveObj,
                                    QStringList{QStringLiteral("case"), QStringLiteral("boardCase")},
                                    0,
                                    &okCase);
            int strands = readInt(moveObj,
                                  QStringList{QStringLiteral("strands"), QStringLiteral("nb_brins"), QStringLiteral("nb brins"), QStringLiteral("nbBrins")},
                                  1,
                                  &okStrands);
            int fromPeg = readInt(moveObj,
                                  QStringList{QStringLiteral("from"), QStringLiteral("depart"), QStringLiteral("d\u00E9part")},
                                  0,
                                  &okFrom);
            int toPeg = readInt(moveObj,
                                QStringList{QStringLiteral("to"), QStringLiteral("arrivee"), QStringLiteral("arriv\u00E9e")},
                                0,
                                &okTo);

            QString sense = moveObj.value(QStringLiteral("sense")).toString().trimmed();
            if (sense.isEmpty()) {
                sense = moveObj.value(QStringLiteral("sens")).toString().trimmed();
            }
            if (sense.isEmpty()) {
                sense = moveObj.value(QStringLiteral("rotation")).toString().trimmed();
            }
            const QChar rotation = sense.isEmpty() ? QLatin1Char('P') : sense.at(0).toUpper();

            if (!okStep) {
                step = rowIndex;
            }

            if (!okCase || !okStrands || !okFrom || !okTo || boardCase <= 0 || strands <= 0 || fromPeg <= 0 || toPeg <= 0) {
                continue;
            }

            Move3082 move{};
            move.step = step;
            move.boardCase = boardCase;
            move.strands = strands;
            move.fromPeg = fromPeg;
            move.toPeg = toPeg;
            move.rotationCode = rotation;
            move.note = moveObj.value(QStringLiteral("note")).toString();
            move.imagePath = moveObj.value(QStringLiteral("image")).toString();
            moves.push_back(move);
        }

        if (moves.isEmpty()) {
            continue;
        }

        std::sort(moves.begin(), moves.end(), [](const Move3082 &a, const Move3082 &b) {
            return a.step < b.step;
        });

        bool exists = false;
        for (const BraidDefinition &existing : *outBraids) {
            if (existing.code.compare(code, Qt::CaseInsensitive) == 0) {
                exists = true;
                break;
            }
        }
        if (exists) {
            continue;
        }

        outBraids->push_back(BraidDefinition{code, name, moves});
    }

    if (outBraids->isEmpty()) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Aucune tresse valide dans le JSON");
        }
        return false;
    }

    return true;
}

bool MainWindow::saveBraidsCatalog(const QVector<BraidDefinition> &braids, QString *errorMessage) const
{
    if (m_braidsJsonPath.isEmpty()) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Chemin JSON non defini");
        }
        return false;
    }

    QJsonArray braidsArray;
    for (const BraidDefinition &braid : braids) {
        if (braid.code.trimmed().isEmpty() || braid.moves.isEmpty()) {
            continue;
        }

        QJsonObject braidObj;
        braidObj.insert(QStringLiteral("code"), braid.code);
        braidObj.insert(QStringLiteral("name"), braid.name);

        QJsonArray movesArray;
        for (const Move3082 &move : braid.moves) {
            QJsonObject moveObj;
            moveObj.insert(QStringLiteral("sequence"), move.step);
            moveObj.insert(QStringLiteral("case"), move.boardCase);
            moveObj.insert(QStringLiteral("strands"), move.strands);
            moveObj.insert(QStringLiteral("from"), move.fromPeg);
            moveObj.insert(QStringLiteral("to"), move.toPeg);
            moveObj.insert(QStringLiteral("sense"), QString(move.rotationCode));
            if (!move.note.trimmed().isEmpty()) {
                moveObj.insert(QStringLiteral("note"), move.note);
            }
            if (!move.imagePath.trimmed().isEmpty()) {
                moveObj.insert(QStringLiteral("image"), move.imagePath);
            }
            movesArray.push_back(moveObj);
        }

        braidObj.insert(QStringLiteral("moves"), movesArray);
        braidsArray.push_back(braidObj);
    }

    QJsonObject root;
    root.insert(QStringLiteral("version"), 1);
    root.insert(QStringLiteral("braids"), braidsArray);

    QSaveFile outFile(m_braidsJsonPath);
    if (!outFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Impossible d'ecrire %1").arg(m_braidsJsonPath);
        }
        return false;
    }

    outFile.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
    if (!outFile.commit()) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Echec d'ecriture de %1").arg(m_braidsJsonPath);
        }
        return false;
    }

    return true;
}

void MainWindow::loadBraidsCatalog()
{
    m_braids.clear();

    const QString currentAppDataPath = currentAppDataBraidsCatalogPath();
    const QString sharedAppDataPath = sharedBraidsCatalogPath();

    QStringList candidates = {
        QDir::current().filePath(QStringLiteral("braids_catalog.json")),
        QDir(QCoreApplication::applicationDirPath()).filePath(QStringLiteral("braids_catalog.json")),
        sharedAppDataPath,
        currentAppDataPath
    };
    candidates.removeDuplicates();

    QVector<BraidDefinition> loadedBraids;
    QString preferredSavePath = sharedAppDataPath;
    QString primaryLoadedPath;
    bool foundAnyCatalog = false;
    bool mergedCatalogs = false;

    auto containsCode = [](const QVector<BraidDefinition> &braids, const QString &code) {
        for (const BraidDefinition &braid : braids) {
            if (braid.code.compare(code, Qt::CaseInsensitive) == 0) {
                return true;
            }
        }
        return false;
    };

    for (const QString &candidate : candidates) {
        QFile inFile(candidate);
        if (!inFile.exists()) {
            continue;
        }
        if (!inFile.open(QIODevice::ReadOnly)) {
            continue;
        }

        QVector<BraidDefinition> loaded;
        QString parseError;
        if (parseBraidsCatalog(inFile.readAll(), &loaded, &parseError) && !loaded.isEmpty()) {
            if (!foundAnyCatalog) {
                loadedBraids = loaded;
                foundAnyCatalog = true;
                primaryLoadedPath = candidate;
                if (candidate != sharedAppDataPath && candidate != currentAppDataPath) {
                    preferredSavePath = candidate;
                }
                continue;
            }

            for (const BraidDefinition &braid : loaded) {
                if (!containsCode(loadedBraids, braid.code)) {
                    loadedBraids.push_back(braid);
                    mergedCatalogs = true;
                }
            }
        }
    }

    if (!foundAnyCatalog) {
        loadedBraids = legacyBraidsCatalog();
    }

    bool injectedDefaults = false;
    for (const BraidDefinition &legacyBraid : legacyBraidsCatalog()) {
        if (!containsCode(loadedBraids, legacyBraid.code)) {
            loadedBraids.push_back(legacyBraid);
            injectedDefaults = true;
        }
    }

    m_braids = loadedBraids;
    sortBraidsByAbok(&m_braids);
    m_braidsJsonPath = preferredSavePath;

    if (!foundAnyCatalog || mergedCatalogs || injectedDefaults ||
        (preferredSavePath == sharedAppDataPath && primaryLoadedPath != preferredSavePath)) {
        saveBraidsCatalog(m_braids, nullptr);
    }
}

void MainWindow::loadUiPreferences()
{
    QSettings s = appSettings();

    const QString savedBraidCode = s.value(selectedBraidSettingKey(), m_selectedBraidCode).toString().trimmed();
    if (!savedBraidCode.isEmpty()) {
        m_selectedBraidCode = savedBraidCode;
    }

    const int storedDuration =
        qBound(kMinAnimationDurationMs,
               s.value(animationDurationSettingKey(), kDefaultAnimationDurationMs).toInt(),
               kMaxAnimationDurationMs);
    m_animationDurationMs = nearestSequentialIntervalMs(storedDuration);
}

void MainWindow::saveSelectedBraidCode() const
{
    if (m_selectedBraidCode.trimmed().isEmpty()) {
        return;
    }

    QSettings s = appSettings();
    s.setValue(selectedBraidSettingKey(), m_selectedBraidCode);
}

void MainWindow::saveAnimationDurationMs() const
{
    QSettings s = appSettings();
    s.setValue(animationDurationSettingKey(), m_animationDurationMs);
}

void MainWindow::applyAnimationDurationMs(int durationMs, bool persistSetting)
{
    const int boundedDuration =
        nearestSequentialIntervalMs(qBound(kMinAnimationDurationMs, durationMs, kMaxAnimationDurationMs));
    m_animationDurationMs = boundedDuration;

    if (m_animationSpeedCombo) {
        const int comboIndex = m_animationSpeedCombo->findData(boundedDuration);
        if (comboIndex >= 0 && m_animationSpeedCombo->currentIndex() != comboIndex) {
            QSignalBlocker blocker(m_animationSpeedCombo);
            m_animationSpeedCombo->setCurrentIndex(comboIndex);
        }
    }

    if (m_boardWidget) {
        m_boardWidget->setAnimationDurationMs(boundedDuration);
    }

    if (persistSetting) {
        saveAnimationDurationMs();
    }
}

void MainWindow::rebuildBraidSelector()
{
    if (!m_braidSelector) {
        return;
    }

    if (m_braids.isEmpty()) {
        m_braids = legacyBraidsCatalog();
    }

    QSignalBlocker blocker(m_braidSelector);
    m_braidSelector->clear();

    for (const BraidDefinition &braid : m_braids) {
        m_braidSelector->addItem(braid.name, braid.code);
    }

    int idx = m_braidSelector->findData(m_selectedBraidCode);
    if (idx < 0) {
        idx = 0;
    }
    if (idx >= 0) {
        m_selectedBraidCode = m_braidSelector->itemData(idx).toString();
        m_braidSelector->setCurrentIndex(idx);
        saveSelectedBraidCode();
    }
}

const MainWindow::BraidDefinition *MainWindow::currentBraidDefinition() const
{
    for (const BraidDefinition &braid : m_braids) {
        if (braid.code == m_selectedBraidCode) {
            return &braid;
        }
    }

    if (!m_braids.isEmpty()) {
        return &m_braids.front();
    }

    return nullptr;
}

const QVector<MainWindow::Move3082> &MainWindow::currentMoveSet() const
{
    static const QVector<Move3082> kEmptyMoves;
    const BraidDefinition *braid = currentBraidDefinition();
    if (!braid) {
        return kEmptyMoves;
    }
    return braid->moves;
}

QString MainWindow::currentBraidTitle() const
{
    const BraidDefinition *braid = currentBraidDefinition();
    if (!braid) {
        return QStringLiteral("Tresse");
    }
    return braid->name;
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
    , ui(new Ui::BraidingMainWindow)
{
    if (parent != nullptr) {
        setWindowFlags(Qt::Widget);
    }

    loadBraidsCatalog();
    loadUiPreferences();
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

void MainWindow::applyChromeStyle()
{
    if (QWidget *topWidget = menuWidget()) {
        topWidget->setObjectName(QStringLiteral("topChromeWidget"));
        topWidget->setStyleSheet(QStringLiteral(
            "#topChromeWidget {"
            " background: #f6ede2;"
            " border-bottom: 1px solid #d7c2aa;"
            " }"
            "#topChromeWidget QLabel {"
            " color: #2f2418;"
            " }"
            "#topChromeWidget QComboBox,"
            "#topChromeWidget QPushButton,"
            "#topChromeWidget QToolButton {"
            " color: #2f2418;"
            " background: #fffdfa;"
            " border: 1px solid #d7c2aa;"
            " border-radius: 8px;"
            " padding: 4px 10px;"
            " }"
            "#topChromeWidget QComboBox::drop-down {"
            " border: none;"
            " width: 18px;"
            " }"
            "#topChromeWidget QComboBox QAbstractItemView {"
            " background: #fffdfa;"
            " color: #2f2418;"
            " selection-background-color: #efe1d0;"
            " selection-color: #221a12;"
            " border: 1px solid #d7c2aa;"
            " }"
            "#topChromeWidget QPushButton:hover,"
            "#topChromeWidget QToolButton:hover,"
            "#topChromeWidget QComboBox:hover {"
            " background: #f3e7d9;"
            " }"
            "#topChromeWidget QPushButton:pressed,"
            "#topChromeWidget QToolButton:pressed {"
            " background: #ead9c7;"
            " }"
            "#topChromeWidget QMenu {"
            " background: #fffdfa;"
            " color: #2f2418;"
            " border: 1px solid #d7c2aa;"
            " }"
            "#topChromeWidget QMenu::item {"
            " color: #2f2418;"
            " padding: 6px 24px 6px 28px;"
            " }"
            "#topChromeWidget QMenu::item:selected {"
            " background: #efe1d0;"
            " color: #221a12;"
            " }"
            "#topChromeWidget QMenu::separator {"
            " height: 1px;"
            " background: #e3d3bf;"
            " margin: 5px 10px;"
            " }"));
    }

    if (statusBar()) {
        statusBar()->setStyleSheet(QStringLiteral(
            "QStatusBar {"
            " background: #f6ede2;"
            " color: #2f2418;"
            " border-top: 1px solid #d7c2aa;"
            " }"
            "QStatusBar QLabel {"
            " color: #2f2418;"
            " }"
            "QStatusBar::item {"
            " border: none;"
            " }"));
    }
}

void MainWindow::setupInterface()
{
    ui->setupUi(this);
    const QString languageCode = effectiveUiLanguageCode();

    m_titleLabel = ui->titleLabel;
    m_progressLabel = ui->progressLabel;
    m_moveLabel = ui->moveLabel;
    m_detailLabel = ui->detailLabel;
    m_noteLabel = ui->noteLabel;
    m_doneButton = ui->doneButton;
    m_restartButton = ui->restartButton;
    m_movesTable = ui->movesTable;
    m_sequentialTimer = new QTimer(this);
    m_sequentialTimer->setSingleShot(true);
    m_sequentialCountdownTimer = new QTimer(this);
    m_sequentialCountdownTimer->setInterval(1000);

    m_animationButton = new QPushButton(
        uiText(languageCode, "Séquentiel", "Sequential", "Sequenziell", "Sequenziale"), this);

    auto *topWidget = new QWidget(this);
    topWidget->setObjectName(QStringLiteral("topChromeWidget"));
    auto *topLayout = new QHBoxLayout(topWidget);
    topLayout->setContentsMargins(8, 4, 8, 4);
    topLayout->setSpacing(8);

    auto *leftControlsLayout = new QHBoxLayout();
    leftControlsLayout->setContentsMargins(0, 0, 0, 0);
    leftControlsLayout->setSpacing(8);

    auto *rightControlsLayout = new QHBoxLayout();
    rightControlsLayout->setContentsMargins(0, 0, 0, 0);
    rightControlsLayout->setSpacing(8);

    m_braidSelector = new QComboBox(topWidget);
    m_braidSelector->setMinimumWidth(340);
    m_braidSelector->setToolTip(uiText(languageCode,
                                       "Sélection de tresse ABoK",
                                       "ABoK braid selection",
                                       "ABoK-Geflecht auswählen",
                                       "Selezione treccia ABoK"));

    m_animationSpeedCombo = new QComboBox(topWidget);
    m_animationSpeedCombo->setMinimumWidth(92);
    m_animationSpeedCombo->setToolTip(uiText(languageCode,
                                            "Intervalle du mode séquentiel",
                                            "Sequential mode interval",
                                            "Intervall des sequenziellen Modus",
                                            "Intervallo della modalità sequenziale"));
    for (const int optionMs : sequentialIntervalOptionsMs()) {
        m_animationSpeedCombo->addItem(sequentialIntervalLabel(optionMs), optionMs);
    }

    m_sequentialCountdownLabel = new QLabel(topWidget);
    m_sequentialCountdownLabel->setMinimumWidth(42);
    m_sequentialCountdownLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    m_sequentialCountdownLabel->setToolTip(uiText(languageCode,
                                                  "Compte à rebours avant le prochain pas",
                                                  "Countdown before the next step",
                                                  "Countdown bis zum nächsten Schritt",
                                                  "Conto alla rovescia prima del prossimo passo"));
    resetSequentialCountdownLabel();

    m_compactLineLabel = new QLabel(topWidget);
    m_compactLineLabel->setTextFormat(Qt::RichText);
    m_compactLineLabel->setAlignment(Qt::AlignCenter);
    m_compactLineLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    m_compactLineLabel->setMinimumWidth(180);
    m_compactLineLabel->setToolTip(QStringLiteral("Ctrl+I : inscription / AuthorID"));
    QFont compactFont = m_compactLineLabel->font();
    compactFont.setPointSize(compactFont.pointSize() + 2);
    compactFont.setBold(true);
    m_compactLineLabel->setFont(compactFont);

    m_loadDrawingButton = new QToolButton(topWidget);
    m_loadDrawingButton->setText(uiText(languageCode, "Dessin", "Drawing", "Zeichnung", "Disegno"));
    m_loadDrawingButton->setToolTip(uiText(languageCode,
                                           "Charger un dessin de référence",
                                           "Load a reference drawing",
                                           "Referenzzeichnung laden",
                                           "Carica un disegno di riferimento"));
    m_loadDrawingButton->setMinimumWidth(62);

    m_openDocButton = new QToolButton(topWidget);
    m_openDocButton->setText(uiText(languageCode, "Doc", "Doc", "Dok", "Doc"));
    m_openDocButton->setToolTip(uiText(languageCode,
                                       "Ouvrir Solid Sinnet variations.pdf",
                                       "Open Solid Sinnet variations.pdf",
                                       "Solid Sinnet variations.pdf öffnen",
                                       "Apri Solid Sinnet variations.pdf"));
    m_openDocButton->setMinimumWidth(44);

    m_mainMenuButton = new QToolButton(topWidget);
    m_mainMenuButton->setText(uiText(languageCode, "Menu", "Menu", "Menü", "Menu"));
    m_mainMenuButton->setToolTip(uiText(languageCode,
                                        "Langues / Aide / À propos",
                                        "Languages / Help / About",
                                        "Sprachen / Hilfe / Info",
                                        "Lingue / Aiuto / Informazioni"));
    m_mainMenuButton->setPopupMode(QToolButton::InstantPopup);
    m_mainMenuButton->setMinimumWidth(60);

    auto *menu = new QMenu(m_mainMenuButton);
    auto *langMenu = menu->addMenu(uiText(languageCode, "Langues", "Languages", "Sprachen", "Lingue"));
    QAction *autoLang = langMenu->addAction(uiText(languageCode, "Système (auto)", "System (auto)", "System (auto)", "Sistema (auto)"));
    QAction *frLang = langMenu->addAction(uiText(languageCode, "Français", "French", "Französisch", "Francese"));
    QAction *enLang = langMenu->addAction(QStringLiteral("English"));
    connect(autoLang, &QAction::triggered, this, [this]() { applyUiLanguage(QStringLiteral("auto")); });
    connect(frLang, &QAction::triggered, this, [this]() { applyUiLanguage(QStringLiteral("fr_FR")); });
    connect(enLang, &QAction::triggered, this, [this]() { applyUiLanguage(QStringLiteral("en_US")); });

    QAction *braidEditorAction = menu->addAction(uiText(languageCode,
                                                        "Éditeur des tresses...",
                                                        "Braids editor...",
                                                        "Geflecht-Editor...",
                                                        "Editor delle trecce..."));
    connect(braidEditorAction, &QAction::triggered, this, &MainWindow::openBraidsEditor);

    QAction *reloadCatalogAction = menu->addAction(uiText(languageCode,
                                                          "Recharger les tresses (JSON)",
                                                          "Reload braids (JSON)",
                                                          "Geflechte neu laden (JSON)",
                                                          "Ricarica le trecce (JSON)"));
    connect(reloadCatalogAction, &QAction::triggered, this, [this, languageCode]() {
        loadBraidsCatalog();
        rebuildBraidSelector();
        reloadCurrentBraid();
        updateView();
        if (statusBar()) {
            statusBar()->showMessage(uiText(languageCode,
                                            "Catalogue recharge depuis %1",
                                            "Catalog reloaded from %1",
                                            "Katalog neu geladen von %1",
                                            "Catalogo ricaricato da %1").arg(m_braidsJsonPath), 4000);
        }
    });

    menu->addSeparator();
    auto *helpMenu = menu->addMenu(uiText(languageCode, "Aide", "Help", "Hilfe", "Aiuto"));
    QAction *quickHelp = helpMenu->addAction(uiText(languageCode, "Aide rapide", "Quick help", "Schnellhilfe", "Aiuto rapido"));
    QAction *openGuide = helpMenu->addAction(uiText(languageCode,
                                                    "Ouvrir le guide Solid Sinnet",
                                                    "Open the Solid Sinnet guide",
                                                    "Solid-Sinnet-Leitfaden öffnen",
                                                    "Apri la guida Solid Sinnet"));
    connect(quickHelp, &QAction::triggered, this, &MainWindow::openQuickHelp);
    connect(openGuide, &QAction::triggered, this, &MainWindow::openReferenceDocument);

    menu->addSeparator();
    QAction *aboutAction = menu->addAction(uiText(languageCode, "À propos", "About", "Info", "Informazioni"));
    connect(aboutAction, &QAction::triggered, this, &MainWindow::openAboutDialog);

    m_mainMenuButton->setMenu(menu);

    m_toggleDetailsButton = new QToolButton(topWidget);
    m_toggleDetailsButton->setText(QStringLiteral("\u2193"));
    m_toggleDetailsButton->setToolTip(uiText(languageCode,
                                             "Afficher/masquer les détails",
                                             "Show or hide details",
                                             "Details ein- oder ausblenden",
                                             "Mostra o nascondi i dettagli"));
    m_toggleDetailsButton->setMinimumWidth(30);

    leftControlsLayout->addWidget(m_braidSelector, 0);
    leftControlsLayout->addWidget(m_animationButton);
    leftControlsLayout->addWidget(m_animationSpeedCombo, 0);
    leftControlsLayout->addWidget(m_sequentialCountdownLabel, 0);

    rightControlsLayout->addWidget(m_loadDrawingButton);
    rightControlsLayout->addWidget(m_openDocButton);
    rightControlsLayout->addWidget(m_mainMenuButton);
    rightControlsLayout->addWidget(m_toggleDetailsButton);

    topLayout->addLayout(leftControlsLayout, 0);
    topLayout->addStretch(1);
    topLayout->addWidget(m_compactLineLabel, 0, Qt::AlignCenter);
    topLayout->addStretch(1);
    topLayout->addLayout(rightControlsLayout, 0);
    setMenuWidget(topWidget);

    QFont infoFont = m_progressLabel->font();
    infoFont.setPointSize(qMax(8, infoFont.pointSize() - 1));
    QFont progressFont = infoFont;
    progressFont.setBold(true);
    m_progressLabel->setFont(progressFont);
    m_moveLabel->setFont(infoFont);
    m_detailLabel->setFont(infoFont);

    QFont buttonFont = m_doneButton->font();
    buttonFont.setPointSize(buttonFont.pointSize() + 1);
    m_doneButton->setFont(buttonFont);
    if (m_animationButton) {
        m_animationButton->setFont(buttonFont);
    }
    m_progressLabel->setWordWrap(true);
    m_moveLabel->setWordWrap(true);
    m_detailLabel->setWordWrap(true);

    QFont actionFont = buttonFont;
    actionFont.setPointSize(actionFont.pointSize() + 16);
    actionFont.setBold(true);
    m_doneButton->setFont(actionFont);
    m_restartButton->setFont(actionFont);
    m_doneButton->setText(QStringLiteral("\u2713"));
    m_restartButton->setText(QStringLiteral("\u2715"));
    m_doneButton->setToolTip(uiText(languageCode, "Mouvement suivant", "Next move", "Nächste Bewegung", "Mossa successiva"));
    m_restartButton->setToolTip(uiText(languageCode,
                                       "Recommencer depuis le début",
                                       "Restart from the beginning",
                                       "Von vorne beginnen",
                                       "Ricomincia dall'inizio"));
    m_doneButton->setCursor(Qt::PointingHandCursor);
    m_restartButton->setCursor(Qt::PointingHandCursor);
    m_doneButton->setFlat(true);
    m_restartButton->setFlat(true);
    m_doneButton->setFixedSize(50, 50);
    m_restartButton->setFixedSize(50, 50);
    m_doneButton->setObjectName(QStringLiteral("moveNextButton"));
    m_restartButton->setObjectName(QStringLiteral("restartMoveButton"));
    m_doneButton->setStyleSheet(QStringLiteral(
        "QPushButton#moveNextButton { border: none; background: transparent; color: #5b61d6; }"
        "QPushButton#moveNextButton:hover { background: rgba(91, 97, 214, 0.10); border-radius: 25px; }"
        "QPushButton#moveNextButton:disabled { color: #b7bae8; }"));
    m_restartButton->setStyleSheet(QStringLiteral(
        "QPushButton#restartMoveButton { border: none; background: transparent; color: #ef4f73; }"
        "QPushButton#restartMoveButton:hover { background: rgba(239, 79, 115, 0.10); border-radius: 25px; }"
        "QPushButton#restartMoveButton:disabled { color: #e9bdc8; }"));

    m_titleLabel->hide();
    m_noteLabel->hide();

    m_boardWidget = new BraidBoardWidget(this);
    m_boardWidget->setMinimumHeight(520);
    m_boardWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    applyAnimationDurationMs(m_animationDurationMs, false);

    auto *contentWidget = new QWidget(this);
    auto *contentLayout = new QHBoxLayout(contentWidget);
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setSpacing(18);

    auto *instructionPanel = new QWidget(contentWidget);
    instructionPanel->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
    instructionPanel->setMaximumWidth(390);
    auto *instructionPanelLayout = new QHBoxLayout(instructionPanel);
    instructionPanelLayout->setContentsMargins(0, 0, 0, 0);
    instructionPanelLayout->setSpacing(12);

    auto *instructionFrame = new QFrame(instructionPanel);
    instructionFrame->setObjectName(QStringLiteral("instructionFrame"));
    instructionFrame->setStyleSheet(QStringLiteral(
        "#instructionFrame { background: #fffdfa; border: 2px solid #111111; border-radius: 4px; }"
        "#instructionFrame QLabel { color: #111111; }"));

    auto *instructionFrameLayout = new QVBoxLayout(instructionFrame);
    instructionFrameLayout->setContentsMargins(12, 10, 12, 10);
    instructionFrameLayout->setSpacing(4);

    ui->verticalLayout->removeWidget(m_progressLabel);
    ui->verticalLayout->removeWidget(m_moveLabel);
    ui->verticalLayout->removeWidget(m_detailLabel);
    instructionFrameLayout->addWidget(m_progressLabel);
    instructionFrameLayout->addWidget(m_moveLabel);
    instructionFrameLayout->addWidget(m_detailLabel);

    if (ui->horizontalLayoutButtons) {
        while (QLayoutItem *item = ui->horizontalLayoutButtons->takeAt(0)) {
            delete item;
        }
        ui->horizontalLayoutButtons->setContentsMargins(0, 0, 0, 0);
        ui->horizontalLayoutButtons->setSpacing(0);
    }

    auto *actionsLayout = new QVBoxLayout();
    actionsLayout->setContentsMargins(0, 0, 0, 0);
    actionsLayout->setSpacing(10);
    actionsLayout->addStretch(1);
    actionsLayout->addWidget(m_doneButton, 0, Qt::AlignHCenter);
    actionsLayout->addWidget(m_restartButton, 0, Qt::AlignHCenter);
    actionsLayout->addStretch(1);

    instructionPanelLayout->addWidget(instructionFrame, 1);
    instructionPanelLayout->addLayout(actionsLayout);

    contentLayout->addWidget(instructionPanel, 0, Qt::AlignTop);
    contentLayout->addWidget(m_boardWidget, 1);

    ui->verticalLayout->replaceWidget(ui->imageLabel, contentWidget);
    ui->imageLabel->hide();
    rebuildBraidSelector();
    reloadCurrentBraid();

    handleColorModeChanged(0);
    handlePalettePresetChanged(0);

    connect(m_doneButton, &QPushButton::clicked, this, &MainWindow::handleMoveDone);
    if (m_animationButton) {
        connect(m_animationButton, &QPushButton::clicked, this, &MainWindow::handleFullAnimation);
    }
    connect(m_restartButton, &QPushButton::clicked, this, &MainWindow::restartSequence);
    connect(m_sequentialTimer, &QTimer::timeout, this, [this]() { advanceSequentialStep(); });
    connect(m_sequentialCountdownTimer, &QTimer::timeout, this, [this]() { updateSequentialCountdown(); });
    connect(m_toggleDetailsButton, &QToolButton::clicked, this, [this]() {
        setDetailsExpanded(!m_detailsExpanded);
    });
    connect(m_loadDrawingButton, &QToolButton::clicked, this, &MainWindow::loadReferenceDrawing);
    connect(m_openDocButton, &QToolButton::clicked, this, &MainWindow::openReferenceDocument);
    connect(m_braidSelector, qOverload<int>(&QComboBox::currentIndexChanged),
            this, &MainWindow::handleBraidSelectionChanged);
    connect(m_animationSpeedCombo, qOverload<int>(&QComboBox::currentIndexChanged),
            this, &MainWindow::handleAnimationSpeedChanged);

    setDetailsExpanded(false);
    applyChromeStyle();
}
void MainWindow::handleBraidSelectionChanged(int index)
{
    if (!m_braidSelector || index < 0) {
        return;
    }

    if (m_sequentialModeActive || (m_boardWidget && m_boardWidget->isAnimating())) {
        QSignalBlocker blocker(m_braidSelector);
        const int currentIndex = m_braidSelector->findData(m_selectedBraidCode);
        if (currentIndex >= 0) {
            m_braidSelector->setCurrentIndex(currentIndex);
        }
        const QString languageCode = effectiveUiLanguageCode();
        statusBar()->showMessage(uiText(languageCode,
                                        "Impossible de changer de tresse pendant un séquentiel.",
                                        "Cannot change braid during sequential mode.",
                                        "Das Geflecht kann im Sequenziell-Modus nicht geändert werden.",
                                        "Impossibile cambiare treccia durante la modalità sequenziale."),
                                 3000);
        return;
    }

    const QString code = m_braidSelector->itemData(index).toString();
    if (code.isEmpty() || code == m_selectedBraidCode) {
        return;
    }

    m_selectedBraidCode = code;
    saveSelectedBraidCode();
    reloadCurrentBraid();
    updateView();
    const QString languageCode = effectiveUiLanguageCode();
    statusBar()->showMessage(uiText(languageCode,
                                    "Tresse sélectionnée : %1",
                                    "Selected braid: %1",
                                    "Ausgewähltes Geflecht: %1",
                                    "Treccia selezionata: %1").arg(currentBraidTitle()),
                             3500);
}


void MainWindow::openBraidsEditor()
{
    QVector<BraidDefinition> draftBraids = m_braids;
    QSet<QString> explicitlyDeletedCodes;
    if (draftBraids.isEmpty()) {
        draftBraids = legacyBraidsCatalog();
    }

    QDialog dialog(this);
    dialog.setWindowTitle(QStringLiteral("Editeur des tresses (JSON)"));
    dialog.setModal(true);
    dialog.resize(940, 640);

    auto *layout = new QVBoxLayout(&dialog);

    auto *selectorRow = new QHBoxLayout();
    auto *selectorLabel = new QLabel(QStringLiteral("Tresse :"), &dialog);
    auto *catalogCombo = new QComboBox(&dialog);
    auto *newBraidButton = new QPushButton(QStringLiteral("Nouvelle"), &dialog);
    auto *deleteBraidButton = new QPushButton(QStringLiteral("Supprimer"), &dialog);
    selectorRow->addWidget(selectorLabel);
    selectorRow->addWidget(catalogCombo, 1);
    selectorRow->addWidget(newBraidButton);
    selectorRow->addWidget(deleteBraidButton);
    layout->addLayout(selectorRow);

    auto *form = new QFormLayout();
    auto *codeEdit = new QLineEdit(&dialog);
    auto *nameEdit = new QLineEdit(&dialog);
    form->addRow(QStringLiteral("Code"), codeEdit);
    form->addRow(QStringLiteral("Nom explicite"), nameEdit);
    layout->addLayout(form);

    auto *table = new QTableWidget(&dialog);
    table->setColumnCount(5);
    table->setHorizontalHeaderLabels({
        QStringLiteral("Sequence/Case"),
        QStringLiteral("nb brins"),
        QStringLiteral("depart"),
        QStringLiteral("arrivee"),
        QStringLiteral("sens")
    });
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->verticalHeader()->setDefaultSectionSize(24);
    layout->addWidget(table, 1);

    auto *rowsButtons = new QHBoxLayout();
    auto *addRowButton = new QPushButton(QStringLiteral("Ajouter ligne"), &dialog);
    auto *removeRowButton = new QPushButton(QStringLiteral("Supprimer ligne"), &dialog);
    rowsButtons->addWidget(addRowButton);
    rowsButtons->addWidget(removeRowButton);
    rowsButtons->addStretch(1);
    layout->addLayout(rowsButtons);

    auto *hintLabel = new QLabel(
        QStringLiteral("Colonnes attendues: sequence_case, nb brins, depart, arrivee + sens via combo Wingdings 3 (P/Q)."),
        &dialog);
    hintLabel->setWordWrap(true);
    layout->addWidget(hintLabel);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel, &dialog);
    layout->addWidget(buttons);

    auto makeItem = [](const QString &text) {
        auto *item = new QTableWidgetItem(text);
        item->setTextAlignment(Qt::AlignCenter);
        return item;
    };

    auto rowText = [table](int row, int col) {
        const QTableWidgetItem *item = table->item(row, col);
        return item ? item->text().trimmed() : QString();
    };

    auto ensureSenseCombo = [&](int row, QChar code) {
        if (row < 0 || row >= table->rowCount()) {
            return;
        }

        auto *combo = qobject_cast<QComboBox *>(table->cellWidget(row, 4));
        if (!combo) {
            combo = new QComboBox(table);
            QFont f = combo->font();
            f.setFamily(QStringLiteral("Wingdings 3"));
            f.setPointSize(f.pointSize() + 2);
            combo->setFont(f);
            combo->addItem(QStringLiteral("P"), QStringLiteral("P"));
            combo->addItem(QStringLiteral("Q"), QStringLiteral("Q"));
            combo->setToolTip(QStringLiteral("P = Horaire, Q = Antihoraire"));
            table->setCellWidget(row, 4, combo);
        }

        const QString codeText = QString(code.toUpper());
        int idx = combo->findData(codeText);
        if (idx < 0) {
            idx = 0;
        }
        combo->setCurrentIndex(idx);
    };

    auto inferSenseFromDepart = [&](int row) -> QChar {
        bool ok = false;
        const int depart = rowText(row, 2).toInt(&ok);
        if (!ok || depart <= 0) {
            return QLatin1Char('P');
        }
        return (depart % 2 == 0) ? QLatin1Char('P') : QLatin1Char('Q');
    };

    auto applyAutoSenseForRow = [&](int row) {
        ensureSenseCombo(row, inferSenseFromDepart(row));
    };

    auto readSenseCode = [&](int row, QString *errorMessage) -> QChar {
        if (auto *combo = qobject_cast<QComboBox *>(table->cellWidget(row, 4))) {
            const QString code = combo->currentData().toString().trimmed().toUpper();
            if (code == QStringLiteral("Q")) {
                return QLatin1Char('Q');
            }
            return QLatin1Char('P');
        }

        const QString senseText = rowText(row, 4).trimmed().toUpper();
        if (senseText.isEmpty()) {
            return QLatin1Char('P');
        }
        if (senseText.startsWith(QLatin1Char('Q')) || senseText.startsWith(QStringLiteral("ANTI"))) {
            return QLatin1Char('Q');
        }
        if (senseText.startsWith(QLatin1Char('P')) || senseText.startsWith(QStringLiteral("H"))) {
            return QLatin1Char('P');
        }

        if (errorMessage) {
            *errorMessage = QStringLiteral("Ligne %1: sens invalide.").arg(row + 1);
        }
        return QChar();
    };

    int activeIndex = 0;
    bool updatingTable = false;

    auto fillCatalogCombo = [&]() {
        QSignalBlocker blocker(catalogCombo);
        catalogCombo->clear();
        for (const BraidDefinition &braid : draftBraids) {
            catalogCombo->addItem(QStringLiteral("%1 (%2)").arg(braid.name, braid.code), braid.code);
        }
        deleteBraidButton->setEnabled(draftBraids.size() > 1);
    };

    auto loadBraidToUi = [&](int index) {
        if (index < 0 || index >= draftBraids.size()) {
            return;
        }

        updatingTable = true;
        activeIndex = index;
        const BraidDefinition &braid = draftBraids[index];

        codeEdit->setText(braid.code);
        nameEdit->setText(braid.name);

        if (braid.moves.isEmpty()) {
            table->setRowCount(1);
            for (int col = 0; col < table->columnCount() - 1; ++col) {
                table->setItem(0, col, makeItem(QString()));
            }
            applyAutoSenseForRow(0);
            updatingTable = false;
            return;
        }

        table->setRowCount(braid.moves.size());
        for (int row = 0; row < braid.moves.size(); ++row) {
            const Move3082 &move = braid.moves[row];
            const int sequenceCase = move.step > 0 ? move.step : move.boardCase;
            table->setItem(row, 0, makeItem(QString::number(sequenceCase)));
            table->setItem(row, 1, makeItem(QString::number(move.strands)));
            table->setItem(row, 2, makeItem(QString::number(move.fromPeg)));
            table->setItem(row, 3, makeItem(QString::number(move.toPeg)));
            ensureSenseCombo(row, move.rotationCode);
        }
        updatingTable = false;
    };

    auto refreshActiveCatalogItem = [&]() {
        if (activeIndex < 0 || activeIndex >= draftBraids.size()) {
            return;
        }

        QString code = codeEdit->text().trimmed();
        QString name = nameEdit->text().trimmed();
        if (name.isEmpty()) {
            name = code.isEmpty() ? QStringLiteral("Nouvelle tresse") : code;
        }

        const QString shownCode = code.isEmpty() ? QStringLiteral("...") : code;
        QSignalBlocker blocker(catalogCombo);
        catalogCombo->setItemText(activeIndex, QStringLiteral("%1 (%2)").arg(name, shownCode));
        catalogCombo->setItemData(activeIndex, code, Qt::UserRole);
    };

    connect(codeEdit, &QLineEdit::textChanged, &dialog, [&](const QString &) {
        refreshActiveCatalogItem();
    });
    connect(nameEdit, &QLineEdit::textChanged, &dialog, [&](const QString &) {
        refreshActiveCatalogItem();
    });

    connect(table, &QTableWidget::itemChanged, &dialog, [&](QTableWidgetItem *item) {
        if (!item || updatingTable) {
            return;
        }
        if (item->column() != 2) {
            return;
        }
        applyAutoSenseForRow(item->row());
    });

    auto storeUiToBraid = [&](int index, bool strict, QString *errorMessage) -> bool {
        if (index < 0 || index >= draftBraids.size()) {
            return true;
        }

        BraidDefinition &braid = draftBraids[index];
        braid.code = codeEdit->text().trimmed();
        braid.name = nameEdit->text().trimmed();

        QVector<Move3082> moves;
        moves.reserve(table->rowCount());

        for (int row = 0; row < table->rowCount(); ++row) {
            const QString sequenceCaseText = rowText(row, 0);
            const QString strandsText = rowText(row, 1);
            const QString fromText = rowText(row, 2);
            const QString toText = rowText(row, 3);

            const bool rowEmpty = sequenceCaseText.isEmpty()
                                  && strandsText.isEmpty()
                                  && fromText.isEmpty()
                                  && toText.isEmpty();
            if (rowEmpty) {
                continue;
            }

            auto parsePositive = [&](const QString &value, const QString &label, int *out) -> bool {
                bool ok = false;
                const int n = value.toInt(&ok);
                if (!ok || n <= 0) {
                    if (errorMessage) {
                        *errorMessage = QStringLiteral("Ligne %1: valeur invalide pour '%2'.").arg(row + 1).arg(label);
                    }
                    return false;
                }
                *out = n;
                return true;
            };

            int sequenceCase = 0;
            int strands = 0;
            int fromPeg = 0;
            int toPeg = 0;

            if (!parsePositive(sequenceCaseText, QStringLiteral("Sequence/Case"), &sequenceCase)
                || !parsePositive(strandsText, QStringLiteral("nb brins"), &strands)
                || !parsePositive(fromText, QStringLiteral("depart"), &fromPeg)
                || !parsePositive(toText, QStringLiteral("arrivee"), &toPeg)) {
                return false;
            }

            QString senseError;
            const QChar rotationCode = readSenseCode(row, &senseError);
            if (rotationCode.isNull()) {
                if (errorMessage) {
                    *errorMessage = senseError;
                }
                return false;
            }

            Move3082 move{};
            move.step = sequenceCase;
            move.boardCase = sequenceCase;
            move.strands = strands;
            move.fromPeg = fromPeg;
            move.toPeg = toPeg;
            move.rotationCode = rotationCode;
            moves.push_back(move);
        }

        if (strict && moves.isEmpty()) {
            if (errorMessage) {
                *errorMessage = QStringLiteral("La tresse doit contenir au moins une ligne de mouvement.");
            }
            return false;
        }

        std::sort(moves.begin(), moves.end(), [](const Move3082 &a, const Move3082 &b) {
            return a.step < b.step;
        });

        braid.moves = moves;
        return true;
    };

    connect(catalogCombo, qOverload<int>(&QComboBox::currentIndexChanged), &dialog, [&](int newIndex) {
        if (newIndex < 0 || newIndex >= draftBraids.size() || newIndex == activeIndex) {
            return;
        }

        QString error;
        if (!storeUiToBraid(activeIndex, false, &error)) {
            QMessageBox::warning(&dialog, QStringLiteral("Editeur des tresses"), error);
            QSignalBlocker blocker(catalogCombo);
            catalogCombo->setCurrentIndex(activeIndex);
            return;
        }

        loadBraidToUi(newIndex);
    });

    connect(newBraidButton, &QPushButton::clicked, &dialog, [&]() {
        QString error;
        if (!storeUiToBraid(activeIndex, false, &error)) {
            QMessageBox::warning(&dialog, QStringLiteral("Editeur des tresses"), error);
            return;
        }

        int suffix = 1;
        QString code;
        while (true) {
            code = QStringLiteral("new_braid_%1").arg(suffix++);
            bool exists = false;
            for (const BraidDefinition &braid : draftBraids) {
                if (braid.code.compare(code, Qt::CaseInsensitive) == 0) {
                    exists = true;
                    break;
                }
            }
            if (!exists) {
                break;
            }
        }

        BraidDefinition created;
        created.code = code;
        created.name = QStringLiteral("Nouvelle tresse");
        draftBraids.push_back(created);

        fillCatalogCombo();
        const int newIndex = draftBraids.size() - 1;
        catalogCombo->setCurrentIndex(newIndex);
        loadBraidToUi(newIndex);
        codeEdit->setFocus();
        codeEdit->selectAll();
    });

    connect(deleteBraidButton, &QPushButton::clicked, &dialog, [&]() {
        if (draftBraids.size() <= 1 || activeIndex < 0 || activeIndex >= draftBraids.size()) {
            return;
        }

        const auto answer = QMessageBox::question(
            &dialog,
            QStringLiteral("Supprimer la tresse"),
            QStringLiteral("Supprimer '%1' ?").arg(draftBraids[activeIndex].name),
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::No);

        if (answer != QMessageBox::Yes) {
            return;
        }

        explicitlyDeletedCodes.insert(draftBraids[activeIndex].code.trimmed().toLower());
        draftBraids.removeAt(activeIndex);
        fillCatalogCombo();

        const int nextIndex = qMin(activeIndex, draftBraids.size() - 1);
        catalogCombo->setCurrentIndex(nextIndex);
        loadBraidToUi(nextIndex);
    });

    connect(addRowButton, &QPushButton::clicked, &dialog, [&]() {
        const int row = table->rowCount();
        table->insertRow(row);
        for (int col = 0; col < table->columnCount() - 1; ++col) {
            table->setItem(row, col, makeItem(QString()));
        }
        table->item(row, 0)->setText(QString::number(row + 1));
        applyAutoSenseForRow(row);
        table->setCurrentCell(row, 0);
        table->editItem(table->item(row, 0));
    });

    connect(removeRowButton, &QPushButton::clicked, &dialog, [&]() {
        if (table->rowCount() <= 0) {
            return;
        }

        int row = table->currentRow();
        if (row < 0) {
            row = table->rowCount() - 1;
        }
        table->removeRow(row);

        if (table->rowCount() == 0) {
            table->setRowCount(1);
            for (int col = 0; col < table->columnCount() - 1; ++col) {
                table->setItem(0, col, makeItem(QString()));
            }
            applyAutoSenseForRow(0);
        }
    });

    connect(buttons, &QDialogButtonBox::accepted, &dialog, [&]() {
        const int comboIndex = catalogCombo->currentIndex();
        if (comboIndex >= 0 && comboIndex < draftBraids.size()) {
            activeIndex = comboIndex;
        }

        QString error;
        if (!storeUiToBraid(activeIndex, true, &error)) {
            QMessageBox::warning(&dialog, QStringLiteral("Editeur des tresses"), error);
            return;
        }

        auto normalizeCodeBase = [](QString text) {
            text = text.trimmed().toLower();
            text.replace(QRegularExpression(QStringLiteral("[^a-z0-9]+")), QStringLiteral("_"));
            text.remove(QRegularExpression(QStringLiteral("^_+|_+$")));
            return text;
        };

        auto makeUniqueCode = [&](const QString &baseCode, int skipIndex) {
            QString candidate = baseCode;
            int suffix = 2;
            auto exists = [&](const QString &code) {
                for (int k = 0; k < draftBraids.size(); ++k) {
                    if (k == skipIndex) {
                        continue;
                    }
                    if (draftBraids[k].code.compare(code, Qt::CaseInsensitive) == 0) {
                        return true;
                    }
                }
                return false;
            };

            while (exists(candidate)) {
                candidate = QStringLiteral("%1_%2").arg(baseCode).arg(suffix++);
            }
            return candidate;
        };

        QVector<int> placeholdersToRemove;
        QString targetCode = (activeIndex >= 0 && activeIndex < draftBraids.size())
                                 ? draftBraids[activeIndex].code
                                 : QString();

        for (int i = 0; i < draftBraids.size(); ++i) {
            BraidDefinition &braid = draftBraids[i];
            braid.name = braid.name.trimmed();

            QString normalizedCode = braid.code.trimmed();
            if (normalizedCode.isEmpty()) {
                const QString baseFromName = normalizeCodeBase(braid.name);
                normalizedCode = baseFromName.isEmpty()
                                     ? QStringLiteral("new_braid_%1").arg(i + 1)
                                     : baseFromName;
                normalizedCode = makeUniqueCode(normalizedCode, i);
            }
            braid.code = normalizedCode;

            if (braid.code.isEmpty()) {
                QMessageBox::warning(&dialog,
                                     QStringLiteral("Editeur des tresses"),
                                     QStringLiteral("Chaque tresse doit avoir un code non vide."));
                return;
            }
            if (braid.name.isEmpty()) {
                braid.name = braid.code;
            }

            if (braid.moves.isEmpty()) {
                const bool looksLikePlaceholderCode = braid.code.startsWith(QStringLiteral("new_braid_"),
                                                                            Qt::CaseInsensitive);
                const bool looksLikePlaceholderName =
                    braid.name.compare(QStringLiteral("Nouvelle tresse"), Qt::CaseInsensitive) == 0
                    || braid.name.compare(braid.code, Qt::CaseInsensitive) == 0;

                if (looksLikePlaceholderCode && looksLikePlaceholderName) {
                    placeholdersToRemove.push_back(i);
                    continue;
                }

                QMessageBox::warning(&dialog,
                                     QStringLiteral("Editeur des tresses"),
                                     QStringLiteral("La tresse '%1' ne contient aucun mouvement.").arg(braid.name));
                {
                    QSignalBlocker blocker(catalogCombo);
                    catalogCombo->setCurrentIndex(i);
                }
                loadBraidToUi(i);
                return;
            }
        }

        std::sort(placeholdersToRemove.begin(), placeholdersToRemove.end(), std::greater<int>());
        for (int indexToRemove : placeholdersToRemove) {
            if (indexToRemove >= 0 && indexToRemove < draftBraids.size()) {
                draftBraids.removeAt(indexToRemove);
            }
        }

        for (int i = 0; i < draftBraids.size(); ++i) {
            const BraidDefinition &braid = draftBraids[i];
            for (int j = i + 1; j < draftBraids.size(); ++j) {
                if (braid.code.compare(draftBraids[j].code, Qt::CaseInsensitive) == 0) {
                    QMessageBox::warning(
                        &dialog,
                        QStringLiteral("Editeur des tresses"),
                        QStringLiteral("Code duplique: '%1'.").arg(braid.code));
                    return;
                }
            }
        }

        if (draftBraids.isEmpty()) {
            QMessageBox::warning(&dialog,
                                 QStringLiteral("Editeur des tresses"),
                                 QStringLiteral("Le catalogue ne contient aucune tresse complete."));
            return;
        }

        // Safety net: keep previously known braids unless user explicitly deleted them.
        for (const BraidDefinition &existing : m_braids) {
            const QString existingCode = existing.code.trimmed().toLower();
            if (existingCode.isEmpty() || explicitlyDeletedCodes.contains(existingCode)) {
                continue;
            }

            bool present = false;
            for (const BraidDefinition &b : draftBraids) {
                if (b.code.compare(existing.code, Qt::CaseInsensitive) == 0) {
                    present = true;
                    break;
                }
            }
            if (!present) {
                draftBraids.push_back(existing);
            }
        }

        for (int i = 0; i < draftBraids.size(); ++i) {
            const BraidDefinition &braid = draftBraids[i];
            for (int j = i + 1; j < draftBraids.size(); ++j) {
                if (braid.code.compare(draftBraids[j].code, Qt::CaseInsensitive) == 0) {
                    QMessageBox::warning(
                        &dialog,
                        QStringLiteral("Editeur des tresses"),
                        QStringLiteral("Code duplique: '%1'.").arg(braid.code));
                    return;
                }
            }
        }

        sortBraidsByAbok(&draftBraids);

        QString saveError;
        if (!saveBraidsCatalog(draftBraids, &saveError)) {
            QMessageBox::warning(
                &dialog,
                QStringLiteral("Editeur des tresses"),
                saveError.isEmpty() ? QStringLiteral("Echec de sauvegarde JSON.") : saveError);
            return;
        }

        m_braids = draftBraids;

        bool targetCodeExists = false;
        for (const BraidDefinition &b : m_braids) {
            if (b.code.compare(targetCode, Qt::CaseInsensitive) == 0) {
                targetCodeExists = true;
                break;
            }
        }
        if ((!targetCodeExists || targetCode.isEmpty()) && !m_braids.isEmpty()) {
            targetCode = m_braids.front().code;
        }
        if (!targetCode.isEmpty()) {
            m_selectedBraidCode = targetCode;
            saveSelectedBraidCode();
        }

        rebuildBraidSelector();
        reloadCurrentBraid();
        updateView();

        if (statusBar()) {
            statusBar()->showMessage(
                QStringLiteral("Catalogue enregistre dans %1").arg(m_braidsJsonPath),
                5000);
        }

        dialog.accept();
    });

    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    fillCatalogCombo();

    int initialIndex = 0;
    if (catalogCombo->count() > 0) {
        const int found = catalogCombo->findData(m_selectedBraidCode);
        if (found >= 0) {
            initialIndex = found;
        }
    }

    catalogCombo->setCurrentIndex(initialIndex);
    loadBraidToUi(initialIndex);

    dialog.exec();
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
        QStringLiteral("Choisir le dessin de r\u00E9f\u00E9rence"),
        initialDir,
        QStringLiteral("Images (*.bmp *.png *.jpg *.jpeg)"));

    if (selectedPath.isEmpty()) {
        return;
    }

    QString error;
    if (!m_boardWidget->loadReferenceImage(selectedPath, &error)) {
        QMessageBox::warning(this,
                             QStringLiteral("Dessin de r\u00E9f\u00E9rence"),
                             error.isEmpty() ? QStringLiteral("Chargement impossible.") : error);
        return;
    }

    s.setValue(key, selectedPath);
    s.sync();
    const QString languageCode = effectiveUiLanguageCode();
    statusBar()->showMessage(uiText(languageCode,
                                    "Dessin chargé : %1",
                                    "Drawing loaded: %1",
                                    "Zeichnung geladen: %1",
                                    "Disegno caricato: %1").arg(QFileInfo(selectedPath).fileName()),
                             3500);
}

void MainWindow::handleColorModeChanged(int index)
{
    Q_UNUSED(index);

    if (!m_boardWidget) {
        return;
    }

    m_boardWidget->setColorApplyMode(BraidBoardWidget::ColorApplyMode::OrbitPath);
    m_boardWidget->setQuickPaletteEnabled(false);
    m_boardWidget->resetQuickPaletteIndex();
}

void MainWindow::handlePalettePresetChanged(int index)
{
    Q_UNUSED(index);

    if (!m_boardWidget) {
        return;
    }

    m_boardWidget->setQuickPalette({});
    m_boardWidget->setQuickPaletteEnabled(false);
    m_boardWidget->resetQuickPaletteIndex();
}

void MainWindow::handleAnimationSpeedChanged(int value)
{
    if (!m_animationSpeedCombo) {
        return;
    }

    const int durationMs = m_animationSpeedCombo->itemData(value).toInt();
    if (durationMs <= 0) {
        return;
    }

    applyAnimationDurationMs(durationMs, true);

    if (statusBar()) {
        const QString languageCode = effectiveUiLanguageCode();
        statusBar()->showMessage(uiText(languageCode,
                                        "Intervalle séquentiel : %1.",
                                        "Sequential interval: %1.",
                                        "Sequenzielles Intervall: %1.",
                                        "Intervallo sequenziale: %1.")
                                     .arg(sequentialIntervalLabel(m_animationDurationMs)),
                                 2000);
    }
}

void MainWindow::openQuickHelp()
{
    const QString languageCode = effectiveUiLanguageCode();
    QMessageBox::information(
        this,
        uiText(languageCode, "Aide rapide", "Quick help", "Schnellhilfe", "Aiuto rapido"),
        uiText(languageCode,
               "1) Choisir la tresse dans la liste.\n"
               "2) Regarder le vecteur rouge de prévisualisation.\n"
               "3) Cliquer sur ✓ pour passer immédiatement au pas suivant.\n"
               "4) Cliquer 'Séquentiel' pour avancer automatiquement d'un pas à intervalle fixe.\n"
               "5) Le combo règle l'intervalle entre 5 s et 60 s entre deux pas.\n"
               "6) Le coloriage reste fixé sur le mode Parcours avec palette libre.\n"
               "7) Cliquer un clou occupé pour colorer un fil ou un parcours complet.\n"
               "8) Les couleurs sont sauvegardées automatiquement en .lbc (JSON).",
               "1) Choose the braid in the list.\n"
               "2) Watch the red preview vector.\n"
               "3) Click ✓ to move immediately to the next step.\n"
               "4) Click 'Sequential' to advance automatically one step at a fixed interval.\n"
               "5) The combo sets the interval between 5 s and 60 s between two steps.\n"
               "6) Coloring stays fixed on Path mode with a free palette.\n"
               "7) Click an occupied peg to color one strand or a complete path.\n"
               "8) Colors are saved automatically in .lbc (JSON).",
               "1) Wählen Sie das Geflecht in der Liste.\n"
               "2) Beobachten Sie den roten Vorschauvektor.\n"
               "3) Klicken Sie auf ✓, um sofort zum nächsten Schritt zu gehen.\n"
               "4) Klicken Sie auf 'Sequenziell', um automatisch in festem Intervall weiterzugehen.\n"
               "5) Die Auswahlliste stellt das Intervall zwischen 5 s und 60 s zwischen zwei Schritten ein.\n"
               "6) Die Färbung bleibt auf Modus Parcours mit freier Palette fixiert.\n"
               "7) Klicken Sie auf einen belegten Nagel, um einen Faden oder einen ganzen Verlauf zu färben.\n"
               "8) Die Farben werden automatisch in .lbc (JSON) gespeichert.",
               "1) Scegli la treccia nell'elenco.\n"
               "2) Osserva il vettore rosso di anteprima.\n"
               "3) Clicca su ✓ per passare subito al passo successivo.\n"
               "4) Clicca 'Sequenziale' per avanzare automaticamente di un passo a intervallo fisso.\n"
               "5) Il combo regola l'intervallo tra 5 s e 60 s tra due passi.\n"
               "6) La colorazione resta fissa in modalità Percorso con tavolozza libera.\n"
               "7) Clicca un chiodo occupato per colorare un filo o un percorso completo.\n"
               "8) I colori vengono salvati automaticamente in .lbc (JSON)."));
}

void MainWindow::openAboutDialog()
{
    const QString languageCode = effectiveUiLanguageCode();
    QMessageBox::about(
        this,
        uiText(languageCode,
               "À propos de LogiBraiding",
               "About LogiBraiding",
               "Über LogiBraiding",
               "Informazioni su LogiBraiding"),
        uiText(languageCode,
               "LogiBraiding\n"
               "Assistant visuel de tressage ABoK\n\n"
               "Version d'essai: 15 jours\n"
               "Inscription: AuthorID (Ctrl+I)\n\n"
               "Suite logicielle: LogiKnotting + LogiBraiding",
               "LogiBraiding\n"
               "ABoK visual braiding assistant\n\n"
               "Trial version: 15 days\n"
               "Registration: AuthorID (Ctrl+I)\n\n"
               "Software suite: LogiKnotting + LogiBraiding",
               "LogiBraiding\n"
               "Visueller ABoK-Flechtassistent\n\n"
               "Testversion: 15 Tage\n"
               "Registrierung: AuthorID (Ctrl+I)\n\n"
               "Software-Suite: LogiKnotting + LogiBraiding",
               "LogiBraiding\n"
               "Assistente visivo di intreccio ABoK\n\n"
               "Versione di prova: 15 giorni\n"
               "Registrazione: AuthorID (Ctrl+I)\n\n"
               "Suite software: LogiKnotting + LogiBraiding"));
}

void MainWindow::applyUiLanguage(const QString &languageCode)
{
    const QString normalized = canonicalUiLanguageCode(languageCode);
    const QString effectiveLanguageCode =
        (normalized == QStringLiteral("auto")) ? effectiveUiLanguageCode() : normalized;

    QSettings s = appSettings();
    s.setValue(uiLanguageSettingKey(), normalized);
    s.sync();

    QSettings shell = shellSettings();
    shell.setValue(uiLanguageSettingKey(), normalized);
    shell.sync();

    const QMessageBox::StandardButton answer = QMessageBox::question(
        this,
        uiText(effectiveLanguageCode, "Langue", "Language", "Sprache", "Lingua"),
        uiText(effectiveLanguageCode,
               "La langue sera appliquée au prochain démarrage.\nRedémarrer maintenant ?",
               "The language will be applied at the next startup.\nRestart now?",
               "Die Sprache wird beim nächsten Start angewendet.\nJetzt neu starten?",
               "La lingua sarà applicata al prossimo avvio.\nRiavvia ora?"),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::Yes);

    if (answer != QMessageBox::Yes) {
        statusBar()->showMessage(uiText(effectiveLanguageCode,
                                        "Langue enregistrée : redémarrage requis.",
                                        "Language saved: restart required.",
                                        "Sprache gespeichert: Neustart erforderlich.",
                                        "Lingua salvata: riavvio richiesto."),
                                 3500);
        return;
    }

    const QString appPath = QCoreApplication::applicationFilePath();
    const QStringList args = QCoreApplication::arguments().mid(1);
    const bool launched = QProcess::startDetached(appPath, args);

    if (!launched) {
        QMessageBox::warning(this,
                             uiText(effectiveLanguageCode, "Langue", "Language", "Sprache", "Lingua"),
                             uiText(effectiveLanguageCode,
                                    "Impossible de relancer automatiquement l'application.",
                                    "Unable to restart the application automatically.",
                                    "Die Anwendung konnte nicht automatisch neu gestartet werden.",
                                    "Impossibile riavviare automaticamente l'applicazione."));
        return;
    }

    qApp->quit();
}

void MainWindow::openReferenceDocument()
{
    const QString languageCode = effectiveUiLanguageCode();
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
        statusBar()->showMessage(uiText(languageCode,
                                        "Document introuvable: %1",
                                        "Document not found: %1",
                                        "Dokument nicht gefunden: %1",
                                        "Documento non trovato: %1").arg(fileName),
                                 5000);
        return;
    }

    if (!QDesktopServices::openUrl(QUrl::fromLocalFile(docPath))) {
        statusBar()->showMessage(uiText(languageCode,
                                        "Impossible d'ouvrir le document de référence.",
                                        "Unable to open the reference document.",
                                        "Das Referenzdokument konnte nicht geöffnet werden.",
                                        "Impossibile aprire il documento di riferimento."),
                                 5000);
        return;
    }

    statusBar()->showMessage(uiText(languageCode,
                                    "Document ouvert: %1",
                                    "Document opened: %1",
                                    "Dokument geöffnet: %1",
                                    "Documento aperto: %1").arg(QFileInfo(docPath).fileName()),
                             3500);
}
void MainWindow::setDetailsExpanded(bool expanded)
{
    m_detailsExpanded = expanded;
    m_progressLabel->setVisible(true);
    m_moveLabel->setVisible(true);
    m_detailLabel->setVisible(true);
    m_movesTable->setVisible(expanded);

    if (m_toggleDetailsButton) {
        m_toggleDetailsButton->setText(expanded ? QStringLiteral("\u2191") : QStringLiteral("\u2193"));
    }
}

void MainWindow::configureMovesTable()
{
    const QString languageCode = effectiveUiLanguageCode();
    m_movesTable->setColumnCount(6);
    m_movesTable->setHorizontalHeaderLabels({
        uiText(languageCode, "Étape", "Step", "Schritt", "Passo"),
        uiText(languageCode, "Case", "Cell", "Feld", "Casella"),
        uiText(languageCode, "Brins", "Strands", "Stränge", "Fili"),
        uiText(languageCode, "Départ", "From", "Start", "Partenza"),
        uiText(languageCode, "Arrivée", "To", "Ziel", "Arrivo"),
        uiText(languageCode, "Rotation", "Rotation", "Rotation", "Rotazione")
    });

    m_movesTable->setRowCount(1);
    m_movesTable->setVerticalHeaderLabels({uiText(languageCode, "Active", "Active", "Aktiv", "Attiva")});
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
            ? QStringLiteral("La p\u00E9riode d'essai de 15 jours est expir\u00E9e. Entrez une cl\u00E9 de d\u00E9bridage pour continuer.")
            : QStringLiteral("Entrez un email valide pour demander une inscription, ou collez directement votre cl\u00E9 de d\u00E9bridage."),
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
    form->addRow(QStringLiteral("Cl\u00E9 de d\u00E9bridage"), keyEdit);

    layout->addLayout(form);

    auto *help = new QLabel(
        QStringLiteral("Validation avec email : ouvre votre client mail vers la bo\u00EEte d'inscription.\nValidation avec cl\u00E9 : d\u00E9bloque imm\u00E9diatement l'application et cr\u00E9e l'AuthorID."),
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
                                     QStringLiteral("Entrez d'abord un email valide associ\u00E9 \u00E0 cette cl\u00E9."));
                return;
            }

            if (!verifyUnlockKey(emailForKey, keyInput)) {
                QMessageBox::warning(this,
                                     QStringLiteral("Inscription"),
                                     QStringLiteral("Cl\u00E9 invalide pour cet email."));
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
                QStringLiteral("Inscription valid\u00E9e. Application d\u00E9brid\u00E9e.\nAuthorID : %1").arg(authorId));
            dialog.accept();
            return;
        }

        if (!isValidEmail(emailInput)) {
            QMessageBox::warning(this,
                                 QStringLiteral("Inscription"),
                                 QStringLiteral("Entrez un email valide ou une cl\u00E9."));
            return;
        }

        sLocal.setValue(QStringLiteral("registration/pending_email"), emailInput);
        sLocal.sync();

        const QString inbox = QString::fromLatin1(kRegistrationInboxEmail).trimmed();
        if (inbox.isEmpty() || inbox.startsWith(QStringLiteral("CHANGE_ME"), Qt::CaseInsensitive)) {
            QMessageBox::information(
                this,
                QStringLiteral("Inscription"),
                QStringLiteral("Email d'inscription non configur\u00E9 dans le code.\nRemplacez kRegistrationInboxEmail dans MainWindow.cpp puis recompilez."));
            return;
        }

        QUrl mailUrl;
        mailUrl.setScheme(QStringLiteral("mailto"));
        mailUrl.setPath(inbox);

        QUrlQuery query;
        query.addQueryItem(QStringLiteral("subject"), QStringLiteral("Demande d'inscription LogiBraiding"));
        query.addQueryItem(QStringLiteral("body"),
                           QStringLiteral("Bonjour,\n\nMerci de m'envoyer une cl\u00E9 de d\u00E9bridage pour cet email :\n%1\n\nCordialement,")
                               .arg(emailInput));
        mailUrl.setQuery(query);

        if (!QDesktopServices::openUrl(mailUrl)) {
            QMessageBox::warning(this,
                                 QStringLiteral("Inscription"),
                                 QStringLiteral("Impossible d'ouvrir le client email par d\u00E9faut."));
            return;
        }

        QMessageBox::information(
            this,
            QStringLiteral("Inscription"),
            QStringLiteral("Demande envoy\u00E9e via votre client email.\nQuand vous recevez la cl\u00E9, revenez ici et collez-la dans le champ cl\u00E9."));
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
                QStringLiteral("Version d'essai : %1 jour(s) restant(s). Ctrl+I pour inscription.").arg(remaining),
                6000);
        }
        return true;
    }

    QMessageBox::warning(this,
                         QStringLiteral("P\u00E9riode d'essai expir\u00E9e"),
                         QStringLiteral("La p\u00E9riode d'essai de 15 jours est termin\u00E9e.\nUne inscription est n\u00E9cessaire pour continuer."));
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
    Q_UNUSED(move);
    Q_UNUSED(totalMoves);

    if (!m_compactLineLabel) {
        return;
    }

    m_compactLineLabel->setText(QStringLiteral("| %1 |").arg(registrationStatusText()));
}

bool MainWindow::performCurrentMove(QString *errorMessage)
{
    if (!m_boardWidget || m_currentMoveIndex >= static_cast<int>(currentMoveSet().size())) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Aucun mouvement restant.");
        }
        return false;
    }

    const auto &move = currentMoveSet()[m_currentMoveIndex];
    QString localError;
    if (!m_boardWidget->stepMove(move.fromPeg, move.toPeg, &localError)) {
        if (errorMessage) {
            *errorMessage = localError;
        }
        return false;
    }

    ++m_currentMoveIndex;

    updateView();
    return true;
}

bool MainWindow::isCompletionHoldState() const
{
    const int totalMoves = static_cast<int>(currentMoveSet().size());
    return totalMoves > 0 && m_currentMoveIndex >= totalMoves;
}

void MainWindow::prepareNextSequence()
{
    if (!m_boardWidget) {
        return;
    }

    m_boardWidget->resetBoard();
    m_currentMoveIndex = 0;
    updateView();
}

void MainWindow::setSequentialModeActive(bool active)
{
    const QString languageCode = effectiveUiLanguageCode();
    m_sequentialModeActive = active;
    const bool hasMoves = !currentMoveSet().isEmpty();

    if (!active && m_sequentialTimer) {
        m_sequentialTimer->stop();
    }
    if (!active && m_sequentialCountdownTimer) {
        m_sequentialCountdownTimer->stop();
    }
    if (!active) {
        m_sequentialRemainingMs = 0;
        resetSequentialCountdownLabel();
    }

    m_doneButton->setEnabled(!active && hasMoves);
    if (m_animationButton) {
        m_animationButton->setEnabled(!active && hasMoves);
        m_animationButton->setText(uiText(languageCode,
                                          "Séquentiel",
                                          "Sequential",
                                          "Sequenziell",
                                          "Sequenziale"));
    }
    m_restartButton->setEnabled(true);
    if (m_braidSelector) {
        m_braidSelector->setEnabled(!active);
    }
    if (m_animationSpeedCombo) {
        m_animationSpeedCombo->setEnabled(!active);
    }
}

void MainWindow::startSequentialCountdown(int durationMs)
{
    m_sequentialRemainingMs = qMax(0, durationMs);
    updateSequentialCountdown();

    if (m_sequentialCountdownTimer) {
        if (m_sequentialRemainingMs > 1000) {
            m_sequentialCountdownTimer->start();
        } else {
            m_sequentialCountdownTimer->stop();
        }
    }
}

void MainWindow::updateSequentialCountdown()
{
    if (!m_sequentialCountdownLabel) {
        return;
    }

    if (!m_sequentialModeActive || m_sequentialRemainingMs <= 0) {
        resetSequentialCountdownLabel();
        return;
    }

    const int remainingSeconds = qMax(1, (m_sequentialRemainingMs + 999) / 1000);
    m_sequentialCountdownLabel->setText(QStringLiteral("%1s").arg(remainingSeconds));

    m_sequentialRemainingMs = qMax(0, m_sequentialRemainingMs - 1000);
    if (m_sequentialRemainingMs <= 0 && m_sequentialCountdownTimer) {
        m_sequentialCountdownTimer->stop();
    }
}

void MainWindow::resetSequentialCountdownLabel()
{
    if (!m_sequentialCountdownLabel) {
        return;
    }

    m_sequentialCountdownLabel->setText(QStringLiteral("--"));
}

void MainWindow::advanceSequentialStep()
{
    const QString languageCode = effectiveUiLanguageCode();
    if (!m_sequentialModeActive) {
        return;
    }

    if (isCompletionHoldState()) {
        prepareNextSequence();
        setSequentialModeActive(false);
        statusBar()->showMessage(uiText(languageCode,
                                        "Séquentiel terminé. Étape 1 prête pour la séquence suivante.",
                                        "Sequential mode finished. Step 1 is ready for the next sequence.",
                                        "Sequenziell beendet. Schritt 1 ist für die nächste Sequenz bereit.",
                                        "Modalità sequenziale terminata. Il passo 1 è pronto per la sequenza successiva."),
                                 4500);
        return;
    }

    const bool wasLastMove = (m_currentMoveIndex == static_cast<int>(currentMoveSet().size()) - 1);

    QString errorMessage;
    if (!performCurrentMove(&errorMessage)) {
        setSequentialModeActive(false);
        statusBar()->showMessage(errorMessage, 5000);
        return;
    }

    if (wasLastMove) {
        if (m_sequentialTimer) {
            m_sequentialTimer->start(m_animationDurationMs);
        }
        startSequentialCountdown(m_animationDurationMs);
        statusBar()->showMessage(uiText(languageCode,
                                        "Fin du tour affichée. Retour à l'étape 1 dans %1.",
                                        "End of round displayed. Returning to step 1 in %1.",
                                        "Rundenende angezeigt. Rückkehr zu Schritt 1 in %1.",
                                        "Fine del giro visualizzata. Ritorno al passo 1 tra %1.")
                                     .arg(sequentialIntervalLabel(m_animationDurationMs)),
                                 3000);
        return;
    }

    if (m_sequentialTimer) {
        m_sequentialTimer->start(m_animationDurationMs);
    }
    startSequentialCountdown(m_animationDurationMs);
    statusBar()->showMessage(uiText(languageCode,
                                    "Séquentiel : prochain pas dans %1.",
                                    "Sequential mode: next step in %1.",
                                    "Sequenziell: nächster Schritt in %1.",
                                    "Sequenziale: prossimo passo tra %1.")
                                 .arg(sequentialIntervalLabel(m_animationDurationMs)),
                             2000);
}

void MainWindow::updateView()
{
    const QString languageCode = effectiveUiLanguageCode();
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
        m_progressLabel->setText(uiText(languageCode, "étape %1/%1", "step %1/%1", "Schritt %1/%1", "passo %1/%1")
                                     .arg(totalMoves));
        m_moveLabel->setText(uiText(languageCode, "tour terminé", "round completed", "Runde beendet", "giro completato"));
        m_detailLabel->setText(uiText(languageCode,
                                      "cliquer sur ✓ pour préparer l'étape 1 suivante",
                                      "click ✓ to prepare the next step 1",
                                      "auf ✓ klicken, um den nächsten Schritt 1 vorzubereiten",
                                      "clicca su ✓ per preparare il prossimo passo 1"));

        m_boardWidget->setPendingMove(-1, -1);
        m_doneButton->setEnabled(!m_sequentialModeActive);
        m_doneButton->setToolTip(uiText(languageCode,
                                        "Afficher l'étape 1 suivante",
                                        "Show the next step 1",
                                        "Nächsten Schritt 1 anzeigen",
                                        "Mostra il prossimo passo 1"));
        if (m_animationButton) {
            m_animationButton->setEnabled(!m_sequentialModeActive);
            m_animationButton->setToolTip(uiText(languageCode,
                                                 "Préparer l'étape 1 puis lancer le séquentiel",
                                                 "Prepare step 1 then start sequential mode",
                                                 "Schritt 1 vorbereiten und dann den Sequenziell-Modus starten",
                                                 "Prepara il passo 1 e poi avvia la modalità sequenziale"));
        }
        m_restartButton->setToolTip(uiText(languageCode,
                                           "Recommencer depuis l'étape 1",
                                           "Restart from step 1",
                                           "Ab Schritt 1 neu beginnen",
                                           "Ricomincia dal passo 1"));

        populateActiveRow(moves.back(), QColor(226, 248, 229));

        if (!m_sequentialModeActive) {
            statusBar()->showMessage(uiText(languageCode,
                                            "Tour terminé. Le tracé final reste visible jusqu'au prochain ✓.",
                                            "Round completed. The final trace remains visible until the next ✓.",
                                            "Runde beendet. Die finale Spur bleibt bis zum nächsten ✓ sichtbar.",
                                            "Giro completato. Il tracciato finale resta visibile fino al prossimo ✓."));
        }
        return;
    }

    const auto &move = moves[m_currentMoveIndex];
    updateCompactLine(&move, totalMoves);
    m_boardWidget->setPendingMove(move.fromPeg, move.toPeg);

    m_progressLabel->setText(uiText(languageCode, "étape %1/%2", "step %1/%2", "Schritt %1/%2", "passo %1/%2")
                                 .arg(move.step)
                                 .arg(totalMoves));

    m_moveLabel->setText(
        uiText(languageCode,
               "prendre le fil intérieur de la case %1\nl'amener à l'extérieur de la case %2 %3",
               "take the inner strand from cell %1\nbring it to the outside of cell %2 %3",
               "den inneren Faden aus Feld %1 nehmen\nnach außen zu Feld %2 führen %3",
               "prendere il filo interno della casella %1\nportarlo all'esterno della casella %2 %3")
            .arg(move.fromPeg)
            .arg(move.toPeg)
            .arg(rotationHintSymbol(move.rotationCode)));

    m_detailLabel->setText(uiText(languageCode,
                                  "puis recentrer les fils restants de la case %1",
                                  "then recenter the remaining strands of cell %1",
                                  "dann die verbleibenden Fäden von Feld %1 neu zentrieren",
                                  "poi ricentrare i fili restanti della casella %1")
                              .arg(move.fromPeg));

    m_doneButton->setEnabled(!m_sequentialModeActive);
    if (m_animationButton) {
        m_animationButton->setEnabled(!m_sequentialModeActive);
    }
    m_doneButton->setToolTip(uiText(languageCode,
                                    "Mouvement suivant : étape %1",
                                    "Next move: step %1",
                                    "Nächste Bewegung: Schritt %1",
                                    "Mossa successiva: passo %1").arg(move.step));
    m_restartButton->setToolTip(uiText(languageCode,
                                       "Recommencer depuis l'étape 1",
                                       "Restart from step 1",
                                       "Ab Schritt 1 neu beginnen",
                                       "Ricomincia dal passo 1"));

    populateActiveRow(move, QColor(224, 238, 255));

    if (!m_sequentialModeActive) {
        statusBar()->showMessage(uiText(languageCode,
                                        "Mode didactique : effectuer le mouvement suivant ou lancer le séquentiel.",
                                        "Didactic mode: perform the next move or start sequential mode.",
                                        "Didaktischer Modus: nächste Bewegung ausführen oder Sequenziell starten.",
                                        "Modalità didattica: esegui la mossa successiva o avvia la modalità sequenziale."));
    }
}

void MainWindow::startCurrentMoveAnimation(bool fullPassage)
{
    const QString languageCode = effectiveUiLanguageCode();
    if (!m_boardWidget || m_sequentialModeActive) {
        return;
    }

    if (isCompletionHoldState()) {
        prepareNextSequence();
        if (!fullPassage) {
            statusBar()->showMessage(uiText(languageCode,
                                            "Étape 1 prête pour la séquence suivante.",
                                            "Step 1 ready for the next sequence.",
                                            "Schritt 1 ist für die nächste Sequenz bereit.",
                                            "Passo 1 pronto per la sequenza successiva."),
                                     3000);
            return;
        }
    }

    if (m_currentMoveIndex >= static_cast<int>(currentMoveSet().size())) {
        return;
    }

    if (fullPassage) {
        setSequentialModeActive(true);
        if (m_sequentialTimer) {
            m_sequentialTimer->start(m_animationDurationMs);
        }
        startSequentialCountdown(m_animationDurationMs);
        statusBar()->showMessage(uiText(languageCode,
                                        "Séquentiel lancé : prochain pas dans %1.",
                                        "Sequential mode started: next step in %1.",
                                        "Sequenziell gestartet: nächster Schritt in %1.",
                                        "Modalità sequenziale avviata: prossimo passo tra %1.")
                                     .arg(sequentialIntervalLabel(m_animationDurationMs)),
                                 2500);
    } else {
        QString errorMessage;
        const bool wasLastMove = (m_currentMoveIndex == static_cast<int>(currentMoveSet().size()) - 1);
        if (!performCurrentMove(&errorMessage)) {
            statusBar()->showMessage(errorMessage, 5000);
        } else if (wasLastMove) {
            statusBar()->showMessage(uiText(languageCode,
                                            "Tour terminé. Le tracé final reste visible ; ✓ préparera l'étape 1 suivante.",
                                            "Round completed. The final trace remains visible; ✓ will prepare the next step 1.",
                                            "Runde beendet. Die finale Spur bleibt sichtbar; ✓ bereitet den nächsten Schritt 1 vor.",
                                            "Giro completato. Il tracciato finale resta visibile; ✓ preparerà il prossimo passo 1."),
                                     4500);
        }
    }
}

void MainWindow::handleMoveDone()
{
    startCurrentMoveAnimation(false);
}

void MainWindow::handleFullAnimation()
{
    startCurrentMoveAnimation(true);
}

void MainWindow::restartSequence()
{
    setSequentialModeActive(false);
    m_currentMoveIndex = 0;
    m_boardWidget->resetBoard();
    clearFocus();
    updateView();
}
























































































} // namespace LogiBraidingApp

