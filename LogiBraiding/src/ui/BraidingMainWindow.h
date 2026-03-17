#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QByteArray>
#include <QString>
#include <QVector>

class QLabel;
class QPushButton;
class QComboBox;
class QTableWidget;
class QToolButton;
class QTimer;
class BraidBoardWidget;

QT_BEGIN_NAMESPACE
namespace Ui {
class BraidingMainWindow;
}
QT_END_NAMESPACE

namespace LogiBraidingApp {

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void handleMoveDone();
    void handleFullAnimation();
    void restartSequence();
    void handleBraidSelectionChanged(int index);
    void loadReferenceDrawing();
    void openReferenceDocument();
    void handleColorModeChanged(int index);
    void handlePalettePresetChanged(int index);
    void handleAnimationSpeedChanged(int value);
    void openQuickHelp();
    void openAboutDialog();
    void openBraidsEditor();

private:
    struct Move3082 {
        int step;
        int boardCase;
        int strands;
        int fromPeg;
        int toPeg;
        QChar rotationCode;
        QString note;
        QString imagePath;
    };

    static const QVector<Move3082> kMoves3082;
    static const QVector<Move3082> kMovesPentalpha78;
    static const QVector<Move3082> kMovesRoseDesVents97;
    struct BraidDefinition {
        QString code;
        QString name;
        QVector<Move3082> moves;
    };

    Ui::BraidingMainWindow *ui = nullptr;

    int m_currentMoveIndex = 0;
    bool m_detailsExpanded = false;
    bool m_sequentialModeActive = false;
    QString m_selectedBraidCode = QStringLiteral("3082");
    QVector<BraidDefinition> m_braids;
    QString m_braidsJsonPath;

    QLabel *m_titleLabel = nullptr;
    QLabel *m_progressLabel = nullptr;
    QLabel *m_moveLabel = nullptr;
    QLabel *m_detailLabel = nullptr;
    QLabel *m_noteLabel = nullptr;
    QLabel *m_compactLineLabel = nullptr;
    QLabel *m_sequentialCountdownLabel = nullptr;

    QPushButton *m_doneButton = nullptr;
    QPushButton *m_animationButton = nullptr;
    QPushButton *m_restartButton = nullptr;
    QToolButton *m_toggleDetailsButton = nullptr;
    QToolButton *m_loadDrawingButton = nullptr;
    QToolButton *m_openDocButton = nullptr;
    QToolButton *m_mainMenuButton = nullptr;
    QComboBox *m_braidSelector = nullptr;
    QComboBox *m_animationSpeedCombo = nullptr;

    QTableWidget *m_movesTable = nullptr;
    BraidBoardWidget *m_boardWidget = nullptr;
    QTimer *m_sequentialTimer = nullptr;
    QTimer *m_sequentialCountdownTimer = nullptr;
    int m_sequentialRemainingMs = 0;
    int m_animationDurationMs = 5000;

    void setupInterface();
    void configureMovesTable();
    void updateView();
    void startCurrentMoveAnimation(bool fullPassage);
    void setDetailsExpanded(bool expanded);
    void updateCompactLine(const Move3082 *move, int totalMoves);
    bool performCurrentMove(QString *errorMessage = nullptr);
    bool isCompletionHoldState() const;
    void prepareNextSequence();
    void setSequentialModeActive(bool active);
    void startSequentialCountdown(int durationMs);
    void updateSequentialCountdown();
    void resetSequentialCountdownLabel();
    void advanceSequentialStep();
    const QVector<Move3082> &currentMoveSet() const;
    QString currentBraidTitle() const;
    void reloadCurrentBraid();
    void loadBraidsCatalog();
    bool saveBraidsCatalog(const QVector<BraidDefinition> &braids, QString *errorMessage = nullptr) const;
    bool parseBraidsCatalog(const QByteArray &jsonData, QVector<BraidDefinition> *outBraids, QString *errorMessage = nullptr) const;
    QVector<BraidDefinition> legacyBraidsCatalog() const;
    const BraidDefinition *currentBraidDefinition() const;
    void rebuildBraidSelector();
    bool openRegistrationDialog(bool trialExpired);
    bool ensureTrialAccess();
    QString currentRegisteredEmail() const;
    QString currentAuthorId() const;
    QString registrationStatusText() const;
    void applyUiLanguage(const QString &languageCode);
    void loadUiPreferences();
    void saveSelectedBraidCode() const;
    void saveAnimationDurationMs() const;
    void applyAnimationDurationMs(int durationMs, bool persistSetting);
};

} // namespace LogiBraidingApp

#endif // MAINWINDOW_H


