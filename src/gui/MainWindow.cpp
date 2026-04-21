#include "MainWindow.h"
#include <QComboBox>
#include <QFileDialog>
#include <QGridLayout>
#include <QHeaderView>
#include <QLabel>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QWidget>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    buildUi();
    refreshView();
}

void MainWindow::buildUi() {
    auto* central = new QWidget(this);
    auto* root = new QVBoxLayout(central);

    auto* top = new QHBoxLayout();
    loadButton_ = new QPushButton("Load ASM");
    stepButton_ = new QPushButton("Step");
    runButton_ = new QPushButton("Run");
    resetButton_ = new QPushButton("Reset");
    modeBox_ = new QComboBox();
    modeBox_->addItems({"No Pipe / No Cache", "Cache Only", "Pipe Only", "Pipe + Cache"});

    top->addWidget(loadButton_);
    top->addWidget(stepButton_);
    top->addWidget(runButton_);
    top->addWidget(resetButton_);
    top->addWidget(modeBox_);
    root->addLayout(top);

    auto* grid = new QGridLayout();
    regTable_ = new QTableWidget(16, 2);
    regTable_->setHorizontalHeaderLabels({"Reg", "Value"});
    pipelineView_ = new QPlainTextEdit();
    pipelineView_->setReadOnly(true);

    l1Table_ = new QTableWidget(L1_NUM_LINES, 5);
    l1Table_->setHorizontalHeaderLabels({"Idx", "Valid", "Tag", "Dirty", "Data"});
    l2Table_ = new QTableWidget(L2_NUM_LINES, 5);
    l2Table_->setHorizontalHeaderLabels({"Idx", "Valid", "Tag", "Dirty", "Data"});
    memTable_ = new QTableWidget(16, 2);
    memTable_->setHorizontalHeaderLabels({"Base", "Words"});

    grid->addWidget(new QLabel("Registers"), 0, 0);
    grid->addWidget(regTable_, 1, 0);
    grid->addWidget(new QLabel("Pipeline"), 0, 1);
    grid->addWidget(pipelineView_, 1, 1);
    grid->addWidget(new QLabel("L1"), 2, 0);
    grid->addWidget(l1Table_, 3, 0);
    grid->addWidget(new QLabel("L2"), 2, 1);
    grid->addWidget(l2Table_, 3, 1);
    grid->addWidget(new QLabel("Memory"), 4, 0, 1, 2);
    grid->addWidget(memTable_, 5, 0, 1, 2);

    root->addLayout(grid);

    statusLabel_ = new QLabel();
    root->addWidget(statusLabel_);

    setCentralWidget(central);
    resize(1200, 850);
    setWindowTitle("SAV_535 Simulator");

    connect(loadButton_, &QPushButton::clicked, this, &MainWindow::onLoadAsm);
    connect(stepButton_, &QPushButton::clicked, this, &MainWindow::onStep);
    connect(runButton_, &QPushButton::clicked, this, &MainWindow::onRun);
    connect(resetButton_, &QPushButton::clicked, this, &MainWindow::onReset);
    connect(modeBox_, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onModeChanged);
}

void MainWindow::refreshView() {
    auto snap = sim_.getSnapshot();

    for (int i = 0; i < 16; ++i) {
        regTable_->setItem(i, 0, new QTableWidgetItem(QString("R%1").arg(i)));
        regTable_->setItem(i, 1, new QTableWidgetItem(QString::number(snap.regs[i])));
    }

    QString pipe =
        "IF : " + QString::fromStdString(snap.ifStage) + "\n" +
        "ID : " + QString::fromStdString(snap.idStage) + "\n" +
        "EX : " + QString::fromStdString(snap.exStage) + "\n" +
        "MEM: " + QString::fromStdString(snap.memStage) + "\n" +
        "WB : " + QString::fromStdString(snap.wbStage);
    pipelineView_->setPlainText(pipe);

    for (int i = 0; i < static_cast<int>(snap.l1Rows.size()); ++i) {
        const auto& r = snap.l1Rows[i];
        QString data = QString("%1 %2 %3 %4")
            .arg(QString::number(r.data[0], 16))
            .arg(QString::number(r.data[1], 16))
            .arg(QString::number(r.data[2], 16))
            .arg(QString::number(r.data[3], 16));
        l1Table_->setItem(i, 0, new QTableWidgetItem(QString::number(r.index)));
        l1Table_->setItem(i, 1, new QTableWidgetItem(r.valid ? "1" : "0"));
        l1Table_->setItem(i, 2, new QTableWidgetItem(QString::number(r.tag)));
        l1Table_->setItem(i, 3, new QTableWidgetItem(r.dirty ? "1" : "0"));
        l1Table_->setItem(i, 4, new QTableWidgetItem(data));
    }

    for (int i = 0; i < static_cast<int>(snap.l2Rows.size()); ++i) {
        const auto& r = snap.l2Rows[i];
        QString data = QString("%1 %2 %3 %4")
            .arg(QString::number(r.data[0], 16))
            .arg(QString::number(r.data[1], 16))
            .arg(QString::number(r.data[2], 16))
            .arg(QString::number(r.data[3], 16));
        l2Table_->setItem(i, 0, new QTableWidgetItem(QString::number(r.index)));
        l2Table_->setItem(i, 1, new QTableWidgetItem(r.valid ? "1" : "0"));
        l2Table_->setItem(i, 2, new QTableWidgetItem(QString::number(r.tag)));
        l2Table_->setItem(i, 3, new QTableWidgetItem(r.dirty ? "1" : "0"));
        l2Table_->setItem(i, 4, new QTableWidgetItem(data));
    }

    memTable_->setRowCount(static_cast<int>(snap.memoryRows.size()));
    for (int i = 0; i < static_cast<int>(snap.memoryRows.size()); ++i) {
        const auto& r = snap.memoryRows[i];
        QString data = QString("%1 %2 %3 %4")
            .arg(QString::number(r.data[0], 16))
            .arg(QString::number(r.data[1], 16))
            .arg(QString::number(r.data[2], 16))
            .arg(QString::number(r.data[3], 16));
        memTable_->setItem(i, 0, new QTableWidgetItem(QString::number(r.baseAddress)));
        memTable_->setItem(i, 1, new QTableWidgetItem(data));
    }

    statusLabel_->setText(QString("PC=%1  Cycles=%2  Halted=%3  Z=%4  L1 H/M=%5/%6  L2 H/M=%7/%8")
        .arg(snap.pc)
        .arg(snap.cycles)
        .arg(snap.halted ? "yes" : "no")
        .arg(snap.zFlag ? "1" : "0")
        .arg(snap.l1Hits)
        .arg(snap.l1Misses)
        .arg(snap.l2Hits)
        .arg(snap.l2Misses));
}

void MainWindow::onLoadAsm() {
    const auto file = QFileDialog::getOpenFileName(this, "Load ASM", QString(), "Assembly (*.asm *.txt)");
    if (file.isEmpty()) return;
    try {
        sim_.loadProgramAsm(file.toStdString());
        refreshView();
    } catch (const std::exception& e) {
        QMessageBox::critical(this, "Load failed", e.what());
    }
}

void MainWindow::onStep() {
    sim_.step();
    refreshView();
}

void MainWindow::onRun() {
    sim_.run();
    refreshView();
}

void MainWindow::onReset() {
    sim_.reset();
    refreshView();
}

void MainWindow::onModeChanged(int index) {
    sim_.setMode(static_cast<ExecMode>(index));
    refreshView();
}
