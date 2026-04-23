#include "MainWindow.h"
#include <QComboBox>
#include <QFileDialog>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QFrame>
#include <QWidget>
#include <QFont>
#include <QGraphicsDropShadowEffect>
#include <QScrollArea>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    buildUi();
    refreshView();
}

void MainWindow::buildUi() {
    // Create scroll area
    auto* scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setStyleSheet(
        "QScrollArea { "
        "   border: none; "
        "   background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
        "       stop:0 #f0f2f5, stop:1 #e8eaed); "
        "}"
    );
    
    auto* central = new QWidget();
    central->setStyleSheet(
        "QWidget#centralWidget { "
        "   background: transparent; "
        "}"
    );
    central->setObjectName("centralWidget");
    
    auto* mainLayout = new QVBoxLayout(central);
    mainLayout->setSpacing(16);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    // ========== HEADER ==========
    auto* headerWidget = new QWidget();
    headerWidget->setStyleSheet(
        "QWidget { "
        "   background: qlineargradient(x1:0, y1:0, x2:1, y2:0, "
        "       stop:0 #667eea, stop:1 #764ba2); "
        "   border-radius: 12px; "
        "}"
    );
    auto* headerLayout = new QVBoxLayout(headerWidget);
    headerLayout->setContentsMargins(24, 16, 24, 16);
    
    auto* headerLabel = new QLabel("SAV_535 RISC Simulator");
    QFont headerFont;
    #ifdef Q_OS_MAC
        headerFont = QFont(".AppleSystemUIFont", 24, QFont::Bold);
    #else
        headerFont = QFont("Segoe UI", 24, QFont::Bold);
    #endif
    headerLabel->setFont(headerFont);
    headerLabel->setStyleSheet("QLabel { color: white; background: transparent; }");
    
    auto* subtitleLabel = new QLabel("32-bit RISC Pipeline Simulator with L1/L2 Cache Hierarchy");
    QFont subtitleFont;
    #ifdef Q_OS_MAC
        subtitleFont = QFont(".AppleSystemUIFont", 12);
    #else
        subtitleFont = QFont("Segoe UI", 12);
    #endif
    subtitleLabel->setFont(subtitleFont);
    subtitleLabel->setStyleSheet("QLabel { color: rgba(255,255,255,0.9); background: transparent; }");
    
    headerLayout->addWidget(headerLabel);
    headerLayout->addWidget(subtitleLabel);
    mainLayout->addWidget(headerWidget);

    // ========== CONTROL PANEL ==========
    auto* controlGroup = new QGroupBox();
    controlGroup->setStyleSheet(
        "QGroupBox { "
        "   background-color: white; "
        "   border-radius: 12px; "
        "   padding: 20px; "
        "   border: 1px solid #e0e0e0; "
        "}"
    );
    auto* controlLayout = new QHBoxLayout();
    controlLayout->setSpacing(12);

    QString buttonStyle = 
        "QPushButton { "
        "   padding: 10px 20px; "
        "   background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
        "       stop:0 #4A90E2, stop:1 #357ABD); "
        "   color: white; "
        "   border: none; "
        "   border-radius: 8px; "
        "   font-size: 14px; "
        "   font-weight: 600; "
        "   min-width: 100px; "
        "} "
        "QPushButton:hover { "
        "   background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
        "       stop:0 #5AA0F2, stop:1 #4A90E2); "
        "} "
        "QPushButton:pressed { "
        "   background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
        "       stop:0 #357ABD, stop:1 #2868A8); "
        "}";
    
    QString resetButtonStyle = 
        "QPushButton { "
        "   padding: 10px 20px; "
        "   background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
        "       stop:0 #FF6B6B, stop:1 #EE5A52); "
        "   color: white; "
        "   border: none; "
        "   border-radius: 8px; "
        "   font-size: 14px; "
        "   font-weight: 600; "
        "   min-width: 100px; "
        "} "
        "QPushButton:hover { "
        "   background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
        "       stop:0 #FF7B7B, stop:1 #FF6B6B); "
        "} "
        "QPushButton:pressed { "
        "   background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
        "       stop:0 #EE5A52, stop:1 #DD4A42); "
        "}";

    loadButton_ = new QPushButton("📁 Load Program");
    stepButton_ = new QPushButton("⏯ Step");
    step10Button_ = new QPushButton("⏭ Step ×10");
    runButton_ = new QPushButton("▶ Run");
    runBpButton_ = new QPushButton("⏸ Run to BP");
    resetButton_ = new QPushButton("🔄 Reset");
    
    loadButton_->setStyleSheet(buttonStyle);
    stepButton_->setStyleSheet(buttonStyle);
    step10Button_->setStyleSheet(buttonStyle);
    runButton_->setStyleSheet(buttonStyle);
    runBpButton_->setStyleSheet(buttonStyle);
    resetButton_->setStyleSheet(resetButtonStyle);

    modeBox_ = new QComboBox();
    modeBox_->setStyleSheet(
        "QComboBox { "
        "   padding: 8px 12px; "
        "   background-color: white; "
        "   border: 2px solid #e0e0e0; "
        "   border-radius: 8px; "
        "   min-width: 220px; "
        "   font-size: 13px; "
        "   color: #2c3e50; "
        "} "
        "QComboBox:hover { border-color: #4A90E2; } "
        "QComboBox:focus { border-color: #357ABD; } "
        "QComboBox::drop-down { border: none; width: 30px; } "
        "QComboBox QAbstractItemView { "
        "   background-color: white; "
        "   border: 2px solid #e0e0e0; "
        "   selection-background-color: #4A90E2; "
        "   selection-color: white; "
        "   padding: 4px; "
        "}"
    );
    modeBox_->addItems({
        "Mode 0: No Pipeline / No Cache",
        "Mode 1: Cache Only",
        "Mode 2: Pipeline Only",
        "Mode 3: Pipeline + Cache"
    });
    modeBox_->setCurrentIndex(3);

    auto* modeLabel = new QLabel("Execution Mode:");
    modeLabel->setStyleSheet("QLabel { color: #2c3e50; font-size: 14px; font-weight: 600; background: transparent; }");

    controlLayout->addWidget(loadButton_);
    controlLayout->addWidget(stepButton_);
    controlLayout->addWidget(step10Button_);
    controlLayout->addWidget(runButton_);
    controlLayout->addWidget(runBpButton_);
    controlLayout->addWidget(resetButton_);
    controlLayout->addStretch();
    controlLayout->addWidget(modeLabel);
    controlLayout->addWidget(modeBox_);
    
    controlGroup->setLayout(controlLayout);
    mainLayout->addWidget(controlGroup);

    // ========== BREAKPOINT PANEL ==========
    auto* bpGroup = new QGroupBox();
    bpGroup->setStyleSheet(
        "QGroupBox { "
        "   background-color: white; "
        "   border-radius: 12px; "
        "   padding: 20px; "
        "   border: 1px solid #e0e0e0; "
        "}"
    );
    auto* bpLayout = new QHBoxLayout();
    bpLayout->setSpacing(12);

    auto* bpLabel = new QLabel("Breakpoints:");
    bpLabel->setStyleSheet("QLabel { color: #2c3e50; font-size: 14px; font-weight: 700; background: transparent; }");

    bpAddressEdit_ = new QLineEdit();
    bpAddressEdit_->setPlaceholderText("Enter address...");
    bpAddressEdit_->setStyleSheet(
        "QLineEdit { "
        "   padding: 8px 12px; "
        "   background-color: #f8f9fa; "
        "   border: 2px solid #e0e0e0; "
        "   border-radius: 8px; "
        "   font-size: 13px; "
        "   max-width: 160px; "
        "   color: #2c3e50; "
        "} "
        "QLineEdit:focus { "
        "   border-color: #4A90E2; "
        "   background-color: white; "
        "}"
    );
    
    QString bpButtonStyle = 
        "QPushButton { "
        "   padding: 8px 16px; "
        "   background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
        "       stop:0 #51CF66, stop:1 #40C057); "
        "   color: white; "
        "   border: none; "
        "   border-radius: 6px; "
        "   font-size: 13px; "
        "   font-weight: 600; "
        "} "
        "QPushButton:hover { "
        "   background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
        "       stop:0 #61DF76, stop:1 #51CF66); "
        "}";
    
    setBpButton_ = new QPushButton("➕ Set");
    clearBpButton_ = new QPushButton("➖ Clear");
    clearAllBpButton_ = new QPushButton("🗑 Clear All");
    
    setBpButton_->setStyleSheet(bpButtonStyle);
    clearBpButton_->setStyleSheet(bpButtonStyle);
    clearAllBpButton_->setStyleSheet(bpButtonStyle);

    bpList_ = new QListWidget();
    bpList_->setMaximumHeight(50);
    bpList_->setStyleSheet(
        "QListWidget { "
        "   background-color: #f8f9fa; "
        "   border: 2px solid #e0e0e0; "
        "   border-radius: 8px; "
        "   padding: 6px; "
        "   font-size: 13px; "
        "   color: #1d1d1f; "
        "} "
        "QListWidget::item { "
        "   color: #1d1d1f; "
        "}"
    );

    bpLayout->addWidget(bpLabel);
    bpLayout->addWidget(bpAddressEdit_);
    bpLayout->addWidget(setBpButton_);
    bpLayout->addWidget(clearBpButton_);
    bpLayout->addWidget(clearAllBpButton_);
    bpLayout->addWidget(bpList_, 1);

    bpGroup->setLayout(bpLayout);
    mainLayout->addWidget(bpGroup);

    // ========== STATE DISPLAY GRID ==========
    auto* gridLayout = new QGridLayout();
    gridLayout->setSpacing(16);

    // Shared styles for uniform panel cards
    QString panelFrameStyle =
        "QFrame {"
        "   background-color: white;"
        "   border-radius: 12px;"
        "   border: 1px solid #e0e0e0;"
        "}";
    QString panelHeaderStyle =
        "QWidget {"
        "   background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #667eea, stop:1 #764ba2);"
        "   border-top-left-radius: 10px;"
        "   border-top-right-radius: 10px;"
        "}";
    QString panelTitleStyle =
        "QLabel { color: white; font-size: 14px; font-weight: 700; background: transparent; }";

    auto makePanelFrame = [&](const QString& title) -> std::pair<QFrame*, QVBoxLayout*> {
        auto* frame = new QFrame();
        frame->setStyleSheet(panelFrameStyle);
        auto* vbox = new QVBoxLayout();
        vbox->setSpacing(8);
        vbox->setContentsMargins(0, 0, 0, 16);
        auto* header = new QWidget();
        header->setStyleSheet(panelHeaderStyle);
        auto* hdr = new QHBoxLayout(header);
        hdr->setContentsMargins(16, 10, 16, 10);
        auto* lbl = new QLabel(title);
        lbl->setStyleSheet(panelTitleStyle);
        hdr->addWidget(lbl);
        hdr->addStretch();
        vbox->addWidget(header);
        return {frame, vbox};
    };

    auto wrapContent = [](QWidget* w, QVBoxLayout* vbox, int l=16, int t=4, int r=16, int b=0) {
        auto* wrapper = new QWidget();
        auto* wl = new QVBoxLayout(wrapper);
        wl->setContentsMargins(l, t, r, b);
        wl->addWidget(w);
        vbox->addWidget(wrapper);
    };

    QString tableStyle = 
        "QTableWidget { "
        "   background-color: #fafbfc; "
        "   border: none; "
        "   gridline-color: #e1e4e8; "
        "   font-size: 13px; "
        "   color: #24292e; "
        "   selection-background-color: #0366d6; "
        "   selection-color: white; "
        "} "
        "QTableWidget::item { "
        "   padding: 6px; "
        "   color: #24292e; "
        "} "
        "QTableWidget::item:selected { "
        "   background-color: #0366d6; "
        "   color: white; "
        "} "
        "QHeaderView::section { "
        "   background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
        "       stop:0 #f6f8fa, stop:1 #e1e4e8); "
        "   padding: 8px; "
        "   border: none; "
        "   border-bottom: 2px solid #d1d5da; "
        "   border-right: 1px solid #e1e4e8; "
        "   font-size: 12px; "
        "   font-weight: 700; "
        "   color: #586069; "
        "}";

    // Registers
    auto [regGroup, regLayout] = makePanelFrame("\U0001F4CB  Registers");
    regTable_ = new QTableWidget(16, 2);
    regTable_->setHorizontalHeaderLabels({"Register", "Value"});
    regTable_->horizontalHeader()->setStretchLastSection(true);
    regTable_->setMaximumHeight(450);
    regTable_->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    regTable_->setStyleSheet(tableStyle);
    wrapContent(regTable_, regLayout);
    regGroup->setLayout(regLayout);
    gridLayout->addWidget(regGroup, 0, 0, 2, 1);

    // Pipeline
    auto [pipeGroup, pipeLayout] = makePanelFrame("⚙️  Pipeline State");
    pipelineView_ = new QPlainTextEdit();
    pipelineView_->setReadOnly(true);
    pipelineView_->setMinimumHeight(160);
    pipelineView_->setMaximumHeight(200);
    #ifdef Q_OS_MAC
        QFont monoFont("Menlo", 12);
    #else
        QFont monoFont("Consolas", 12);
    #endif
    pipelineView_->setFont(monoFont);
    pipelineView_->setStyleSheet(
        "QPlainTextEdit { "
        "   background-color: #f6f8fa; "
        "   border: 2px solid #e1e4e8; "
        "   border-radius: 8px; "
        "   padding: 12px; "
        "   color: #24292e; "
        "   line-height: 1.5; "
        "}"
    );
    wrapContent(pipelineView_, pipeLayout);
    pipeGroup->setLayout(pipeLayout);
    gridLayout->addWidget(pipeGroup, 0, 1);

    // L1 Cache
    auto [l1Group, l1Layout] = makePanelFrame("\U0001F4BE  L1 Cache (8 lines × 4 words)");
    l1Table_ = new QTableWidget(L1_NUM_LINES, 5);
    l1Table_->setHorizontalHeaderLabels({"Index", "Valid", "Tag", "Dirty", "Data [4 words]"});
    l1Table_->horizontalHeader()->setStretchLastSection(true);
    l1Table_->setMinimumHeight(240);
    l1Table_->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    l1Table_->verticalHeader()->setDefaultSectionSize(26);
    l1Table_->setStyleSheet(tableStyle);
    wrapContent(l1Table_, l1Layout);
    l1Group->setLayout(l1Layout);
    gridLayout->addWidget(l1Group, 1, 1);

    // L2 Cache
    auto [l2Group, l2Layout] = makePanelFrame("\U0001F4BE  L2 Cache (32 lines × 4 words)");
    l2Table_ = new QTableWidget(L2_NUM_LINES, 5);
    l2Table_->setHorizontalHeaderLabels({"Index", "Valid", "Tag", "Dirty", "Data [4 words]"});
    l2Table_->horizontalHeader()->setStretchLastSection(true);
    l2Table_->setMaximumHeight(350);
    l2Table_->setMinimumHeight(350);
    l2Table_->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    l2Table_->setStyleSheet(tableStyle);
    wrapContent(l2Table_, l2Layout);
    l2Group->setLayout(l2Layout);
    gridLayout->addWidget(l2Group, 2, 0);

    // Memory - header bar includes the Start Address spinner on the right
    auto* memGroup = new QFrame();
    memGroup->setStyleSheet(panelFrameStyle);
    auto* memLayout = new QVBoxLayout();
    memLayout->setSpacing(8);
    memLayout->setContentsMargins(0, 0, 0, 16);

    auto* memHeader = new QWidget();
    memHeader->setStyleSheet(panelHeaderStyle);
    auto* memHeaderLayout = new QHBoxLayout(memHeader);
    memHeaderLayout->setContentsMargins(16, 10, 12, 10);
    memHeaderLayout->setSpacing(10);

    auto* memTitleLabel = new QLabel("\U0001F5C4  Main Memory");
    memTitleLabel->setStyleSheet(panelTitleStyle);

    auto* memAddrLabel = new QLabel("Start Address:");
    memAddrLabel->setStyleSheet(
        "QLabel { color: rgba(255,255,255,0.88); font-size: 12px; font-weight: 600; background: transparent; }"
    );

    memStartSpin_ = new QSpinBox();
    memStartSpin_->setRange(0, 32768);
    memStartSpin_->setValue(0);
    memStartSpin_->setSingleStep(16);
    memStartSpin_->setFixedWidth(105);
    memStartSpin_->setStyleSheet(
        "QSpinBox {"
        "   padding: 3px 5px;"
        "   background-color: rgba(255,255,255,0.92);"
        "   border: none;"
        "   border-radius: 5px;"
        "   font-size: 13px;"
        "   color: #24292e;"
        "}"
        "QSpinBox::up-button {"
        "   subcontrol-origin: border;"
        "   subcontrol-position: top right;"
        "   width: 18px;"
        "   border-left: 1px solid #ccc;"
        "   border-top-right-radius: 4px;"
        "   background: rgba(255,255,255,0.6);"
        "}"
        "QSpinBox::down-button {"
        "   subcontrol-origin: border;"
        "   subcontrol-position: bottom right;"
        "   width: 18px;"
        "   border-left: 1px solid #ccc;"
        "   border-bottom-right-radius: 4px;"
        "   background: rgba(255,255,255,0.6);"
        "}"
        "QSpinBox::up-arrow { width: 7px; height: 7px; }"
        "QSpinBox::down-arrow { width: 7px; height: 7px; }"
    );

    memHeaderLayout->addWidget(memTitleLabel);
    memHeaderLayout->addStretch();
    memHeaderLayout->addWidget(memAddrLabel);
    memHeaderLayout->addWidget(memStartSpin_);
    memLayout->addWidget(memHeader);

    memTable_ = new QTableWidget(16, 2);
    memTable_->setHorizontalHeaderLabels({"Address", "Data [4 words]"});
    memTable_->horizontalHeader()->setStretchLastSection(true);
    memTable_->setMaximumHeight(350);
    memTable_->setMinimumHeight(350);
    memTable_->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    memTable_->setStyleSheet(tableStyle);
    wrapContent(memTable_, memLayout);

    memGroup->setLayout(memLayout);
    gridLayout->addWidget(memGroup, 2, 1);

    mainLayout->addLayout(gridLayout);

    // ========== STATUS BAR ==========
    statusLabel_ = new QLabel();
    #ifdef Q_OS_MAC
        QFont statusFont("Menlo", 12, QFont::Medium);
    #else
        QFont statusFont("Consolas", 12, QFont::Medium);
    #endif
    statusLabel_->setFont(statusFont);
    statusLabel_->setStyleSheet(
        "QLabel { "
        "   background: qlineargradient(x1:0, y1:0, x2:1, y2:0, "
        "       stop:0 #2c3e50, stop:1 #34495e); "
        "   color: #ecf0f1; "
        "   padding: 14px 20px; "
        "   border-radius: 10px; "
        "   border: 2px solid #1a252f; "
        "}"
    );
    mainLayout->addWidget(statusLabel_);

    scrollArea->setWidget(central);
    setCentralWidget(scrollArea);
    setWindowTitle("SAV_535 RISC Simulator - Sanchita & Vaishnavi");
    resize(1600, 900);

    // ========== CONNECTIONS ==========
    connect(loadButton_, &QPushButton::clicked, this, &MainWindow::onLoadAsm);
    connect(stepButton_, &QPushButton::clicked, this, &MainWindow::onStep);
    connect(step10Button_, &QPushButton::clicked, this, &MainWindow::onStep10);
    connect(runButton_, &QPushButton::clicked, this, &MainWindow::onRun);
    connect(runBpButton_, &QPushButton::clicked, this, &MainWindow::onRunToBreakpoint);
    connect(resetButton_, &QPushButton::clicked, this, &MainWindow::onReset);
    connect(modeBox_, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onModeChanged);
    connect(setBpButton_, &QPushButton::clicked, this, &MainWindow::onSetBreakpoint);
    connect(clearBpButton_, &QPushButton::clicked, this, &MainWindow::onClearBreakpoint);
    connect(clearAllBpButton_, &QPushButton::clicked, this, &MainWindow::onClearAllBreakpoints);
    connect(memStartSpin_, QOverload<int>::of(&QSpinBox::valueChanged), this, &MainWindow::onMemoryStartChanged);
}

void MainWindow::refreshView() {
    auto snap = sim_.getSnapshot(memStartSpin_ ? memStartSpin_->value() : 0, 16);

    // Update registers
    for (int i = 0; i < 16; ++i) {
        QString regName;
        if (i == 0) regName = "R0 (zero)";
        else if (i == 14) regName = "R14 (SP)";
        else if (i == 15) regName = "R15 (RA)";
        else regName = QString("R%1").arg(i);
        
        regTable_->setItem(i, 0, new QTableWidgetItem(regName));
        regTable_->setItem(i, 1, new QTableWidgetItem(QString::number(snap.regs[i])));
    }

    // Update pipeline
    auto pad = [](const std::string& s) {
        return QString::fromStdString(s).left(35).leftJustified(35);
    };

    // Show in-flight fetch info in the IF line when the stage is empty but a fetch is pending
    QString ifDisplay;
    if (snap.fetchInFlight && snap.ifStage == "<empty>") {
        ifDisplay = QString("[Fetching PC=%1, %2 cyc left]")
                        .arg(snap.fetchPc)
                        .arg(snap.fetchRemaining)
                        .left(35).leftJustified(35);
    } else {
        ifDisplay = pad(snap.ifStage);
    }

    QString pipeText =
        "┌─────────────────────────────────────────┐\n"
        "│ IF  : " + ifDisplay            + " │\n" +
        "│ ID  : " + pad(snap.idStage)    + " │\n" +
        "│ EX  : " + pad(snap.exStage)    + " │\n" +
        "│ MEM : " + pad(snap.memStage)   + " │\n" +
        "│ WB  : " + pad(snap.wbStage)    + " │\n" +
        "└─────────────────────────────────────────┘";
    pipelineView_->setPlainText(pipeText);

    // Update L1 cache
    for (size_t i = 0; i < snap.l1Rows.size(); ++i) {
        const auto& r = snap.l1Rows[i];
        QString data = QString("%1 %2 %3 %4")
            .arg(r.data[0], 8, 16, QChar('0'))
            .arg(r.data[1], 8, 16, QChar('0'))
            .arg(r.data[2], 8, 16, QChar('0'))
            .arg(r.data[3], 8, 16, QChar('0'));
        
        l1Table_->setItem(i, 0, new QTableWidgetItem(QString::number(r.index)));
        l1Table_->setItem(i, 1, new QTableWidgetItem(r.valid ? "✓" : "✗"));
        l1Table_->setItem(i, 2, new QTableWidgetItem(QString::number(r.tag, 16).toUpper()));
        l1Table_->setItem(i, 3, new QTableWidgetItem(r.dirty ? "✓" : "✗"));
        l1Table_->setItem(i, 4, new QTableWidgetItem(data.toUpper()));
    }

    // Update L2 cache
    for (size_t i = 0; i < snap.l2Rows.size(); ++i) {
        const auto& r = snap.l2Rows[i];
        QString data = QString("%1 %2 %3 %4")
            .arg(r.data[0], 8, 16, QChar('0'))
            .arg(r.data[1], 8, 16, QChar('0'))
            .arg(r.data[2], 8, 16, QChar('0'))
            .arg(r.data[3], 8, 16, QChar('0'));
        
        l2Table_->setItem(i, 0, new QTableWidgetItem(QString::number(r.index)));
        l2Table_->setItem(i, 1, new QTableWidgetItem(r.valid ? "✓" : "✗"));
        l2Table_->setItem(i, 2, new QTableWidgetItem(QString::number(r.tag, 16).toUpper()));
        l2Table_->setItem(i, 3, new QTableWidgetItem(r.dirty ? "✓" : "✗"));
        l2Table_->setItem(i, 4, new QTableWidgetItem(data.toUpper()));
    }

    // Update memory
    memTable_->setRowCount(static_cast<int>(snap.memoryRows.size()));
    for (size_t i = 0; i < snap.memoryRows.size(); ++i) {
        const auto& r = snap.memoryRows[i];
        QString data = QString("%1 %2 %3 %4")
            .arg(r.data[0], 8, 16, QChar('0'))
            .arg(r.data[1], 8, 16, QChar('0'))
            .arg(r.data[2], 8, 16, QChar('0'))
            .arg(r.data[3], 8, 16, QChar('0'));
        
        memTable_->setItem(i, 0, new QTableWidgetItem(QString::number(r.baseAddress)));
        memTable_->setItem(i, 1, new QTableWidgetItem(data.toUpper()));
    }

    // Update status
    statusLabel_->setText(QString(
        "PC: %1  │  Cycles: %2  │  Halted: %3  │  Z-Flag: %4  │  "
        "L1 Hits/Misses: %5/%6  │  L2 Hits/Misses: %7/%8  │  Mode: %9"
    )
        .arg(snap.pc)
        .arg(snap.cycles)
        .arg(snap.halted ? "YES" : "NO")
        .arg(snap.zFlag ? "1" : "0")
        .arg(snap.l1Hits)
        .arg(snap.l1Misses)
        .arg(snap.l2Hits)
        .arg(snap.l2Misses)
        .arg(QString::fromStdString(snap.mode))
    );

    // Update breakpoint list
    bpList_->clear();
    auto bps = sim_.getBreakpoints();
    for (auto addr : bps) {
        bpList_->addItem(QString("• Address %1").arg(addr));
    }
}

void MainWindow::onLoadAsm() {
    QString file = QFileDialog::getOpenFileName(
        this, 
        "Load Assembly Program", 
        "../demo",
        "Assembly Files (*.asm *.txt);;All Files (*)"
    );
    if (file.isEmpty()) return;
    
    try {
        sim_.loadProgramAsm(file.toStdString());
        refreshView();
        QMessageBox::information(this, "Success", "Program loaded successfully!");
    } catch (const std::exception& e) {
        QMessageBox::critical(this, "Load Error", QString("Failed to load program:\n%1").arg(e.what()));
    }
}

void MainWindow::onStep() {
    sim_.step();
    refreshView();
}

void MainWindow::onStep10() {
    for (int i = 0; i < 10; ++i) sim_.step();
    refreshView();
}

void MainWindow::onRun() {
    QString result = QString::fromStdString(sim_.run());
    refreshView();
    if (result.contains("HALTED")) {
        QMessageBox::information(this, "Execution Complete", "Program halted successfully!");
    }
}

void MainWindow::onRunToBreakpoint() {
    QString msg = QString::fromStdString(sim_.runUntilBreakpoint());
    refreshView();
    if (msg.contains("breakpoint", Qt::CaseInsensitive)) {
        QMessageBox::information(this, "Breakpoint Hit", msg);
    } else if (msg.contains("HALTED")) {
        QMessageBox::information(this, "Execution Complete", "Program halted before hitting breakpoint");
    }
}

void MainWindow::onReset() {
    sim_.reset();
    refreshView();
    QMessageBox::information(this, "Reset", "Simulator reset to initial state");
}

void MainWindow::onModeChanged(int index) {
    sim_.setMode(static_cast<ExecMode>(index));
    refreshView();
}

void MainWindow::onSetBreakpoint() {
    bool ok = false;
    uint32_t addr = bpAddressEdit_->text().toUInt(&ok);
    if (ok) {
        sim_.setBreakpoint(addr);
        bpAddressEdit_->clear();
        refreshView();
    } else {
        QMessageBox::warning(this, "Invalid Input", "Please enter a valid address number");
    }
}

void MainWindow::onClearBreakpoint() {
    auto selected = bpList_->selectedItems();
    if (!selected.isEmpty()) {
        QString text = selected[0]->text();
        QStringList parts = text.split(" ");
        if (parts.size() >= 3) {
            uint32_t addr = parts[2].toUInt();
            sim_.clearBreakpoint(addr);
            refreshView();
        }
    } else {
        QMessageBox::information(this, "No Selection", "Please select a breakpoint to clear");
    }
}

void MainWindow::onClearAllBreakpoints() {
    sim_.clearAllBreakpoints();
    refreshView();
}

void MainWindow::onMemoryStartChanged(int value) {
    (void)value;
    refreshView();
}