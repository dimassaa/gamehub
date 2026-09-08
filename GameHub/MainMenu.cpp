#include "MainMenu.h"
#include "AdminMenu.h"
#include "CustomerMenu.h"
#include "FileManager.h"

MainMenu::MainMenu(Store& s, Wishlist& w, RecommendationSystem& rs, Report& r, NotificationSystem& ns)
    : Menu(s, w, rs, r, ns) {
}

void MainMenu::show() 
{
    while (true)
    {
        displayHeader("GAMEHUB - ������� ����");

        std::vector<std::string> options = 
        {
            "����� � �������",
            "������������������",
            "������������ ������",
            "���������� � �������"
        };

        TableFormatter::displayMenu(options, "������� ����");

        int choice = InputValidation::getValidatedNumber("�������� ��������: ", 0, 4);

        switch (choice) 
        {
        case 1:
            if (login()) 
            {
                showUserMenu();
            }
            break;
        case 2:
            registerUser();
            break;
        case 3:
            recoverPassword();
            break;
        case 4:
            aboutSystem();
            break;
        case 0:
            TableFormatter::displaySuccessMessage("�� ��������!");
            return;
        }
    }
}

bool MainMenu::login() 
{
    displayHeader("���� � �������");

    std::string username = InputValidation::getValidatedString("��� ������������: ", 3, 20);
    std::cout << "������: ";
    std::string password = InputValidation::maskPassword();

    FileManager fileManager;
    auto users = fileManager.loadFromFile<std::shared_ptr<User>>(Config::USERS_FILE);

    for (const auto& user : users) 
    {
        if (user->getUsername() == username) 
        {
            if (user->isBlocked()) 
            {
                TableFormatter::displayErrorMessage("������� ������������ ��: " + user->getUnlockDate());
                TableFormatter::pause();
                return false;
            }

            if (User::verifyPassword(password, user->getPasswordHash())) 
            {
                currentUser = user;
                TableFormatter::displaySuccessMessage("���� �������� �������!");
                TableFormatter::pause();
                return true;
            }
            else 
            {
                TableFormatter::displayErrorMessage("�������� ������!");
                TableFormatter::pause();
                return false;
            }
        }
    }

    TableFormatter::displayErrorMessage("������������ �� ������!");
    TableFormatter::pause();
    return false;
}

void MainMenu::registerUser() 
{
    displayHeader("�����������");

    FileManager fileManager;
    auto users = fileManager.loadFromFile<std::shared_ptr<User>>(Config::USERS_FILE);

    std::string username;
    while (true) 
    {
        username = InputValidation::getValidatedString("��� ������������: ", 3, 20);

        bool usernameExists = false;
        for (const auto& user : users) 
        {
            if (user->getUsername() == username) 
            {
                usernameExists = true;
                break;
            }
        }

        if (!usernameExists) 
        {
            break;
        }
        else 
        {
            TableFormatter::displayErrorMessage("��� ������������ ��� ������!");
        }
    }

    std::string email = InputValidation::getValidatedEmail();
    std::string phone = InputValidation::getValidatedPhone();
    std::string birthday = InputValidation::getValidatedBirthday();

    std::cout << "������: ";
    std::string password = InputValidation::maskPassword();
    std::cout << "����������� ������: ";
    std::string confirmPassword = InputValidation::maskPassword();

    if (password != confirmPassword) 
    {
        TableFormatter::displayErrorMessage("������ �� ���������!");
        TableFormatter::pause();
        return;
    }

    int newId = 1;
    if (!users.empty()) 
    {
        for (const auto& user : users) 
        {
            if (user->getId() >= newId) 
            {
                newId = user->getId() + 1;
            }
        }
    }

    std::string storedPassword = User::createPasswordHash(password);

    auto newCustomer = std::make_shared<Customer>(newId, username, storedPassword, email, phone, birthday);
    users.push_back(newCustomer);

    if (fileManager.saveToFile(Config::USERS_FILE, users)) 
    {
        TableFormatter::displaySuccessMessage("����������� �������! �������� ��������� ��������������.");
        logger->log(LogLevel::INFO, "New user registered: " + username);

        notificationSystem.notifyNewRegistration(username);
    }
    else 
    {
        TableFormatter::displayErrorMessage("������ ��� ���������� ������!");
    }

    TableFormatter::pause();
}

void MainMenu::recoverPassword() 
{
    displayHeader("�������������� ������");

    std::string email = InputValidation::getValidatedEmail("������� ��� email: ");
    std::string username = InputValidation::getValidatedString("������� ��� ������������: ", 3, 20);

    FileManager fileManager;
    auto users = fileManager.loadFromFile<std::shared_ptr<User>>(Config::USERS_FILE);

    for (auto& user : users) 
    {
        if (user->getEmail() == email && user->getUsername() == username) 
        {
            std::string newPassword = InputValidation::generateRandomPassword();
            std::string newStoredPassword = User::createPasswordHash(newPassword);

            user->setPasswordHash(newStoredPassword);

            if (fileManager.saveToFile(Config::USERS_FILE, users)) 
            {
                TableFormatter::displaySuccessMessage("������ ������� �������!");
                TableFormatter::displayWarningMessage("��� ����� ������: " + newPassword);
                TableFormatter::displayWarningMessage("��������� ��� � ���������� ����� � ���������� ������� ����� �����!");
                logger->log(LogLevel::INFO, "Password recovered for user: " + username);
            }
            else 
            {
                TableFormatter::displayErrorMessage("������ ��� ���������� ������ ������!");
            }
            TableFormatter::pause();
            return;
        }
    }

    TableFormatter::displayErrorMessage("������������ � ��������� email � ������ �� ������!");
    TableFormatter::pause();
}

void MainMenu::showUserMenu() 
{
    if (!currentUser) return;

    if (currentUser->getRole() == "admin") 
    {
        AdminMenu adminMenu(store, wishlist, recommendationSystem, report, notificationSystem, currentUser);
        adminMenu.show();
    }
    else 
    {
        CustomerMenu customerMenu(store, wishlist, recommendationSystem, report, notificationSystem, currentUser);
        customerMenu.show();
    }

    currentUser = nullptr;
}

void MainMenu::aboutSystem()
{
    displayHeader("���������� � �������");

    Config::setColor(Config::COLOR_HEADER);
    std::cout << "=== GAMEHUB - ������� ����� ������ ===" << std::endl;
    Config::resetColor();

    std::cout << "������: 1.0" << std::endl;
    std::cout << "�����������: �������� �.�." << std::endl;
    std::cout << "���: 2025" << std::endl;
    std::cout << std::endl;

    Config::setColor(Config::COLOR_HEADER);
    std::cout << "�������� �����������:" << std::endl;
    Config::resetColor();

    std::cout << " ���������� �������������� (��������������/����������)" << std::endl;
    std::cout << " ������� ������������ ��� � ������� ���������" << std::endl;
    std::cout << " ������� ������ � ����������" << std::endl;
    std::cout << " ������ ��������� ��� �����������" << std::endl;
    std::cout << " ������� ������ � �����" << std::endl;
    std::cout << " ���������������� �������" << std::endl;
    std::cout << " ������ � ���������" << std::endl;
    std::cout << " ������� �����������" << std::endl;
    std::cout << std::endl;

    Config::setColor(Config::COLOR_HEADER);
    std::cout << "����������� ��������������:" << std::endl;
    Config::resetColor();

    std::cout << "����: C++" << std::endl;
    std::cout << "�����������: ������������ (Data Layer, Business Logic, Presentation)" << std::endl;
    std::cout << "�������� ������: �������� �����" << std::endl;
    std::cout << "������������: ����������� ������� � �����" << std::endl;
    std::cout << "���������: ���������� ���������� � �������� �����������" << std::endl;
    std::cout << std::endl;

    FileManager fileManager;
    auto users = fileManager.loadFromFile<std::shared_ptr<User>>(Config::USERS_FILE);
    auto products = fileManager.loadFromFile<std::shared_ptr<Product>>(Config::PRODUCTS_FILE);
    auto sales = fileManager.loadFromFile<Sale>(Config::SALES_FILE);

    Config::setColor(Config::COLOR_HEADER);
    std::cout << "���������� �������:" << std::endl;
    Config::resetColor();

    std::cout << "���������������� �������������: " << users.size() << std::endl;

    int adminCount = 0;
    int customerCount = 0;
    int approvedCustomers = 0;
    for (const auto& user : users) 
    {
        if (user->getRole() == "admin") 
        {
            adminCount++;
        }
        else
        {
            customerCount++;
            if (user->getIsApproved()) 
            {
                approvedCustomers++;
            }
        }
    }

    std::cout << " - ���������������: " << adminCount << std::endl;
    std::cout << " - �����������: " << customerCount << " (��������: " << approvedCustomers << ")" << std::endl;
    std::cout << "������� � ��������: " << products.size() << std::endl;
    std::cout << "���������� ������: " << sales.size() << std::endl;
    std::cout << std::endl;

    Config::setColor(Config::COLOR_SUCCESS);
    std::cout << "������� �������� ���������!" << std::endl;
    Config::resetColor();

    TableFormatter::pause();
}
