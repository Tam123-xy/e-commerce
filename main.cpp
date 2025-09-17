#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <map>

struct Product {
    int id;
    std::string name;
    std::string category;
    double price;
    double rating;
    std::vector<std::string> tags;

    void display() const {
        std::cout << "ID: " << id << ", Name: " << name
                  << ", Category: " << category << ", Price: $"
                  << std::fixed << std::setprecision(2) << price
                  << ", Rating: " << rating << std::endl;
    }
};

class ProductDatabase {
private:
    std::vector<Product> products;

public:
    bool loadFromFile(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cout << "Error: Could not open file " << filename << std::endl;
            return false;
        }

        std::string line;
        while (std::getline(file, line)) {
            if (line.empty() || line[0] == '#') continue;

            std::istringstream iss(line);
            std::string token;
            std::vector<std::string> tokens;

            while (std::getline(iss, token, ',')) {
                tokens.push_back(token);
            }

            if (tokens.size() >= 5) {
                try {
                    int id = std::stoi(tokens[0]);
                    std::string name = tokens[1];
                    std::string category = tokens[2];
                    double price = std::stod(tokens[3]);
                    double rating = std::stod(tokens[4]);

                    std::vector<std::string> tags;
                    for (size_t i = 5; i < tokens.size(); i++) {
                        tags.push_back(tokens[i]);
                    }

                    products.push_back({id, name, category, price, rating, tags});
                } catch (...) {
                    // Skip invalid lines
                }
            }
        }

        file.close();
        return true;
    }

    void addSampleData() {
        products.push_back({1, "iPhone 15", "Electronics", 999.99, 4.8, {"smartphone", "apple"}});
        products.push_back({2, "Samsung Galaxy", "Electronics", 899.99, 4.7, {"smartphone", "android"}});
        products.push_back({3, "MacBook Pro", "Electronics", 1999.99, 4.9, {"laptop", "apple"}});
        products.push_back({4, "Nike Air Max", "Clothing", 129.99, 4.5, {"shoes", "sports"}});
        products.push_back({5, "Adidas Ultraboost", "Clothing", 149.99, 4.6, {"shoes", "running"}});
    }

    std::vector<Product> searchProducts(const std::string& query) const {
        std::vector<Product> result;
        std::string lowerQuery = toLower(query);

        for (const auto& product : products) {
            std::string lowerName = toLower(product.name);
            std::string lowerCategory = toLower(product.category);

            // Search in name, category, and tags
            if (lowerName.find(lowerQuery) != std::string::npos ||
                lowerCategory.find(lowerQuery) != std::string::npos ||
                hasTag(product, lowerQuery)) {
                result.push_back(product);
            }
        }
        return result;
    }

    std::vector<Product> getAllProducts() const {
        return products;
    }

    std::vector<std::string> getAllCategories() const {
        std::vector<std::string> categories;
        for (const auto& product : products) {
            if (std::find(categories.begin(), categories.end(), product.category) == categories.end()) {
                categories.push_back(product.category);
            }
        }
        return categories;
    }

    std::vector<Product> getProductsByCategory(const std::string& category) const {
        std::vector<Product> result;
        for (const auto& product : products) {
            if (product.category == category) {
                result.push_back(product);
            }
        }
        return result;
    }

    void displayMatchingProducts(const std::string& query) const {
        auto matches = searchProducts(query);

        if (matches.empty()) {
            std::cout << "No products found matching: " << query << std::endl;
        } else {
            std::cout << "\nProducts matching '" << query << "':" << std::endl;
            for (const auto& product : matches) {
                product.display();
            }
        }
    }

private:
    std::string toLower(const std::string& str) const {
        std::string result = str;
        std::transform(result.begin(), result.end(), result.begin(), ::tolower);
        return result;
    }

    bool hasTag(const Product& product, const std::string& searchTag) const {
        std::string lowerSearchTag = toLower(searchTag);
        for (const auto& tag : product.tags) {
            if (toLower(tag).find(lowerSearchTag) != std::string::npos) {
                return true;
            }
        }
        return false;
    }
};

class SuggestionEngine {
private:
    ProductDatabase& database;

    double calculateSimilarity(const Product& a, const Product& b) {
        double similarity = 0.0;

        // Same category adds 0.5 to similarity
        if (a.category == b.category) {
            similarity += 0.5;
        }

        // Common tags add 0.1 each
        for (const auto& tag : a.tags) {
            if (std::find(b.tags.begin(), b.tags.end(), tag) != b.tags.end()) {
                similarity += 0.1;
            }
        }

        // Higher rating products get preference
        similarity += b.rating * 0.1;

        return similarity;
    }

public:
    SuggestionEngine(ProductDatabase& db) : database(db) {}

    std::vector<Product> suggestSimilarProducts(const Product& target) {
        std::vector<std::pair<double, Product>> scoredProducts;

        for (const auto& product : database.getAllProducts()) {
            if (product.id != target.id) {
                double score = calculateSimilarity(target, product);
                scoredProducts.push_back({score, product});
            }
        }

        // Sort by score (highest first)
        std::sort(scoredProducts.begin(), scoredProducts.end(),
                 [](const auto& a, const auto& b) {
                     return a.first > b.first;
                 });

        // Return top 5 suggestions
        std::vector<Product> suggestions;
        for (int i = 0; i < std::min(5, (int)scoredProducts.size()); i++) {
            suggestions.push_back(scoredProducts[i].second);
        }

        return suggestions;
    }

    std::vector<Product> suggestByPriceRange(double minPrice, double maxPrice) {
        std::vector<Product> result;

        for (const auto& product : database.getAllProducts()) {
            if (product.price >= minPrice && product.price <= maxPrice) {
                result.push_back(product);
            }
        }

        // Sort by rating (highest first)
        std::sort(result.begin(), result.end(),
                 [](const Product& a, const Product& b) {
                     return a.rating > b.rating;
                 });

        return result;
    }

    std::vector<Product> suggestTopRated() {
        auto products = database.getAllProducts();

        // Sort by rating (highest first)
        std::sort(products.begin(), products.end(),
                 [](const Product& a, const Product& b) {
                     return a.rating > b.rating;
                 });

        // Return top 5
        if (products.size() > 5) {
            products.resize(5);
        }

        return products;
    }

    std::vector<Product> suggestBasedOnSearch(const std::string& input) {
        auto matchingProducts = database.searchProducts(input);

        if (matchingProducts.empty()) {
            return {};
        }

        // Get suggestions based on all matching products, not just the first one
        std::vector<Product> allSuggestions;
        std::map<int, bool> seenProducts;

        // First, add the actual matching products
        for (const auto& product : matchingProducts) {
            if (seenProducts.find(product.id) == seenProducts.end()) {
                allSuggestions.push_back(product);
                seenProducts[product.id] = true;
            }
        }

        // Then add similar products for each match
        for (const auto& product : matchingProducts) {
            auto similar = suggestSimilarProducts(product);
            for (const auto& suggestion : similar) {
                if (seenProducts.find(suggestion.id) == seenProducts.end()) {
                    allSuggestions.push_back(suggestion);
                    seenProducts[suggestion.id] = true;
                }
            }
        }

        // Limit to 10 total suggestions
        if (allSuggestions.size() > 10) {
            allSuggestions.resize(10);
        }

        return allSuggestions;
    }
};

void displayMenu() {
    std::cout << "\n=== PRODUCT SUGGESTION SYSTEM ===" << std::endl;
    std::cout << "1. Search products" << std::endl;
    std::cout << "2. Browse categories" << std::endl;
    std::cout << "3. Filter by price" << std::endl;
    std::cout << "4. Top rated products" << std::endl;
    std::cout << "5. Exit" << std::endl;
    std::cout << "Choose an option (1-5): ";
}

void searchProducts(ProductDatabase& db, SuggestionEngine& engine) {
    std::string query;
    std::cout << "Enter search keyword: ";
    std::cin.ignore();
    std::getline(std::cin, query);

    // First show exact matches
    db.displayMatchingProducts(query);

    // Then show suggestions based on the search
    auto suggestions = engine.suggestBasedOnSearch(query);

    if (!suggestions.empty()) {
        std::cout << "\nYou might also like:" << std::endl;
        for (const auto& product : suggestions) {
            product.display();
        }
    }
}

void browseCategories(ProductDatabase& db, SuggestionEngine& engine) {
    auto categories = db.getAllCategories();
    std::cout << "\nAvailable categories:" << std::endl;
    for (size_t i = 0; i < categories.size(); i++) {
        std::cout << i + 1 << ". " << categories[i] << std::endl;
    }

    int choice;
    std::cout << "Choose a category (1-" << categories.size() << "): ";
    std::cin >> choice;

    if (choice < 1 || choice > categories.size()) {
        std::cout << "Invalid choice!" << std::endl;
        return;
    }

    std::string selectedCategory = categories[choice - 1];
    auto products = db.getProductsByCategory(selectedCategory);

    std::cout << "\nProducts in " << selectedCategory << ":" << std::endl;
    for (const auto& product : products) {
        product.display();
    }
}

void filterByPrice(SuggestionEngine& engine) {
    double minPrice, maxPrice;
    std::cout << "Enter minimum price: $";
    std::cin >> minPrice;
    std::cout << "Enter maximum price: $";
    std::cin >> maxPrice;

    auto products = engine.suggestByPriceRange(minPrice, maxPrice);

    if (products.empty()) {
        std::cout << "No products found between $" << minPrice << " and $" << maxPrice << std::endl;
    } else {
        std::cout << "\nProducts in your price range:" << std::endl;
        for (const auto& product : products) {
            product.display();
        }
    }
}

void showTopRated(SuggestionEngine& engine) {
    auto products = engine.suggestTopRated();

    std::cout << "\nTop rated products:" << std::endl;
    for (const auto& product : products) {
        product.display();
    }
}

int main() {
    ProductDatabase db;
    std::string filename = "data.txt";

    std::cout << "Loading product data..." << std::endl;

    if (!db.loadFromFile(filename)) {
        std::cout << "Using sample data..." << std::endl;
        db.addSampleData();
    }

    std::cout << "Database loaded with " << db.getAllProducts().size() << " products!" << std::endl;

    SuggestionEngine engine(db);

    int choice;
    do {
        displayMenu();
        std::cin >> choice;

        switch (choice) {
            case 1:
                searchProducts(db, engine);
                break;
            case 2:
                browseCategories(db, engine);
                break;
            case 3:
                filterByPrice(engine);
                break;
            case 4:
                showTopRated(engine);
                break;
            case 5:
                std::cout << "Thank you for using the Product Suggestion System!" << std::endl;
                break;
            default:
                std::cout << "Please choose 1-5" << std::endl;
        }

        // Pause before showing menu again
        if (choice != 5) {
            std::cout << "\nPress Enter to continue...";
            std::cin.ignore();
            std::cin.get();
        }

    } while (choice != 5);

    return 0;
}
