////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <cmath>
#include <ctime>
#include <cstdlib>
#include <vector>

#ifdef SFML_SYSTEM_IOS
#include <SFML/Main.hpp>
#endif

std::string resourcesDir()
{
#ifdef SFML_SYSTEM_IOS
    return "";
#else
    return "resources/";
#endif
}

////////////////////////////////////////////////////////////
/// Entry point of application
///
/// \return Application exit code
///
////////////////////////////////////////////////////////////
int main()
{
    std::srand(static_cast<unsigned int>(std::time(NULL)));

    // Define some constants
    const float pi = 3.14159f;
    const float gameWidth = 800;
    const float gameHeight = 600;
    sf::Vector2f paddleSize(25, 100);
    float ballRadius = 10.f;

    // Create the window of the application
    sf::RenderWindow window(sf::VideoMode(static_cast<unsigned int>(gameWidth), static_cast<unsigned int>(gameHeight), 32), "SFML Tennis",
        sf::Style::Titlebar | sf::Style::Close);
    window.setVerticalSyncEnabled(true);

    // Load the sounds used in the game
    sf::SoundBuffer ballSoundBuffer;
    if (!ballSoundBuffer.loadFromFile(resourcesDir() + "ball.wav"))
        return EXIT_FAILURE;
    sf::Sound ballSound(ballSoundBuffer);

    // Create the SFML logo texture:
    sf::Texture sfmlLogoTexture;
    if (!sfmlLogoTexture.loadFromFile(resourcesDir() + "sfml_logo.png"))
        return EXIT_FAILURE;
    sf::Sprite sfmlLogo;
    sfmlLogo.setTexture(sfmlLogoTexture);
    sfmlLogo.setPosition(170, 50);

    // Create the left paddle
    sf::RectangleShape leftPaddle;
    leftPaddle.setSize(paddleSize - sf::Vector2f(3, 3));
    leftPaddle.setOutlineThickness(3);
    leftPaddle.setOutlineColor(sf::Color::Black);
    leftPaddle.setFillColor(sf::Color(100, 100, 200));
    leftPaddle.setOrigin(paddleSize / 2.f);

    // Create the right paddle
    sf::RectangleShape rightPaddle;
    rightPaddle.setSize(paddleSize - sf::Vector2f(3, 3));
    rightPaddle.setOutlineThickness(3);
    rightPaddle.setOutlineColor(sf::Color::Black);
    rightPaddle.setFillColor(sf::Color(200, 100, 100));
    rightPaddle.setOrigin(paddleSize / 2.f);

    // Create the ball
    sf::CircleShape ball;
    ball.setRadius(ballRadius - 3);
    ball.setOutlineThickness(2);
    ball.setOutlineColor(sf::Color::Black);
    ball.setFillColor(sf::Color::White);
    ball.setOrigin(ballRadius / 2, ballRadius / 2);

    // Create additional balls vector
    std::vector<sf::CircleShape> balls;
    balls.push_back(ball);

    // Load the text font
    sf::Font font;
    if (!font.loadFromFile(resourcesDir() + "tuffy.ttf"))
        return EXIT_FAILURE;

    // Define the pause message
    std::string inputString;

#ifdef SFML_SYSTEM_IOS
    inputString = "Touch the screen to restart.";
#else
    inputString = "Press space to restart or\nescape to exit.";
#endif

    sf::Text pauseMessage;
    pauseMessage.setFont(font);
    pauseMessage.setCharacterSize(40);
    pauseMessage.setPosition(170.f, 200.f);
    pauseMessage.setFillColor(sf::Color::White);
    pauseMessage.setString("Welcome to SFML Tennis!\n\n" + inputString);

    // Initialize the score
    int leftScore = 0;
    int rightScore = 0;
    const int winningScore = 5;

    sf::Text leftScoreText;
    leftScoreText.setFont(font);
    leftScoreText.setCharacterSize(30);
    leftScoreText.setFillColor(sf::Color::White);
    leftScoreText.setPosition(gameWidth / 2.f - 60.f, 10.f);

    sf::Text rightScoreText;
    rightScoreText.setFont(font);
    rightScoreText.setCharacterSize(30);
    rightScoreText.setFillColor(sf::Color::White);
    rightScoreText.setPosition(gameWidth / 2.f + 30.f, 10.f);

    // Define the paddles properties
    sf::Clock AITimer;
    const sf::Time AITime = sf::seconds(0.1f);
    const float paddleSpeed = 400.f;
    float rightPaddleSpeed = 0.f;
    float ballSpeed = 400.f;
    float ballAngle = 0.f; // to be changed later

    sf::Clock clock;
    sf::Clock gameClock; // Timer for the game
    sf::Clock newBallClock; // Timer for new balls
    bool isPlaying = false;
    while (window.isOpen())
    {
        // Handle events
        sf::Event event;
        while (window.pollEvent(event))
        {
            // Window closed or escape key pressed: exit
            if ((event.type == sf::Event::Closed) ||
                ((event.type == sf::Event::KeyPressed) && (event.key.code == sf::Keyboard::Escape)))
            {
                window.close();
                break;
            }

            // Space key pressed: play
            if (((event.type == sf::Event::KeyPressed) && (event.key.code == sf::Keyboard::Space)) ||
                (event.type == sf::Event::TouchBegan))
            {
                if (!isPlaying)
                {
                    // (re)start the game
                    isPlaying = true;
                    clock.restart();
                    gameClock.restart();
                    newBallClock.restart();

                    // Reset the position of the paddles and ball
                    leftPaddle.setPosition(10.f + paddleSize.x / 2.f, gameHeight / 2.f);
                    rightPaddle.setPosition(gameWidth - 10.f - paddleSize.x / 2.f, gameHeight / 2.f);
                    balls.clear();

                    sf::CircleShape newBall;
                    newBall.setRadius(ballRadius - 3);
                    newBall.setOutlineThickness(2);
                    newBall.setOutlineColor(sf::Color::Black);
                    newBall.setFillColor(sf::Color::White);
                    newBall.setOrigin(ballRadius / 2, ballRadius / 2);
                    newBall.setPosition(gameWidth / 2.f, gameHeight / 2.f);

                    balls.push_back(newBall);

                    // Reset the ball angle
                    do
                    {
                        // Make sure the ball initial angle is not too much vertical
                        ballAngle = static_cast<float>(std::rand() % 360) * 2.f * pi / 360.f;
                    } while (std::abs(std::cos(ballAngle)) < 0.7f);

                    // Reset ball speed
                    ballSpeed = 400.f;
                }
            }

            // Window size changed, adjust view appropriately
            if (event.type == sf::Event::Resized)
            {
                sf::View view;
                view.setSize(gameWidth, gameHeight);
                view.setCenter(gameWidth / 2.f, gameHeight / 2.f);
                window.setView(view);
            }
        }

        if (isPlaying)
        {
            float deltaTime = clock.restart().asSeconds();

            // Move the player's paddle
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up) &&
                (leftPaddle.getPosition().y - paddleSize.y / 2 > 5.f))
            {
                leftPaddle.move(0.f, -paddleSpeed * deltaTime);
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down) && (leftPaddle.getPosition().y + paddleSize.y / 2 < gameHeight - 5.f))
            {
                leftPaddle.move(0.f, paddleSpeed * deltaTime);
            }

            // Move the computer's paddle
            if (((rightPaddleSpeed < 0.f) && (rightPaddle.getPosition().y - paddleSize.y / 2 > 5.f)) ||
                ((rightPaddleSpeed > 0.f) && (rightPaddle.getPosition().y + paddleSize.y / 2 < gameHeight - 5.f)))
            {
                rightPaddle.move(0.f, rightPaddleSpeed * deltaTime);
            }

            // Update the computer's paddle direction according to the ball position
            if (AITimer.getElapsedTime() > AITime)
            {
                AITimer.restart();
                if (balls[0].getPosition().y + ballRadius > rightPaddle.getPosition().y + paddleSize.y / 2)
                    rightPaddleSpeed = paddleSpeed;
                else if (balls[0].getPosition().y - ballRadius < rightPaddle.getPosition().y - paddleSize.y / 2)
                    rightPaddleSpeed = -paddleSpeed;
                else
                    rightPaddleSpeed = 0.f;
            }

            // Move the balls
            for (auto& ball : balls)
            {
                float factor = ballSpeed * deltaTime;
                ball.move(std::cos(ballAngle) * factor, std::sin(ballAngle) * factor);

                // Check collisions between the ball and the screen
                if (ball.getPosition().x - ballRadius < 0.f)
                {
                    isPlaying = false;
                    rightScore++;
                    if (rightScore >= winningScore)
                    {
                        pauseMessage.setString("Red Team Wins!\n\n" + inputString);
                        leftScore = 0;
                        rightScore = 0;
                    }
                    else
                    {
                        pauseMessage.setString("Blue Team Scores!\n\n" + inputString);
                    }
                }
                if (ball.getPosition().x + ballRadius > gameWidth)
                {
                    isPlaying = false;
                    leftScore++;
                    if (leftScore >= winningScore)
                    {
                        pauseMessage.setString("Blue Team Wins!\n\n" + inputString);
                        leftScore = 0;
                        rightScore = 0;
                    }
                    else
                    {
                        pauseMessage.setString("Red Team Scores!\n\n" + inputString);
                    }
                }
                if (ball.getPosition().y - ballRadius < 0.f)
                {
                    ballSound.play();
                    ballAngle = -ballAngle;
                    ball.setPosition(ball.getPosition().x, ballRadius + 0.1f);
                }
                if (ball.getPosition().y + ballRadius > gameHeight)
                {
                    ballSound.play();
                    ballAngle = -ballAngle;
                    ball.setPosition(ball.getPosition().x, gameHeight - ballRadius - 0.1f);
                }

                // Check the collisions between the ball and the paddles
                // Left Paddle
                if (ball.getPosition().x - ballRadius < leftPaddle.getPosition().x + paddleSize.x / 2 &&
                    ball.getPosition().x - ballRadius > leftPaddle.getPosition().x &&
                    ball.getPosition().y + ballRadius >= leftPaddle.getPosition().y - paddleSize.y / 2 &&
                    ball.getPosition().y - ballRadius <= leftPaddle.getPosition().y + paddleSize.y / 2)
                {
                    if (ball.getPosition().y > leftPaddle.getPosition().y)
                        ballAngle = pi - ballAngle + static_cast<float>(std::rand() % 20) * pi / 180;
                    else
                        ballAngle = pi - ballAngle - static_cast<float>(std::rand() % 20) * pi / 180;

                    ballSound.play();
                    ball.setPosition(leftPaddle.getPosition().x + ballRadius + paddleSize.x / 2 + 0.1f, ball.getPosition().y);
                    // Increase ball speed
                    ballSpeed += 10.f;
                }

                // Right Paddle
                if (ball.getPosition().x + ballRadius > rightPaddle.getPosition().x - paddleSize.x / 2 &&
                    ball.getPosition().x + ballRadius < rightPaddle.getPosition().x &&
                    ball.getPosition().y + ballRadius >= rightPaddle.getPosition().y - paddleSize.y / 2 &&
                    ball.getPosition().y - ballRadius <= rightPaddle.getPosition().y + paddleSize.y / 2)
                {
                    if (ball.getPosition().y > rightPaddle.getPosition().y)
                        ballAngle = pi - ballAngle + static_cast<float>(std::rand() % 20) * pi / 180;
                    else
                        ballAngle = pi - ballAngle - static_cast<float>(std::rand() % 20) * pi / 180;

                    ballSound.play();
                    ball.setPosition(rightPaddle.getPosition().x - ballRadius - paddleSize.x / 2 - 0.1f, ball.getPosition().y);
                    // Increase ball speed
                    ballSpeed += 10.f;
                }
            }

            // Add a new ball every 10 seconds
            if (newBallClock.getElapsedTime().asSeconds() >= 10.f)
            {
                sf::CircleShape newBall;
                newBall.setRadius(ballRadius - 3);
                newBall.setOutlineThickness(2);
                newBall.setOutlineColor(sf::Color::Black);
                newBall.setFillColor(sf::Color::White);
                newBall.setOrigin(ballRadius / 2, ballRadius / 2);
                newBall.setPosition(gameWidth / 2.f, gameHeight / 2.f);
                balls.push_back(newBall);
                newBallClock.restart();
            }
        }

        // Clear the window
        window.clear(sf::Color(50, 50, 50));

        if (isPlaying)
        {
            // Draw the paddles and the balls
            window.draw(leftPaddle);
            window.draw(rightPaddle);
            for (const auto& ball : balls)
            {
                window.draw(ball);
            }
        }
        else
        {
            // Draw the pause message
            window.draw(pauseMessage);
            window.draw(sfmlLogo);
        }

        // Update the score display
        leftScoreText.setString(std::to_string(leftScore));
        rightScoreText.setString(std::to_string(rightScore));

        // Draw the scores
        window.draw(leftScoreText);
        window.draw(rightScoreText);

        // Display time
        sf::Text timeText;
        timeText.setFont(font);
        timeText.setCharacterSize(20);
        timeText.setFillColor(sf::Color::White);
        timeText.setString("Time: " + std::to_string(static_cast<int>(gameClock.getElapsedTime().asSeconds())) + "s");
        timeText.setPosition((gameWidth - timeText.getLocalBounds().width) / 2.f, gameHeight - 50.f);
        window.draw(timeText);

        // Display things on screen
        window.display();
    }

    return EXIT_SUCCESS;
}
