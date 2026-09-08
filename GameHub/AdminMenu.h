#pragma once
#include "Menu.h"

class AdminMenu : public Menu 
{
public:
    AdminMenu(Store& s, Wishlist& w, RecommendationSystem& rs, Report& r,
              NotificationSystem& ns, std::shared_ptr<User> user);
    void show() override;

private:
    PaginationState pagination;

    // User management
    void manageUsers();
    void approveUser();
    void blockUser();
    void unblockUser();
    void changeUserRole();
    void searchAndFilterUsers();
    void clearUnapprovedUsers();
    std::vector<std::shared_ptr<User>> filterUsers(
        const std::vector<std::shared_ptr<User>>& users,
        const std::string& searchType, const std::string& searchValue);

    // Product management
    void manageProducts();
    void addNewProduct();
    void editProduct();
    void deleteProduct();
    void addKeysToProduct();
    void addNewGenre();
    void deleteGenre();
    void addNewTag();
    void deleteTag();
    void searchAndFilterProducts();
    std::vector<std::shared_ptr<Product>> filterProducts(
        const std::vector<std::shared_ptr<Product>>& products,
        const std::string& searchType, const std::string& searchValue);

    // Discount management
    void manageDiscounts();
    void addNewDiscount();
    void editDiscount();
    void deleteDiscount();
    void viewDiscountedProducts();
    void selectProductsForDiscount(std::vector<std::string>& targets);
    void selectGenresForDiscount(std::vector<std::string>& targets);
    void selectTagsForDiscount(std::vector<std::string>& targets);
    void selectDevelopersForDiscount(std::vector<std::string>& targets);

    // Reports, logs, profile, notifications
    void viewReports();
    void viewLogs();
    void changeMyProfile();
    void changeAdminUsername();
    void changeAdminEmail();
    void changeAdminPhone();
    void changeAdminPassword();
    void viewNotifications();
};
