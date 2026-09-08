#pragma once
#include <iostream>
#include <vector>
#include <memory>
#include <algorithm>
#include "User.h"
#include "Product.h"
#include "Store.h"
#include "Wishlist.h"
#include "RecommendationSystem.h"
#include "Report.h"
#include "Notification.h"
#include "InputValidation.h"
#include "TableFormatter.h"
#include "Logger.h"
#include "Exceptions.h"

class Menu 
{
protected:
    Store& store;
    Wishlist& wishlist;
    RecommendationSystem& recommendationSystem;
    Report& report;
    NotificationSystem& notificationSystem;
    std::shared_ptr<User> currentUser;
    Logger* logger;

    struct PaginationState {
        int currentPage = 1;
        int perPage = 5;

        int getTotalPages(int totalItems) const;
        void nextPage(int totalItems);
        void prevPage();
        bool goToPage(int page, int totalItems);
        void clamp(int totalItems);
    };

public:
    Menu(Store& s, Wishlist& w, RecommendationSystem& rs, Report& r, NotificationSystem& ns);
    virtual void show() = 0;
    virtual ~Menu() = default;

protected:
    void displayHeader(const std::string& title);
    bool confirmAction(const std::string& message);
};
