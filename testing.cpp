#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <sstream>
#include <iomanip>

// Product structure
struct Product {
    int id;
    std::string name;
    std::string category;
    double price;
    double rating;
    std::string tag1;
    std::string tag2;
    
    // Calculate recommendation score (rating * category weight)
    double calculateScore(double categoryWeight) const {
        return rating * categoryWeight;
    }
};

// Preference weight structure
struct Preference {
    std::string ageRange;
    std::string gender;
    std::string category;
    double weight;
};

class RecommendationSystem {
private:
    std::vector<Product> products;
    std::vector<Preference> preferences;
    
    // String split function
    std::vector<std::string> split(const std::string& str, char delimiter) {
        std::vector<std::string> tokens;
        std::stringstream ss(str);
        std::string token;
        while (std::getline(ss, token, delimiter)) {
            tokens.push_back(token);
        }
        return tokens;
    }
    
    // Trim whitespace from string
    std::string trim(const std::string& str) {
        size_t first = str.find_first_not_of(' ');
        if (first == std::string::npos) return "";
        size_t last = str.find_last_not_of(' ');
        return str.substr(first, (last - first + 1));
    }
    
    // Convert string to uppercase
    std::string toUpper(const std::string& str) {
        std::string result = str;
        std::transform(result.begin(), result.end(), result.begin(), ::toupper);
        return result;
    }
    

public:
    // Load data file
    bool loadData(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cout << "Unable to open file: " << filename << std::endl;
            return false;
        }
        
        std::string line;
        bool inPreferences = false;
        bool inProducts = false;
        bool skipHeader = false;
        
        while (std::getline(file, line)) {
            line = trim(line);
            
            // Skip empty lines and comments
            if (line.empty() || line[0] == '#') continue;
            
            // Check section identifiers
            if (line == "[PREFERENCES]") {
                inPreferences = true;
                inProducts = false;
                skipHeader = true;
                continue;
            } else if (line == "[PRODUCTS]") {
                inProducts = true;
                inPreferences = false;
                skipHeader = true;
                continue;
            }
            
            // Skip header row
            if (skipHeader) {
                skipHeader = false;
                continue;
            }
            
            // Parse preference data
            if (inPreferences) {
                std::vector<std::string> tokens = split(line, ',');
                if (tokens.size() >= 4) {
                    Preference pref;
                    pref.ageRange = trim(tokens[0]);
                    pref.gender = trim(tokens[1]);
                    pref.category = trim(tokens[2]);
                    pref.weight = std::stod(trim(tokens[3]));
                    preferences.push_back(pref);
                }
            }
            // Parse product data
            else if (inProducts) {
                std::vector<std::string> tokens = split(line, ',');
                if (tokens.size() >= 7) {
                    Product product;
                    product.id = std::stoi(trim(tokens[0]));
                    product.name = trim(tokens[1]);
                    product.category = trim(tokens[2]);
                    product.price = std::stod(trim(tokens[3]));
                    product.rating = std::stod(trim(tokens[4]));
                    product.tag1 = trim(tokens[5]);
                    product.tag2 = trim(tokens[6]);
                    products.push_back(product);
                }
            }
        }
        
        file.close();
        std::cout << "Data loaded: " << preferences.size() << " preferences, " 
                  << products.size() << " products" << std::endl;
        return true;
    }
    
    // Get category weight based on age and gender
    double getCategoryWeight(const std::string& ageRange, const std::string& gender, 
                           const std::string& category) {
        for (const auto& pref : preferences) {
            if (pref.ageRange == ageRange && 
                pref.gender == toUpper(gender) && 
                pref.category == toUpper(category)) {
                    return pref.weight;
                }
        }
        return 0.1; // Default weight
    }
    
    // Get all categories
    std::vector<std::string> getAllCategories() {
        std::vector<std::string> categories;
        for (const auto& product : products) {
            if (std::find(categories.begin(), categories.end(), product.category) == categories.end()) {
                categories.push_back(product.category);
            }
        }
        return categories;
    }
    
    // Mode 1: Direct recommendation based on age and gender
    std::vector<Product> recommendByDemographics(const std::string& ageRange, 
                                                const std::string& gender, 
                                                int count = 3) {
        std::vector<std::pair<Product, double>> scoredProducts;
        
        // Calculate recommendation score for each product
        for (const auto& product : products) {
            double weight = getCategoryWeight(ageRange, gender, product.category);
            double score = product.calculateScore(weight);
            scoredProducts.push_back({product, score});
        }
        
        // Sort by score
        std::sort(scoredProducts.begin(), scoredProducts.end(), 
                 [](const auto& a, const auto& b) { return a.second > b.second; });
        
        // Return top count products
        std::vector<Product> result;
        for (int i = 0; i < std::min(count, (int)scoredProducts.size()); i++) {
            result.push_back(scoredProducts[i].first);
        }
        return result;
    }
    
    // Mode 2: Category selection + additional recommendation
    std::pair<std::vector<Product>, std::vector<Product>> 
    recommendByCategory(const std::string& category, const std::string& ageRange, 
                       const std::string& gender, int categoryCount = 3, int extraCount = 2) {
        
        // Category recommendations: sort by rating
        std::vector<Product> categoryProducts;
        for (const auto& product : products) {
            if (product.category == category) {
                categoryProducts.push_back(product);
            }
        }
        std::sort(categoryProducts.begin(), categoryProducts.end(), 
                 [](const auto& a, const auto& b) { return a.rating > b.rating; });
        
        std::vector<Product> categoryRecommendations;
        for (int i = 0; i < std::min(categoryCount, (int)categoryProducts.size()); i++) {
            categoryRecommendations.push_back(categoryProducts[i]);
        }
        
        // Additional recommendations: based on age and gender, excluding selected category
        std::vector<std::pair<Product, double>> scoredProducts;
        for (const auto& product : products) {
            if (product.category != category) {
                double weight = getCategoryWeight(ageRange, gender, product.category);
                double score = product.calculateScore(weight);
                scoredProducts.push_back({product, score});
            }
        }
        
        std::sort(scoredProducts.begin(), scoredProducts.end(), 
                 [](const auto& a, const auto& b) { return a.second > b.second; });
        
        std::vector<Product> extraRecommendations;
        for (int i = 0; i < std::min(extraCount, (int)scoredProducts.size()); i++) {
            extraRecommendations.push_back(scoredProducts[i].first);
        }
        
        return {categoryRecommendations, extraRecommendations};
    }
    
    // Mode 3: Keyword filtering + additional recommendation
    std::pair<std::vector<Product>, std::vector<Product>> 
    recommendByKeyword(const std::string& keyword, const std::string& ageRange, 
                      const std::string& gender, int keywordCount = 3, int extraCount = 2) {
        
        // Keyword filtering: match name and tags
        std::vector<Product> keywordProducts;
        std::string upperKeyword = toUpper(keyword);
        
        for (const auto& product : products) {
            if (toUpper(product.name).find(upperKeyword) != std::string::npos ||
                toUpper(product.tag1).find(upperKeyword) != std::string::npos ||
                toUpper(product.tag2).find(upperKeyword) != std::string::npos) {
                keywordProducts.push_back(product);
            }
        }
        
        // Sort by rating
        std::sort(keywordProducts.begin(), keywordProducts.end(), 
                 [](const auto& a, const auto& b) { return a.rating > b.rating; });
        
        std::vector<Product> keywordRecommendations;
        for (int i = 0; i < std::min(keywordCount, (int)keywordProducts.size()); i++) {
            keywordRecommendations.push_back(keywordProducts[i]);
        }
        
        // Additional recommendations: based on age and gender
        std::vector<Product> extraRecommendations = 
            recommendByDemographics(ageRange, gender, extraCount);
        
        return {keywordRecommendations, extraRecommendations};
    }
    
    // Display product information
    void displayProducts(const std::vector<Product>& products, const std::string& title) {
        std::cout << "\n=== " << title << " ===" << std::endl;
        std::cout << std::setw(5) << "ID" << std::setw(25) << "Product Name" 
                  << std::setw(15) << "Category" << std::setw(10) << "Price" 
                  << std::setw(8) << "Rating" << std::setw(15) << "Tags" << std::endl;
        std::cout << std::string(82, '-') << std::endl;
        
        for (const auto& product : products) {
            std::cout << std::setw(5) << product.id 
                      << std::setw(25) << product.name.substr(0, 22)
                      << std::setw(15) << product.category
                      << std::setw(10) << std::fixed << std::setprecision(2) << product.price
                      << std::setw(8) << std::setprecision(1) << product.rating
                      << std::setw(15) << (product.tag1 + "," + product.tag2).substr(0, 12)
                      << std::endl;
        }
    }
        std::vector<Product> getTopRated(int count = 5) {
        std::vector<Product> sortedProducts = products;
        std::sort(sortedProducts.begin(), sortedProducts.end(),
                [](const Product& a, const Product& b) {
                    return a.rating > b.rating;
                });
        if (sortedProducts.size() > count) {
            sortedProducts.resize(count);
        }
        return sortedProducts;
    }

    std::vector<Product> filterByPrice(double minPrice, double maxPrice) {
        std::vector<Product> result;
        for (const auto& product : products) {
            if (product.price >= minPrice && product.price <= maxPrice) {
                result.push_back(product);
            }
        }
        std::sort(result.begin(), result.end(),
                [](const Product& a, const Product& b) {
                    return a.rating > b.rating;
                });
        return result;
    }

};

int main() {
    RecommendationSystem system;
    
    // Load data file
    if (!system.loadData("data.txt")) {
        return 1;
    }
    
    std::cout << "\n=== Product Recommendation System ===" << std::endl;
    
    // Get user input
    std::string ageRange, gender;
    int ageInput;
    std::cout << "\nPlease enter your age: ";
    std::cin >> ageInput;

    if (ageInput < 18) {
        std::cout << "Age too low, using default 18-24" << std::endl;
        ageRange = "18-24";
    } else if (ageInput <= 24) {
        ageRange = "18-24";
    } else if (ageInput <= 34) {
        ageRange = "25-34";
    } else if (ageInput <= 44) {
        ageRange = "35-44";
    } else if (ageInput <= 54) {
        ageRange = "45-54";
    } else if (ageInput <= 64) {
        ageRange = "55-64";
    } else {
        ageRange = "65+";
    }
    
    std::cout << "\nPlease select gender (M/F): ";
    std::cin >> gender;
    if (gender != "M" && gender != "F" && gender != "m" && gender != "f") {
        std::cout << "Invalid selection, using default value M" << std::endl;
        gender = "M";
    }

    int mode;
    do {
        std::cout << "\nPlease select recommendation mode:" << std::endl;
        std::cout << "1. Direct recommendation" << std::endl;
        std::cout << "2. Browse categories" << std::endl;
        std::cout << "3. Search products" << std::endl;
        std::cout << "4. Top rated products" << std::endl;
        std::cout << "5. Filter by price range" << std::endl;
        std::cout << "6. Exit" << std::endl;
        std::cin >> mode;
        
        switch (mode) {
            case 1: {
                auto recommendations = system.recommendByDemographics(ageRange, gender, 5);
                system.displayProducts(recommendations, "Recommendations Based on Demographics");
                break;
            }
            case 2: {
                auto categories = system.getAllCategories();
                std::cout << "\nAvailable categories:" << std::endl;
                for (int i = 0; i < categories.size(); i++) {
                    std::cout << i + 1 << ". " << categories[i] << std::endl;
                }
                int categoryChoice;
                std::cout << "Please select category (1-" << categories.size() << "): ";
                std::cin >> categoryChoice;
                if (categoryChoice >= 1 && categoryChoice <= categories.size()) {
                    std::string selectedCategory = categories[categoryChoice - 1];
                    auto result = system.recommendByCategory(selectedCategory, ageRange, gender);
                    system.displayProducts(result.first, selectedCategory + " Category Recommendations");
                    system.displayProducts(result.second, "Additional Recommendations for You");
                } else {
                    std::cout << "Invalid selection!" << std::endl;
                }
                break;
            }
            case 3: {
                std::string keyword;
                std::cout << "Please enter keyword: ";
                std::cin >> keyword;
                auto result = system.recommendByKeyword(keyword, ageRange, gender);
                if (result.first.empty()) {
                    std::cout << "No products found matching \"" << keyword << "\"" << std::endl;
                } else {
                    system.displayProducts(result.first, "Keyword \"" + keyword + "\" Search Results");
                }
                system.displayProducts(result.second, "Additional Recommendations for You");
                break;
            }
            case 4: {
                auto topRated = system.getTopRated(5);
                system.displayProducts(topRated, "Top Rated Products");
                break;
            }
            case 5: {
                double minPrice, maxPrice;
                std::cout << "Enter minimum price: ";
                std::cin >> minPrice;
                std::cout << "Enter maximum price: ";
                std::cin >> maxPrice;
                auto filtered = system.filterByPrice(minPrice, maxPrice);
                if (filtered.empty()) {
                    std::cout << "No products found in this price range." << std::endl;
                } else {
                    system.displayProducts(filtered, "Products in Price Range");
                }
                break;
            }
            case 6:
                std::cout << "Exiting... Goodbye!" << std::endl;
                break;
            default:
                std::cout << "Invalid choice!" << std::endl;
        }
        if (mode != 6) {
            std::cout << "\nPress Enter to continue...";
            std::cin.ignore();
            std::cin.get();
        }
    } while (mode != 6);

    return 0;
}
