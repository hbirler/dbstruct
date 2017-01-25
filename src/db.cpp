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
	int integ = 1;

	std::default_random_engine generator;
	std::uniform_int_distribution<int> distribution(1, testsize);
	
	btree<int, int64_t> mytree;
	for (int i = 1; i < testsize; i++)
	{
		int num = distribution(generator);
		mytree.emplace(num, (int64_t)num * num);
	}
	
	integ = mytree.check_integrity();
	cout << "Integrity(insert) " << integ << endl;
	assert(integ);

	for (int i = 1; i < testsize; i++)
	{
		int num = distribution(generator);
		auto val = mytree.find(num);
		if (val != NULL)
			assert(*val == (int64_t)num * num);
	}

	integ = mytree.check_integrity();
	cout << "Integrity(find) " << integ << endl;
	assert(integ);

	for (int i = 1; i < testsize; i++)
	{
		int num = distribution(generator);
		mytree.erase(num);
	}

	integ = mytree.check_integrity();
	cout << "Integrity(erase) " << integ << endl;
	assert(integ);




	btree<int, int64_t> t2;
	for (int i = 0; i < testsize; i++)
		t2.emplace(i, (int64_t)i*i);

	integ = t2.check_integrity();
	cout << "Integrity(insert2) " << integ << endl;
	assert(integ);

	for (int i = 0; i < testsize; i++)
		assert(*t2.find(i) == (int64_t)i*i);

	for (int i = 0; i < testsize; i++)
		t2.erase(i);

	integ = t2.check_integrity();
	cout << "Integrity(erase2) " << integ << endl;
	assert(integ);

	for (int i = 0; i < testsize; i++)
		assert(t2.find(i) == NULL);



	btree<int, int64_t> t3;
	for (int i = testsize; i >= 0; i--)
	{
		if (i == 2)
			i = 2;
		t3.emplace(i, (int64_t)i*i);
	}

	integ = t3.check_integrity();
	cout << "Integrity(insert3) " << integ << endl;
	assert(integ);

	for (int i = 0; i < testsize; i++)
		assert(*t3.find(i) == (int64_t)i*i);

	for (int i = testsize; i >= 0; i--)
		t3.erase(i);

	integ = t3.check_integrity();
	cout << "Integrity(erase3) " << integ << endl;
	assert(integ);

	for (int i = 0; i < testsize; i++)
		assert(t3.find(i) == NULL);

	
	return 0;
}

void easy_test()
{
	int size = 100000;
	int integ = 1;
	btree<int, int64_t> mt;
	for (int i = 0; i < size; i++)
		mt.emplace(i, i*i);
	for (int i = size; i >= 0; i--)
		mt.erase(i);

	integ = mt.check_integrity();
	cout << "Integrity(erase) " << integ << endl;
	assert(integ);
}

int main_demo()
{
	const int DEMO_SIZE = 1000010;
	const int RANGE = 10;

	/*btree<int, int64_t> mt;
	for (int64_t i = 1; i <= DEMO_SIZE; i++)
	{
		mt.emplace(i, i*i);
	}

	for (int64_t i = RANGE * 2; i <= DEMO_SIZE - RANGE * 2; i++)
	{
		mt.erase(i);
	}

	for (int64_t val : mt.find_range(RANGE, DEMO_SIZE - RANGE))
		cout << val << " ";
	cout << endl;*/

	btree<int, int64_t> t2;
	t2[3] = 5;
	cout << t2[3] << endl;

	

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
		if (strcmp(argv[1], "demo") == 0)
			return main_demo();
	}
	

	//benchy();
	main_demo();
	//easy_test();
	//main_bench();

	system("pause");

	return 0;
}




void benchy()
{
	btree<int, int64_t> mytree;
	mytree.emplace(1, 1);
	mytree.emplace(1, 3);

	int testmax = 1000000;
	int testr = 5;

	std::default_random_engine generator;
	std::uniform_int_distribution<int> distribution(1, testmax);

	for (int i = 2; i < testmax; i++)
	{
		int num = distribution(generator);
		//int num = i;
		//cout << num << endl;
		mytree.emplace(num, (int64_t)num*num);
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