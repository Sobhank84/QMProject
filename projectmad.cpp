#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>
#include <set>
#include <map>
using namespace std;

// تابع تبدیل عدد باینری به گری
int binaryToGray(int n) {
    return n ^ (n >> 1);
}

// تابع تبدیل عدد گری به باینری
int grayToBinary(int gray) {
    int bin = 0;
    for (; gray; gray >>= 1)
        bin ^= gray;
    return bin;
}

// گرفتن ایندکس جدول کارنو با استفاده از گری‌کدها
int getKMapIndex(int row, int col, int rows, int cols) {
    return binaryToGray(row) * cols + binaryToGray(col);
}

// تولید ترکیب متغیرها برای یک ایندکس مشخص (برای چاپ سطر یا ستون)
string varsForIndex(int index, int bits, int startVar) {
    string res = "";
    for (int i = bits - 1; i >= 0; --i) {
        char var = 'A' + startVar + (bits - 1 - i);
        if ((index & (1 << i)) == 0)
            res += var + string("'");
        else
            res += var;
    }
    return res;
}

// گرفتن شماره مینترم با توجه به مقدار گری سطر و ستون
int getMintermFromRowCol(int row, int col, int rowBits, int colBits) {
    int rowBin = grayToBinary(row);
    int colBin = grayToBinary(col);
    return (rowBin << colBits) | colBin;
}

// بررسی اینکه آیا یک گروه (زیرماتریس) معتبر است یا نه
bool isValidGroup(const vector<int>& kmap, int rows, int cols,
                  int r, int c, int h, int w, vector<vector<bool>>& visited) {
    for (int i = 0; i < h; ++i) {
        int row = (r + i) % rows;
        for (int j = 0; j < w; ++j) {
            int col = (c + j) % cols;
            int idx = getKMapIndex(row, col, rows, cols);
            if (kmap[idx] != 1 || visited[row][col]) return false;
        }
    }
    return true;
}

// علامت‌گذاری خانه‌های یک گروه به عنوان بازدید شده
void markGroup(vector<vector<bool>>& visited, int r, int c, int h, int w, int rows, int cols) {
    for (int i = 0; i < h; ++i) {
        int row = (r + i) % rows;
        for (int j = 0; j < w; ++j) {
            int col = (c + j) % cols;
            visited[row][col] = true;
        }
    }
}

// استخراج عبارت بولی معادل یک گروه خاص
string extractTerm(int r, int c, int h, int w, int rowBits, int colBits, int rows, int cols, int startVarRow, int startVarCol) {
    set<int> minterms;
    for (int i = 0; i < h; ++i) {
        int row = (r + i) % rows;
        for (int j = 0; j < w; ++j) {
            int col = (c + j) % cols;
            minterms.insert(getMintermFromRowCol(row, col, rowBits, colBits));
        }
    }

    int totalBits = rowBits + colBits;
    int andAll = (1 << totalBits) - 1;
    int orAll = 0;

    for (int m : minterms) {
        andAll &= m;
        orAll |= m;
    }

    string res = "";
    for (int i = totalBits - 1; i >= 0; --i) {
        char var;
        if (i >= colBits)
            var = 'A' + startVarRow + (totalBits - 1 - i);
        else
            var = 'A' + startVarCol + (colBits - 1 - i);

        int bit = 1 << i;
        if ((andAll & bit) == (orAll & bit)) {
            if ((andAll & bit) == 0)
                res += var + string("'");
            else
                res += var;
        }
    }
    return res;
}

// ساده‌سازی جدول کارنو برای 2 تا 4 متغیر
string simplifyKMap(const vector<int>& kmap, int rows, int cols, int rowBits, int colBits, int startVarRow, int startVarCol) {
    vector<vector<bool>> visited(rows, vector<bool>(cols, false));
    vector<string> terms;

    // اولویت بررسی گروه‌ها از بزرگ به کوچک
    vector<pair<int, int>> groupSizes = {{4,2}, {2,4}, {4,1}, {1,4}, {2,2}, {2,1}, {1,2}, {1,1}};

    for (auto [h, w] : groupSizes) {
        for (int r = 0; r < rows; ++r) {
            for (int c = 0; c < cols; ++c) {
                if (h <= rows && w <= cols && isValidGroup(kmap, rows, cols, r, c, h, w, visited)) {
                    markGroup(visited, r, c, h, w, rows, cols);
                    string term = extractTerm(r, c, h, w, rowBits, colBits, rows, cols, startVarRow, startVarCol);
                    if(!term.empty())
                        terms.push_back(term);
                }
            }
        }
    }

    set<string> uniqueTerms(terms.begin(), terms.end());
    string result = "";
    for (auto it = uniqueTerms.begin(); it != uniqueTerms.end(); ++it) {
        if (it != uniqueTerms.begin()) result += " + ";
        result += *it;
    }
    return result;
}

// ساده‌سازی جدول کارنو برای 5 متغیر با تقسیم به دو بخش E=0 و E=1
string simplify5Var(const vector<int>& kmap) {
    vector<int> part0(16,0), part1(16,0);
    for (int i=0; i<16; i++) {
        part0[i] = kmap[i];
        part1[i] = kmap[i+16];
    }

    string expr0 = simplifyKMap(part0, 4, 4, 2, 2, 0, 2);
    string expr1 = simplifyKMap(part1, 4, 4, 2, 2, 0, 2);

    if(expr0 == expr1) {
        if(expr0.empty()) return "0";
        return expr0;
    }
    else {
        string res = "";
        if(!expr0.empty()) res += expr0 + "E'";
        if(!expr1.empty()) {
            if(!res.empty()) res += " + ";
            res += expr1 + "E";
        }
        if(res.empty()) return "0";
        return res;
    }
}

// چاپ جدول کارنو 1 لایه (برای 2 تا 4 متغیر یا لایه‌های 5 متغیره)
void printSingleKMap(const vector<int>& kMapValues, int rows, int cols, int rowBits, int colBits, int startVarRow, int startVarCol) {
    cout << "      ";
    for (int col = 0; col < cols; ++col) {
        int grayCol = binaryToGray(col);
        cout << setw(6) << varsForIndex(grayCol, colBits, startVarCol);
    }
    cout << endl;

    cout << "     +" << string(cols * 6, '-') << endl;

    for (int row = 0; row < rows; ++row) {
        int grayRow = binaryToGray(row);
        cout << setw(5) << varsForIndex(grayRow, rowBits, startVarRow) << "|";

        for (int col = 0; col < cols; ++col) {
            int idx = getKMapIndex(row, col, rows, cols);
            cout << setw(6) << kMapValues[idx];
        }
        cout << endl;
    }
}

// چاپ جدول کارنو کلی (برای تمام متغیرها)
void printKarnaughMap(const vector<int>& kMapValues, int numVariables) {
    if (numVariables == 5) {
        // نمایش دو لایه برای 5 متغیر
        cout << "\nKarnaugh Map for E=0:\n";
        vector<int> part0(16, 0);
        vector<int> part1(16, 0);
        for (int i = 0; i < 16; ++i) {
            part0[i] = kMapValues[i];
            part1[i] = kMapValues[i + 16];
        }
        printSingleKMap(part0, 4, 4, 2, 2, 0, 2);
        cout << "\nKarnaugh Map for E=1:\n";
        printSingleKMap(part1, 4, 4, 2, 2, 0, 2);

        string simplified = simplify5Var(kMapValues);
        cout << "\nSimplified SOP Expression: " << simplified << endl;
    }
    else {
        int rows, cols, rowVarsStart, colVarsStart;
        int rowBits, colBits;

        // تنظیم تعداد سطر و ستون و موقعیت متغیرها بر اساس تعداد متغیر
        if (numVariables == 2) {
            rows = 2; cols = 2; rowVarsStart = 0; colVarsStart = 1;
            rowBits = 1; colBits = 1;
        }
        else if (numVariables == 3) {
            rows = 2; cols = 4; rowVarsStart = 0; colVarsStart = 1;
            rowBits = 1; colBits = 2;
        }
        else if (numVariables == 4) {
            rows = 4; cols = 4; rowVarsStart = 0; colVarsStart = 2;
            rowBits = 2; colBits = 2;
        }
        else {
            rows = 1; cols = 1; rowVarsStart = 0; colVarsStart = 0;
            rowBits = 0; colBits = 0;
        }

        // چاپ جدول
        cout << "\nKarnaugh Map:\n      ";
        for (int col = 0; col < cols; ++col)
            cout << setw(6) << varsForIndex(binaryToGray(col), colBits, colVarsStart);
        cout << "\n     +" << string(cols * 6, '-') << "\n";

        for (int row = 0; row < rows; ++row) {
            cout << setw(5) << varsForIndex(binaryToGray(row), rowBits, rowVarsStart) << "|";
            for (int col = 0; col < cols; ++col)
                cout << setw(6) << kMapValues[getKMapIndex(row, col, rows, cols)];
            cout << endl;
        }

        string simplified = simplifyKMap(kMapValues, rows, cols, rowBits, colBits, rowVarsStart, colVarsStart);
        cout << "\nSimplified SOP Expression: " << simplified << endl;
    }
}

// تابع اصلی
int main() {
    int numVariables;
    cout << "Enter number of variables (2-5): ";
    cin >> numVariables;

    if (numVariables < 2 || numVariables > 5) {
        cout << "Number of variables must be between 2 and 5." << endl;
        return 1;
    }

    int totalCells = pow(2, numVariables);
    vector<int> karnoMap(totalCells, 0);

    // دریافت مینترم‌ها از کاربر
    cout << "Enter minterms (0 to " << totalCells - 1 << "). Enter -1 to finish." << endl;
    while (true) {
        int m;
        cout << "Minterm: ";
        cin >> m;
        if (m == -1) break;
        if (m >= 0 && m < totalCells)
            karnoMap[m] = 1;
        else
            cout << "Invalid minterm." << endl;
    }

    // نمایش جدول و عبارت ساده‌شده
    printKarnaughMap(karnoMap, numVariables);
    return 0;
}
