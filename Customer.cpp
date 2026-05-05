#include "S26-12_header.h"
using namespace std;
void Customer::signin()
{
	int tries = 0, set = 0;
	string reCaptchaImg[5][2] = { {"reCaptcha1.png", "A7kP9"}, {"reCaptcha2.png", "ALliteRaTIoN"}, {"reCaptcha3.png", "hou-se"}, {"reCaptcha4.png", "b4ckgr0uzd"}, {"reCaptcha5.png", "JkL5Np"} };
	srand(time(nullptr));
	string username, password;
	cout << "Enter username of Customer: \n";
	cin >> username;
	setUsername(username);
	if (!findUsername())
	{
		cout << "\nUsername not found\n";
		return;
	}
	system("cls");
	int num = rand() % 5;

	// reCaptcha check window 
	sf::RenderWindow window(sf::VideoMode(400, 350), "reCaptcha check");
	sf::Font font;
	if (!font.loadFromFile("arial.ttf"))
	{
		return;
	}

	sf::RectangleShape textBox;
	textBox.setSize(sf::Vector2f(400, 200));
	textBox.setPosition(0.f, 200.f);
	textBox.setFillColor(sf::Color::Cyan);
	textBox.setOutlineColor(sf::Color::Black);
	textBox.setOutlineThickness(5);

	string input;
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

	sf::Texture texture;
	if (!texture.loadFromFile(reCaptchaImg[num][0]))
	{
		cout << "Could not load image file! \n";
		return;
	}
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
				if (event.text.unicode == 8 && !input.empty())
				{
					input.pop_back();
				}
				else if (event.text.unicode == 13)
				{
					window.close();
				}
				else if (event.text.unicode >= 32 && event.text.unicode <= 128)
				{
					input += static_cast<char>(event.text.unicode);
				}
				inText.setString(input);
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

	if (input == reCaptchaImg[num][1])
	{
		cout << "reCaptcha entered successfuly\n\n";
	}
	else
	{
		cout << "reCaptcha failed\n\n";
		return;
	}

	cout << "Enter password of Customer: \n";
	cin >> password;
	setPassword(password);
	while (checkPassword())
	{
		tries++;
		if (tries == 3)
		{
			countdownTimer(set);
			tries = 0;
		}
		cout << "\nWrong password entered, enter correct password: \n";
		cin >> password;
		setPassword(password);
	}
	cout << "\nCustomer login was successful\n";
}
void Customer::signup()
{
	string username, password, entry;
	bool fileEmpty = false;
	cout << "Enter a new customer username: \n";
	cin >> username;
	setUsername(username);
	ifstream fileIn("CustomerInfo.csv");
	if (!fileIn.is_open())
	{
		cout << "Error: File failed to open\n";
		return;
	}
	if (fileIn.peek() == ifstream::traits_type::eof())
	{
		fileEmpty = true;
	}
	fileIn.close();
	bool usernamefound = findUsername();
	while (findUsername())
	{
		system("cls");
		cout << "Username already exists, choose a new username: \n";
		cin >> username;
		setUsername(username);
	}
	fstream fileApp("CustomerInfo.csv", ios::app);
	if (fileEmpty)
	{
		fileApp << "username,password";
	}
	cout << "\nEnter a new customer password: \n";
	cin >> password;
	setPassword(password);
	while (!checkStrength())
	{
		cin >> password;
		setPassword(password);
	}
	entry = getUsername() + ',' + getPassword();
	fileApp << entry;
	fileApp.close();
	cout << "\nCustomer details entered successfully \n";
}
bool Customer::findUsername()
{
	ifstream fileIn("CustomerInfo.csv");
	string line, username;
	if (!fileIn.is_open())
	{
		cout << "\nError: file failed to open\n";
	}
	else
	{
		getline(fileIn, line);
		while (getline(fileIn, line))
		{
			username = "";
			for (int i = 0; line[i] != ','; i++)
			{
				username = username + line[i];
			}
			if (username == getUsername())
			{
				fileIn.close();
				return true;
			}
		}
		fileIn.close();
	}
	return false;
}
bool Customer::checkPassword()
{
	ifstream fileIn("CustomerInfo.csv");
	string line, username, password;
	int delimPos;
	if (!fileIn.is_open())
	{
		cout << "\nError: file failed to open\n";
	}
	else
	{
		getline(fileIn, line);
		while (getline(fileIn, line))
		{
			username = "";
			password = "";
			delimPos = 0;
			for (int i = 0; line[i] != ','; i++)
			{
				username = username + line[i];
				delimPos++;
			}
			if (username == getUsername())
			{
				for (int i = delimPos + 1; i < line.length(); i++)
				{
					password = password + line[i];
				}
				if (password == getPassword())
				{
					fileIn.close();
					return true;
				}
			}
		}
		fileIn.close();
	}
	return false;
}
int Customer::checkGeneric(string password, int& numGeneric)
{
	int numStored = 0;
	string line;
	ifstream fileIn("Generic.txt");
	while (getline(fileIn, line))
	{
		numStored++;
	}
	string* storedGeneric = new string[numStored];
	fileIn.clear();
	fileIn.seekg(0, ios::beg);
	for (int i = 0; i < numStored; i++)
	{
		getline(fileIn, line);
		storedGeneric[i] = line;
	}
	fileIn.close();
	for (int i = 0; i < numStored; i++)
	{
		if (password.length() >= storedGeneric[i].length())
		{
			for (int j = 0; j < password.length(); j++)
			{
				if (password[j] == storedGeneric[i][0])
				{
					bool foundGeneric = true;
					bool reachedEnd = false;
					for (int k = 0, l = j; k < storedGeneric[i].length() && l < password.length(); k++, l++)
					{
						if (storedGeneric[i][k] != password[l])
						{
							foundGeneric = false;
							break;
						}
						if (k == storedGeneric[i].length() - 1)
						{
							reachedEnd = true;
						}
					}
					if (foundGeneric && reachedEnd)
					{
						numGeneric++;
						break;
					}
				}
			}
		}
	}
	delete[] storedGeneric;
	return numGeneric;
}
bool Customer::checkStrength()
{
	string password = getPassword();
	string line;
	int numSpecial = 0, numInteger = 0, numLower = 0, numUpper = 0, numGeneric = 0;
	checkGeneric(password, numGeneric);
	for (int i = 0; i < password.length(); i++)
	{
		if ((password[i] >= 32 && password[i] <= 47) || (password[i] >= 58 && password[i] <= 64) || (password[i] >= 91 && password[i] <= 96) || (password[i] >= 123 && password[i] <= 126))
		{
			numSpecial++;
		}
		else if (password[i] >= 48 && password[i] <= 57)
		{
			numInteger++;
		}
		else if (password[i] >= 65 && password[i] <= 90)
		{
			numUpper++;
		}
		else if (password[i] >= 97 && password[i] <= 122)
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
