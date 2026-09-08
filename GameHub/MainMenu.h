#pragma once
#include "Menu.h"

class MainMenu : public Menu 
{
public:
    MainMenu(Store& s, Wishlist& w, RecommendationSystem& rs, Report& r, NotificationSystem& ns);
    void show() override;

private:
    bool login();
    void registerUser();
    void recoverPassword();
    void showUserMenu();
    void aboutSystem();
};
