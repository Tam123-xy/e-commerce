#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <algorithm>
using namespace std;

struct Product {
    string ID;
    string ProductName;
    string SuitableAges;
    string SuitableGender;
    string Category;
    double Price;
    string SellerName;
    string ProductDescription;
};

vector<Product> loadProducts(const string &filename) {
    vector<Product> products;
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Error opening file: " << filename << endl;
        return products;
    }

    string line;
    while (getline(file, line)) {
        if (line.empty()) continue;

        stringstream ss(line);
        Product p;
        string priceStr;

        getline(ss, p.ID, ',');
        getline(ss, p.ProductName, ',');
        getline(ss, p.SuitableAges, ',');
        getline(ss, p.SuitableGender, ',');
        getline(ss, p.Category, ',');
        getline(ss, priceStr, ',');
        getline(ss, p.SellerName, ',');
        getline(ss, p.ProductDescription, '\n');

        try {
            p.Price = stod(priceStr);
        } catch (...) {
            p.Price = 0.0;
        }

        products.push_back(p);
    }
    return products;
}

void searchByKeyword(const vector<Product> &products, const string &keyword) {
    bool found = false;
    for (const auto &p : products) {
        string lowerName = p.ProductName;
        string lowerDesc = p.ProductDescription;

        // Convert both to lowercase for case-insensitive search
        transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);
        transform(lowerDesc.begin(), lowerDesc.end(), lowerDesc.begin(), ::tolower);
        string lowerKey = keyword;
        transform(lowerKey.begin(), lowerKey.end(), lowerKey.begin(), ::tolower);

        if (lowerName.find(lowerKey) != string::npos || lowerDesc.find(lowerKey) != string::npos) {
            found = true;
            cout << "\n--- Product Found ---\n";
            cout << "Name: " << p.ProductName << "\n";
            cout << "Suitable Ages: " << p.SuitableAges << "\n";
            cout << "Suitable Gender: " << p.SuitableGender << "\n";
            cout << "Category: " << p.Category << "\n";
            cout << "Price: $" << p.Price << "\n";
            cout << "Seller: " << p.SellerName << "\n";
            cout << "Description: " << p.ProductDescription << "\n";
            cout << "----------------------\n";
        }
    }
    if (!found) {
        cout << "No products found matching keyword: " << keyword << "\n";
    }
}

int main() {
    vector<Product> products = loadProducts("data.txt");
    if (products.empty()) {
        cout << "No products loaded. Exiting...\n";
        return 0;
    }

    int choice;
    do {
        cout << "\n=== Product Recommendation System ===\n";
        cout << "1. Browse by Keyword\n";
        cout << "4. Exit\n";
        cout << "Enter choice: ";
        cin >> choice;
        cin.ignore(); // flush newline

        if (choice == 1) {
            string keyword;
            cout << "Enter keyword: ";
            getline(cin, keyword);
            searchByKeyword(products, keyword);
        } else if (choice == 4) {
            cout << "Exiting system. Goodbye!\n";
        } else {
            cout << "Invalid choice. Try again.\n";
        }
    } while (choice != 4);

    return 0;
}
