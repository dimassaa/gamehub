# GameHub

A console-based computer game store management system built in C++ for Windows (MSVC).

## Features

### Customer
- Browse game catalog with filtering and search
- Purchase games (activation keys)
- Wishlist with discount and stock notifications
- Personalized recommendations based on purchase history
- Purchase history and profile management

### Administrator
- User management (approve, block, change roles)
- Product management (CRUD, genres, tags, activation keys)
- Discount management (by product, genre, tag, developer)
- Reporting (sales, financial, inventory)
- System log viewer

## Architecture

Three-tier architecture:

| Layer | Files | Responsibility |
|-------|-------|----------------|
| **Presentation** | `Menu`, `TableFormatter` | UI, input/output |
| **Business Logic** | `Store`, `RecommendationSystem`, `Report`, `Wishlist`, `Notification` | Rules, data processing |
| **Data Layer** | `FileManager`, `Config` | Serialization, storage |

### File Structure

```
GameHub/
├── GameHub.cpp                    # Entry point, first admin setup
├── Menu.h / Menu.cpp              # Base menu class (PaginationState, confirmAction)
├── MainMenu.h / .cpp              # Main menu (login, registration)
├── AdminMenu.h / .cpp             # Admin panel
├── CustomerMenu.h / .cpp          # Customer interface
├── Store.h / .cpp                 # Store: products, discounts, sales
├── User.h / .cpp                  # Users (Admin, Customer)
├── Product.h / .cpp               # Product (Game)
├── Sale.h / .cpp                  # Sales
├── Wishlist.h / .cpp              # Wishlists
├── RecommendationSystem.h / .cpp  # Recommendations
├── Report.h / .cpp                # Reports
├── Notification.h / .cpp          # Notifications
├── TableFormatter.h / .cpp        # Table formatting
├── InputValidation.h / .cpp       # Input validation
├── FileManager.h / .cpp           # File serialization
├── Config.h / .cpp                # Configuration, colors, paths
├── Logger.h / .cpp                # Logging
└── Exceptions.h                   # Custom exceptions
```

## Requirements

- Windows 10+
- Visual Studio 2022 (MSVC v143)
- Windows SDK 10.0
- Encoding: CP1251

## Building & Running

1. Open `GameHub.sln` in Visual Studio
2. Select configuration (Debug/Release) and platform (x64/Win32)
3. Press `Ctrl+F5` to run

On first launch the system prompts to create an administrator account.

## Data Files

| File | Contents |
|------|----------|
| `users.dat` | Users |
| `products.dat` | Products |
| `sales.dat` | Sales |
| `discounts.dat` | Discounts |
| `wishlists.dat` | Wishlists |
| `system.log` | System logs |
