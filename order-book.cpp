#include <map>
#include <set>
#include <list>
#include <cmath>
#include <ctime>
#include <deque>
#include <queue>
#include <stack>
#include <string>
#include <bitset>
#include <cstdio>
#include <limits>
#include <vector>
#include <climits>
#include <cstring>
#include <cstdlib>
#include <fstream>
#include <numeric>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <unordered_map>

using namespace std;

struct Order {
	long id;
	string type;
	string side;
	long value1;
	double value2;
	bool active;
	Order(const long id, const string & type, const string & side, const long value1, const double value2) {
		this->id = id;
		this->type = type;
		this->side = side;
		this->value1 = value1;
		this->value2 = value2;
		this->active = true;
	}
};

class Trade {
public:
		Trade() {
			count = 0;
			table.clear();
			table.push_back(NULL);
		}
		~Trade() {
			count = 0;
			for (auto & i : table) {
				if (i) {
					delete i;
					i = NULL;
				}
			}
		}
		Order * push(const string & line) {
			++count;
			size_t n = line.size(), i = 0;
			while (i < n and line[i] != ' ') {
				++i;
			}
			string type = line.substr(0, i);
			size_t j = ++i;
			while (i < n and line[i] != ' ') {
				++i;
			}
			string side = line.substr(j, i - j);
			j = ++i;
			while (i < n and line[i] != ' ') {
				++i;
			}
			long value1 = stol(line.substr(j, i - j));
			j = ++i;
			while (i < n and line[i] != ' ') {
				++i;
			}
			double value2 = stod(line.substr(j, i - j));
			Order * order = new Order(count, type, side, value1, value2);
			table.push_back(order);
			return order;
		}
		string execute(const string & line) {
			Order * order = push(line);
			if (order->type == "market" or order->type == "limit") {
				return toMarketAndtoLimit(order);
			}
			if (order->type == "stop") {
				return toStop(order);
			}
			if (order->type == "cancel") {
				return toCancel(order);
			}
			return "";
		}
private:
		long count;
		vector<Order*> table;
		bool condition1(const Order * order, const double value2, const double price) {
			if (order->type == "market") {
				return order->side == "buy" ? value2 < price : value2 > price;   
			}
			if (order->side == "buy") {
				return value2 <= order->value2 and value2 < price;
			}
			if (order->side == "sell") {
				return value2 >= order->value2 and value2 > price;
			}
			return false;
		}
		bool condition2(const string & side, const double value2, const double price) {
			return side == "buy" ? value2 <= price : value2 >= price;
		}
		string double2string(const double val, const int n = 2) {
			ostringstream result;
			result.precision(n);
			result << fixed << val;
			return result.str();
		}
		string toMarketAndtoLimit(Order * order) {
			if (!order) {
				return "";
			}
			string result;
			double bound = order->side == "buy" ? numeric_limits<double>::max() : numeric_limits<double>::min();
			string orderSide = order->side, orderOpposite = orderSide == "buy" ? "sell" : "buy";
			list<Order*> l;
			while (order->value1 > 0) {
				long id = -1;
				double price = bound;
				for (const auto & i : table) {
					if (!i or !i->active) {
						continue;
					}
					if (i->type == "limit" and i->side == orderOpposite and i->value1 > 0 and condition1(order, i->value2, price)) {
						id = i->id;
						price = i->value2;
					}
				}
				if (id == -1) {
					break;
				}
				long volume = min(order->value1, table[id]->value1);
				result += "match " + to_string(order->id) + " " + to_string(id) + " " + to_string(volume) + " " + double2string(price) + "\n";
				order->value1 -= volume;
				table[id]->value1 -= volume;
				if (order->value1 <= 0) {
					order->active = false;
				}
				if (table[id]->value1 <= 0) {
					table[id]->active = false;
				}
				for (const auto & i : table) {
					if (!i or !i->active) {
						continue;
					}
					if (i->type == "stop" and i->side == orderSide and i->value1 > 0 and condition2(orderSide, i->value2, price)) {
						l.push_back(i);
						continue;
					}
					if (i->type == "stop" and i->side == orderOpposite and i->value1 > 0 and condition2(orderOpposite, i->value2, price)) {
						l.push_back(i);
						continue;
					}
				}
			}
			if (order->active and order->type == "market") {
				order->active = false;
			}
			for (auto & i : l) {
				if (!i or !i->active) {
					continue;
				}
				i->type = "market";
				result += toMarketAndtoLimit(i);
				i->active = false;
			}
			return result;
		}
		string toStop(Order * order) {
			return "";
		}
		string toCancel(Order * order) {
			if (!order or order->id > count) {
				return "";
			}
			table[order->id]->active = false;
			if (order->value1 >= 1 and order->value1 <= count) {
				table[order->value1]->active = false;
			}
			return "";
		}
};

// int main(void) {
// 	Trade trade;
// 	string line;
// 	while (getline(cin, line)) {
// 		cout << trade.execute(line);
// 	}
// 	return 0;
// }

int main(void) {
	Trade trade;
	ifstream file("input001.txt");
	string line;
	while (getline(file, line)) {
		cout << trade.execute(line);
	}
	file.close();
	cout << '\n';
	file = ifstream("input002.txt");
	while (getline(file, line)) {
		cout << trade.execute(line);
	}
	file.close();
	cout << '\n';
	return 0;
}