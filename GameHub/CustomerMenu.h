#pragma once
#include "Menu.h"

class CustomerMenu : public Menu 
{
public:
    CustomerMenu(Store& s, Wishlist& w, RecommendationSystem& rs, Report& r,
                 NotificationSystem& ns, std::shared_ptr<User> user);
    void show() override;

private:
    PaginationState pagination;

    // Limited menu (unapproved users)
    void showLimitedMenu();
    void viewCatalogLimited();
    void viewProductDetailsLimited();
    void searchGamesLimited();

    // Full menu
    void viewCatalog();
    void searchGames();
    void viewProductDetails();
    void viewProductDetailsFromSearch(const std::vector<std::shared_ptr<Product>>& searchResults);
    void addToWishlistFromCatalog();
    void addToWishlistFromSearch();
    void addToWishlistFromRecommendations(const std::vector<std::shared_ptr<Product>>& recommendations);
    void purchaseFromCatalog();
    void purchaseFromSearch(const std::vector<std::shared_ptr<Product>>& searchResults);
    void purchaseFromRecommendations(const std::vector<std::shared_ptr<Product>>& recommendations);
    void purchaseProduct(std::shared_ptr<Product> product);
    void viewPurchaseHistory();
    void viewSaleDetails(const std::vector<Sale>& userSales);
    void manageWishlist();
    void viewRecommendations();
    void viewProfile();
    void changeEmail();
    void changePhone();
    void changePassword();
    void viewNotifications();
};
