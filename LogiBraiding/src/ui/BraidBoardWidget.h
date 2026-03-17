#ifndef BRAIDBOARDWIDGET_H
#define BRAIDBOARDWIDGET_H

#include <QColor>
#include <QHash>
#include <QImage>
#include <QPointF>
#include <QString>
#include <QVector>
#include <QWidget>

class QMouseEvent;
class QVariantAnimation;

class BraidBoardWidget : public QWidget
{
    Q_OBJECT

public:
    enum class ColorApplyMode {
        OrbitPath,
        OriginCase
    };

    explicit BraidBoardWidget(QWidget *parent = nullptr);

    void initializeCases(const QVector<int> &initialThreadCounts);
    void resetBoard();
    void setPendingMove(int fromCase, int toCase);
    void setCaseRouteMap(const QVector<int> &nextCaseByCase);
    void setColorApplyMode(ColorApplyMode mode);
    void setQuickPalette(const QVector<QColor> &colors);
    void setQuickPaletteEnabled(bool enabled);
    void resetQuickPaletteIndex();
    void setColorProfileKey(const QString &profileKey);
    bool loadReferenceImage(const QString &filePath, QString *errorMessage = nullptr);
    void clearReferenceImage();
    void setReferenceOverlayEnabled(bool enabled);
    void setReferenceOverlayOpacity(qreal opacity);
    void setAnimationDurationMs(int durationMs);
    int animationDurationMs() const;
    bool stepMove(int fromCase, int toCase, QString *errorMessage = nullptr);

    bool isAnimating() const;
    void animateMove(int fromCase, int toCase);

signals:
    void moveAnimationFinished(bool success, const QString &errorMessage);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

private:
    struct CaseState {
        int capacity = 0;
        int initialThreads = 0;
        QVector<int> threads;
    };

    struct MoveTrace {
        int fromCase = -1;
        int toCase = -1;
        int toPeg = -1;
        int threadId = -1;
    };

    QVector<CaseState> m_cases;
    QVector<MoveTrace> m_completedTraces;
    QHash<int, QColor> m_threadColors;
    QHash<int, QPair<int, int>> m_threadOrigins;

    int m_nextThreadId = 1;

    int m_pendingFrom = -1;
    int m_pendingTo = -1;

    QVariantAnimation *m_animation = nullptr;
    bool m_animating = false;
    int m_animFrom = -1;
    int m_animTo = -1;
    int m_animDestPeg = -1;
    int m_animThreadId = -1;
    QPointF m_animStart;
    QPointF m_animEnd;
    QPointF m_animPos;

    QString m_colorsStorageDir;
    QString m_colorProfileKey;
    QString m_colorsJsonPath;
    QVector<int> m_caseRouteNext;
    QVector<int> m_caseOrbitId;
    ColorApplyMode m_colorApplyMode = ColorApplyMode::OrbitPath;
    QVector<QColor> m_quickPalette;
    bool m_quickPaletteEnabled = false;
    int m_quickPaletteIndex = 0;
    QImage m_referenceImage;
    bool m_referenceOverlayEnabled = true;
    qreal m_referenceOpacity = 0.30;

    QPointF pegPosition(int caseIndex, int pegIndex) const;
    QPointF centerPoint() const;
    void applyMove(int fromCase, int toCase, QString *errorMessage = nullptr);
    QColor colorForThread(int threadId) const;
    bool pickOccupiedPegAt(const QPointF &pos, int *caseIndex, int *pegIndex, int *threadId) const;
    void loadThreadColorsFromJson();
    void saveThreadColorsToJson() const;
};

#endif // BRAIDBOARDWIDGET_H
