#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <algorithm>
#include <random>
#include <iostream>

using namespace std;
using namespace sf;

// Перечисление состояний игры
enum class GameState {
    MAIN_MENU,
    PLAYING,
    GAME_OVER
};

// Класс игры Мемори
class MemoryGame {
private:
    struct Card {
        RectangleShape shape;
        Text text;
        int value;
        bool isFlipped;
        bool isMatched;

        Card() : value(0), isFlipped(false), isMatched(false) {}
    };

    RenderWindow window;
    Font font;
    vector<Card> cards;
    vector<int> selectedCards;

    // Текстуры для фона
    Texture menuBackgroundTexture;
    Texture gameBackgroundTexture;
    Sprite menuBackground;
    Sprite gameBackground;

    GameState currentState;

    // Текст
    Text scoreText;
    Text timeText;
    Text gameOverText;
    Text menuText;
    Text controlsText;

    // Игровые переменные
    int gridSize;
    int cardSize;
    int margin;
    int score;
    float gameTime;
    bool gameStarted;
    bool gameFinished;
    Clock gameClock;
    Clock flipClock;  // Таймер для задержки переворота

    // Выбор в меню
    int menuSelection;
    int selectedCard;

    // Состояние для задержки
    bool waitingForFlipBack;
    float flipBackTime;

    // Цвета
    Color cardColor = Color::Yellow;
    Color flippedColor = Color::White;
    Color matchedColor = Color::Green;
    Color selectedCardColor = Color::Magenta;
    Color menuSelectedColor = Color::Yellow;
    Color menuNormalColor = Color::White;

public:
    MemoryGame() : gridSize(4), cardSize(100), margin(20), score(0),
        gameTime(0.0f), gameStarted(false), gameFinished(false),
        currentState(GameState::MAIN_MENU),
        menuSelection(0), selectedCard(0),
        waitingForFlipBack(false), flipBackTime(0.0f) {
        window.create(VideoMode(1280, 720), "Memory Game");

        // Загрузка текстур
        loadTextures();
        initializeGame();
    }

    void loadTextures() {
        // Попытка загрузить фоновое изображение для меню
        if (!menuBackgroundTexture.loadFromFile("memory.jpg")) {
            cout << "Не удалось загрузить menu_background.jpg. Используется стандартный фон." << endl;
            // Создаем простой градиентный фон программно
            Image gradientImage;
            gradientImage.create(1280, 720, Color::Transparent);
            for (int y = 0; y < 720; ++y) {
                for (int x = 0; x < 1280; ++x) {
                    // Градиент от темно-синего к фиолетовому
                    int r = 30 + (x * 20 / 1280);
                    int g = 20;
                    int b = 60 + (y * 40 / 720);
                    gradientImage.setPixel(x, y, Color(r, g, b));
                }
            }
            menuBackgroundTexture.loadFromImage(gradientImage);
        }

        // Попытка загрузить фоновое изображение для игры
        if (!gameBackgroundTexture.loadFromFile("gamefon.jpg")) {
            cout << "Не удалось загрузить game_background.jpg. Используется стандартный фон." << endl;
            // Создаем простой градиентный фон программно
            Image gradientImage;
            gradientImage.create(1280, 720, Color::Transparent);
            for (int y = 0; y < 720; ++y) {
                for (int x = 0; x < 1280; ++x) {
                    // Градиент от темно-зеленого к темно-синему
                    int r = 20;
                    int g = 40 + (x * 10 / 1280);
                    int b = 60 + (y * 20 / 720);
                    gradientImage.setPixel(x, y, Color(r, g, b));
                }
            }
            gameBackgroundTexture.loadFromImage(gradientImage);
        }

        menuBackground.setTexture(menuBackgroundTexture);
        gameBackground.setTexture(gameBackgroundTexture);
    }
    void initializeGame() {
        // Загрузка шрифта
        if (!font.loadFromFile("arial.ttf")) {
            cout << "Шрифт не найден! Используется стандартный." << endl;
        }

        // Создание пар карт
        vector<int> values;
        int totalPairs = (gridSize * gridSize) / 2;
        for (int i = 0; i < totalPairs; ++i) {
            values.push_back(i + 1);
            values.push_back(i + 1);
        }

        // Перемешивание значений
        random_device rd;
        mt19937 g(rd());
        shuffle(values.begin(), values.end(), g);

        // Создание карт
        cards.clear();
        for (int i = 0; i < gridSize * gridSize; ++i) {
            Card card;

            // Позиция карты
            int row = i / gridSize;
            int col = i % gridSize;
            float x = margin + col * (cardSize + margin);
            float y = margin + row * (cardSize + margin) + 80;

            // Прямоугольник карты
            card.shape.setSize(Vector2f(cardSize, cardSize));
            card.shape.setPosition(x, y);
            card.shape.setFillColor(cardColor);
            card.shape.setOutlineThickness(2);
            card.shape.setOutlineColor(Color::Black);

            // Текст на карте
            card.text.setFont(font);
            card.text.setString(to_string(values[i]));
            card.text.setCharacterSize(30);
            card.text.setFillColor(Color::Black);

            // Центрирование текста
            FloatRect textBounds = card.text.getLocalBounds();
            card.text.setPosition(
                x + (cardSize - textBounds.width) / 2,
                y + (cardSize - textBounds.height) / 2
            );

            card.value = values[i];
            card.isFlipped = false;
            card.isMatched = false;

            cards.push_back(card);
        }

        // Текст счета
        scoreText.setFont(font);
        scoreText.setString("Score: 0");
        scoreText.setCharacterSize(24);
        scoreText.setPosition(20, 10);
        scoreText.setFillColor(Color::White);

        // Текст времени
        timeText.setFont(font);
        timeText.setString("Time: 0.0s");
        timeText.setCharacterSize(24);
        timeText.setPosition(200, 10);
        timeText.setFillColor(Color::White);

        // Текст окончания игры
        gameOverText.setFont(font);
        gameOverText.setString("Game Over! Final Score: 0");
        gameOverText.setCharacterSize(32);
        gameOverText.setFillColor(Color::Yellow);

        // Текст меню
        menuText.setFont(font);
        menuText.setCharacterSize(36);
        menuText.setFillColor(Color::White);

        // Текст управления
        controlsText.setFont(font);
        controlsText.setCharacterSize(20);
        controlsText.setFillColor(Color(200, 200, 200));

        score = 0;
        gameTime = 0.0f;
        gameStarted = false;
        gameFinished = false;
        selectedCards.clear();
        selectedCard = 0;
        waitingForFlipBack = false;
        flipBackTime = 0.0f;
        gameClock.restart();
        flipClock.restart();

        // Выделяем первую карту
        updateCardSelection();
    }

    void run() {
        while (window.isOpen()) {
            handleEvents();
            update();
            render();
        }
    }

private:
    void handleEvents() {
        Event event;
        while (window.pollEvent(event)) {
            if (event.type == Event::Closed) {
                window.close();
            }

            if (event.type == Event::KeyPressed) {
                if (currentState == GameState::MAIN_MENU) {
                    handleMainMenuInput(event.key.code);
                }
                else if (currentState == GameState::PLAYING) {
                    handleGameInput(event.key.code);
                }
                else if (currentState == GameState::GAME_OVER) {
                    handleGameOverInput(event.key.code);
                }
            }
        }
    }
    void handleMainMenuInput(Keyboard::Key key) {
        switch (key) {
        case Keyboard::Up:
            menuSelection = 0;
            updateMenuDisplay();
            break;
        case Keyboard::Down:
            menuSelection = 1;
            updateMenuDisplay();
            break;
        case Keyboard::Enter:
            if (menuSelection == 0) {
                currentState = GameState::PLAYING;
                initializeGame();
            }
            else {
                window.close();
            }
            break;
        default:
            break;
        }
    }

    void handleGameInput(Keyboard::Key key) {
        // Если ждем переворота карт, блокируем ввод
        if (waitingForFlipBack) {
            return;
        }

        switch (key) {
        case Keyboard::Escape:
            currentState = GameState::MAIN_MENU;
            break;
        case Keyboard::R:
            initializeGame();
            currentState = GameState::PLAYING;
            break;
        case Keyboard::Left:
            if (selectedCard > 0) {
                selectedCard--;
                updateCardSelection();
            }
            break;
        case Keyboard::Right:
            if (selectedCard < cards.size() - 1) {
                selectedCard++;
                updateCardSelection();
            }
            break;
        case Keyboard::Up:
            if (selectedCard >= gridSize) {
                selectedCard -= gridSize;
                updateCardSelection();
            }
            break;
        case Keyboard::Down:
            if (selectedCard < cards.size() - gridSize) {
                selectedCard += gridSize;
                updateCardSelection();
            }
            break;
        case Keyboard::Space:
        case Keyboard::Enter:
            flipSelectedCard();
            break;
        default:
            break;
        }
    }

    void handleGameOverInput(Keyboard::Key key) {
        switch (key) {
        case Keyboard::Enter:
        case Keyboard::Space:
            currentState = GameState::MAIN_MENU;
            break;
        case Keyboard::R:
            currentState = GameState::PLAYING;
            initializeGame();
            break;
        default:
            break;
        }
    }

    void updateCardSelection() {
        for (size_t i = 0; i < cards.size(); ++i) {
            if (!cards[i].isMatched) {
                if (i == selectedCard) {
                    cards[i].shape.setOutlineColor(selectedCardColor);
                    cards[i].shape.setOutlineThickness(4);
                }
                else {
                    cards[i].shape.setOutlineColor(Color::Black);
                    cards[i].shape.setOutlineThickness(2);
                }
            }
        }
    }

    void flipSelectedCard() {
        if (!cards[selectedCard].isFlipped && !cards[selectedCard].isMatched && selectedCards.size() < 2) {
            gameStarted = true;
            flipCard(selectedCard);

            if (selectedCards.size() == 2) {
                // Запускаем проверку совпадения с задержкой
                waitingForFlipBack = true;
                flipBackTime = 0.0f;
                flipClock.restart();
            }
        }
    }

    void flipCard(int index) {
        cards[index].isFlipped = true;
        selectedCards.push_back(index);
        cards[index].shape.setFillColor(flippedColor);
    }

    void flipBackCards() {
        if (selectedCards.size() == 2) {
            int index1 = selectedCards[0];
            int index2 = selectedCards[1];
            cards[index1].isFlipped = false;
            cards[index2].isFlipped = false;
            cards[index1].shape.setFillColor(cardColor);
            cards[index2].shape.setFillColor(cardColor);

            selectedCards.clear();
            waitingForFlipBack = false;
        }
    }

    void checkMatch() {
        if (selectedCards.size() == 2) {
            int index1 = selectedCards[0];
            int index2 = selectedCards[1];

            if (cards[index1].value == cards[index2].value) {
                // Карты совпали
                cards[index1].isMatched = true;
                cards[index2].isMatched = true;
                cards[index1].shape.setFillColor(matchedColor);
                cards[index2].shape.setFillColor(matchedColor);
                score += 10;
                scoreText.setString("Score: " + to_string(score));

                selectedCards.clear();
                waitingForFlipBack = false;

                // Проверка окончания игры
                checkGameOver();
            }
            else {
                // Карты не совпали - устанавливаем флаг для переворота обратно
                waitingForFlipBack = true;
                flipBackTime = 0.0f;
                flipClock.restart();
            }
        }
    }

    void checkGameOver() {
        bool allMatched = true;
        for (const auto& card : cards) {
            if (!card.isMatched) {
                allMatched = false;
                break;
            }
        }

        if (allMatched) {
            gameFinished = true;
            currentState = GameState::GAME_OVER;
            gameOverText.setString("Game Over! Final Score: " + to_string(score));
            FloatRect textBounds = gameOverText.getLocalBounds();
            gameOverText.setPosition(950 - textBounds.width / 2, 250);
        }
    }

    void updateMenuDisplay() {
        string menuString = "\n\n";

        if (menuSelection == 0) {
            menuString += "> START GAME <\n";
            menuString += "  EXIT";
        }
        else {
            menuString += "  START GAME\n";
            menuString += "> EXIT <";
        }

        menuText.setString(menuString);
        FloatRect textBounds = menuText.getLocalBounds();
        menuText.setPosition(640 - textBounds.width / 2, 300);
    }

    void update() {
        if (currentState == GameState::PLAYING && gameStarted && !gameFinished) {
            gameTime = gameClock.getElapsedTime().asSeconds();
            timeText.setString("Time: " + to_string((int)gameTime) + "s");

            // Обработка задержки для переворота карт
            if (waitingForFlipBack) {
                flipBackTime += flipClock.restart().asSeconds();

                if (flipBackTime >= 1.0f) { // Задержка 1 секунда
                    if (selectedCards.size() == 2) {
                        int index1 = selectedCards[0];
                        int index2 = selectedCards[1];

                        if (cards[index1].value == cards[index2].value) {
                            // Карты совпали - оставляем открытыми
                            cards[index1].isMatched = true;
                            cards[index2].isMatched = true;
                            score += 10;
                            scoreText.setString("Score: " + to_string(score));
                            checkGameOver();
                        }
                        else {
                            // Карты не совпали - переворачиваем обратно
                            flipBackCards();
                            score = max(0, score - 2);
                            scoreText.setString("Score: " + to_string(score));
                        }
                        selectedCards.clear();
                        waitingForFlipBack = false;
                    }
                }
            }

            // Автоматическая проверка совпадения после выбора второй карты
            if (selectedCards.size() == 2 && !waitingForFlipBack) {
                checkMatch();
            }
        }
    }

    void render() {
        window.clear(Color(30, 30, 60));

        switch (currentState) {
        case GameState::MAIN_MENU:
            renderMainMenu();
            break;

        case GameState::PLAYING:
            renderGame();
            break;

        case GameState::GAME_OVER:
            renderGame();
            renderGameOver();
            break;
        }

        window.display();
    }

    void renderMainMenu() {
        // Фоновое изображение меню
        window.draw(menuBackground);



        updateMenuDisplay();
        window.draw(menuText);

        // Инструкция управления
        controlsText.setString(
            "CONTROLS:\n"
            "UP/DOWN - Select menu option\n"
            "ENTER - Confirm selection\n\n"
            "IN GAME:\n"
            "ARROWS - Move card selection\n"
            "SPACE/ENTER - Flip card\n"
            "R - Restart game\n"
            "ESC - Main menu"
        );
        FloatRect controlsBounds = controlsText.getLocalBounds();
        controlsText.setPosition(200 - controlsBounds.width / 2, 350);
        window.draw(controlsText);
    }

    void renderGame() {
        // Фоновое изображение игры
        window.draw(gameBackground);

        // Полупрозрачный overlay для игровой области
        RectangleShape gameArea(Vector2f(1280, 720));
        gameArea.setFillColor(Color(0, 0, 0, 50));
        window.draw(gameArea);

        // Отрисовка карт
        for (const auto& card : cards) {
            window.draw(card.shape);
            if (card.isFlipped || card.isMatched) {
                window.draw(card.text);
            }
        }

        // Отрисовка текста
        window.draw(scoreText);
        window.draw(timeText);

        // Инструкция во время игры
        controlsText.setString(
            "CONTROLS: ARROWS=MOVE  SPACE=FLIP  R=RESTART  ESC=MENU"
        );
        controlsText.setPosition(400 - controlsText.getLocalBounds().width / 2, 50);
        window.draw(controlsText);

        // Отображение таймера переворота (для отладки)
        if (waitingForFlipBack) {
            Text flipText;
            flipText.setFont(font);
            flipText.setString("Checking...");
            flipText.setCharacterSize(20);
            flipText.setFillColor(Color::Yellow);
            flipText.setPosition(600, 10);
            window.draw(flipText);
        }
    }

    void renderGameOver() 
    {


        window.draw(gameOverText);

        // Текст с инструкцией
        Text restartText;
        restartText.setFont(font);
        restartText.setString("Press ENTER for Main Menu or R to Restart");
        restartText.setCharacterSize(24);
        restartText.setFillColor(Color::White);
        FloatRect textBounds = restartText.getLocalBounds();
        restartText.setPosition(950 - textBounds.width / 2, 300);
        window.draw(restartText);
    }
};

int main() 
{
    MemoryGame game;
    game.run();
}
