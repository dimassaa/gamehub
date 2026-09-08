#include "AdminMenu.h"
#include "FileManager.h"

AdminMenu::AdminMenu(Store& s, Wishlist& w, RecommendationSystem& rs, Report& r, NotificationSystem& ns, std::shared_ptr<User> user)
    : Menu(s, w, rs, r, ns)
{
    currentUser = user;
}

void AdminMenu::show() 
{
    while (true) 
    {
        displayHeader("Главное меню администратора");

        std::vector<std::string> options =
        {
            "Управление пользователями",
            "Управление продуктами",
            "Управление скидками",
            "Просмотр отчётов",
            "Просмотр журнала событий",
            "Изменить мои данные",
            "Уведомления"
        };

        TableFormatter::displayMenu(options, "Выберите действие");

        int choice = InputValidation::getValidatedNumber("Введите выбор: ", 0, 7);

        switch (choice)
        {
        case 1:
            manageUsers();
            break;
        case 2:
            manageProducts();
            break;
        case 3:
            manageDiscounts();
            break;
        case 4:
            viewReports();
            break;
        case 5:
            viewLogs();
            break;
        case 6:
            changeMyProfile();
            break;
        case 7:
            viewNotifications();
            break;
        case 0:
            return;
        }
    }
}

std::vector<std::shared_ptr<User>> AdminMenu::filterUsers(const std::vector<std::shared_ptr<User>>& users, const std::string& searchType, const std::string& searchValue) 
{
    std::vector<std::shared_ptr<User>> filteredUsers;

    if (searchValue.empty())
    {
        return users;
    }

    std::string lowerSearchValue = searchValue;
    std::transform(lowerSearchValue.begin(), lowerSearchValue.end(), lowerSearchValue.begin(), ::tolower);

    for (const auto& user : users) 
    {
        bool matches = false;

        if (searchType == "all") 
        {
            std::string username = user->getUsername();
            std::string email = user->getEmail();
            std::string phone = user->getPhone();
            std::string role = user->getRole();

            std::transform(username.begin(), username.end(), username.begin(), ::tolower);
            std::transform(email.begin(), email.end(), email.begin(), ::tolower);
            std::transform(phone.begin(), phone.end(), phone.begin(), ::tolower);
            std::transform(role.begin(), role.end(), role.begin(), ::tolower);

            matches = username.find(lowerSearchValue) != std::string::npos ||
                email.find(lowerSearchValue) != std::string::npos ||
                phone.find(lowerSearchValue) != std::string::npos ||
                role.find(lowerSearchValue) != std::string::npos;
        }
        else if (searchType == "username")
        {
            std::string username = user->getUsername();
            std::transform(username.begin(), username.end(), username.begin(), ::tolower);
            matches = username.find(lowerSearchValue) != std::string::npos;
        }
        else if (searchType == "email") 
        {
            std::string email = user->getEmail();
            std::transform(email.begin(), email.end(), email.begin(), ::tolower);
            matches = email.find(lowerSearchValue) != std::string::npos;
        }
        else if (searchType == "phone")
        {
            std::string phone = user->getPhone();
            std::transform(phone.begin(), phone.end(), phone.begin(), ::tolower);
            matches = phone.find(lowerSearchValue) != std::string::npos;
        }
        else if (searchType == "role") 
        {
            std::string role = user->getRole();
            std::transform(role.begin(), role.end(), role.begin(), ::tolower);
            matches = role.find(lowerSearchValue) != std::string::npos;
        }
        else if (searchType == "status") 
        {
            std::string status;
            if (user->isBlocked()) 
            {
                status = "заблокирован";
            }
            else if (user->getIsApproved())
            {
                status = "одобрен";
            }
            else 
            {
                status = "ожидает";
            }
            std::transform(status.begin(), status.end(), status.begin(), ::tolower);
            matches = status.find(lowerSearchValue) != std::string::npos;
        }

        if (matches) 
        {
            filteredUsers.push_back(user);
        }
    }

    return filteredUsers;
}

void AdminMenu::searchAndFilterUsers()
{
    FileManager fileManager;
    auto allUsers = fileManager.loadFromFile<std::shared_ptr<User>>(Config::USERS_FILE);

    std::string searchType = "all";
    std::string searchValue = "";
    PaginationState searchPagination;
    searchPagination.perPage = 5;

    while (true) 
    {
        displayHeader("Поиск и фильтрация пользователей");

        auto filteredUsers = filterUsers(allUsers, searchType, searchValue);
        int totalPages = searchPagination.getTotalPages(filteredUsers.size());

        searchPagination.clamp(filteredUsers.size());

        std::cout << "Текущий фильтр: ";
        if (searchValue.empty()) 
        {
            std::cout << "не задан" << std::endl;
        }
        else 
        {
            std::cout << searchType << " = '" << searchValue << "'" << std::endl;
        }
        std::cout << "Найдено пользователей: " << filteredUsers.size() << std::endl;

        int blockedCount = 0;
        for (const auto& user : filteredUsers) 
        {
            if (user->isBlocked())
            {
                blockedCount++;
            }
        }
        if (blockedCount > 0) 
        {
            Config::setColor(Config::COLOR_WARNING);
            std::cout << "Заблокированных пользователей: " << blockedCount << std::endl;
            Config::resetColor();
        }

        std::cout << std::endl;

        TableFormatter::displayUsersTableWithPagination(filteredUsers, searchPagination.currentPage, totalPages, filteredUsers.size());

        std::vector<std::string> options = 
        {
            "Выбрать тип поиска",
            "Введите значение для поиска",
            "Следующая страница",
            "Предыдущая страница",
            "Перейти к странице",
            "Показать заблокированных",
            "Сбросить поиск"
        };

        TableFormatter::displayMenu(options, "Поиск и фильтрация");

        int choice = InputValidation::getValidatedNumber("Введите выбор: ", 0, 7);

        switch (choice) 
        {
        case 1: 
        {
            std::vector<std::string> searchOptions =
            {
                "По всем",
                "По имени пользователя",
                "Email",
                "Телефону",
                "Роли",
                "Статусу"
            };
            TableFormatter::displayMenu(searchOptions, "Выберите тип поиска");
            int searchChoice = InputValidation::getValidatedNumber("Выберите тип поиска: ", 1, 6);
            switch (searchChoice)
            {
            case 1: searchType = "all"; break;
            case 2: searchType = "username"; break;
            case 3: searchType = "email"; break;
            case 4: searchType = "phone"; break;
            case 5: searchType = "role"; break;
            case 6: searchType = "status"; break;
            }
            searchPagination.currentPage = 1;
            break;
        }
        case 2: 
        {
            searchValue = InputValidation::getValidatedString("Введите значение для поиска: ", 0, 100);
            searchPagination.currentPage = 1;
            break;
        }
        case 3:
        {
            size_t before = searchPagination.currentPage;
            searchPagination.nextPage(filteredUsers.size());
            if (searchPagination.currentPage == before)
            {
                TableFormatter::displayWarningMessage("Вы уже на последней странице!");
                TableFormatter::pause();
            }
            break;
        }
        case 4:
        {
            size_t before = searchPagination.currentPage;
            searchPagination.prevPage();
            if (searchPagination.currentPage == before)
            {
                TableFormatter::displayWarningMessage("Вы уже на первой странице!");
                TableFormatter::pause();
            }
            break;
        }
        case 5: 
        {
            int newPage = InputValidation::getValidatedNumber("Введите номер страницы: ", 1, totalPages);
            if (!searchPagination.goToPage(newPage, filteredUsers.size()))
            {
                TableFormatter::displayWarningMessage("Такой страницы не существует!");
                TableFormatter::pause();
            }
            break;
        }
        case 6: 
            searchType = "status";
            searchValue = "заблокирован";
            searchPagination.currentPage = 1;
            break;
        case 7:
            searchType = "all";
            searchValue = "";
            searchPagination.currentPage = 1;
            break;
        case 0:
            return;
        }
    }
}


void AdminMenu::manageUsers()
{
    FileManager fileManager;
    auto allUsers = fileManager.loadFromFile<std::shared_ptr<User>>(Config::USERS_FILE);
    pagination.perPage = 5;

    while (true) 
    {
        displayHeader("Управление пользователями");

        int totalPages = pagination.getTotalPages(allUsers.size());

        TableFormatter::displayUsersTableWithPagination(allUsers, pagination.currentPage, totalPages, allUsers.size());

        std::vector<std::string> options = 
        {
            "Одобрить пользователя",
            "Заблокировать пользователя",
            "Разблокировать пользователя",
            "Изменить роль пользователя",
            "Поиск и фильтрация пользователей",
            "Следующая страница",
            "Предыдущая страница",
            "Перейти к странице",
            "Удалить неодобренных пользователей"
        };

        TableFormatter::displayMenu(options, "Управление пользователями");

        int choice = InputValidation::getValidatedNumber("Введите выбор: ", 0, 9);

        switch (choice) 
        {
        case 1:
            approveUser();
            allUsers = fileManager.loadFromFile<std::shared_ptr<User>>(Config::USERS_FILE);
            pagination.clamp(allUsers.size());
            break;
        case 2:
            blockUser();
            allUsers = fileManager.loadFromFile<std::shared_ptr<User>>(Config::USERS_FILE);
            pagination.clamp(allUsers.size());
            break;
        case 3: 
            unblockUser();
            allUsers = fileManager.loadFromFile<std::shared_ptr<User>>(Config::USERS_FILE);
            pagination.clamp(allUsers.size());
            break;
        case 4:
            changeUserRole();
            allUsers = fileManager.loadFromFile<std::shared_ptr<User>>(Config::USERS_FILE);
            pagination.clamp(allUsers.size());
            break;
        case 5:
            searchAndFilterUsers();
            allUsers = fileManager.loadFromFile<std::shared_ptr<User>>(Config::USERS_FILE);
            pagination.clamp(allUsers.size());
            break;
        case 6:
        {
            size_t before = pagination.currentPage;
            pagination.nextPage(allUsers.size());
            if (pagination.currentPage == before)
            {
                TableFormatter::displayWarningMessage("Вы уже на последней странице!");
                TableFormatter::pause();
            }
            break;
        }
        case 7: 
        {
            size_t before = pagination.currentPage;
            pagination.prevPage();
            if (pagination.currentPage == before)
            {
                TableFormatter::displayWarningMessage("Вы уже на первой странице!");
                TableFormatter::pause();
            }
            break;
        }
        case 8: 
        {
            int totalPages = pagination.getTotalPages(allUsers.size());
            int newPage = InputValidation::getValidatedNumber("Введите номер страницы: ", 1, totalPages);
            if (!pagination.goToPage(newPage, allUsers.size()))
            {
                TableFormatter::displayWarningMessage("Такой страницы не существует!");
                TableFormatter::pause();
            }
            break;
        }
        case 9: 
            clearUnapprovedUsers();
            allUsers = fileManager.loadFromFile<std::shared_ptr<User>>(Config::USERS_FILE);
            pagination.clamp(allUsers.size());
            break;
        case 0:
            return;
        }
    }
}

void AdminMenu::clearUnapprovedUsers() 
{
    FileManager fileManager;
    auto users = fileManager.loadFromFile<std::shared_ptr<User>>(Config::USERS_FILE);

    int unapprovedCount = 0;
    for (const auto& user : users)
    {
        if (user->getRole() == "customer" && !user->getIsApproved()) 
        {
            unapprovedCount++;
        }
    }

    if (unapprovedCount == 0) 
    {
        TableFormatter::displayWarningMessage("Нет ожидающих подтверждения пользователей.");
        TableFormatter::pause();
        return;
    }

    if (!confirmAction("Вы уверены, что хотите удалить все " + std::to_string(unapprovedCount) + " ожидающих подтверждения? Это необратимое действие!")) 
    {
        TableFormatter::displayWarningMessage("Операция удаления отменена.");
        TableFormatter::pause();
        return;
    }

    auto updatedUsers = users;
    auto it = std::remove_if(updatedUsers.begin(), updatedUsers.end(),
        [](const std::shared_ptr<User>& user) 
        {
            return user->getRole() == "customer" && !user->getIsApproved();
        });

    if (it != updatedUsers.end()) 
    {
        updatedUsers.erase(it, updatedUsers.end());
        if (fileManager.saveToFile(Config::USERS_FILE, updatedUsers)) 
        {
            TableFormatter::displaySuccessMessage("Удалено " + std::to_string(unapprovedCount) + " ожидающих подтверждения!");
            logger->log(LogLevel::INFO, "Cleared " + std::to_string(unapprovedCount) + " unapproved users");
        }
    }
}

void AdminMenu::approveUser() 
{
    int userId = InputValidation::getValidatedNumber("Введите ID пользователя для одобрения: ", 1, 1000);

    FileManager fileManager;
    auto users = fileManager.loadFromFile<std::shared_ptr<User>>(Config::USERS_FILE);

    for (auto& user : users) 
    {
        if (user->getId() == userId && user->getRole() == "customer") 
        {
            if (!confirmAction("Вы уверены, что хотите одобрить '" + user->getUsername() + "'?")) 
            {
                TableFormatter::displayWarningMessage("Одобрение отменено.");
                TableFormatter::pause();
                return;
            }

            user->setIsApproved(true);
            if (fileManager.saveToFile(Config::USERS_FILE, users))
            {
                TableFormatter::displaySuccessMessage("Пользователь одобрен!");
                logger->log(LogLevel::INFO, "User approved: " + user->getUsername());
            }
            TableFormatter::pause();
            return;
        }
    }
    TableFormatter::displayErrorMessage("Пользователь не найден!");
    TableFormatter::pause();
}

void AdminMenu::blockUser()
{
    int userId = InputValidation::getValidatedNumber("Введите ID пользователя для блокировки: ", 1, 1000);
    int days = InputValidation::getValidatedNumber("Введите количество дней блокировки: ", 1, 365);

    FileManager fileManager;
    auto users = fileManager.loadFromFile<std::shared_ptr<User>>(Config::USERS_FILE);

    for (auto& user : users) 
    {
        if (user->getId() == userId) 
        {
            if (user->getRole() == "admin") 
            {
                int adminCount = 0;
                for (const auto& u : users) 
                {
                    if (u->getRole() == "admin") adminCount++;
                }
                if (adminCount <= 1) 
                {
                    TableFormatter::displayErrorMessage("Нельзя заблокировать последнего администратора!");
                    TableFormatter::pause();
                    return;
                }
            }

            if (!confirmAction("Вы уверены, что хотите заблокировать '" + user->getUsername() + "' на " + std::to_string(days) + " дней?")) 
            {
                TableFormatter::displayWarningMessage("Блокировка отменена.");
                TableFormatter::pause();
                return;
            }

            auto now = std::chrono::system_clock::now();
            auto unlockTime = now + std::chrono::hours(24 * days);
            std::time_t unlockTime_t = std::chrono::system_clock::to_time_t(unlockTime);
            std::tm tm;
            localtime_s(&tm, &unlockTime_t);

            char buffer[80];
            std::strftime(buffer, sizeof(buffer), Config::DATE_FORMAT.c_str(), &tm);
            user->setUnlockDate(buffer);

            if (fileManager.saveToFile(Config::USERS_FILE, users)) 
            {
                TableFormatter::displaySuccessMessage("Пользователь заблокирован до: " + std::string(buffer));
                TableFormatter::pause();
                logger->log(LogLevel::INFO, "User blocked: " + user->getUsername());
            }
            return;
        }
    }
    TableFormatter::displayErrorMessage("Пользователь не найден!");
    TableFormatter::pause();
}

void AdminMenu::unblockUser() 
{
    int userId = InputValidation::getValidatedNumber("Введите ID пользователя для разблокировки: ", 1, 1000);

    FileManager fileManager;
    auto users = fileManager.loadFromFile<std::shared_ptr<User>>(Config::USERS_FILE);

    for (auto& user : users) 
    {
        if (user->getId() == userId) 
        {
            if (!user->isBlocked()) 
            {
                TableFormatter::displayWarningMessage("Пользователь не заблокирован!");
                TableFormatter::pause();
                return;
            }

            if (!confirmAction("Вы уверены, что хотите разблокировать '" + user->getUsername() + "'?")) 
            {
                TableFormatter::displayWarningMessage("Разблокировка отменена.");
                TableFormatter::pause();
                return;
            }

            user->setUnlockDate("");

            if (fileManager.saveToFile(Config::USERS_FILE, users)) 
            {
                TableFormatter::displaySuccessMessage("Пользователь успешно разблокирован!");
                TableFormatter::pause();
                logger->log(LogLevel::INFO, "User unblocked: " + user->getUsername());
            }
            return;
        }
    }
    TableFormatter::displayErrorMessage("Пользователь не найден!");
}

void AdminMenu::changeUserRole() 
{
    int userId = InputValidation::getValidatedNumber("Введите ID пользователя (0 для отмены): ", 1, 1000);
    if (userId == 0)
    {
        TableFormatter::displayWarningMessage("Операция отменена.");
        TableFormatter::pause();
        return;
    }

    std::vector<std::string> roleOptions = { "admin", "customer" };
    TableFormatter::displayMenu(roleOptions, "Выберите роль");

    int roleChoice = InputValidation::getValidatedNumber("Выберите роль (0 для отмены): ", 0, 2);
    if (roleChoice == 0) {
        TableFormatter::displayWarningMessage("Операция отменена.");
        TableFormatter::pause();
        return;
    }
    std::string newRole = (roleChoice == 1) ? "admin" : "customer";

    FileManager fileManager;
    auto users = fileManager.loadFromFile<std::shared_ptr<User>>(Config::USERS_FILE);

    for (auto& user : users) 
    {
        if (user->getId() == userId) 
        {
            if (user->getRole() == "admin" && newRole == "customer") 
            {
                int adminCount = 0;
                for (const auto& u : users) 
                {
                    if (u->getRole() == "admin") adminCount++;
                }
                if (adminCount <= 1) 
                {
                    TableFormatter::displayErrorMessage("Нельзя понизить последнего администратора!");
                    TableFormatter::pause();
                    return;
                }
            }

            if (!confirmAction("Вы уверены, что хотите изменить роль '" + user->getUsername() + "' с '" + user->getRole() + "' на '" + newRole + "'?")) {
                TableFormatter::displayWarningMessage("Смена роли отменена.");
                TableFormatter::pause();
                return;
            }

            user->setRole(newRole);
            if (newRole == "admin")
            {
                user->setIsApproved(true);
            }

            if (fileManager.saveToFile(Config::USERS_FILE, users)) 
            {
                TableFormatter::displaySuccessMessage("Роль пользователя изменена на: " + newRole);
                TableFormatter::pause();
                logger->log(LogLevel::INFO, "User role changed: " + user->getUsername() + " -> " + newRole);
            }
            return;
        }
    }
    TableFormatter::displayErrorMessage("Пользователь не найден!");
    TableFormatter::pause();
}

void AdminMenu::manageProducts() 
{
    pagination.perPage = 5;

    while (true)
    {
        displayHeader("Управление продуктами");

        auto allProducts = store.getAllProducts();
        int totalPages = pagination.getTotalPages(allProducts.size());

        TableFormatter::displayProductsTableWithPagination(allProducts, pagination.currentPage, totalPages, allProducts.size());

        std::vector<std::string> options = 
        {
            "Добавить новый продукт",
            "Редактировать продукт",
            "Удалить продукт",
            "Добавить ключи к продукту",
            "Добавить новый жанр",
            "Удалить жанр",
            "Добавить новый тег",
            "Удалить тег",
            "Поиск и фильтрация продуктов",
            "Следующая страница",
            "Предыдущая страница",
            "Перейти к странице"
        };

        TableFormatter::displayMenu(options, "Управление продуктами");

        int choice = InputValidation::getValidatedNumber("Введите выбор: ", 0, 12);

        switch (choice) 
        {
        case 1:
            addNewProduct();
            break;
        case 2:
            editProduct();
            break;
        case 3:
            deleteProduct();
            break;
        case 4:
            addKeysToProduct();
            break;
        case 5:
            addNewGenre();
            break;
        case 6:
            deleteGenre();
            break;
        case 7:
            addNewTag();
            break;
        case 8:
            deleteTag();
            break;
        case 9:
            searchAndFilterProducts();
            break;
        case 10:
        {
            size_t before = pagination.currentPage;
            pagination.nextPage(allProducts.size());
            if (pagination.currentPage == before)
            {
                TableFormatter::displayWarningMessage("Вы уже на последней странице!");
            }
            TableFormatter::pause();
            break;
        }
        case 11:
        {
            size_t before = pagination.currentPage;
            pagination.prevPage();
            if (pagination.currentPage == before)
            {
                TableFormatter::displayWarningMessage("Вы уже на первой странице!");
            }
            TableFormatter::pause();
            break;
        }
        case 12:
        {
            int totalPages = pagination.getTotalPages(allProducts.size());
            if (totalPages > 1) 
            {
                int newPage = InputValidation::getValidatedNumber("Введите номер страницы: ", 1, totalPages);
                pagination.goToPage(newPage, allProducts.size());
            }
            else 
            {
                TableFormatter::displayWarningMessage("Такой страницы не существует!");
                TableFormatter::pause();
            }
            break;
        }
        case 0:
            return;
        }
    }
}

void AdminMenu::addNewProduct() 
{
    displayHeader("Добавление нового продукта");

    if (!confirmAction("Вы уверены, что хотите добавить новый продукт?")) 
    {
        TableFormatter::displayWarningMessage("Добавление отменено.");
        TableFormatter::pause();
        return;
    }

    FileManager fileManager;
    auto genres = fileManager.loadStringList(Config::GENRES_FILE);
    auto tags = fileManager.loadStringList(Config::TAGS_FILE);

    std::string title = InputValidation::getValidatedString("Введите название: ", 1, 100);
    double price = InputValidation::getValidatedDouble("Цена: ", 0.0, 10000.0);

    auto developers = store.getUniqueDevelopers();
    std::cout << "Существующие разработчики: ";
    for (size_t i = 0; i < developers.size(); ++i)
    {
        std::cout << developers[i];
        if (i < developers.size() - 1) std::cout << ", ";
    }
    std::cout << std::endl;
    std::string developer = InputValidation::getValidatedString("Разработчик: ", 1, 50);

    if (!genres.empty()) 
    {
        std::cout << "Доступные жанры: ";
        for (size_t i = 0; i < genres.size(); ++i) 
        {
            std::cout << genres[i];
            if (i < genres.size() - 1) std::cout << ", ";
        }
        std::cout << std::endl;
    }
    std::string genre = InputValidation::getValidatedString("Жанр: ", 1, 30);

    std::string publisher = InputValidation::getValidatedString("Издатель: ", 1, 50);
    int ageRating = InputValidation::getValidatedNumber("Возрастной рейтинг: ", 0, 21);
    std::string description = InputValidation::getValidatedString("Описание: ", 1, 500);
    double averageRating = InputValidation::getValidatedDouble("Рейтинг (0.0-5.0): ", 0.0, 5.0);
    std::string releaseDate = InputValidation::getValidatedDate("Дата релиза (дд.мм.гггг чч:мм:сс): ");
    int initialKeys = InputValidation::getValidatedNumber("Начальное количество ключей: ", 0, 10000);

    auto newProduct = std::make_shared<Game>(0, title, price, developer, publisher,
        genre, ageRating, description, releaseDate);
    newProduct->setActivationKeys(initialKeys);

    if (!tags.empty())
    {
        std::cout << "Доступные теги: ";
        for (size_t i = 0; i < tags.size(); ++i) 
        {
            std::cout << tags[i];
            if (i < tags.size() - 1) std::cout << ", ";
        }
        std::cout << std::endl;

        std::cout << "Введите список тегов через запятую (или оставьте пустым): ";
        std::string tagsInput;
        std::getline(std::cin, tagsInput);

        if (!tagsInput.empty()) 
        {
            std::istringstream iss(tagsInput);
            std::string tag;
            while (std::getline(iss, tag, ',')) 
            {
                tag.erase(0, tag.find_first_not_of(' '));
                tag.erase(tag.find_last_not_of(' ') + 1);
                if (!tag.empty())
                {
                    newProduct->addTag(tag);
                }
            }
        }
    }

    store.addProduct(newProduct);
    TableFormatter::displaySuccessMessage("Продукт успешно добавлен!");
    TableFormatter::pause();
}

void AdminMenu::editProduct()
{
    displayHeader("Редактирование продукта");
    auto allProducts = store.getAllProducts();
    int totalPages = (allProducts.size() + 5 - 1) / 5; 
    TableFormatter::displayProductsTableWithPagination(allProducts, 1, totalPages, allProducts.size());

    int productId = InputValidation::getValidatedNumber("Введите ID продукта для редактирования (0 для отмены): ", 0, 10000);
    if (productId == 0) 
    {
        TableFormatter::displayWarningMessage("Редактирование отменено.");
        TableFormatter::pause();
        return;
    }

    auto product = store.getProductById(productId);

    if (!product) 
    {
        TableFormatter::displayErrorMessage("Продукт с таким ID не найден!");
        TableFormatter::pause();
        return;
    }

    TableFormatter::displayProductDetails(product);
    std::cout << std::endl;

    if (!confirmAction("Вы уверены, что хотите изменить этот продукт?")) 
    {
        TableFormatter::displayWarningMessage("Редактирование отменено.");
        TableFormatter::pause();
        return;
    }

    FileManager fileManager;
    auto genres = fileManager.loadStringList(Config::GENRES_FILE);
    auto tags = fileManager.loadStringList(Config::TAGS_FILE);

    std::string newTitle = InputValidation::getValidatedString("Новое название (оставьте пустым для сохранения текущего): ", 0, 100);
    if (!newTitle.empty()) product->setTitle(newTitle);

    double newPrice = InputValidation::getValidatedDouble("Новая цена (введите -1 для сохранения текущей): ", -1, 10000.0);
    if (newPrice >= 0) product->setBasePrice(newPrice);

    auto developers = store.getUniqueDevelopers();
    std::cout << "Существующие разработчики: ";
    for (size_t i = 0; i < developers.size(); ++i) 
    {
        std::cout << developers[i];
        if (i < developers.size() - 1) std::cout << ", ";
    }
    std::cout << std::endl;
    std::string newDeveloper = InputValidation::getValidatedString("Новый разработчик (оставьте пустым для сохранения текущего): ", 0, 50);
    if (!newDeveloper.empty()) product->setDeveloper(newDeveloper);

    if (!genres.empty()) 
    {
        std::cout << "Доступные жанры: ";
        for (size_t i = 0; i < genres.size(); ++i) 
        {
            std::cout << genres[i];
            if (i < genres.size() - 1) std::cout << ", ";
        }
        std::cout << std::endl;
    }
    std::string newGenre = InputValidation::getValidatedString("Новый жанр (оставьте пустым для сохранения текущего): ", 0, 30);
    if (!newGenre.empty()) product->setGenre(newGenre);

    std::string newPublisher = InputValidation::getValidatedString("Новый издатель (оставьте пустым для сохранения текущего): ", 0, 50);
    if (!newPublisher.empty()) product->setPublisher(newPublisher);

    int newAgeRating = InputValidation::getValidatedNumber("Новый возрастной рейтинг (введите -1 для сохранения текущего): ", -1, 21);
    if (newAgeRating >= 0) product->setAgeRating(newAgeRating);

    std::string newDescription = InputValidation::getValidatedString("Новое описание (оставьте пустым для сохранения текущего): ", 0, 500);
    if (!newDescription.empty()) product->setDescription(newDescription);

    if (store.updateProduct(productId, product)) 
    {
        TableFormatter::displaySuccessMessage("Продукт успешно обновлён!");
    }
    else 
    {
        TableFormatter::displayErrorMessage("Ошибка при обновлении продукта!");
    }
    TableFormatter::pause();
}

void AdminMenu::deleteProduct()
{
    displayHeader("Удаление продукта");
    auto allProducts = store.getAllProducts();
    int totalPages = (allProducts.size() + 5 - 1) / 5; 
    TableFormatter::displayProductsTableWithPagination(allProducts, 1, totalPages, allProducts.size());

    int productId = InputValidation::getValidatedNumber("Введите ID продукта для удаления (0 для отмены): ", 0, 10000);
    if (productId == 0) 
    {
        TableFormatter::displayWarningMessage("Удаление отменено.");
        TableFormatter::pause();
        return;
    }

    auto product = store.getProductById(productId);
    if (product) 
    {
        TableFormatter::displayProductDetails(product);
        std::cout << std::endl;

        if (confirmAction("Вы уверены, что хотите удалить этот продукт?"))
        {
            if (store.deleteProduct(productId)) 
            {
                TableFormatter::displaySuccessMessage("Продукт успешно удалён!");
            }
            else 
            {
                TableFormatter::displayErrorMessage("Ошибка при удалении продукта!");
            }
        }
        else 
        {
            TableFormatter::displayWarningMessage("Удаление отменено.");
        }
    }
    else 
    {
        TableFormatter::displayErrorMessage("Продукт с таким ID не найден!");
    }
    TableFormatter::pause();
}

void AdminMenu::addKeysToProduct() 
{
    displayHeader("Добавление ключей");
    auto allProducts = store.getAllProducts();
    int totalPages = (allProducts.size() + 5 - 1) / 5; 
    TableFormatter::displayProductsTableWithPagination(allProducts, 1, totalPages, allProducts.size());

    int productId = InputValidation::getValidatedNumber("Введите ID продукта (0 для отмены): ", 0, 10000);
    if (productId == 0) 
    {
        TableFormatter::displayWarningMessage("Операция отменена.");
        TableFormatter::pause();
        return;
    }

    auto product = store.getProductById(productId);

    if (!product) 
    {
        TableFormatter::displayErrorMessage("Продукт с таким ID не найден!");
        TableFormatter::pause();
        return;
    }

    TableFormatter::displayProductDetails(product);
    std::cout << std::endl;

    int oldKeyCount = product->getActivationKeys();

    int keysToAdd = InputValidation::getValidatedNumber("Сколько ключей добавить? (0 для отмены): ", 0, 10000);
    if (keysToAdd == 0) 
    {
        TableFormatter::displayWarningMessage("Операция отменена.");
        TableFormatter::pause();
        return;
    }

    if (store.addKeys(productId, keysToAdd)) 
    {
        TableFormatter::displaySuccessMessage("Ключи успешно добавлены!");

        auto updatedProduct = store.getProductById(productId);
        if (updatedProduct) 
        {
            if (updatedProduct->getActivationKeys() < 5)
            {
                notificationSystem.notifyLowStock(productId, updatedProduct->getTitle());
            }

            if (oldKeyCount == 0 && updatedProduct->getActivationKeys() > 0) 
            {
                auto wishlistProducts = wishlist.getWishlistProductsForAllUsers();

                for (const auto& wishlistItem : wishlistProducts) 
                {
                    if (wishlistItem.productId == productId) 
                    {
                        notificationSystem.notifyWishlistBackInStock(
                            wishlistItem.customerId,
                            updatedProduct->getTitle()
                        );

                        logger->log(LogLevel::INFO,
                            "Back in stock notification sent to customer " +
                            std::to_string(wishlistItem.customerId) +
                            " for product: " + updatedProduct->getTitle()
                        );
                    }
                }
            }
        }
    }
    else 
    {
        TableFormatter::displayErrorMessage("Ошибка при добавлении ключей!");
    }
    TableFormatter::pause();
}
void AdminMenu::addNewGenre() 
{
    displayHeader("Добавление нового жанра");

    FileManager fileManager;
    auto genres = fileManager.loadStringList(Config::GENRES_FILE);

    if (!genres.empty()) 
    {
        std::cout << "Список жанров: ";
        for (size_t i = 0; i < genres.size(); ++i) 
        {
            std::cout << genres[i];
            if (i < genres.size() - 1) std::cout << ", ";
        }
        std::cout << std::endl;
    }

    std::string newGenre = InputValidation::getValidatedString("Введите название нового жанра (0 для отмены): ", 0, 30);
    if (newGenre == "0") 
    {
        TableFormatter::displayWarningMessage("Добавление жанра отменено.");
        TableFormatter::pause();
        return;
    }

    if (std::find(genres.begin(), genres.end(), newGenre) != genres.end()) {
        TableFormatter::displayWarningMessage("Такой жанр уже существует!");
    }
    else 
    {
        genres.push_back(newGenre);
        if (fileManager.saveStringList(Config::GENRES_FILE, genres)) 
        {
            TableFormatter::displaySuccessMessage("Жанр успешно добавлен!");
        }
        else 
        {
            TableFormatter::displayErrorMessage("Ошибка при сохранении жанра!");
        }
    }
    TableFormatter::pause();
}

void AdminMenu::deleteGenre()
{
    displayHeader("Удаление жанра");

    FileManager fileManager;
    auto genres = fileManager.loadStringList(Config::GENRES_FILE);

    if (genres.empty()) 
    {
        TableFormatter::displayWarningMessage("Нет жанров для удаления.");
        TableFormatter::pause();
        return;
    }

    std::cout << "Список жанров: ";
    for (size_t i = 0; i < genres.size(); ++i) 
    {
        std::cout << genres[i];
        if (i < genres.size() - 1) std::cout << ", ";
    }
    std::cout << std::endl;

    std::string genreToDelete = InputValidation::getValidatedString("Введите жанр для удаления (0 для отмены): ", 0, 30);
    if (genreToDelete == "0") 
    {
        TableFormatter::displayWarningMessage("Удаление жанра отменено.");
        TableFormatter::pause();
        return;
    }

    auto it = std::find(genres.begin(), genres.end(), genreToDelete);
    if (it == genres.end()) 
    {
        TableFormatter::displayErrorMessage("Такой жанр не найден!");
    }
    else
    {
        if (confirmAction("Вы уверены, что хотите удалить жанр '" + genreToDelete + "'?")) 
        {
            genres.erase(it);
            if (fileManager.saveStringList(Config::GENRES_FILE, genres)) 
            {
                TableFormatter::displaySuccessMessage("Жанр успешно удалён!");
            }
            else 
            {
                TableFormatter::displayErrorMessage("Ошибка при сохранении списка жанров!");
            }
        }
        else 
        {
            TableFormatter::displayWarningMessage("Удаление жанра отменено.");
        }
    }
    TableFormatter::pause();
}

void AdminMenu::addNewTag() 
{
    displayHeader("Добавление нового тега");

    FileManager fileManager;
    auto tags = fileManager.loadStringList(Config::TAGS_FILE);

    if (!tags.empty()) 
    {
        std::cout << "Список тегов: ";
        for (size_t i = 0; i < tags.size(); ++i)
        {
            std::cout << tags[i];
            if (i < tags.size() - 1) std::cout << ", ";
        }
        std::cout << std::endl;
    }

    std::string newTag = InputValidation::getValidatedString("Введите название нового тега (0 для отмены): ", 0, 30);
    if (newTag == "0") 
    {
        TableFormatter::displayWarningMessage("Добавление тега отменено.");
        TableFormatter::pause();
        return;
    }

    if (std::find(tags.begin(), tags.end(), newTag) != tags.end()) 
    {
        TableFormatter::displayWarningMessage("Такой тег уже существует!");
    }
    else 
    {
        tags.push_back(newTag);
        if (fileManager.saveStringList(Config::TAGS_FILE, tags)) 
        {
            TableFormatter::displaySuccessMessage("Тег успешно добавлен!");
        }
        else 
        {
            TableFormatter::displayErrorMessage("Ошибка при сохранении тега!");
        }
    }
    TableFormatter::pause();
}

void AdminMenu::deleteTag()
{
    displayHeader("Удаление тега");

    FileManager fileManager;
    auto tags = fileManager.loadStringList(Config::TAGS_FILE);

    if (tags.empty()) 
    {
        TableFormatter::displayWarningMessage("Нет тегов для удаления.");
        TableFormatter::pause();
        return;
    }

    std::cout << "Список тегов: ";
    for (size_t i = 0; i < tags.size(); ++i)
    {
        std::cout << tags[i];
        if (i < tags.size() - 1) std::cout << ", ";
    }
    std::cout << std::endl;

    std::string tagToDelete = InputValidation::getValidatedString("Введите тег для удаления (0 для отмены): ", 0, 30);
    if (tagToDelete == "0")
    {
        TableFormatter::displayWarningMessage("Удаление тега отменено.");
        TableFormatter::pause();
        return;
    }

    auto it = std::find(tags.begin(), tags.end(), tagToDelete);
    if (it == tags.end()) 
    {
        TableFormatter::displayErrorMessage("Такой тег не найден!");
    }
    else 
    {
        if (confirmAction("Вы уверены, что хотите удалить тег '" + tagToDelete + "'?")) 
        {
            tags.erase(it);
            if (fileManager.saveStringList(Config::TAGS_FILE, tags)) 
            {
                TableFormatter::displaySuccessMessage("Тег успешно удалён!");
            }
            else 
            {
                TableFormatter::displayErrorMessage("Ошибка при сохранении списка тегов!");
            }
        }
        else 
        {
            TableFormatter::displayWarningMessage("Удаление тега отменено.");
        }
    }
    TableFormatter::pause();
}

std::vector<std::shared_ptr<Product>> AdminMenu::filterProducts(const std::vector<std::shared_ptr<Product>>& products, const std::string& searchType, const std::string& searchValue) 
{
    std::vector<std::shared_ptr<Product>> filteredProducts;

    if (searchValue.empty()) 
    {
        return products;
    }

    std::string lowerSearchValue = searchValue;
    std::transform(lowerSearchValue.begin(), lowerSearchValue.end(), lowerSearchValue.begin(), ::tolower);

    for (const auto& product : products) 
    {
        bool matches = false;

        if (searchType == "all") 
        {
            std::string title = product->getTitle();
            std::string developer = product->getDeveloper();
            std::string genre = product->getGenre();
            std::string publisher = product->getPublisher();

            std::transform(title.begin(), title.end(), title.begin(), ::tolower);
            std::transform(developer.begin(), developer.end(), developer.begin(), ::tolower);
            std::transform(genre.begin(), genre.end(), genre.begin(), ::tolower);
            std::transform(publisher.begin(), publisher.end(), publisher.begin(), ::tolower);

            matches = title.find(lowerSearchValue) != std::string::npos ||
                developer.find(lowerSearchValue) != std::string::npos ||
                genre.find(lowerSearchValue) != std::string::npos ||
                publisher.find(lowerSearchValue) != std::string::npos;
        }
        else if (searchType == "title") 
        {
            std::string title = product->getTitle();
            std::transform(title.begin(), title.end(), title.begin(), ::tolower);
            matches = title.find(lowerSearchValue) != std::string::npos;
        }
        else if (searchType == "developer") 
        {
            std::string developer = product->getDeveloper();
            std::transform(developer.begin(), developer.end(), developer.begin(), ::tolower);
            matches = developer.find(lowerSearchValue) != std::string::npos;
        }
        else if (searchType == "genre") 
        {
            std::string genre = product->getGenre();
            std::transform(genre.begin(), genre.end(), genre.begin(), ::tolower);
            matches = genre.find(lowerSearchValue) != std::string::npos;
        }
        else if (searchType == "publisher")
        {
            std::string publisher = product->getPublisher();
            std::transform(publisher.begin(), publisher.end(), publisher.begin(), ::tolower);
            matches = publisher.find(lowerSearchValue) != std::string::npos;
        }
        else if (searchType == "in_stock")
        {
            matches = product->hasKeys();
        }
        else if (searchType == "out_of_stock") 
        {
            matches = !product->hasKeys();
        }

        if (matches) 
        {
            filteredProducts.push_back(product);
        }
    }

    return filteredProducts;
}

void AdminMenu::searchAndFilterProducts() 
{
    displayHeader("Поиск и фильтрация продуктов");

    auto allProducts = store.getAllProducts();
    std::string searchType = "all";
    std::string searchValue = "";
    PaginationState searchPagination;
    searchPagination.perPage = 5;

    while (true) 
    {
        displayHeader("Поиск и фильтрация продуктов");

        auto filteredProducts = filterProducts(allProducts, searchType, searchValue);
        int totalPages = searchPagination.getTotalPages(filteredProducts.size());

        searchPagination.clamp(filteredProducts.size());

        std::cout << "Текущий фильтр: ";
        if (searchValue.empty()) 
        {
            std::cout << "не задан" << std::endl;
        }
        else 
        {
            std::cout << searchType << " = '" << searchValue << "'" << std::endl;
        }
        std::cout << "Найдено продуктов: " << filteredProducts.size() << std::endl;
        std::cout << std::endl;

        TableFormatter::displayProductsTableWithPagination(filteredProducts, searchPagination.currentPage, totalPages, filteredProducts.size());

        std::vector<std::string> options = 
        {
            "Выбрать тип поиска",
            "Введите значение для поиска",
            "Следующая страница",
            "Предыдущая страница",
            "Перейти к странице",
            "Сбросить поиск"
        };

        TableFormatter::displayMenu(options, "Поиск и фильтрация");

        int choice = InputValidation::getValidatedNumber("Введите выбор: ", 0, 6);

        switch (choice) 
        {
        case 1:
        {
            std::vector<std::string> searchOptions = 
            {
                "По всем",
                "По названию",
                "По разработчику",
                "По жанру",
                "По издателю",
                "В наличии",
                "Нет в наличии"
            };
            TableFormatter::displayMenu(searchOptions, "Выберите тип поиска");
            int searchChoice = InputValidation::getValidatedNumber("Выберите тип поиска: ", 0, 7);
            switch (searchChoice) 
            {
            case 1: searchType = "all"; break;
            case 2: searchType = "title"; break;
            case 3: searchType = "developer"; break;
            case 4: searchType = "genre"; break;
            case 5: searchType = "publisher"; break;
            case 6: searchType = "in_stock"; break;
            case 7: searchType = "out_of_stock"; break;
            case 0:
                return;
            }
            searchPagination.currentPage = 1;
            break;
        }
        case 2: 
        {
            if (searchType == "in_stock" || searchType == "out_of_stock")
            {
                TableFormatter::displayWarningMessage("Для этого типа поиска не требуется значение!");
            }
            else 
            {
                searchValue = InputValidation::getValidatedString("Введите значение для поиска: ", 0, 100);
            }
            searchPagination.currentPage = 1;
            break;
        }
        case 3:
        {
            size_t before = searchPagination.currentPage;
            searchPagination.nextPage(filteredProducts.size());
            if (searchPagination.currentPage == before)
            {
                TableFormatter::displayWarningMessage("Вы уже на последней странице!");
                TableFormatter::pause();
            }
            break;
        }
        case 4:
        {
            size_t before = searchPagination.currentPage;
            searchPagination.prevPage();
            if (searchPagination.currentPage == before)
            {
                TableFormatter::displayWarningMessage("Вы уже на первой странице!");
                TableFormatter::pause();
            }
            break;
        }
        case 5: 
        {
            int newPage = InputValidation::getValidatedNumber("Введите номер страницы: ", 1, totalPages);
            if (!searchPagination.goToPage(newPage, filteredProducts.size()))
            {
                TableFormatter::displayWarningMessage("Такой страницы не существует!");
                TableFormatter::pause();
            }
            break;
        }
        case 6:
            searchType = "all";
            searchValue = "";
            searchPagination.currentPage = 1;
            break;
        case 0:
            return;
        }
    }
}

void AdminMenu::manageDiscounts() 
{
    pagination.perPage = 5;

    while (true) 
    {
        displayHeader("Управление скидками");

        auto allDiscounts = store.getAllDiscounts();
        int totalPages = pagination.getTotalPages(allDiscounts.size());

        TableFormatter::displayDiscountsTableWithPagination(allDiscounts, pagination.currentPage, totalPages, allDiscounts.size());

        std::vector<std::string> options = 
        {
            "Добавить новую скидку",
            "Редактировать скидку",
            "Удалить скидку",
            "Просмотреть продукты со скидками",
            "Следующая страница",
            "Предыдущая страница",
            "Перейти к странице"
        };

        TableFormatter::displayMenu(options, "Управление скидками");

        int choice = InputValidation::getValidatedNumber("Введите выбор: ", 0, 7);

        switch (choice) 
        {
        case 1:
            addNewDiscount();
            allDiscounts = store.getAllDiscounts();
            pagination.clamp(allDiscounts.size());
            break;
        case 2:
            editDiscount();
            allDiscounts = store.getAllDiscounts();
            pagination.clamp(allDiscounts.size());
            break;
        case 3:
            deleteDiscount();
            allDiscounts = store.getAllDiscounts();
            pagination.clamp(allDiscounts.size());
            break;
        case 4:
            viewDiscountedProducts();
            break;
        case 5:
        {
            size_t before = pagination.currentPage;
            pagination.nextPage(allDiscounts.size());
            if (pagination.currentPage == before)
            {
                TableFormatter::displayWarningMessage("Вы уже на последней странице!");
            }
            TableFormatter::pause();
            break;
        }
        case 6:
        {
            size_t before = pagination.currentPage;
            pagination.prevPage();
            if (pagination.currentPage == before)
            {
                TableFormatter::displayWarningMessage("Вы уже на первой странице!");
            }
            TableFormatter::pause();
            break;
        }
        case 7:
        {
            int totalPages = pagination.getTotalPages(allDiscounts.size());
            if (totalPages > 1) 
            {
                int newPage = InputValidation::getValidatedNumber("Введите номер страницы: ", 1, totalPages);
                pagination.goToPage(newPage, allDiscounts.size());
            }
            else
            {
                TableFormatter::displayWarningMessage("Такой страницы не существует!");
                TableFormatter::pause();
            }
            break;
        }
        case 0:
            return;
        }
    }
}

void AdminMenu::addNewDiscount() 
{
    displayHeader("Добавление новой скидки");

    if (!confirmAction("Вы уверены, что хотите добавить новую скидку?")) 
    {
        TableFormatter::displayWarningMessage("Добавление отменено.");
        TableFormatter::pause();
        return;
    }

    std::vector<std::string> discountTypes =
    {
        "На продукт (по ID)",
        "На жанр",
        "На тег",
        "На разработчика"
    };

    TableFormatter::displayMenu(discountTypes, "Выберите тип скидки");

    int typeChoice = InputValidation::getValidatedNumber("Выберите тип скидки: ", 0, 4);
    std::string discountType;

    switch (typeChoice) 
    {
    case 1: discountType = "product"; break;
    case 2: discountType = "genre"; break;
    case 3: discountType = "tag"; break;
    case 4: discountType = "developer"; break;
    case 0:
        return;
    }

    std::vector<std::string> targets;

    switch (typeChoice) 
    {
    case 1: selectProductsForDiscount(targets); break;
    case 2: selectGenresForDiscount(targets); break;
    case 3: selectTagsForDiscount(targets); break;
    case 4: selectDevelopersForDiscount(targets); break;
    }

    if (targets.empty()) 
    {
        TableFormatter::displayWarningMessage("Не выбрано ни одной цели для скидки!");
        TableFormatter::pause();
        return;
    }

    double percentage = InputValidation::getValidatedDouble("Введите размер скидки (1-100): ", 1.0, 100.0);

    std::string startDate = InputValidation::getValidatedDate("Введите дату начала скидки (дд.мм.гггг чч:мм:сс): ");
    std::string endDate = InputValidation::getValidatedDate("Введите дату окончания скидки (дд.мм.гггг чч:мм:сс): ");

    if (startDate >= endDate) {
        TableFormatter::displayErrorMessage("Дата окончания должна быть позже даты начала!");
        TableFormatter::pause();
        return;
    }

    Discount newDiscount;
    newDiscount.type = discountType;
    newDiscount.targets = targets;
    newDiscount.percentage = percentage;
    newDiscount.startDate = startDate;
    newDiscount.endDate = endDate;

    TableFormatter::displaySuccessMessage("Детали скидки:");
    std::cout << "Тип: " << discountType << std::endl;
    std::cout << "Цели: ";
    for (size_t i = 0; i < targets.size(); ++i) 
    {
        std::cout << targets[i];
        if (i < targets.size() - 1) std::cout << ", ";
    }
    std::cout << std::endl;
    std::cout << "Размер: " << percentage << "%" << std::endl;
    std::cout << "Период: " << startDate << " - " << endDate << std::endl;

    if (confirmAction("Создать эту скидку?")) 
    {
        store.addDiscount(newDiscount);
        TableFormatter::displaySuccessMessage("Скидка успешно создана!");
        logger->log(LogLevel::INFO, "Discount added: " + discountType + " " + std::to_string(percentage) + "%");
    }
    else 
    {
        TableFormatter::displayWarningMessage("Скидка не была создана.");
    }

    TableFormatter::pause();
}

void AdminMenu::selectProductsForDiscount(std::vector<std::string>& targets) 
{
    displayHeader("Выбор продуктов для скидки");

    auto products = store.getAllProducts();
    TableFormatter::displayProductsTable(products);

    TableFormatter::displaySuccessMessage("Введите ID продуктов для добавления в скидку (0 для завершения):");

    while (true) 
    {
        int productId = InputValidation::getValidatedNumber("ID продукта: ", 0, 10000);

        if (productId == 0) 
        {
            break;
        }

        auto product = store.getProductById(productId);
        if (product) 
        {
            if (std::find(targets.begin(), targets.end(), std::to_string(productId)) == targets.end()) 
            {
                targets.push_back(std::to_string(productId));
                TableFormatter::displaySuccessMessage("Продукт добавлен: " + product->getTitle());
            }
            else 
            {
                TableFormatter::displayWarningMessage("Продукт уже выбран!");
            }
        }
        else 
        {
            TableFormatter::displayErrorMessage("Продукт с таким ID не найден!");
        }

        if (targets.size() >= 10) 
        {
            TableFormatter::displayWarningMessage("Достигнут лимит в 10 продуктов.");
            break;
        }
    }
}

void AdminMenu::selectGenresForDiscount(std::vector<std::string>& targets) 
{
    displayHeader("Выбор жанров для скидки");

    FileManager fileManager;
    auto genres = fileManager.loadStringList(Config::GENRES_FILE);

    if (genres.empty()) 
    {
        TableFormatter::displayWarningMessage("Нет доступных жанров.");
        return;
    }

    for (size_t i = 0; i < genres.size(); ++i) 
    {
        std::cout << i + 1 << ". " << genres[i] << std::endl;
    }

    TableFormatter::displaySuccessMessage("Введите номера жанров для добавления в скидку (0 для завершения):");

    while (true) 
    {
        int genreChoice = InputValidation::getValidatedNumber("Номер жанра: ", 0, genres.size());

        if (genreChoice == 0) 
        {
            break;
        }

        std::string selectedGenre = genres[genreChoice - 1];
        if (std::find(targets.begin(), targets.end(), selectedGenre) == targets.end())
        {
            targets.push_back(selectedGenre);
            TableFormatter::displaySuccessMessage("Жанр добавлен: " + selectedGenre);
        }
        else 
        {
            TableFormatter::displayWarningMessage("Жанр уже выбран!");
        }
    }
}

void AdminMenu::selectTagsForDiscount(std::vector<std::string>& targets) 
{
    displayHeader("Выбор тегов для скидки");

    FileManager fileManager;
    auto tags = fileManager.loadStringList(Config::TAGS_FILE);

    if (tags.empty()) 
    {
        TableFormatter::displayWarningMessage("Нет доступных тегов.");
        return;
    }

    for (size_t i = 0; i < tags.size(); ++i) 
    {
        std::cout << i + 1 << ". " << tags[i] << std::endl;
    }

    TableFormatter::displaySuccessMessage("Введите номера тегов для добавления в скидку (0 для завершения):");

    while (true)
    {
        int tagChoice = InputValidation::getValidatedNumber("Номер тега: ", 0, tags.size());

        if (tagChoice == 0) 
        {
            break;
        }

        std::string selectedTag = tags[tagChoice - 1];
        if (std::find(targets.begin(), targets.end(), selectedTag) == targets.end()) 
        {
            targets.push_back(selectedTag);
            TableFormatter::displaySuccessMessage("Тег добавлен: " + selectedTag);
        }
        else 
        {
            TableFormatter::displayWarningMessage("Тег уже выбран!");
        }
    }
}

void AdminMenu::selectDevelopersForDiscount(std::vector<std::string>& targets)
{
    displayHeader("Выбор разработчиков для скидки");

    auto developers = store.getUniqueDevelopers();

    if (developers.empty()) 
    {
        TableFormatter::displayWarningMessage("Нет доступных разработчиков.");
        return;
    }

    for (size_t i = 0; i < developers.size(); ++i) 
    {
        std::cout << i + 1 << ". " << developers[i] << std::endl;
    }

    TableFormatter::displaySuccessMessage("Введите номера разработчиков для добавления в скидку (0 для завершения):");

    while (true)
    {
        int devChoice = InputValidation::getValidatedNumber("Номер разработчика: ", 0, developers.size());

        if (devChoice == 0) 
        {
            break;
        }

        std::string selectedDev = developers[devChoice - 1];
        if (std::find(targets.begin(), targets.end(), selectedDev) == targets.end()) 
        {
            targets.push_back(selectedDev);
            TableFormatter::displaySuccessMessage("Разработчик добавлен: " + selectedDev);
        }
        else 
        {
            TableFormatter::displayWarningMessage("Разработчик уже выбран!");
        }
    }
}

void AdminMenu::editDiscount() 
{
    displayHeader("Редактирование скидки");

    auto discounts = store.getAllDiscounts();

    if (discounts.empty()) 
    {
        TableFormatter::displayWarningMessage("Нет скидок для редактирования.");
        TableFormatter::pause();
        return;
    }

    TableFormatter::displayDiscountsTable(discounts);

    int discountIndex = InputValidation::getValidatedNumber("Введите номер скидки для редактирования (0 для отмены): ", 0, discounts.size());

    if (discountIndex == 0) 
    {
        TableFormatter::displayWarningMessage("Редактирование отменено.");
        TableFormatter::pause();
        return;
    }

    Discount& discount = discounts[discountIndex - 1];

    TableFormatter::displaySuccessMessage("Текущие данные скидки:");
    std::cout << "Тип: " << discount.type << std::endl;
    std::cout << "Цели: ";
    for (size_t i = 0; i < discount.targets.size(); ++i) 
    {
        std::cout << discount.targets[i];
        if (i < discount.targets.size() - 1) std::cout << ", ";
    }
    std::cout << std::endl;
    std::cout << "Размер: " << discount.percentage << "%" << std::endl;
    std::cout << "Начало: " << discount.startDate << std::endl;
    std::cout << "Конец: " << discount.endDate << std::endl;
    std::cout << std::endl;

    if (!confirmAction("Вы уверены, что хотите изменить эту скидку?")) 
    {
        TableFormatter::displayWarningMessage("Редактирование отменено.");
        TableFormatter::pause();
        return;
    }

    double newPercentage = InputValidation::getValidatedDouble("Новый размер скидки (текущий: " + std::to_string(discount.percentage) + "%, -1 для сохранения): ", -1, 100.0);
    if (newPercentage >= 0)
    {
        discount.percentage = newPercentage;
    }

    std::string newStartDate = InputValidation::getValidatedDate("Новая дата начала (текущая: " + discount.startDate + ", оставьте пустым для сохранения): ");
    if (!newStartDate.empty()) 
    {
        discount.startDate = newStartDate;
    }

    std::string newEndDate = InputValidation::getValidatedDate("Новая дата окончания (текущая: " + discount.endDate + ", оставьте пустым для сохранения): ");
    if (!newEndDate.empty()) 
    {
        discount.endDate = newEndDate;
    }

    if (discount.startDate >= discount.endDate) 
    {
        TableFormatter::displayErrorMessage("Дата окончания должна быть позже даты начала!");
        TableFormatter::pause();
        return;
    }

    store.saveDiscounts();
    TableFormatter::displaySuccessMessage("Скидка успешно обновлена!");
    logger->log(LogLevel::INFO, "Discount edited: " + discount.type + " " + std::to_string(discount.percentage) + "%");
    TableFormatter::pause();
}

void AdminMenu::deleteDiscount() 
{
    displayHeader("Удаление скидки");

    auto discounts = store.getAllDiscounts();

    if (discounts.empty()) 
    {
        TableFormatter::displayWarningMessage("Нет скидок для удаления.");
        TableFormatter::pause();
        return;
    }

    TableFormatter::displayDiscountsTable(discounts);

    int discountIndex = InputValidation::getValidatedNumber("Введите номер скидки для удаления (0 для отмены): ", 0, discounts.size());

    if (discountIndex == 0) 
    {
        TableFormatter::displayWarningMessage("Удаление отменено.");
        TableFormatter::pause();
        return;
    }

    const Discount& discount = discounts[discountIndex - 1];

    TableFormatter::displaySuccessMessage("Информация о скидке для удаления:");
    std::cout << "Тип: " << discount.type << std::endl;
    std::cout << "Цели: ";
    for (size_t i = 0; i < discount.targets.size(); ++i)
    {
        std::cout << discount.targets[i];
        if (i < discount.targets.size() - 1) std::cout << ", ";
    }
    std::cout << std::endl;
    std::cout << "Размер: " << discount.percentage << "%" << std::endl;
    std::cout << "Период: " << discount.startDate << " - " << discount.endDate << std::endl;
    std::cout << std::endl;

    if (!confirmAction("Вы уверены, что хотите удалить эту скидку?")) 
    {
        TableFormatter::displayWarningMessage("Удаление отменено.");
        TableFormatter::pause();
        return;
    }

    auto updatedDiscounts = discounts;
    updatedDiscounts.erase(updatedDiscounts.begin() + discountIndex - 1);

    FileManager fileManager;
    if (fileManager.saveToFile(Config::DISCOUNTS_FILE, updatedDiscounts)) 
    {
        TableFormatter::displaySuccessMessage("Скидка успешно удалена!");
        logger->log(LogLevel::INFO, "Discount deleted: " + discount.type + " " + std::to_string(discount.percentage) + "%");

        store.loadData();
    }
    else 
    {
        TableFormatter::displayErrorMessage("Ошибка при удалении скидки!");
    }

    TableFormatter::pause();
}

void AdminMenu::viewDiscountedProducts() 
{
    displayHeader("Продукты со скидками");

    auto products = store.getAllProducts();
    std::vector<std::shared_ptr<Product>> discountedProducts;
    std::vector<double> discountedPrices;

    for (const auto& product : products)
    {
        double discountedPrice = store.getDiscountedPrice(product);
        if (discountedPrice < product->getBasePrice()) 
        {
            discountedProducts.push_back(product);
            discountedPrices.push_back(discountedPrice);
        }
    }

    if (discountedProducts.empty()) 
    {
        TableFormatter::displayWarningMessage("В магазине нет продуктов со скидками.");
    }
    else 
    {
        TableFormatter::displayDiscountedProducts(discountedProducts, discountedPrices);
    }

    TableFormatter::pause();
}

void AdminMenu::viewReports() 
{
    while (true)
    {
        displayHeader("Просмотр отчётов");

        std::vector<std::string> options = 
        {
            "Отчёт по продажам",
            "Финансовый отчёт",
            "Отчёт по остаткам товаров",
            "Загрузить отчёт из файла"
        };

        TableFormatter::displayReportMenu(options, "Выберите отчёт");

        int choice = InputValidation::getValidatedNumber("Выберите опцию: ", 0, 4);

        switch (choice) 
        {
        case 1: 
        {
            displayHeader("Отчёт по продажам");
            std::string startDate = InputValidation::getValidatedDate("Введите начальную дату (дд.мм.гггг чч:мм:сс): ");
            std::string endDate = InputValidation::getValidatedDate("Введите конечную дату (дд.мм.гггг чч:мм:сс): ");

            if (startDate >= endDate) 
            {
                TableFormatter::displayErrorMessage("Начальная дата должна быть меньше конечной!");
                TableFormatter::pause();
                break;
            }

            auto salesReport = report.generateSalesReport(startDate, endDate);
            TableFormatter::displaySalesReport(salesReport, store);

            if (confirmAction("Сохранить отчёт в файл?")) 
            {
                std::string filename = "sales_report_" + Config::getCurrentDateTime() + ".txt";
                std::replace(filename.begin(), filename.end(), ':', '-'); 
                report.saveReportToFile(salesReport, filename);
                TableFormatter::displaySuccessMessage("Отчёт сохранён в файл: " + filename);
            }

            TableFormatter::pause();
            break;
        }
        case 2:
        {
            displayHeader("Финансовый отчёт");
            std::string startDate = InputValidation::getValidatedDate("Введите начальную дату (дд.мм.гггг чч:мм:сс): ");
            std::string endDate = InputValidation::getValidatedDate("Введите конечную дату (дд.мм.гггг чч:мм:сс): ");

            if (startDate >= endDate) 
            {
                TableFormatter::displayErrorMessage("Начальная дата должна быть меньше конечной!");
                TableFormatter::pause();
                break;
            }

            auto financialReport = report.generateFinancialReport(startDate, endDate);
            TableFormatter::displayFinancialReport(financialReport);

            if (confirmAction("Сохранить отчёт в файл?")) 
            {
                std::string filename = "financial_report_" + Config::getCurrentDateTime() + ".txt";
                std::replace(filename.begin(), filename.end(), ':', '-');
                report.saveReportToFile(financialReport, filename);
                TableFormatter::displaySuccessMessage("Отчёт сохранён в файл: " + filename);
            }

            TableFormatter::pause();
            break;
        }
        case 3:
            report.generateInventoryReport();
            TableFormatter::pause();
            break;
        case 4: 
        {
            displayHeader("Загрузка отчёта из файла");
            std::string filename = InputValidation::getValidatedString("Введите имя файла: ", 1, 100);
            try 
            {
                report.loadReportInConsole(filename);
            }
            catch (const FileIOException& e) 
            {
                TableFormatter::displayErrorMessage(e.what());
            }
            TableFormatter::pause();
            break;
        }
        case 0:
            return;
        }
    }
}

void AdminMenu::viewLogs() 
{
    PaginationState logPagination;
    logPagination.perPage = 10;

    std::ifstream logFile(Config::LOG_FILE);
    std::vector<std::string> logs;
    std::string line;

    if (logFile.is_open()) 
    {
        while (std::getline(logFile, line)) 
        {
            logs.push_back(line);
        }
        logFile.close();
    }
    else 
    {
        TableFormatter::displayErrorMessage("Не удалось открыть файл журнала!");
        TableFormatter::pause();
        return;
    }

    std::reverse(logs.begin(), logs.end());

    while (true) 
    {
        displayHeader("Просмотр журнала событий");

        int totalPages = logPagination.getTotalPages(logs.size());

        TableFormatter::displayLogsTable(logs, logPagination.currentPage, totalPages, logs.size());

        std::vector<std::string> options =
        {
            "Следующая страница",
            "Предыдущая страница",
            "Перейти к странице",
            "Обновить журнал",
            "Очистить журнал"
        };

        TableFormatter::displayMenu(options, "Управление журналом");

        int choice = InputValidation::getValidatedNumber("Введите выбор: ", 0, 5);

        switch (choice)
        {
        case 1:
        {
            size_t before = logPagination.currentPage;
            logPagination.nextPage(logs.size());
            if (logPagination.currentPage == before)
            {
                TableFormatter::displayWarningMessage("Вы уже на последней странице!");
            }
            TableFormatter::pause();
            break;
        }
        case 2:
        {
            size_t before = logPagination.currentPage;
            logPagination.prevPage();
            if (logPagination.currentPage == before)
            {
                TableFormatter::displayWarningMessage("Вы уже на первой странице!");
            }
            TableFormatter::pause();
            break;
        }
        case 3:
        {
            int totalPages = logPagination.getTotalPages(logs.size());
            int newPage = InputValidation::getValidatedNumber("Введите номер страницы: ", 1, totalPages);
            if (!logPagination.goToPage(newPage, logs.size()))
            {
                TableFormatter::displayWarningMessage("Такой страницы не существует!");
            }
            TableFormatter::pause();
            break;
        }
        case 4:
            logs.clear();
            logFile.open(Config::LOG_FILE);
            if (logFile.is_open()) 
            {
                while (std::getline(logFile, line)) 
                {
                    logs.push_back(line);
                }
                logFile.close();
            }
            std::reverse(logs.begin(), logs.end());
            logPagination.clamp(logs.size());
            TableFormatter::displaySuccessMessage("Журнал обновлён!");
            break;
        case 5:
            if (confirmAction("Вы уверены, что хотите очистить весь журнал? Это необратимое действие!")) 
            {
                std::ofstream clearFile(Config::LOG_FILE, std::ios::trunc);
                clearFile.close();
                logs.clear();
                logPagination.currentPage = 1;
                TableFormatter::displaySuccessMessage("Журнал очищен!");
                TableFormatter::pause();
            }
            break;
        case 0:
            return;
        }
    }
}

void AdminMenu::changeMyProfile()
{
    TableFormatter::clearScreen();
    if (auto admin = std::dynamic_pointer_cast<Admin>(currentUser)) 
    {
        admin->displayProfile();

        std::vector<std::string> options = 
        {
            "Изменить имя пользователя",
            "Изменить email",
            "Изменить телефон",
            "Изменить пароль"
        };

        TableFormatter::displayMenu(options, "Редактирование данных администратора");

        int choice = InputValidation::getValidatedNumber("Введите выбор: ", 0, 4);

        switch (choice) 
        {
        case 1:
            changeAdminUsername();
            break;
        case 2:
            changeAdminEmail();
            break;
        case 3:
            changeAdminPhone();
            break;
        case 4:
            changeAdminPassword();
            break;
        }
    }
    TableFormatter::pause();
}

void AdminMenu::changeAdminUsername() 
{
    std::string newUsername = InputValidation::getValidatedString("Введите новое имя пользователя: ", 3, 20);

    FileManager fileManager;
    auto users = fileManager.loadFromFile<std::shared_ptr<User>>(Config::USERS_FILE);

    for (const auto& user : users) 
    {
        if (user->getUsername() == newUsername && user->getId() != currentUser->getId())
        {
            TableFormatter::displayErrorMessage("Это имя пользователя уже занято!");
            TableFormatter::pause();
            return;
        }
    }

    for (auto& user : users) 
    {
        if (user->getId() == currentUser->getId()) 
        {
            std::string oldUsername = user->getUsername();
            user->setUsername(newUsername);
            if (fileManager.saveToFile(Config::USERS_FILE, users)) 
            {
                TableFormatter::displaySuccessMessage("Имя пользователя успешно изменено!");
                currentUser->setUsername(newUsername);
                logger->log(LogLevel::INFO, "Admin username changed: " + oldUsername + " -> " + newUsername);
            }
            return;
        }
    }
}

void AdminMenu::changeAdminEmail() 
{
    std::string newEmail = InputValidation::getValidatedEmail("Введите новый email: ");

    FileManager fileManager;
    auto users = fileManager.loadFromFile<std::shared_ptr<User>>(Config::USERS_FILE);

    for (auto& user : users) 
    {
        if (user->getId() == currentUser->getId()) 
        {
            std::string oldEmail = user->getEmail();
            user->setEmail(newEmail);
            if (fileManager.saveToFile(Config::USERS_FILE, users)) 
            {
                TableFormatter::displaySuccessMessage("Email успешно изменён!");
                currentUser->setEmail(newEmail);
                logger->log(LogLevel::INFO, "Admin email changed: " + oldEmail + " -> " + newEmail);
            }
            return;
        }
    }
}

void AdminMenu::changeAdminPhone()
{
    std::string newPhone = InputValidation::getValidatedPhone("Введите новый телефон: ");

    FileManager fileManager;
    auto users = fileManager.loadFromFile<std::shared_ptr<User>>(Config::USERS_FILE);

    for (auto& user : users) 
    {
        if (user->getId() == currentUser->getId()) 
        {
            std::string oldPhone = user->getPhone();
            user->setPhone(newPhone);
            if (fileManager.saveToFile(Config::USERS_FILE, users))
            {
                TableFormatter::displaySuccessMessage("Телефон успешно изменён!");
                currentUser->setPhone(newPhone);
                logger->log(LogLevel::INFO, "Admin phone changed: " + oldPhone + " -> " + newPhone);
            }
            return;
        }
    }
}

void AdminMenu::changeAdminPassword() 
{
    std::cout << "Введите текущий пароль: ";
    std::string currentPassword = InputValidation::maskPassword();

    if (!User::verifyPassword(currentPassword, currentUser->getPasswordHash())) 
    {
        TableFormatter::displayErrorMessage("Неверный текущий пароль!");
        TableFormatter::pause();
        return;
    }

    std::cout << "Новый пароль: ";
    std::string newPassword = InputValidation::maskPassword();
    std::cout << "Подтвердите новый пароль: ";
    std::string confirmPassword = InputValidation::maskPassword();

    if (newPassword != confirmPassword) 
    {
        TableFormatter::displayErrorMessage("Пароли не совпадают!");
        TableFormatter::pause();
        return;
    }

    std::string newStoredPassword = User::createPasswordHash(newPassword);

    FileManager fileManager;
    auto users = fileManager.loadFromFile<std::shared_ptr<User>>(Config::USERS_FILE);

    for (auto& user : users)
    {
        if (user->getId() == currentUser->getId()) 
        {
            user->setPasswordHash(newStoredPassword);
            if (fileManager.saveToFile(Config::USERS_FILE, users))
            {
                TableFormatter::displaySuccessMessage("Пароль успешно изменён!");
                currentUser->setPasswordHash(newStoredPassword);
                logger->log(LogLevel::INFO, "Admin password changed for user: " + currentUser->getUsername());
            }
            return;
        }
    }
}

void AdminMenu::viewNotifications() 
{
    displayHeader("Управление уведомлениями");

    notificationSystem.displayAdminNotifications();

    auto notifications = notificationSystem.getAdminNotifications();

    if (!notifications.empty()) 
    {
        std::vector<std::string> options =
        {
            "Отметить все уведомления как прочитанные",
            "Очистить все уведомления"
        };

        TableFormatter::displayMenu(options, "Действия с уведомлениями");

        int choice = InputValidation::getValidatedNumber("Введите выбор: ", 0, 2);

        switch (choice) 
        {
        case 1:
            notificationSystem.markAdminNotificationsAsRead();
            TableFormatter::displaySuccessMessage("Уведомления отмечены как прочитанные!");
            break;
        case 2:
            if (confirmAction("Вы уверены, что хотите очистить все уведомления? Это необратимое действие!")) 
            {
                notificationSystem.clearAdminNotifications();
                TableFormatter::displaySuccessMessage("Уведомления очищены!");
            }
            break;
        case 0:
            return;
        }
    }

    TableFormatter::pause();
}
