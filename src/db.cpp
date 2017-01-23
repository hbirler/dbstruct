#include "db.h"

using namespace std;

void benchmark()
{
	btree<int, int64_t> mytree;
	mytree.insert(1, 1);
	mytree.insert(1, 3);

	int testmax = 1000000;
	int testr = 5;

	for (int i = 2; i < testmax; i++)
	{
		mytree.insert(i, (int64_t)i*i);
	}

	for (int i = 1; i < testr; i++)
		cout << *mytree.find(i) << " ";
	cout << endl;
	for (int i = testmax - testr; i < testmax; i++)
		cout << *mytree.find(i) << " ";
	cout << endl;

	for (int i = testr; i < testmax - testr; i++)
	{
		mytree.erase(i);
	}

	for (auto& val : mytree.find_range(1, testmax))
		cout << val << " ";
	cout << endl;
}

int main_test()
{

	cout << "Testing" << endl;

	typedef std::chrono::high_resolution_clock Time;
	typedef std::chrono::milliseconds ms;
	typedef std::chrono::duration<float> fsec;
	auto t0 = Time::now();

	benchmark();

	auto t1 = Time::now();
	fsec fs = t1 - t0;
	ms d = std::chrono::duration_cast<ms>(fs);
	std::cout << fs.count() << "s\n";
	std::cout << d.count() << "ms\n";
	system("pause");

	
	return 0;
}

int main(int argc, char* argv[])
{
	cout << "Topkek" << endl;
	if (argc > 1)
	{
		if (strcmp(argv[1], "test") == 0)
			return main_test();
	}
	
	main_test();

	return 0;
}