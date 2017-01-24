#include "btree.h"
#include <cstring>
#include <iostream>
#include <chrono>
#include <random>
#include <map>
#include <cassert>

using namespace std;


int64_t getval(btree<int, int64_t>& mytree, int key)
{
	auto v = mytree.find(key);
	if (v == 0)
		return -1;
	else
		return *v;
}

template <class T>
void benchmark(string name, int type, T& mytree, int testsize = 1000000)
{
	int testr = 10;

	std::default_random_engine generator;
	std::uniform_int_distribution<int> distribution(1, testsize);

	if (type == 0)
	for (int i = 2; i < testsize; i++)
	{
		int num = distribution(generator);
		mytree.emplace(num, (int64_t)num * num);
	}

	//cout << "Integrity(insert) " << mytree.check_integrity() << endl;
	if (type == 1)
	for (int i = 2; i < testsize; i++)
	{
		int num = distribution(generator);
		mytree.find(num);
	}

	//cout << "Integrity(find) " << mytree.check_integrity() << endl;
	if (type == 2)
	for (int i = 2; i < testsize; i++)
	{
		int num = distribution(generator);
		mytree.erase(num);
	}

	//cout << "Integrity(erase) " << mytree.check_integrity() << endl;
}

template <class T>
void time_benchmark(string name)
{
	typedef std::chrono::high_resolution_clock Time;
	typedef std::chrono::milliseconds ms;
	typedef std::chrono::duration<float> fsec;
	auto t0 = Time::now();


	string testnames[] = { "insert", "find", "erase" };
	T mytree;
	for (int i = 0; i < 3; i++)
	{
		cout << "Bench " << testnames[i] << " " << name << ":";

		typedef std::chrono::high_resolution_clock Time;
		typedef std::chrono::milliseconds ms;
		typedef std::chrono::duration<float> fsec;
		auto t0 = Time::now();

		benchmark<T>(name, i, mytree);

		auto t1 = Time::now();
		fsec fs = t1 - t0;
		ms d = std::chrono::duration_cast<ms>(fs);
		//std::cout << fs.count() << "s\n";
		std::cout << "\t" << d.count() << "ms" << endl;
	}

	auto t1 = Time::now();
	fsec fs = t1 - t0;
	ms d = std::chrono::duration_cast<ms>(fs);
	//std::cout << fs.count() << "s\n";
	std::cout << "Total " << name << ": " << d.count() << "ms" << endl << endl;
}

void benchy();

int main_bench()
{
	//benchy();
	time_benchmark<btree<int, int64_t>>("btree");
	time_benchmark<map<int, int64_t>>("std::map");

	//system("pause");


	return 0;
}


int main_test()
{
	int testsize = 1000000;

	std::default_random_engine generator;
	std::uniform_int_distribution<int> distribution(1, testsize);
	
	btree<int, int64_t> mytree;
	for (int i = 1; i < testsize; i++)
	{
		int num = distribution(generator);
		mytree.emplace(num, (int64_t)num * num);
	}
	
	assert(mytree.check_integrity());

	for (int i = 1; i < testsize; i++)
	{
		int num = distribution(generator);
		auto val = mytree.find(num);
		if (val != NULL)
			assert(*val == (int64_t)num * num);
	}

	assert(mytree.check_integrity());

	for (int i = 1; i < testsize; i++)
	{
		int num = distribution(generator);
		mytree.erase(num);
	}

	assert(mytree.check_integrity());

	
	return 0;
}

int main(int argc, char* argv[])
{
	cout << "Topkek" << endl;	//For good luck
	if (argc > 1)
	{
		if (strcmp(argv[1], "test") == 0)
			return main_test();
		if (strcmp(argv[1], "bench") == 0)
			return main_bench();
	}
	
	main_test();

	return 0;
}




void benchy()
{
	btree<int, int64_t> mytree;
	mytree.insert(1, 1);
	mytree.insert(1, 3);

	int testmax = 1000000;
	int testr = 5;

	std::default_random_engine generator;
	std::uniform_int_distribution<int> distribution(1, testmax);

	for (int i = 2; i < testmax; i++)
	{
		int num = distribution(generator);
		//int num = i;
		//cout << num << endl;
		mytree.insert(num, (int64_t)num*num);
	}

	for (int i = 1; i < testr; i++)
		cout << getval(mytree, i) << " ";
	cout << endl;
	for (int i = testmax - testr; i < testmax; i++)
		cout << getval(mytree, i) << " ";
	cout << endl;

	for (int i = testr; i < testmax - testr; i++)
	{
		mytree.erase(i);
	}

	for (auto& val : mytree.find_range(1, testmax))
		cout << val << " ";
	cout << endl;
}