#include "Menu.h"
#include "FileManager.h"

Menu::Menu(Store& s, Wishlist& w, RecommendationSystem& rs, Report& r, NotificationSystem& ns)
    : store(s), wishlist(w), recommendationSystem(rs), report(r), notificationSystem(ns) 
{
    logger = Logger::getInstance();
}

void Menu::displayHeader(const std::string& title) 
{
    TableFormatter::clearScreen();
    Config::setColor(Config::COLOR_HEADER);
    std::cout << "==========================================" << std::endl;
    std::cout << "           " << title << std::endl;
    std::cout << "==========================================" << std::endl;
    Config::resetColor();

    if (currentUser) 
    {
        std::cout << "Пользователь: " << currentUser->getUsername();
        std::cout << " (" << currentUser->getUserType() << ")" << std::endl;
    }
    std::cout << std::endl;
}

bool Menu::confirmAction(const std::string& message) 
{
    Config::setColor(Config::COLOR_WARNING);
    std::cout << message << std::endl;
    Config::resetColor();

    int choice = InputValidation::getValidatedNumber("Введите 1 для подтверждения или 0 для отмены: ", 0, 1);
    return (choice == 1);
}

int Menu::PaginationState::getTotalPages(int totalItems) const {
    int pages = (totalItems + perPage - 1) / perPage;
    return pages == 0 ? 1 : pages;
}

void Menu::PaginationState::nextPage(int totalItems) {
    int tp = getTotalPages(totalItems);
    if (currentPage < tp) currentPage++;
}

void Menu::PaginationState::prevPage() {
    if (currentPage > 1) currentPage--;
}

bool Menu::PaginationState::goToPage(int page, int totalItems) {
    int tp = getTotalPages(totalItems);
    if (tp > 1 && page >= 1 && page <= tp) {
        currentPage = page;
        return true;
    }
    return false;
}

void Menu::PaginationState::clamp(int totalItems) {
    int tp = getTotalPages(totalItems);
    if (currentPage > tp) currentPage = tp;
    if (currentPage < 1) currentPage = 1;
}
