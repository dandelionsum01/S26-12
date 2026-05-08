#include "header.h"
using namespace std;
namespace
{
    string sanitize(string s)
    {
        if (s.size() >= 3 && static_cast<unsigned char>(s[0]) == 0xEF && static_cast<unsigned char>(s[1]) == 0xBB && static_cast<unsigned char>(s[2]) == 0xBF)
        {
            s.erase(0, 3);
        }
        while (!s.empty() && (s.back() == '\r' || s.back() == '\n' || s.back() == ' ' || s.back() == '\t'))
        {
            s.pop_back();
        }
        return s;
    }
    bool consoleCaptcha(const string &expected)
    {
        cout << "(Console reCaptcha) Please type: " << expected << "\n> ";
        string typed;
        getline(cin, typed);
        return typed == expected;
    }
}

void Customer::signin()
{
    const string reCaptchaImg[5][2] = {
        {"reCaptcha1.png", "A7kP9"},
        {"reCaptcha2.png", "ALliteRaTIoN"},
        {"reCaptcha3.png", "hou-se"},
        {"reCaptcha4.png", "b4ckgr0uzd"},
        {"reCaptcha5.png", "JkL5Np"}};
    srand(static_cast<unsigned>(time(nullptr)));
    string username, password;
    cout << "Enter username of Customer\n";
    cin >> username;
    setUsername(username);
    if (!findUsername())
    {
        cout << "\nUsername not found\n";
        return;
    }
    system("cls");
    int num = rand() % 5;
    string captchaInput;
    sf::Font font;
    bool fontLoaded = font.loadFromFile("arial.ttf");
    sf::Texture texture;
    bool textureLoaded = texture.loadFromFile(reCaptchaImg[num][0]);
    if (!fontLoaded || !textureLoaded)
    {
        cout << "(GUI assets missing -- using console reCaptcha)\n";
        if (!consoleCaptcha(reCaptchaImg[num][1]))
        {
            cout << "reCaptcha failed\n\n";
            return;
        }
        cout << "reCaptcha entered successfully\n\n";
    }
    else
    {
        sf::RenderWindow window(sf::VideoMode(400, 350), "reCaptcha check");
        sf::RectangleShape textBox;
        textBox.setSize(sf::Vector2f(400, 200));
        textBox.setPosition(0.f, 200.f);
        textBox.setFillColor(sf::Color::Cyan);
        textBox.setOutlineColor(sf::Color::Black);
        textBox.setOutlineThickness(5);
        sf::Text outText("Enter reCaptcha shown:", font, 30);
        outText.setFillColor(sf::Color::Black);
        outText.setStyle(sf::Text::Bold);
        outText.setPosition(0.f, 200.f);
        sf::Text inText("", font, 30);
        inText.setFillColor(sf::Color::Black);
        inText.setPosition(0.f, 210.f + outText.getCharacterSize());
        sf::RectangleShape buttonBox;
        buttonBox.setSize(sf::Vector2f(100, 40));
        buttonBox.setPosition(150.f, 240.f + 2 * outText.getCharacterSize());
        buttonBox.setFillColor(sf::Color::White);
        sf::Text buttonText("submit", font, 30);
        buttonText.setFillColor(sf::Color::Black);
        buttonText.setPosition(155.f, 245.f + 2 * outText.getCharacterSize());
        sf::Sprite sprite(texture);
        while (window.isOpen())
        {
            sf::Event event;
            while (window.pollEvent(event))
            {
                if (event.type == sf::Event::Closed)
                {
                    window.close();
                }
                sf::Vector2i pos = sf::Mouse::getPosition(window);
                if (buttonBox.getGlobalBounds().contains(static_cast<float>(pos.x), static_cast<float>(pos.y)))
                {
                    buttonBox.setFillColor(sf::Color::Blue);
                }
                else
                {
                    buttonBox.setFillColor(sf::Color::White);
                }
                if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)
                {
                    if (buttonBox.getGlobalBounds().contains(static_cast<float>(pos.x), static_cast<float>(pos.y)))
                    {
                        window.close();
                    }
                }

                if (event.type == sf::Event::TextEntered)
                {
                    if (event.text.unicode == 8 && !captchaInput.empty())
                    {
                        captchaInput.pop_back();
                    }
                    else if (event.text.unicode == 13)
                    {
                        window.close();
                    }
                    else if (event.text.unicode >= 32 && event.text.unicode <= 128)
                    {
                        captchaInput += static_cast<char>(event.text.unicode);
                    }
                    inText.setString(captchaInput);
                }
            }
            window.clear();
            window.draw(sprite);
            window.draw(textBox);
            window.draw(buttonBox);
            window.draw(buttonText);
            window.draw(outText);
            window.draw(inText);
            window.display();
        }
        if (captchaInput == reCaptchaImg[num][1])
        {
            cout << "reCaptcha entered successfully\n\n";
        }
        else
        {
            cout << "reCaptcha failed\n\n";
            return;
        }
    }
    int tries = 0, set = 0;
    cout << "Enter password of Customer:\n";
    cin >> password;
    setPassword(password);
    while (!checkPassword())
    {
        tries++;
        if (tries == 3)
        {
            countdownTimer(set);
            tries = 0;
        }
        cout << "\nWrong password entered, enter correct password:\n";
        cin >> password;
        setPassword(password);
    }
    cout << "\nCustomer login was successful\n";
}

void Customer::signup()
{
    string username, password, entry;
    cout << "Enter a new customer username:\n";
    cin >> username;
    setUsername(username);
    bool fileEmpty = false;
    {
        ifstream fileIn("CustomerInfo.csv");
        if (!fileIn.is_open())
        {
            fileEmpty = true;
        }
        else
        {
            fileIn.seekg(0, ios::end);
            fileEmpty = (fileIn.tellg() == 0);
            fileIn.close();
        }
    }
    while (findUsername())
    {
        system("cls");
        cout << "Username already exists, choose a new username:\n";
        cin >> username;
        setUsername(username);
    }
    cout << "\nEnter a new customer password:\n";
    cin >> password;
    setPassword(password);
    while (!checkStrength())
    {
        cin >> password;
        setPassword(password);
    }
    ofstream fileApp("CustomerInfo.csv", ios::app);
    if (!fileApp.is_open())
    {
        cout << "Error: cannot open CustomerInfo.csv for writing\n";
        return;
    }
    if (fileEmpty)
    {
        fileApp << "username,password\n";
    }
    entry = getUsername() + "," + getPassword() + "\n";
    fileApp << entry;
    fileApp.close();
    cout << "\nCustomer details entered successfully\n";
}

bool Customer::findUsername()
{
    ifstream fileIn("CustomerInfo.csv");
    if (!fileIn.is_open())
    {
        return false;
    }
    string line;
    getline(fileIn, line);
    while (getline(fileIn, line))
    {
        size_t comma = line.find(',');
        string username = (comma == string::npos) ? line : line.substr(0, comma);
        username = sanitize(username);
        if (username == getUsername())
        {
            fileIn.close();
            return true;
        }
    }
    fileIn.close();
    return false;
}

bool Customer::checkPassword()
{
    ifstream fileIn("CustomerInfo.csv");
    if (!fileIn.is_open())
    {
        return false;
    }
    string line;
    getline(fileIn, line);
    while (getline(fileIn, line))
    {
        size_t comma = line.find(',');
        if (comma == string::npos)
        {
            continue;
        }
        string username = sanitize(line.substr(0, comma));
        string password = sanitize(line.substr(comma + 1));
        if (username == getUsername() && password == getPassword())
        {
            fileIn.close();
            return true;
        }
    }
    fileIn.close();
    return false;
}

int Customer::checkGeneric(const string &password, int &numGeneric)
{
    ifstream fileIn("Generic.txt");
    if (!fileIn.is_open())
    {
        return numGeneric;
    }
    string line;
    while (getline(fileIn, line))
    {
        line = sanitize(line);
        if (line.empty())
        {
            continue;
        }
        if (password.find(line) != string::npos)
        {
            numGeneric++;
        }
    }
    fileIn.close();
    return numGeneric;
}

bool Customer::checkStrength()
{
    string password = getPassword();
    int numSpecial = 0, numInteger = 0, numLower = 0, numUpper = 0, numGeneric = 0;
    checkGeneric(password, numGeneric);
    for (size_t i = 0; i < password.length(); ++i)
    {
        unsigned char c = static_cast<unsigned char>(password[i]);
        if ((c >= 32 && c <= 47) || (c >= 58 && c <= 64) || (c >= 91 && c <= 96) || (c >= 123 && c <= 126))
        {
            numSpecial++;
        }
        else if (c >= 48 && c <= 57)
        {
            numInteger++;
        }
        else if (c >= 65 && c <= 90)
        {
            numUpper++;
        }
        else if (c >= 97 && c <= 122)
        {
            numLower++;
        }
    }

    if (password.length() < 8 || numSpecial < 1 || numInteger < 2 || numLower < 1 || numUpper < 1 || numGeneric > 0)
    {
        cout << "\nPassword must contain: ";
        if (password.length() < 8)
        {
            cout << "at least 8 characters, ";
        }
        if (numSpecial < 1)
        {
            cout << "at least 1 special character, ";
        }
        if (numInteger < 2)
        {
            cout << "at least 2 integers, ";
        }
        if (numLower < 1)
        {
            cout << "at least 1 lowercase character, ";
        }
        if (numUpper < 1)
        {
            cout << "at least 1 uppercase character, ";
        }
        if (numGeneric > 0)
        {
            cout << "no generic words, ";
        }
        cout << "\nEnter a stronger password\n";
        return false;
    }
    return true;
}
