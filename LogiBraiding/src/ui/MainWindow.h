#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>
#include <QVector>

class QLabel;
class QPushButton;
class QComboBox;
class QTableWidget;
class QToolButton;
class BraidBoardWidget;

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void handleMoveDone();
    void restartSequence();
    void handleBraidSelectionChanged(int index);
    void loadReferenceDrawing();
    void openReferenceDocument();
    void handleColorModeChanged(int index);
    void handlePalettePresetChanged(int index);
    void openQuickHelp();
    void openAboutDialog();

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

    Ui::MainWindow *ui = nullptr;

    int m_currentMoveIndex = 0;
    bool m_detailsExpanded = false;
    QString m_selectedBraidCode = QStringLiteral("3082");

    QLabel *m_titleLabel = nullptr;
    QLabel *m_progressLabel = nullptr;
    QLabel *m_moveLabel = nullptr;
    QLabel *m_detailLabel = nullptr;
    QLabel *m_noteLabel = nullptr;
    QLabel *m_compactLineLabel = nullptr;

    QPushButton *m_doneButton = nullptr;
    QPushButton *m_restartButton = nullptr;
    QToolButton *m_toggleDetailsButton = nullptr;
    QToolButton *m_loadDrawingButton = nullptr;
    QToolButton *m_openDocButton = nullptr;
    QToolButton *m_mainMenuButton = nullptr;
    QComboBox *m_braidSelector = nullptr;
    QComboBox *m_colorModeSelector = nullptr;
    QComboBox *m_palettePresetSelector = nullptr;

    QTableWidget *m_movesTable = nullptr;
    BraidBoardWidget *m_boardWidget = nullptr;

    void setupInterface();
    void configureMovesTable();
    void updateView();
    void setDetailsExpanded(bool expanded);
    void updateCompactLine(const Move3082 *move, int totalMoves);
    const QVector<Move3082> &currentMoveSet() const;
    QString currentBraidTitle() const;
    void reloadCurrentBraid();
    bool openRegistrationDialog(bool trialExpired);
    bool ensureTrialAccess();
    QString currentRegisteredEmail() const;
    QString currentAuthorId() const;
    QString registrationStatusText() const;
    void applyUiLanguage(const QString &languageCode);
};

#endif // MAINWINDOW_H
