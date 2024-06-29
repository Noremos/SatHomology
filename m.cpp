#include <vector>
#include <algorithm>
#include <set>
#include <iostream>

int main()
{
	setlocale(0, "ru_RU.UTF8");
	std::vector<int> numbers(1000000);
	for (int i = 0; i < numbers.size(); ++i)
	{
	numbers[i] = rand() % 2000 - 1000;
	}

	std::multiset<int>positiveNumbers;
	std::multiset<int>negativeNumbers;

	for (int numbers : numbers)
	{
	if (numbers % 2 == 0)
	{
	positiveNumbers.insert(numbers);
	}
	else
	{
	negativeNumbers.insert(numbers);
	}
	}
	std::cout << "Четные числа, которые отсортированы:" << std::endl;
	for (int numbers : positiveNumbers)
	{
		std::cout << numbers << " ";
	}
	std::cout << std::endl;
}