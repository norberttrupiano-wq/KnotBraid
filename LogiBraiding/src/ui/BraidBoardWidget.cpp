#include "BraidBoardWidget.h"

#include <QColorDialog>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMouseEvent>
#include <QPainter>
#include <QPen>
#include <QPolygonF>
#include <QRegularExpression>
#include <QSet>
#include <QStandardPaths>
#include <QVariantAnimation>
#include <QtMath>

#include <algorithm>

namespace {
constexpr int kMinPegsPerCase = 3;
const QColor kDefaultThreadColor(0, 181, 235);
const char *kLegacyJsonFileName = "thread_colors_3082.json";

void drawArrowVector(QPainter &painter, const QPointF &start, const QPointF &end, const QColor &color, qreal width)
{
    QLineF line(start, end);
    if (line.length() < 1.0) {
        return;
    }

    painter.setPen(QPen(color, width, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    painter.setBrush(Qt::NoBrush);
    painter.drawLine(line);

    const QLineF unit = line.unitVector();
    const QPointF dir = unit.p2() - unit.p1();
    const QPointF normal(-dir.y(), dir.x());

    const qreal arrowLen = 10.0;
    const qreal halfBase = 3.8;

    const QPointF tip = end;
    const QPointF base = tip - dir * arrowLen;
    const QPointF left = base + normal * halfBase;
    const QPointF right = base - normal * halfBase;

    QPolygonF head;
    head << tip << left << right;

    painter.setPen(Qt::NoPen);
    painter.setBrush(color);
    painter.drawPolygon(head);
}
} // namespace

BraidBoardWidget::BraidBoardWidget(QWidget *parent)
    : QWidget(parent)
    , m_animation(new QVariantAnimation(this))
{
    setMinimumSize(600, 600);

    const QString appDataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (!appDataDir.isEmpty()) {
        QDir().mkpath(appDataDir);
        m_colorsStorageDir = appDataDir;
    } else {
        m_colorsStorageDir = QDir::currentPath();
    }

    setColorProfileKey(QStringLiteral("3082"));

    m_animation->setDuration(650);
    m_animation->setStartValue(0.0);
    m_animation->setEndValue(1.0);
    m_animation->setEasingCurve(QEasingCurve::InOutCubic);

    connect(m_animation, &QVariantAnimation::valueChanged, this, [this](const QVariant &value) {
        const qreal t = value.toReal();
        m_animPos = m_animStart + (m_animEnd - m_animStart) * t;
        update();
    });

    connect(m_animation, &QVariantAnimation::finished, this, [this]() {
        QString error;
        applyMove(m_animFrom + 1, m_animTo + 1, &error);

        m_animating = false;
        m_animFrom = -1;
        m_animTo = -1;
        m_animDestPeg = -1;
        m_animThreadId = -1;

        update();
        emit moveAnimationFinished(error.isEmpty(), error);
    });
}

bool BraidBoardWidget::loadReferenceImage(const QString &filePath, QString *errorMessage)
{
    if (filePath.trimmed().isEmpty()) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Chemin du dessin vide.");
        }
        return false;
    }

    const QImage image(filePath);
    if (image.isNull()) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Impossible de charger le dessin: %1").arg(filePath);
        }
        return false;
    }

    m_referenceImage = image;
    update();
    return true;
}

void BraidBoardWidget::clearReferenceImage()
{
    if (m_referenceImage.isNull()) {
        return;
    }

    m_referenceImage = QImage();
    update();
}

void BraidBoardWidget::setReferenceOverlayEnabled(bool enabled)
{
    if (m_referenceOverlayEnabled == enabled) {
        return;
    }

    m_referenceOverlayEnabled = enabled;
    update();
}

void BraidBoardWidget::setReferenceOverlayOpacity(qreal opacity)
{
    const qreal bounded = qBound<qreal>(0.0, opacity, 1.0);
    if (qFuzzyCompare(m_referenceOpacity, bounded)) {
        return;
    }

    m_referenceOpacity = bounded;
    update();
}
void BraidBoardWidget::setColorProfileKey(const QString &profileKey)
{
    QString normalized = profileKey.trimmed().toLower();
    if (normalized.isEmpty()) {
        normalized = QStringLiteral("default");
    }

    normalized.replace(QRegularExpression(QStringLiteral("[^a-z0-9_-]+")), QStringLiteral("_"));

    if (normalized == m_colorProfileKey) {
        return;
    }

    m_colorProfileKey = normalized;
    m_colorsJsonPath = QDir(m_colorsStorageDir).filePath(
        QStringLiteral("thread_colors_%1.lbc").arg(m_colorProfileKey));

    m_threadColors.clear();
    loadThreadColorsFromJson();
    update();
}

void BraidBoardWidget::initializeCases(const QVector<int> &initialThreadCounts)
{
    m_cases.clear();
    m_cases.resize(initialThreadCounts.size());

    for (int i = 0; i < initialThreadCounts.size(); ++i) {
        auto &state = m_cases[i];
        const int initialThreads = qMax(0, initialThreadCounts[i]);
        state.capacity = qMax(kMinPegsPerCase, initialThreads);
        state.initialThreads = qMin(initialThreads, state.capacity);
    }

    resetBoard();
}

void BraidBoardWidget::resetBoard()
{
    m_nextThreadId = 1;
    m_completedTraces.clear();
    m_threadOrigins.clear();

    for (int caseIdx = 0; caseIdx < m_cases.size(); ++caseIdx) {
        auto &state = m_cases[caseIdx];
        state.threads.clear();
        state.threads.reserve(state.capacity);

        for (int slot = 0; slot < state.initialThreads; ++slot) {
            const int threadId = m_nextThreadId++;
            state.threads.append(threadId);
            m_threadOrigins.insert(threadId, qMakePair(caseIdx + 1, slot + 1));
            if (!m_threadColors.contains(threadId)) {
                m_threadColors.insert(threadId, kDefaultThreadColor);
            }
        }
    }

    m_animating = false;
    m_animFrom = -1;
    m_animTo = -1;
    m_animDestPeg = -1;
    m_animThreadId = -1;

    update();
}

void BraidBoardWidget::setPendingMove(int fromCase, int toCase)
{
    m_pendingFrom = fromCase - 1;
    m_pendingTo = toCase - 1;

    const int caseCount = m_cases.size();
    if (fromCase <= 0 || toCase <= 0 || m_pendingFrom < 0 || m_pendingTo < 0
        || m_pendingFrom >= caseCount || m_pendingTo >= caseCount) {
        m_pendingFrom = -1;
        m_pendingTo = -1;
    }

    update();
}

void BraidBoardWidget::setCaseRouteMap(const QVector<int> &nextCaseByCase)
{
    const int caseCount = m_cases.size();

    m_caseRouteNext = nextCaseByCase;
    if (m_caseRouteNext.size() < caseCount) {
        m_caseRouteNext.resize(caseCount);
    }

    for (int i = 0; i < m_caseRouteNext.size(); ++i) {
        const int next = m_caseRouteNext[i];
        if (next < 0 || next >= caseCount) {
            m_caseRouteNext[i] = -1;
        }
    }

    m_caseOrbitId.fill(-1, caseCount);
    int nextOrbitId = 0;

    for (int i = 0; i < caseCount; ++i) {
        if (m_caseOrbitId[i] != -1) {
            continue;
        }

        QHash<int, int> pathPos;
        QVector<int> path;

        int cur = i;
        while (cur >= 0 && cur < caseCount
               && m_caseOrbitId[cur] == -1
               && !pathPos.contains(cur)) {
            pathPos.insert(cur, path.size());
            path.append(cur);
            cur = m_caseRouteNext.value(cur, -1);
        }

        int orbitId = -1;
        if (cur >= 0 && cur < caseCount) {
            if (m_caseOrbitId[cur] != -1) {
                orbitId = m_caseOrbitId[cur];
            } else if (pathPos.contains(cur)) {
                orbitId = nextOrbitId++;
            }
        }

        if (orbitId < 0) {
            orbitId = nextOrbitId++;
        }

        for (const int node : path) {
            m_caseOrbitId[node] = orbitId;
        }
    }
}


void BraidBoardWidget::setColorApplyMode(ColorApplyMode mode)
{
    m_colorApplyMode = mode;
}

void BraidBoardWidget::setQuickPalette(const QVector<QColor> &colors)
{
    QVector<QColor> filtered;
    filtered.reserve(colors.size());
    for (const QColor &color : colors) {
        if (color.isValid()) {
            filtered.push_back(color);
        }
    }

    m_quickPalette = filtered;
    if (m_quickPaletteIndex >= m_quickPalette.size()) {
        m_quickPaletteIndex = 0;
    }
}

void BraidBoardWidget::setQuickPaletteEnabled(bool enabled)
{
    m_quickPaletteEnabled = enabled;
    if (!enabled) {
        m_quickPaletteIndex = 0;
    }
}

void BraidBoardWidget::resetQuickPaletteIndex()
{
    m_quickPaletteIndex = 0;
}
bool BraidBoardWidget::isAnimating() const
{
    return m_animating;
}

void BraidBoardWidget::animateMove(int fromCase, int toCase)
{
    if (m_animating) {
        return;
    }

    const int caseCount = m_cases.size();
    const int from = fromCase - 1;
    const int to = toCase - 1;
    if (from < 0 || from >= caseCount || to < 0 || to >= caseCount) {
        emit moveAnimationFinished(false, QStringLiteral("Mouvement hors limites."));
        return;
    }

    auto &fromState = m_cases[from];
    auto &toState = m_cases[to];

    if (fromState.threads.isEmpty()) {
        emit moveAnimationFinished(false, QStringLiteral("Aucun fil a deplacer dans la case %1.").arg(fromCase));
        return;
    }

    const int destPeg = toState.threads.size();
    if (destPeg >= toState.capacity) {
        emit moveAnimationFinished(false, QStringLiteral("La case d'arrivee %1 est deja pleine.").arg(toCase));
        return;
    }

    m_animating = true;
    m_animFrom = from;
    m_animTo = to;
    m_animDestPeg = destPeg;
    m_animThreadId = fromState.threads.first();

    m_animStart = pegPosition(from, 0);
    m_animEnd = pegPosition(to, destPeg);
    m_animPos = m_animStart;

    update();
    m_animation->start();
}

QPointF BraidBoardWidget::centerPoint() const
{
    return rect().center();
}

QPointF BraidBoardWidget::pegPosition(int caseIndex, int pegIndex) const
{
    const int caseCount = m_cases.size();
    if (caseCount <= 0 || caseIndex < 0 || caseIndex >= caseCount) {
        return centerPoint();
    }

    const QRectF bounds = rect().adjusted(24, 24, -24, -24);
    const QPointF center = bounds.center();

    const qreal angleDeg = -90.0 + (360.0 / caseCount) * caseIndex;
    const qreal angleRad = qDegreesToRadians(angleDeg);
    const QPointF direction(qCos(angleRad), qSin(angleRad));

    const auto &state = m_cases[caseIndex];
    const qreal outerRadius = qMin(bounds.width(), bounds.height()) * 0.46;
    const qreal spacing = qMax<qreal>(10.0, qMin(bounds.width(), bounds.height()) * 0.018);
    const qreal innerRadius = outerRadius - spacing * qMax(0, state.capacity - 1);
    const qreal pegRadius = innerRadius + spacing * pegIndex;

    return center + direction * pegRadius;
}

void BraidBoardWidget::applyMove(int fromCase, int toCase, QString *errorMessage)
{
    const int caseCount = m_cases.size();
    const int from = fromCase - 1;
    const int to = toCase - 1;

    if (from < 0 || from >= caseCount || to < 0 || to >= caseCount) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Mouvement hors limites.");
        }
        return;
    }

    auto &fromState = m_cases[from];
    auto &toState = m_cases[to];

    if (fromState.threads.isEmpty()) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Aucun fil a deplacer dans la case %1.").arg(fromCase);
        }
        return;
    }

    const int destPeg = toState.threads.size();
    if (destPeg >= toState.capacity) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("La case d'arrivee %1 est deja pleine.").arg(toCase);
        }
        return;
    }

    const int movingThread = fromState.threads.first();
    m_completedTraces.push_back({from, to, destPeg, movingThread});

    fromState.threads.takeFirst();
    toState.threads.append(movingThread);
}

QColor BraidBoardWidget::colorForThread(int threadId) const
{
    return m_threadColors.value(threadId, kDefaultThreadColor);
}

bool BraidBoardWidget::pickOccupiedPegAt(const QPointF &pos, int *caseIndex, int *pegIndex, int *threadId) const
{
    qreal bestDistance = 12.0;
    bool found = false;

    for (int i = 0; i < m_cases.size(); ++i) {
        const auto &state = m_cases[i];
        for (int peg = 0; peg < state.threads.size(); ++peg) {
            if (m_animating && i == m_animFrom && peg == 0) {
                continue;
            }

            const QPointF candidate = pegPosition(i, peg);
            const qreal distance = QLineF(pos, candidate).length();
            if (distance <= bestDistance) {
                bestDistance = distance;
                found = true;
                if (caseIndex) {
                    *caseIndex = i;
                }
                if (pegIndex) {
                    *pegIndex = peg;
                }
                if (threadId) {
                    *threadId = state.threads[peg];
                }
            }
        }
    }

    return found;
}

void BraidBoardWidget::loadThreadColorsFromJson()
{
    if (m_colorsJsonPath.isEmpty()) {
        return;
    }

    QString pathToRead = m_colorsJsonPath;
    QFile file(pathToRead);

    if (!file.exists()) {
        if (m_colorProfileKey == QStringLiteral("3082")) {
            const QString legacyPath = QDir(m_colorsStorageDir).filePath(QString::fromLatin1(kLegacyJsonFileName));
            if (QFile::exists(legacyPath)) {
                pathToRead = legacyPath;
                file.setFileName(pathToRead);
            }
        }
    }

    if (!file.exists() || !file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return;
    }

    const QByteArray raw = file.readAll();
    file.close();

    const QJsonDocument doc = QJsonDocument::fromJson(raw);
    if (!doc.isObject()) {
        return;
    }

    const QJsonArray colors = doc.object().value(QStringLiteral("colors")).toArray();
    for (const QJsonValue &value : colors) {
        if (!value.isObject()) {
            continue;
        }

        const QJsonObject obj = value.toObject();
        const int threadId = obj.value(QStringLiteral("threadId")).toInt(-1);
        const QString colorHex = obj.value(QStringLiteral("color")).toString();
        const QColor color(colorHex);
        if (threadId > 0 && color.isValid()) {
            m_threadColors.insert(threadId, color);
        }
    }
}

void BraidBoardWidget::saveThreadColorsToJson() const
{
    if (m_colorsJsonPath.isEmpty()) {
        return;
    }

    QVector<int> ids = m_threadOrigins.keys().toVector();
    std::sort(ids.begin(), ids.end());

    QJsonArray colors;
    for (const int id : ids) {
        const QPair<int, int> origin = m_threadOrigins.value(id, qMakePair(-1, -1));

        QJsonObject obj;
        obj.insert(QStringLiteral("threadId"), id);
        obj.insert(QStringLiteral("case"), origin.first);
        obj.insert(QStringLiteral("fil"), origin.second);
        obj.insert(QStringLiteral("color"), colorForThread(id).name(QColor::HexRgb).toUpper());
        colors.append(obj);
    }

    QJsonObject root;
    root.insert(QStringLiteral("pattern"), m_colorProfileKey);
    root.insert(QStringLiteral("format"), QStringLiteral(".lbc (json)"));
    root.insert(QStringLiteral("colors"), colors);

    QFile file(m_colorsJsonPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        return;
    }

    file.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
    file.close();
}

void BraidBoardWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && !m_animating) {
        int caseIdx = -1;
        int pegIdx = -1;
        int threadId = -1;

        if (pickOccupiedPegAt(event->position(), &caseIdx, &pegIdx, &threadId) && threadId > 0) {
            const QColor current = colorForThread(threadId);
            QColor picked;
            if (m_quickPaletteEnabled && !m_quickPalette.isEmpty()) {
                picked = m_quickPalette[m_quickPaletteIndex % m_quickPalette.size()];
                m_quickPaletteIndex = (m_quickPaletteIndex + 1) % m_quickPalette.size();
            } else {
                picked = QColorDialog::getColor(current, this, QStringLiteral("Couleur du fil"));
            }

            if (picked.isValid()) {
                QSet<int> targetOriginCases;

                int originCaseIdx = caseIdx;
                const QPair<int, int> origin = m_threadOrigins.value(threadId, qMakePair(caseIdx + 1, pegIdx + 1));
                if (origin.first > 0) {
                    originCaseIdx = origin.first - 1;
                }

                if (m_colorApplyMode == ColorApplyMode::OrbitPath
                    && originCaseIdx >= 0
                    && originCaseIdx < m_caseOrbitId.size()) {
                    const int orbitId = m_caseOrbitId[originCaseIdx];
                    if (orbitId >= 0) {
                        for (int i = 0; i < m_caseOrbitId.size(); ++i) {
                            if (m_caseOrbitId[i] == orbitId) {
                                targetOriginCases.insert(i);
                            }
                        }
                    }
                }

                if (targetOriginCases.isEmpty() && originCaseIdx >= 0) {
                    targetOriginCases.insert(originCaseIdx);
                }

                bool applied = false;
                for (auto it = m_threadOrigins.constBegin(); it != m_threadOrigins.constEnd(); ++it) {
                    const int id = it.key();
                    const int originCase = it.value().first - 1;
                    if (targetOriginCases.contains(originCase)) {
                        m_threadColors.insert(id, picked);
                        applied = true;
                    }
                }

                if (!applied) {
                    m_threadColors.insert(threadId, picked);
                }

                saveThreadColorsToJson();
                update();
            }
            event->accept();
            return;
        }
    }

    QWidget::mousePressEvent(event);
}

void BraidBoardWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.fillRect(rect(), QColor(250, 250, 250));

    const int caseCount = m_cases.size();
    if (caseCount <= 0) {
        return;
    }

    const QRectF bounds = rect().adjusted(18, 18, -18, -18);
    const QPointF center = bounds.center();

    painter.setPen(QPen(QColor(230, 230, 230), 1));
    painter.drawEllipse(center, qMin(bounds.width(), bounds.height()) * 0.46, qMin(bounds.width(), bounds.height()) * 0.46);

    if (m_referenceOverlayEnabled && !m_referenceImage.isNull()) {
        const QImage scaled = m_referenceImage.scaled(bounds.size().toSize(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        const QPointF topLeft(center.x() - scaled.width() / 2.0,
                              center.y() - scaled.height() / 2.0);

        painter.save();
        painter.setOpacity(m_referenceOpacity);
        painter.drawImage(topLeft, scaled);
        painter.restore();
    }

    for (const MoveTrace &trace : m_completedTraces) {
        if (trace.fromCase < 0 || trace.fromCase >= caseCount
            || trace.toCase < 0 || trace.toCase >= caseCount || trace.toPeg < 0) {
            continue;
        }

        const QColor traceColor = (trace.threadId > 0) ? colorForThread(trace.threadId) : QColor(20, 130, 220);
        painter.setPen(QPen(traceColor, 4.0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));

        const QPointF start = pegPosition(trace.fromCase, 0);
        const QPointF end = pegPosition(trace.toCase, trace.toPeg);
        painter.drawLine(start, end);
    }

    if (!m_animating && m_pendingFrom >= 0 && m_pendingFrom < caseCount
        && m_pendingTo >= 0 && m_pendingTo < caseCount) {
        const auto &fromState = m_cases[m_pendingFrom];
        const auto &toState = m_cases[m_pendingTo];

        if (!fromState.threads.isEmpty()) {
            const int previewDestPeg = toState.threads.size();
            if (previewDestPeg < toState.capacity) {
                const QPointF previewStart = pegPosition(m_pendingFrom, 0);
                const QPointF previewEnd = pegPosition(m_pendingTo, previewDestPeg);
                drawArrowVector(painter, previewStart, previewEnd, QColor(210, 60, 50), 1.0);
            }
        }
    }

    for (int i = 0; i < caseCount; ++i) {
        const auto &state = m_cases[i];
        if (state.capacity <= 0) {
            continue;
        }

        const qreal angleDeg = -90.0 + (360.0 / caseCount) * i;
        const qreal angleRad = qDegreesToRadians(angleDeg);
        const QPointF direction(qCos(angleRad), qSin(angleRad));

        const QPointF numberPos = pegPosition(i, state.capacity - 1) + direction * 18.0;
        painter.setPen(QPen(QColor(70, 70, 70), 1));
        painter.setFont(QFont(QStringLiteral("Segoe UI"), 8));
        painter.drawText(QRectF(numberPos.x() - 10, numberPos.y() - 8, 20, 16), Qt::AlignCenter, QString::number(i + 1));

        for (int peg = 0; peg < state.capacity; ++peg) {
            const QPointF pos = pegPosition(i, peg);
            bool occupied = peg < state.threads.size();

            if (m_animating && i == m_animFrom && peg == 0) {
                occupied = false;
            }

            QColor fill = QColor(255, 255, 255);
            if (occupied) {
                const int threadId = state.threads[peg];
                fill = colorForThread(threadId);
            }

            QColor border(55, 55, 55);

            if (i == m_pendingFrom && peg == 0) {
                border = QColor(220, 70, 60);
            }

            if (i == m_pendingTo && peg == state.threads.size() && !m_animating) {
                border = QColor(230, 150, 20);
            }

            if (m_animating && i == m_animTo && peg == m_animDestPeg) {
                fill = QColor(255, 245, 200);
                border = QColor(230, 150, 20);
            }

            painter.setPen(QPen(border, 1.5));
            painter.setBrush(fill);
            painter.drawEllipse(pos, 6.5, 6.5);
        }
    }

    if (m_animating) {
        painter.setPen(QPen(QColor(220, 50, 40), 2));
        painter.drawLine(m_animStart, m_animPos);

        const QColor animColor = (m_animThreadId > 0) ? colorForThread(m_animThreadId) : QColor(240, 70, 60);
        painter.setPen(QPen(QColor(50, 50, 50), 1));
        painter.setBrush(animColor);
        painter.drawEllipse(m_animPos, 8.0, 8.0);
    }
}

















