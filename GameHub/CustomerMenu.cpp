#include "CustomerMenu.h"
#include "FileManager.h"

CustomerMenu::CustomerMenu(Store& s, Wishlist& w, RecommendationSystem& rs, Report& r, NotificationSystem& ns, std::shared_ptr<User> user)
    : Menu(s, w, rs, r, ns) 
{
    currentUser = user;
}

void CustomerMenu::show() 
{
    if (!currentUser->getIsApproved()) 
    {
        showLimitedMenu();
        return;
    }
    while (true) 
    {
        displayHeader("Главное меню");

        std::vector<std::string> options = 
        {
            "Просмотр каталога",
            "Поиск игр",
            "История покупок",
            "Управление списком",
            "Рекомендации",
            "Мой профиль",
            "Уведомления"
        };

        TableFormatter::displayMenu(options, "Выбор действия");

        int choice = InputValidation::getValidatedNumber("Введите действие: ", 0, 7);

        switch (choice) 
        {
        case 1:
            viewCatalog();
            break;
        case 2:
            searchGames();
            break;
        case 3:
            viewPurchaseHistory();
            break;
        case 4:
            manageWishlist();
            break;
        case 5:
            viewRecommendations();
            break;
        case 6:
            viewProfile();
            break;
        case 7:
            viewNotifications();
            break;
        case 0:
            return;
        }
    }
}

void CustomerMenu::showLimitedMenu() 
{
    while (true) 
    {
        displayHeader("Главное меню (неподтверждённый)");

        std::vector<std::string> options = 
        {
            "Просмотр каталога",
            "Поиск игр",
            "Мой профиль",
        };

        TableFormatter::displayMenu(options, "Ограниченное меню");

        int choice = InputValidation::getValidatedNumber("Введите действие: ", 0, 3);

        switch (choice) 
        {
        case 1:
            viewCatalogLimited();
            break;
        case 2:
            searchGamesLimited();
            break;
        case 3:
            viewProfile();
            break;
        case 0:
            return;
        }
    }
}

void CustomerMenu::viewCatalogLimited() 
{
    pagination.perPage = 5;

    while (true) 
    {
        displayHeader("Каталог игр (только просмотр)");

        auto allProducts = store.getAllProducts();
        int totalPages = pagination.getTotalPages(allProducts.size());

        TableFormatter::displayProductsTableWithPagination(allProducts, pagination.currentPage, totalPages, allProducts.size());

        std::vector<std::string> options = 
        {
            "Следующая страница",
            "Предыдущая страница",
            "Перейти к игре",
            "Просмотреть детали игры",
        };

        TableFormatter::displayMenu(options, "Выберите действие");

        int choice = InputValidation::getValidatedNumber("Введите действие: ", 0, 4);

        switch (choice) 
        {
        case 1:
            pagination.nextPage(allProducts.size());
            break;
        case 2:
            pagination.prevPage();
            break;
        case 3:
            if (totalPages > 1) 
            {
                int newPage = InputValidation::getValidatedNumber("Введите номер страницы: ", 1, totalPages);
                pagination.goToPage(newPage, allProducts.size());
            }
            break;
        case 4:
            viewProductDetailsLimited();
            break;
        case 0:
            return;
        }
    }
}

void CustomerMenu::viewProductDetailsLimited() 
{
    int productId = InputValidation::getValidatedNumber("Введите ID игры для просмотра: ", 1, 10000);

    auto product = store.getProductById(productId);
    if (product)
    {
        TableFormatter::displayProductDetails(product);
        TableFormatter::displayWarningMessage("Для покупки игры необходимо зарегистрироваться и подтвердить аккаунт.");
    }
    else 
    {
        TableFormatter::displayErrorMessage("Игра с таким ID не найдена!");
    }
    TableFormatter::pause();
}

void CustomerMenu::searchGamesLimited()
{
    PaginationState searchPagination;
    searchPagination.perPage = 5;
    std::vector<std::shared_ptr<Product>> searchResults;
    std::string searchType = "all";
    std::string searchValue = "";

    while (true)
    {
        displayHeader("Поиск и фильтрация игр (только просмотр)");

        if (!searchResults.empty()) 
        {
            int totalPages = searchPagination.getTotalPages(searchResults.size());
            searchPagination.clamp(searchResults.size());

            std::cout << "Текущий фильтр: ";
            if (searchValue.empty())
            {
                std::cout << "не задано" << std::endl;
            }
            else 
            {
                std::cout << searchType << " = '" << searchValue << "'" << std::endl;
            }
            std::cout << "Найдено игр: " << searchResults.size() << std::endl << std::endl;

            TableFormatter::displayProductsTableWithPagination(searchResults, searchPagination.currentPage, totalPages, searchResults.size());
        }
        else
        {
            TableFormatter::displayWarningMessage("Нет результатов поиска. Выполните поиск или фильтрацию для отображения.");
        }

        std::vector<std::string> options = 
        {
            "Поиск по названию",
            "Фильтр по жанру",
            "Фильтр по разработчику",
            "Фильтр по тегу",
            "Фильтр по скидкам",
            "Фильтр по наличию",
            "Сбросить фильтры",
            "Следующая страница",
            "Предыдущая страница",
            "Перейти к странице",
            "Просмотреть детали игры"
        };

        TableFormatter::displayMenu(options, "Поиск и фильтрация");

        int choice = InputValidation::getValidatedNumber("Введите действие: ", 0, 11);

        switch (choice) 
        {
        case 1: 
        {
            std::string query = InputValidation::getValidatedString("Введите ключевое слово для поиска: ", 1, 100);
            searchResults = store.searchProducts(query);
            searchType = "title";
            searchValue = query;
            searchPagination.currentPage = 1;
            break;
        }
        case 2: 
        {
            auto genres = store.getUniqueGenres();
            if (genres.empty())
            {
                TableFormatter::displayWarningMessage("Нет доступных жанров.");
                TableFormatter::pause();
                break;
            }

            std::cout << "Доступные жанры:" << std::endl;
            for (size_t i = 0; i < genres.size(); ++i) 
            {
                std::cout << i + 1 << ". " << genres[i] << std::endl;
            }

            int genreChoice = InputValidation::getValidatedNumber("Выберите жанр: ", 1, genres.size());
            searchResults = store.filterProducts("genre", genres[genreChoice - 1]);
            searchType = "genre";
            searchValue = genres[genreChoice - 1];
            searchPagination.currentPage = 1;
            break;
        }
        case 3: 
        {
            auto developers = store.getUniqueDevelopers();
            if (developers.empty()) 
            {
                TableFormatter::displayWarningMessage("Нет доступных разработчиков.");
                TableFormatter::pause();
                break;
            }

            std::cout << "Доступные разработчики:" << std::endl;
            for (size_t i = 0; i < developers.size(); ++i)
            {
                std::cout << i + 1 << ". " << developers[i] << std::endl;
            }

            int devChoice = InputValidation::getValidatedNumber("Выберите разработчика: ", 1, developers.size());
            searchResults = store.filterProducts("developer", developers[devChoice - 1]);
            searchType = "developer";
            searchValue = developers[devChoice - 1];
            searchPagination.currentPage = 1;
            break;
        }
        case 4: 
        {
            FileManager fileManager;
            auto tags = fileManager.loadStringList(Config::TAGS_FILE);
            if (tags.empty())
            {
                TableFormatter::displayWarningMessage("Нет доступных тегов.");
                TableFormatter::pause();
                break;
            }

            std::cout << "Доступные теги:" << std::endl;
            for (size_t i = 0; i < tags.size(); ++i)
            {
                std::cout << i + 1 << ". " << tags[i] << std::endl;
            }

            int tagChoice = InputValidation::getValidatedNumber("Выберите тег: ", 1, tags.size());
            searchResults = store.filterProducts("tag", tags[tagChoice - 1]);
            searchType = "tag";
            searchValue = tags[tagChoice - 1];
            searchPagination.currentPage = 1;
            break;
        }
        case 5:
            searchResults = store.filterProducts("has_discount", "");
            searchType = "has_discount";
            searchValue = "";
            searchPagination.currentPage = 1;
            break;
        case 6:
            searchResults = store.filterProducts("in_stock", "");
            searchType = "in_stock";
            searchValue = "";
            searchPagination.currentPage = 1;
            break;
        case 7:
            searchResults.clear();
            searchType = "all";
            searchValue = "";
            searchPagination.currentPage = 1;
            break;
        case 8:
            searchPagination.nextPage(searchResults.size());
            break;
        case 9:
            searchPagination.prevPage();
            break;
        case 10:
            if (!searchResults.empty())
            {
                int totalPages = searchPagination.getTotalPages(searchResults.size());
                if (totalPages > 1) 
                {
                    int newPage = InputValidation::getValidatedNumber("Введите номер страницы: ", 1, totalPages);
                    searchPagination.goToPage(newPage, searchResults.size());
                }
                else 
                {
                    TableFormatter::displayWarningMessage("Только одна страница результатов!");
                    TableFormatter::pause();
                }
            }
            break;
        case 11:
            if (!searchResults.empty()) 
            {
                viewProductDetailsLimited();
            }
            else 
            {
                TableFormatter::displayWarningMessage("Нет результатов для просмотра.");
                TableFormatter::pause();
            }
            break;
        case 0:
            return;
        }
    }
}

void CustomerMenu::viewCatalog() 
{
    pagination.perPage = 5;

    while (true) 
    {
        displayHeader("Каталог игр");

        auto allProducts = store.getAllProducts();
        int totalPages = pagination.getTotalPages(allProducts.size());

        TableFormatter::displayProductsTableWithPagination(allProducts, pagination.currentPage, totalPages, allProducts.size());

        std::vector<std::string> options = 
        {
            "Следующая страница",
            "Предыдущая страница",
            "Перейти к странице",
            "Поиск игр",
            "Просмотреть детали игры",
            "Добавить в список желаний",
            "Купить игру"
        };

        TableFormatter::displayMenu(options, "Выберите действие");

        int choice = InputValidation::getValidatedNumber("Введите действие: ", 0, 7);

        switch (choice) 
        {
        case 1:
            pagination.nextPage(allProducts.size());
            break;
        case 2:
            pagination.prevPage();
            break;
        case 3:
            if (totalPages > 1) 
            {
                int newPage = InputValidation::getValidatedNumber("Введите номер страницы: ", 1, totalPages);
                pagination.goToPage(newPage, allProducts.size());
            }
            else 
            {
                TableFormatter::displayWarningMessage("Только одна страница результатов!");
                TableFormatter::pause();
            }
            break;
        case 4:
            searchGames();
            break;
        case 5:
            viewProductDetails();
            break;
        case 6:
            addToWishlistFromCatalog();
            break;
        case 7:
            purchaseFromCatalog();
            break;
        case 0:
            return;
        }
    }
}

void CustomerMenu::viewProductDetails() 
{
    TableFormatter::clearScreen();
    displayHeader("Просмотр деталей игры");

    auto allProducts = store.getAllProducts();
    int totalPages = pagination.getTotalPages(allProducts.size());
    TableFormatter::displayProductsTableWithPagination(allProducts, 1, totalPages, allProducts.size());

    int productId = InputValidation::getValidatedNumber("Введите ID игры для просмотра (0 для отмены): ", 0, 10000);
    if (productId == 0) 
    {
        TableFormatter::displayWarningMessage("Просмотр отменён.");
        TableFormatter::pause();
        return;
    }

    auto product = store.getProductById(productId);
    if (product) 
    {
        TableFormatter::displayProductDetails(product);

        std::vector<std::string> options = 
        {
            "Добавить в список желаний",
            "Купить игру"
        };

        TableFormatter::displayMenu(options, "Дополнительные действия");

        int choice = InputValidation::getValidatedNumber("Введите действие: ", 0, 2);

        switch (choice) 
        {
        case 1:
            if (wishlist.addToWishlist(currentUser->getId(), productId)) 
            {
                TableFormatter::displaySuccessMessage("Игра добавлена в список желаний!");
            }
            else 
            {
                TableFormatter::displayErrorMessage("Ошибка при добавлении в список желаний!");
            }
            break;
        case 2:
            purchaseProduct(product);
            break;
        case 0:
            return;
        }
    }
    else 
    {
        TableFormatter::displayErrorMessage("Игра с таким ID не найдена!");
    }
    TableFormatter::pause();
}

void CustomerMenu::addToWishlistFromCatalog() 
{
    TableFormatter::clearScreen();
    displayHeader("Добавление в список желаний");

    auto allProducts = store.getAllProducts();
    int totalPages = pagination.getTotalPages(allProducts.size());
    TableFormatter::displayProductsTableWithPagination(allProducts, 1, totalPages, allProducts.size());

    int productId = InputValidation::getValidatedNumber("Введите ID игры для добавления в список желаний (0 для отмены): ", 1, 10000);
    if (productId == 0) {
        TableFormatter::displayWarningMessage("Добавление отменено.");
        TableFormatter::pause();
        return;
    }

    if (wishlist.addToWishlist(currentUser->getId(), productId)) 
    {
        TableFormatter::displaySuccessMessage("Игра добавлена в список желаний!");
    }
    else 
    {
        TableFormatter::displayErrorMessage("Ошибка при добавлении в список желаний или игра не найдена!");
    }
    TableFormatter::pause();
}

void CustomerMenu::purchaseFromCatalog() 
{
    TableFormatter::clearScreen();
    displayHeader("Покупка игры");
    auto allProducts = store.getAllProducts();
    int totalPages = pagination.getTotalPages(allProducts.size());
    TableFormatter::displayProductsTableWithPagination(allProducts, 1, totalPages, allProducts.size());

    int productId = InputValidation::getValidatedNumber("Введите ID игры для покупки (0 для отмены): ", 1, 10000);
    if (productId == 0) 
    {
        TableFormatter::displayWarningMessage("Покупка отменена.");
        TableFormatter::pause();
        return;
    }

    auto product = store.getProductById(productId);
    if (product) 
    {
        purchaseProduct(product);
    }
    else 
    {
        TableFormatter::displayErrorMessage("Игра с таким ID не найдена!");
        TableFormatter::pause();
    }
}

void CustomerMenu::purchaseProduct(std::shared_ptr<Product> product) 
{
    if (!product->hasKeys()) 
    {
        TableFormatter::displayErrorMessage("У игры закончились активационные ключи!");
        TableFormatter::displayWarningMessage("Вы можете добавить игру в список желаний и получить уведомление, когда она появится.");

        if (InputValidation::getValidatedNumber("Добавить в список желаний? (1 - да, 0 - нет): ", 0, 1) == 1) 
        {
            wishlist.addToWishlist(currentUser->getId(), product->getId());
            TableFormatter::displaySuccessMessage("Игра добавлена в список желаний!");
        }
        TableFormatter::pause();
        return;
    }

    TableFormatter::displayProductDetails(product);

    double discountedPrice = store.getDiscountedPrice(product);
    if (discountedPrice < product->getBasePrice())
    {
        double discountPercent = ((product->getBasePrice() - discountedPrice) / product->getBasePrice()) * 100;
        Config::setColor(Config::COLOR_SUCCESS);
        std::cout << "Цена со скидкой: " << discountedPrice << " руб. (скидка "
            << std::fixed << std::setprecision(1) << discountPercent << "%)" << std::endl;
        Config::resetColor();
    }

    int quantity = InputValidation::getValidatedNumber("Введите количество: ", 1, product->getActivationKeys());

    if (InputValidation::getValidatedNumber("Подтвердить покупку? (1 - да, 0 - нет): ", 0, 1) == 1)
    {
        try 
        {
            std::vector<std::pair<int, int>> items = { {product->getId(), quantity} };
            if (store.processSale(currentUser->getId(), items)) 
            {
                TableFormatter::displaySuccessMessage("Покупка успешно выполнена!");
                TableFormatter::displaySuccessMessage("Ключи активации отправлены на ваш email: " + currentUser->getEmail());

                if (auto customer = std::dynamic_pointer_cast<Customer>(currentUser)) 
                {
                    customer->addPurchase(discountedPrice * quantity);
                    customer->setLastPurchasedProductId(product->getId());
                }

                notificationSystem.notifyPurchaseComplete(currentUser->getId(), product->getTitle());
            }
        }
        catch (const BusinessLogicError& e) 
        {
            TableFormatter::displayErrorMessage(e.what());
        }
    }
    else 
    {
        TableFormatter::displayWarningMessage("Покупка отменена.");
    }
    TableFormatter::pause();
}

void CustomerMenu::searchGames() 
{
    PaginationState searchPagination;
    searchPagination.perPage = 5;
    std::vector<std::shared_ptr<Product>> searchResults;
    std::string searchType = "all";
    std::string searchValue = "";

    while (true) 
    {
        displayHeader("Поиск и фильтрация игр");

        if (!searchResults.empty()) 
        {
            int totalPages = searchPagination.getTotalPages(searchResults.size());
            searchPagination.clamp(searchResults.size());

            std::cout << "Текущий фильтр: ";
            if (searchValue.empty()) 
            {
                std::cout << "не задано" << std::endl;
            }
            else 
            {
                std::cout << searchType << " = '" << searchValue << "'" << std::endl;
            }
            std::cout << "Найдено игр: " << searchResults.size() << std::endl << std::endl;

            TableFormatter::displayProductsTableWithPagination(searchResults, searchPagination.currentPage, totalPages, searchResults.size());
        }
        else 
        {
            TableFormatter::displayWarningMessage("Нет результатов поиска. Выполните поиск или фильтрацию для отображения.");
        }

        std::vector<std::string> options = 
        {
            "Поиск по названию",
            "Фильтр по жанру",
            "Фильтр по разработчику",
            "Фильтр по тегу",
            "Фильтр по скидкам",
            "Фильтр по наличию",
            "Сбросить фильтры",
            "Следующая страница",
            "Предыдущая страница",
            "Перейти к странице",
            "Просмотреть детали игры",
            "Добавить в список желаний",
            "Купить игру"
        };

        TableFormatter::displayMenu(options, "Поиск и фильтрация");

        int choice = InputValidation::getValidatedNumber("Введите действие: ", 0, 13);

        switch (choice) 
        {
        case 1: {
            std::string query = InputValidation::getValidatedString("Введите ключевое слово для поиска: ", 1, 100);
            searchResults = store.searchProducts(query);
            searchType = "title";
            searchValue = query;
            searchPagination.currentPage = 1;
            break;
        }
        case 2: 
        {
            auto genres = store.getUniqueGenres();
            if (genres.empty()) 
            {
                TableFormatter::displayWarningMessage("Нет доступных жанров.");
                TableFormatter::pause();
                break;
            }

            std::cout << "Доступные жанры:" << std::endl;
            for (size_t i = 0; i < genres.size(); ++i)
            {
                std::cout << i + 1 << ". " << genres[i] << std::endl;
            }

            int genreChoice = InputValidation::getValidatedNumber("Выберите жанр: ", 1, genres.size());
            searchResults = store.filterProducts("genre", genres[genreChoice - 1]);
            searchType = "genre";
            searchValue = genres[genreChoice - 1];
            searchPagination.currentPage = 1;
            break;
        }
        case 3: 
        {
            auto developers = store.getUniqueDevelopers();
            if (developers.empty()) 
            {
                TableFormatter::displayWarningMessage("Нет доступных разработчиков.");
                TableFormatter::pause();
                break;
            }

            std::cout << "Доступные разработчики:" << std::endl;
            for (size_t i = 0; i < developers.size(); ++i) 
            {
                std::cout << i + 1 << ". " << developers[i] << std::endl;
            }

            int devChoice = InputValidation::getValidatedNumber("Выберите разработчика: ", 1, developers.size());
            searchResults = store.filterProducts("developer", developers[devChoice - 1]);
            searchType = "developer";
            searchValue = developers[devChoice - 1];
            searchPagination.currentPage = 1;
            break;
        }
        case 4: 
        {
            FileManager fileManager;
            auto tags = fileManager.loadStringList(Config::TAGS_FILE);
            if (tags.empty()) 
            {
                TableFormatter::displayWarningMessage("Нет доступных тегов.");
                TableFormatter::pause();
                break;
            }

            std::cout << "Доступные теги:" << std::endl;
            for (size_t i = 0; i < tags.size(); ++i) 
            {
                std::cout << i + 1 << ". " << tags[i] << std::endl;
            }

            int tagChoice = InputValidation::getValidatedNumber("Выберите тег: ", 1, tags.size());
            searchResults = store.filterProducts("tag", tags[tagChoice - 1]);
            searchType = "tag";
            searchValue = tags[tagChoice - 1];
            searchPagination.currentPage = 1;
            break;
        }
        case 5:
            searchResults = store.filterProducts("has_discount", "");
            searchType = "has_discount";
            searchValue = "";
            searchPagination.currentPage = 1;
            break;
        case 6:
            searchResults = store.filterProducts("in_stock", "");
            searchType = "in_stock";
            searchValue = "";
            searchPagination.currentPage = 1;
            break;
        case 7:
            searchResults.clear();
            searchType = "all";
            searchValue = "";
            searchPagination.currentPage = 1;
            break;
        case 8:
            searchPagination.nextPage(searchResults.size());
            break;
        case 9:
            searchPagination.prevPage();
            break;
        case 10:
            if (!searchResults.empty()) 
            {
                int totalPages = searchPagination.getTotalPages(searchResults.size());
                if (totalPages > 1) 
                {
                    int newPage = InputValidation::getValidatedNumber("Введите номер страницы: ", 1, totalPages);
                    searchPagination.goToPage(newPage, searchResults.size());
                }
                else 
                {
                    TableFormatter::displayWarningMessage("Только одна страница результатов!");
                    TableFormatter::pause();
                }
            }
            break;
        case 11:
            if (!searchResults.empty())
            {
                viewProductDetailsFromSearch(searchResults);
            }
            else 
            {
                TableFormatter::displayWarningMessage("Нет результатов для просмотра.");
                TableFormatter::pause();
            }
            break;
        case 12:
            if (!searchResults.empty()) 
            {
                addToWishlistFromSearch();
            }
            else 
            {
                TableFormatter::displayWarningMessage("Нет результатов для добавления.");
                TableFormatter::pause();
            }
            break;
        case 13:
            if (!searchResults.empty()) 
            {
                purchaseFromSearch(searchResults);
            }
            else 
            {
                TableFormatter::displayWarningMessage("Нет результатов для покупки.");
                TableFormatter::pause();
            }
            break;
        case 0:
            return;
        }
    }
}

void CustomerMenu::viewProductDetailsFromSearch(const std::vector<std::shared_ptr<Product>>& searchResults) {
    int productId = InputValidation::getValidatedNumber("Введите ID игры для просмотра: ", 1, 10000);

    bool found = false;
    std::shared_ptr<Product> product;
    for (const auto& p : searchResults) 
    {
        if (p->getId() == productId) 
        {
            product = p;
            found = true;
            break;
        }
    }

    if (found && product) 
    {
        TableFormatter::displayProductDetails(product);
        TableFormatter::pause();
    }
    else 
    {
        TableFormatter::displayErrorMessage("Игра с таким ID не найдена в текущих результатах!");
        TableFormatter::pause();
    }
}

void CustomerMenu::addToWishlistFromSearch() 
{
    int productId = InputValidation::getValidatedNumber("Введите ID игры для добавления в список желаний: ", 1, 10000);

    if (wishlist.addToWishlist(currentUser->getId(), productId)) 
    {
        TableFormatter::displaySuccessMessage("Игра добавлена в список желаний!");
    }
    else 
    {
        TableFormatter::displayErrorMessage("Ошибка при добавлении в список желаний или игра не найдена!");
    }
    TableFormatter::pause();
}

void CustomerMenu::purchaseFromSearch(const std::vector<std::shared_ptr<Product>>& searchResults) 
{
    int productId = InputValidation::getValidatedNumber("Введите ID игры для покупки: ", 1, 10000);

    bool found = false;
    std::shared_ptr<Product> product;
    for (const auto& p : searchResults)
    {
        if (p->getId() == productId) 
        {
            product = p;
            found = true;
            break;
        }
    }

    if (found && product) 
    {
        purchaseProduct(product);
    }
    else 
    {
        TableFormatter::displayErrorMessage("Игра с таким ID не найдена в текущих результатах!");
        TableFormatter::pause();
    }
}

void CustomerMenu::viewPurchaseHistory()
{
    pagination.perPage = 5;

    auto allSales = store.getAllSales();
    std::vector<Sale> userSales;

    for (const auto& sale : allSales) 
    {
        if (sale.getCustomerId() == currentUser->getId()) 
        {
            userSales.push_back(sale);
        }
    }

    if (userSales.empty())
    {
        displayHeader("История покупок");
        TableFormatter::displayWarningMessage("У вас ещё нет покупок.");
        TableFormatter::pause();
        return;
    }

    int totalPages = pagination.getTotalPages(userSales.size());

    while (true) 
    {
        displayHeader("История покупок");

        TableFormatter::displaySalesTableWithPagination(userSales, pagination.currentPage, totalPages, userSales.size());

        std::vector<std::string> options = 
        {
            "Следующая страница",
            "Предыдущая страница",
            "Перейти к странице",
            "Просмотреть детали покупки"
        };

        TableFormatter::displayMenu(options, "История покупок");

        int choice = InputValidation::getValidatedNumber("Введите действие: ", 0, 4);

        switch (choice) 
        {
        case 1:
            pagination.nextPage(userSales.size());
            break;
        case 2:
            pagination.prevPage();
            break;
        case 3:
            if (totalPages > 1) 
            {
                int newPage = InputValidation::getValidatedNumber("Введите номер страницы: ", 1, totalPages);
                pagination.goToPage(newPage, userSales.size());
            }
            else 
            {
                TableFormatter::displayWarningMessage("Только одна страница результатов!");
            }
            break;
        case 4:
            viewSaleDetails(userSales);
            break;
        case 0:
            return;
        }
    }
}

void CustomerMenu::viewSaleDetails(const std::vector<Sale>& userSales) 
{
    int saleId = InputValidation::getValidatedNumber("Введите ID покупки для просмотра: ", 1, 10000);

    bool found = false;
    std::vector<Sale> targetSale;
    for (const auto& sale : userSales)
    {
        if (sale.getSaleId() == saleId)
        {
            found = true;
            targetSale.push_back(sale);
            break;
        }
    }

    if (found) 
    {
        displayHeader("Детали покупки #" + std::to_string(saleId));
        TableFormatter::displaySalesTableDetailed(targetSale, store, currentUser->getId());
    }
    else 
    {
        TableFormatter::displayErrorMessage("Покупка с таким ID не найдена в вашей истории!");
    }

    TableFormatter::pause();
}

void CustomerMenu::manageWishlist() 
{
    auto customer = std::dynamic_pointer_cast<Customer>(currentUser);
    if (!customer) return;

    while (true) 
    {
        displayHeader("Управление списком желаний");

        auto products = store.getAllProducts();
        wishlist.displayWishlist(currentUser->getId(), products);

        std::vector<std::string> options = 
        {
            "Добавить игру в список",
            "Убрать игру из списка",
            "Очистить весь список"
        };

        TableFormatter::displayMenu(options, "Управление списком желаний");

        int choice = InputValidation::getValidatedNumber("Введите действие: ", 0, 3);

        switch (choice) 
        {
        case 1: 
        {
            int productId = InputValidation::getValidatedNumber("Введите ID игры: ", 1, 1000);
            if (wishlist.addToWishlist(currentUser->getId(), productId)) 
            {
                TableFormatter::displaySuccessMessage("Игра добавлена в список желаний!");
            }
            else 
            {
                TableFormatter::displayErrorMessage("Ошибка при добавлении в список желаний или игра не найдена!");
            }
            break;
        }
        case 2: 
        {
            int productId = InputValidation::getValidatedNumber("Введите ID игры: ", 1, 1000);
            if (wishlist.removeFromWishlist(currentUser->getId(), productId)) 
            {
                TableFormatter::displaySuccessMessage("Игра убрана из списка желаний!");
            }
            else 
            {
                TableFormatter::displayErrorMessage("Игра не найдена в списке желаний!");
            }
            break;
        }
        case 3:
            wishlist.clearWishlist(currentUser->getId());
            TableFormatter::displaySuccessMessage("Список желаний очищен!");
            break;
        case 0:
            return;
        }
        TableFormatter::pause();
    }
}

void CustomerMenu::viewRecommendations() 
{
    auto customer = std::dynamic_pointer_cast<Customer>(currentUser);
    if (!customer) 
    {
        TableFormatter::displayErrorMessage("Ошибка: клиент не является покупателем!");
        TableFormatter::pause();
        return;
    }

    displayHeader("Рекомендации для вас");

    auto recommendations = recommendationSystem.getSimilarGames(customer);

    if (recommendations.empty()) 
    {
        if (customer->getLastPurchasedProductId() == -1) 
        {
            TableFormatter::displayWarningMessage("Рекомендации недоступны для новичков без покупок!");
            TableFormatter::displaySuccessMessage("Вот наши самые популярные игры:");

            auto popularProducts = recommendationSystem.getPopularProducts();
            if (!popularProducts.empty()) 
            {
                TableFormatter::displayProductsTable(popularProducts);
            }
        }
        else 
        {
            TableFormatter::displayWarningMessage("К сожалению, рекомендации недоступны.");
            TableFormatter::displaySuccessMessage("Попробуйте игры со скидкой:");

            auto discountedProducts = recommendationSystem.getDiscountedProducts();
            if (!discountedProducts.empty()) 
            {
                TableFormatter::displayProductsTable(discountedProducts);
            }
        }
    }
    else
    {
        TableFormatter::displaySuccessMessage("Вот игры, которые могут вам понравиться:");
        TableFormatter::displayProductsTable(recommendations);

        std::vector<std::string> options = 
        {
            "Добавить игру в список желаний",
            "Купить рекомендованную игру"
        };

        TableFormatter::displayMenu(options, "Дополнительные действия");

        int choice = InputValidation::getValidatedNumber("Введите действие: ", 0, 2);

        switch (choice)
        {
        case 1:
            addToWishlistFromRecommendations(recommendations);
            break;
        case 2:
            purchaseFromRecommendations(recommendations);
            break;
        case 0:
            return;
        }
    }

    TableFormatter::pause();
}

void CustomerMenu::addToWishlistFromRecommendations(const std::vector<std::shared_ptr<Product>>& recommendations) 
{
    auto allProducts = store.getAllProducts();
    int totalPages = pagination.getTotalPages(allProducts.size());
    TableFormatter::displayProductsTableWithPagination(allProducts, 1, totalPages, allProducts.size());

    int productId = InputValidation::getValidatedNumber("Введите ID игры для добавления в список желаний: ", 1, 10000);

    bool found = false;
    for (const auto& product : recommendations) 
    {
        if (product->getId() == productId) 
        {
            found = true;
            break;
        }
    }

    if (found) 
    {
        if (wishlist.addToWishlist(currentUser->getId(), productId)) 
        {
            TableFormatter::displaySuccessMessage("Игра добавлена в список желаний!");
        }
        else 
        {
            TableFormatter::displayErrorMessage("Ошибка при добавлении в список желаний!");
        }
    }
    else 
    {
        TableFormatter::displayErrorMessage("Игра с таким ID не найдена в рекомендациях!");
    }
}

void CustomerMenu::purchaseFromRecommendations(const std::vector<std::shared_ptr<Product>>& recommendations)
{
    auto allProducts = store.getAllProducts();
    int totalPages = pagination.getTotalPages(allProducts.size());
    TableFormatter::displayProductsTableWithPagination(allProducts, 1, totalPages, allProducts.size());

    int productId = InputValidation::getValidatedNumber("Введите ID игры для покупки: ", 1, 10000);

    bool found = false;
    std::shared_ptr<Product> productToPurchase;
    for (const auto& product : recommendations) 
    {
        if (product->getId() == productId) 
        {
            found = true;
            productToPurchase = product;
            break;
        }
    }

    if (found && productToPurchase) 
    {
        purchaseProduct(productToPurchase);
    }
    else 
    {
        TableFormatter::displayErrorMessage("Игра с таким ID не найдена в рекомендациях!");
        TableFormatter::pause();
    }
}

void CustomerMenu::viewProfile()
{
    TableFormatter::clearScreen();
    if (auto customer = std::dynamic_pointer_cast<Customer>(currentUser)) 
    {
        customer->displayProfile();

        std::vector<std::string> options = 
        {
            "Изменить email",
            "Изменить телефон",
            "Изменить пароль"
        };

        TableFormatter::displayMenu(options, "Дополнительные действия");

        int choice = InputValidation::getValidatedNumber("Введите действие: ", 0, 3);

        switch (choice) 
        {
        case 1:
            changeEmail();
            break;
        case 2:
            changePhone();
            break;
        case 3:
            changePassword();
            break;
        case 0:
            return;
        }
    }

    TableFormatter::pause();
}

void CustomerMenu::changeEmail()
{
    std::string newEmail = InputValidation::getValidatedEmail("Введите новый email: ");

    FileManager fileManager;
    auto users = fileManager.loadFromFile<std::shared_ptr<User>>(Config::USERS_FILE);

    for (auto& user : users) 
    {
        if (user->getId() == currentUser->getId()) 
        {
            user->setEmail(newEmail);
            if (fileManager.saveToFile(Config::USERS_FILE, users)) 
            {
                TableFormatter::displaySuccessMessage("Email успешно изменён!");
                currentUser->setEmail(newEmail);
            }
            return;
        }
    }
}

void CustomerMenu::changePhone() 
{
    std::string newPhone = InputValidation::getValidatedPhone("Введите новый телефон: ");

    FileManager fileManager;
    auto users = fileManager.loadFromFile<std::shared_ptr<User>>(Config::USERS_FILE);

    for (auto& user : users) 
    {
        if (user->getId() == currentUser->getId()) 
        {
            user->setPhone(newPhone);
            if (fileManager.saveToFile(Config::USERS_FILE, users)) 
            {
                TableFormatter::displaySuccessMessage("Телефон успешно изменён!");
                currentUser->setPhone(newPhone);
            }
            return;
        }
    }
}

void CustomerMenu::changePassword() 
{
    std::cout << "Введите пароль: ";
    std::string currentPassword = InputValidation::maskPassword();
    std::cout << "Новый пароль: ";
    std::string newPassword = InputValidation::maskPassword();
    std::cout << "Подтвердите новый пароль: ";
    std::string confirmPassword = InputValidation::maskPassword();

    if (newPassword != confirmPassword)
    {
        TableFormatter::displayErrorMessage("Пароли не совпадают!");
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
            }
            return;
        }
    }
}

void CustomerMenu::viewNotifications() 
{
    displayHeader("Ваши уведомления");

    int customerId = currentUser->getId();
    notificationSystem.displayCustomerNotifications(customerId);

    auto notifications = notificationSystem.getCustomerNotifications(customerId);

    if (!notifications.empty()) 
    {
        std::vector<std::string> options = 
        {
            "Отметить все уведомления как прочитанные",
            "Очистить все уведомления"
        };

        TableFormatter::displayMenu(options, "Работа с уведомлениями");

        int choice = InputValidation::getValidatedNumber("Введите действие: ", 0, 2);

        switch (choice) 
        {
        case 1:
            notificationSystem.markCustomerNotificationsAsRead(customerId);
            TableFormatter::displaySuccessMessage("Уведомления отмечены как прочитанные!");
            break;
        case 2:
            if (InputValidation::getValidatedNumber("Вы уверены, что хотите удалить все уведомления? (1 - да, 0 - нет): ", 0, 1) == 1) {
                notificationSystem.clearCustomerNotifications(customerId);
                TableFormatter::displaySuccessMessage("Уведомления очищены!");
            }
            break;
        case 0:
            return;
        }
    }

    TableFormatter::pause();
}
