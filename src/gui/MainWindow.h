#pragma once
#include <QMainWindow>
#include "Simulator.h"

class QComboBox;
class QLabel;
class QPlainTextEdit;
class QPushButton;
class QTableWidget;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);

private slots:
    void onLoadAsm();
    void onStep();
    void onRun();
    void onReset();
    void onModeChanged(int index);

private:
    void buildUi();
    void refreshView();

    Simulator sim_;

    QPushButton* loadButton_ = nullptr;
    QPushButton* stepButton_ = nullptr;
    QPushButton* runButton_ = nullptr;
    QPushButton* resetButton_ = nullptr;
    QComboBox* modeBox_ = nullptr;

    QTableWidget* regTable_ = nullptr;
    QTableWidget* l1Table_ = nullptr;
    QTableWidget* l2Table_ = nullptr;
    QTableWidget* memTable_ = nullptr;
    QPlainTextEdit* pipelineView_ = nullptr;
    QLabel* statusLabel_ = nullptr;
};
