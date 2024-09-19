#include <QApplication>
#include <QWidget>
#include <QPushButton>
#include <QGridLayout>
#include <vector>
#include <ctime>
#include <cstdlib>
#include <QMessageBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QMouseEvent>

// Cell class represents each cell in the Minesweeper game
class Cell : public QPushButton {
    Q_OBJECT

public:
    explicit Cell(QWidget *parent = nullptr);

signals:
    void rightClicked();

protected:
    void mousePressEvent(QMouseEvent *event) override;
};

// Constructor for Cell class
Cell::Cell(QWidget *parent) : QPushButton(parent) {}

// Handle mouse press events
void Cell::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::RightButton) {
        emit rightClicked();
    } else {
        QPushButton::mousePressEvent(event);
    }
}

// Minesweeper class represents the entire Minesweeper game
class Minesweeper : public QWidget {
    Q_OBJECT

public:
    explicit Minesweeper(int rows = 10, int columns = 10, int mines = 10, QWidget *parent = nullptr);

private:
    std::vector<std::vector<int>> grid;
    std::vector<std::vector<bool>> revealed;
    std::vector<std::vector<bool>> flagged;
    QGridLayout *gridLayout;
    QLabel *scoreLabel;
    QPushButton *restartButton;
    QPushButton *hintButton;
    int score;
    int hintRow, hintCol;
    bool hintGiven;
    int rows;
    int columns;
    int mines;

    void placeMines();
    int countAdjacentMines(int row, int col);
    void revealCell(int row, int col);
    void gameOver();
    void checkWin();
    void resetGame();
    void giveHint();
    std::pair<int, int> findSafeCell();
    bool isSafeCell(int row, int col);

private slots:
    void handleButtonClick(int row, int col);
    void handleRestartClick();
    void handleHintClick();
    void handleRightClick(int row, int col);
};

// Constructor for Minesweeper class
Minesweeper::Minesweeper(int rows, int columns, int mines, QWidget *parent)
    : QWidget(parent), score(0), hintRow(-1), hintCol(-1), hintGiven(false), rows(rows), columns(columns), mines(mines) {
    grid.resize(rows, std::vector<int>(columns, 0));
    revealed.resize(rows, std::vector<bool>(columns, false));
    flagged.resize(rows, std::vector<bool>(columns, false));

    srand(time(nullptr));
    placeMines();

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    scoreLabel = new QLabel("Score: 0", this);
    mainLayout->addWidget(scoreLabel);

    QHBoxLayout *topLayout = new QHBoxLayout();
    restartButton = new QPushButton("Restart", this);
    hintButton = new QPushButton("Hint", this);

    connect(restartButton, &QPushButton::clicked, this, &Minesweeper::handleRestartClick);
    connect(hintButton, &QPushButton::clicked, this, &Minesweeper::handleHintClick);

    topLayout->addWidget(restartButton);
    topLayout->addWidget(hintButton);
    mainLayout->addLayout(topLayout);

    gridLayout = new QGridLayout();
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < columns; ++j) {
            Cell *button = new Cell(this);
            button->setFixedSize(30, 30);
            button->setProperty("row", i);
            button->setProperty("col", j);
            connect(button, &QPushButton::clicked, this, [=]() {
                int row = button->property("row").toInt();
                int col = button->property("col").toInt();
                handleButtonClick(row, col);
            });
            connect(button, &Cell::rightClicked, this, [=]() {
                int row = button->property("row").toInt();
                int col = button->property("col").toInt();
                handleRightClick(row, col);
            });
            gridLayout->addWidget(button, i, j);
        }
    }
    mainLayout->addLayout(gridLayout);
    setLayout(mainLayout);
}

// Place mines randomly on the grid
void Minesweeper::placeMines() {
    int count = 0;
    while (count < mines) {
        int row = rand() % rows;
        int col = rand() % columns;
        if (grid[row][col] != 9) {
            grid[row][col] = 9;
            ++count;
        }
    }
}

// Count adjacent mines around a cell
int Minesweeper::countAdjacentMines(int row, int col) {
    int count = 0;
    for (int i = -1; i <= 1; ++i) {
        for (int j = -1; j <= 1; ++j) {
            int newRow = row + i;
            int newCol = col + j;
            if (newRow >= 0 && newRow < rows && newCol >= 0 && newCol < columns && grid[newRow][newCol] == 9) {
                ++count;
            }
        }
    }
    return count;
}

// Reveal a cell and update the game state
void Minesweeper::revealCell(int row, int col) {
    if (revealed[row][col]) {
        return;
    }
    revealed[row][col] = true;
    ++score;
    scoreLabel->setText("Score: " + QString::number(score));
    Cell *button = qobject_cast<Cell *>(gridLayout->itemAtPosition(row, col)->widget());
    button->setEnabled(false);

    // Set shadow color for revealed button
    button->setStyleSheet("background-color: #d3d3d3; color: #000000;");

    int adjacentMines = countAdjacentMines(row, col);
    if (adjacentMines > 0) {
        button->setText(QString::number(adjacentMines));
    } else {
        for (int i = -1; i <= 1; ++i) {
            for (int j = -1; j <= 1; ++j) {
                int newRow = row + i;
                int newCol = col + j;
                if (newRow >= 0 && newRow < rows && newCol >= 0 && newCol < columns) {
                    revealCell(newRow, newCol);
                }
            }
        }
    }
}

// Handle game over scenario
void Minesweeper::gameOver() {
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < columns; ++j) {
            if (grid[i][j] == 9) {
                Cell *button = qobject_cast<Cell *>(gridLayout->itemAtPosition(i, j)->widget());
                button->setText("*");
            }
        }
    }
    QMessageBox::information(this, "Game Over", "You hit a mine!");
    resetGame();
}

// Check if the player has won the game
void Minesweeper::checkWin() {
    bool win = true;
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < columns; ++j) {
            if (!revealed[i][j] && grid[i][j] != 9) {
                win = false;
                break;
            }
        }
    }
    if (win) {
        QMessageBox::information(this, "Congratulations", "You won the game!");
        resetGame();
    }
}

// Reset the game to its initial state
void Minesweeper::resetGame() {
    score = 0;
    scoreLabel->setText("Score: 0");
    grid = std::vector<std::vector<int>>(rows, std::vector<int>(columns, 0));
    revealed = std::vector<std::vector<bool>>(rows, std::vector<bool>(columns, false));
    flagged = std::vector<std::vector<bool>>(rows, std::vector<bool>(columns, false));
    hintRow = -1;
    hintCol = -1;
    hintGiven = false;
    placeMines();

    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < columns; ++j) {
            Cell *button = qobject_cast<Cell *>(gridLayout->itemAtPosition(i, j)->widget());
            button->setEnabled(true);
            button->setText("");
            button->setStyleSheet(""); // Reset style
        }
    }
}

// Check if a cell is safe to reveal
bool Minesweeper::isSafeCell(int row, int col) {
    if (revealed[row][col] || grid[row][col] == 9) {
        return false;
    }

    // Check adjacent revealed cells
    for (int i = -1; i <= 1; ++i) {
        for (int j = -1; j <= 1; ++j) {
            int newRow = row + i;
            int newCol = col + j;
            if (newRow >= 0 && newRow < rows && newCol >= 0 && newCol < columns && revealed[newRow][newCol]) {
                return true;
            }
        }
    }

    return false;
}

// Find a safe cell to reveal
std::pair<int, int> Minesweeper::findSafeCell() {
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < columns; ++j) {
            if (isSafeCell(i, j)) {
                return {i, j};
            }
        }
    }
    return {-1, -1};
}

// Provide a hint to the player
void Minesweeper::giveHint() {
    if (hintGiven && hintRow != -1 && hintCol != -1 && !revealed[hintRow][hintCol]) {
        revealCell(hintRow, hintCol);
        checkWin();
        hintRow = -1;
        hintCol = -1;
        hintGiven = false;
    } else {
        auto [row, col] = findSafeCell();
        if (row != -1 && col != -1) {
            hintRow = row;
            hintCol = col;
            hintGiven = true;
            Cell *button = qobject_cast<Cell *>(gridLayout->itemAtPosition(row, col)->widget());
            button->setStyleSheet("background-color: #90EE90; ");
        } else {
            hintGiven = false;
        }
    }
}

// Handle button click event
void Minesweeper::handleButtonClick(int row, int col) {
    if (hintGiven && row == hintRow && col == hintCol) {
        hintRow = -1;
        hintCol = -1;
        hintGiven = false;
    }
    if (grid[row][col] == 9) {
        gameOver();
    } else {
        revealCell(row, col);
        checkWin();
    }
}

// Handle restart button click event
void Minesweeper::handleRestartClick() {
    resetGame();
}

// Handle hint button click event
void Minesweeper::handleHintClick() {
    giveHint();
}

// Handle right-click event
void Minesweeper::handleRightClick(int row, int col) {
    if (flagged[row][col]) {
        flagged[row][col] = false;
        Cell *button = qobject_cast<Cell *>(gridLayout->itemAtPosition(row, col)->widget());
        button->setText("");
    } else {
        flagged[row][col] = true;
        Cell *button = qobject_cast<Cell *>(gridLayout->itemAtPosition(row, col)->widget());
        button->setText("F");
    }
}

// Entry point of the application
int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    Minesweeper minesweeper(0, 10, 10); // Default grid size 10x10 with 10 mines
    minesweeper.setWindowTitle("Minesweeper");
    minesweeper.show();
    return app.exec();
}

#include "main.moc"
