#include "Utils.hpp"

int ft_stoi(const std::string& str) {
	size_t i = 0;
	int result = 0;
	bool isNegative = false;

	// Boş string kontrolü
	if (str.empty()) {
		throw std::invalid_argument("Invalid input: empty string");
	}

	// Başında boşlukları geç
	while (i < str.length() && isspace(str[i])) {
		++i;
	}

	// İşaret kontrolü
	if (i < str.length() && (str[i] == '-' || str[i] == '+')) {
		isNegative = (str[i] == '-');
		++i;
	}

	// Sayısal karakter kontrolü ve dönüştürme
	while (i < str.length()) {
		if (!isdigit(str[i])) {
			throw std::invalid_argument("Invalid input: non-numeric character found");
		}
		result = result * 10 + (str[i] - '0');
		++i;
	}

	return isNegative ? -result : result;
}

// perror alternatifi
void printError(const char* msg) {
	std::cerr << msg << ": " << strerror(errno) << std::endl;
}