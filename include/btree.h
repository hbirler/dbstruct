#pragma once
#include <functional>
#include <utility>
#include <memory>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <climits>

using namespace std;

const int IL = 25;
const int LL = IL;
const int IH = 2 * IL + 1;	//inner high
const int LH = 2 * LL + 1;	//leaf high


template <class K, class V, class Lt = less<K>>
class btree
{
	int psize;
	public:
	btree();
	void emplace(const K&, const V&);
	void insert(const K&, const V&);
	void erase(const K&);
	V* find(const K&);
	bool contains(const K&);
	int check_integrity();
	vector<V> find_range(const K& low, const K& high);
	int size();
	string to_string();
	
	struct split_info;
	struct redis_info;

	class node
	{
		friend class btree<K, V, Lt>;
	protected:
		int size;
	public:
		virtual bool is_leaf() = 0;
		virtual bool is_full() = 0;
		virtual bool is_empty() = 0;
		virtual split_info split() = 0;
		virtual redis_info redis(const K&, node*) = 0;
		virtual string to_string(int depth = 0) = 0;
		virtual int check_integrity(int mmax = 0) = 0;
		//inline bool is_full() { return size == IH; }
		//inline bool is_empty() { return size == IL; }
	};

	struct split_info
	{
		split_info();
		split_info(K, node*, node*);
		K key;
		node* left;
		node* right;
	};
	struct redis_info
	{
		redis_info();
		bool merged;
		K newkey;
		node* newnode;
	};

	class inner : public node
	{
		friend class btree<K, V, Lt>;
	public:
		K keys[IH];
		node* ptrs[IH + 1];

		inner() { size = 0; }
		bool is_leaf() { return false; }
		inline bool is_full() { return size == IH; }
		inline bool is_empty() { return size == IL; }
		void insert(int pos, const K&, node* left, node* right);
		int check_integrity(int mmax) override;
		int get_pos(const K&);
		split_info split() override;
		redis_info redis(const K&, node*) override;
		void erase(int, node*);
		string to_string(int depth = 0) override;
	};
	class leaf : public node
	{
		friend class btree<K, V, Lt>;
	public:
		K keys[LH];
		V values[LH];
		leaf* next = NULL;

		leaf() :next(NULL) { size = 0; }
		bool is_leaf() { return true; }
		inline bool is_full() { return size == LH; }
		inline bool is_empty() { return size == LL; }
		void insert(const K&, const V&);
		void insert(int, const K&, const V&);
		int check_integrity(int mmax) override;
		V* get(const K&);
		V* get(int, const K&);
		int get_pos(const K&);
		split_info split() override;
		redis_info redis(const K&, node*) override;
		void erase(const K&);
		void erase(int, const K&);
		string to_string(int depth = 0) override;
	};

	node* root;
	
	//class iterator;
};


template<class K, class V, class Lt>
btree<K, V, Lt>::btree()
{
	this->psize = 0;
	this->root = NULL;
}

template<class K, class V, class Lt>
inline void btree<K, V, Lt>::emplace(const K& key, const V& value)
{
	this->insert(key, value);
}


template <class K, class V, class Lt>
inline V* btree<K,V,Lt>::find(const K& key)
{
	if (!root)
		return NULL;
	
	node* cur = root;
	
	while (!cur->is_leaf())
	{
		inner* me = (inner*)cur;
		int npos = me->get_pos(key);
		node* next = me->ptrs[npos];
		cur = next;
	}
	return ((leaf*)cur)->get(key);
}

template <class K, class V, class Lt>
inline bool btree<K, V, Lt>::contains(const K& key)
{
	return !find(key);
}

template<class K, class V, class Lt>
inline int btree<K, V, Lt>::check_integrity()
{
	if (root->check_integrity() != INT_MAX)
		return 1;
	else
		return 0;
}

template <class K, class V, class Lt>
vector<V> btree<K, V, Lt>::find_range(const K& low, const K& high)
{
	vector<V> retval;
	if (!root)
		return retval;

	node* cur = root;

	while (!cur->is_leaf())
	{
		inner* me = (inner*)cur;
		int npos = me->get_pos(low);
		node* next = me->ptrs[npos];
		cur = next;
	}
	leaf* cl = (leaf*)cur;
	int pos = cl->get_pos(low) - 1;
	if (Lt()(cl->keys[pos], low))
		pos++;
	for (int i = pos; i < cl->size; i++)
	{
		if (Lt()(high, cl->keys[i]))
			return retval;
		retval.push_back(cl->values[i]);
	}
	cl = cl->next;
	while (cl != NULL)
	{
		for (int i = 0; i < cl->size; i++)
		{
			if (Lt()(high, cl->keys[i]))
				return retval;
			retval.push_back(cl->values[i]);
		}
		cl = cl->next;
	}
	return retval;
}

template<class K, class V, class Lt>
int btree<K, V, Lt>::size()
{
	return psize;
}

template<class K, class V, class Lt>
string btree<K, V, Lt>::to_string()
{
	return root->to_string();
}

template <class K, class V, class Lt>
void btree<K,V,Lt>::insert(const K& key, const V& value)
{
	/*V* found = find(key);
	if (found)
	{
		*found = value;
		return;
	}*/

	if (!root)
	{
		root = new leaf();
		((leaf*)root)->insert(key, value);
		
		return;
	}
	
	node* cur = root;
	
	if (root->is_full())
	{
		split_info sp = root->split();
		root = new inner();
		inner* ir = (inner*)root;
		ir->keys[0] = sp.key;
		ir->ptrs[0] = sp.left;
		ir->ptrs[1] = sp.right;
		ir->size = 1;
		if (Lt()(key, sp.key))
			cur = sp.left;
		else
			cur = sp.right;
	}
	
	while (!cur->is_leaf())
	{
		inner* me = (inner*)cur;
		int npos = me->get_pos(key);
		node* next = me->ptrs[npos];
		
		if (next->is_full())
		{
			split_info sp = next->split();
			me->insert(npos, sp.key, sp.left, sp.right);
			
			if (Lt()(key, sp.key))
				cur = sp.left;
			else
				cur = sp.right;
		}
		else
		{
			cur = next;
		}
	}
	int pos = ((leaf*)cur)->get_pos(key) - 1;
	V* mv = ((leaf*)cur)->get(pos, key);
	if (mv == NULL)
		((leaf*)cur)->insert(pos + 1, key, value);
	else
		*mv = value;

	this->psize += 1;
}

template<class K, class V, class Lt>
void btree<K, V, Lt>::erase(const K& key)
{
	/*V* found = find(key);
	if (!found)
		return;*/

	if (!root)
		return;

	node* cur = root;

	while (!cur->is_leaf())
	{
		inner* me = (inner*)cur;
		int npos = me->get_pos(key);
		node* next = me->ptrs[npos];

		if (next->is_empty())
		{
			int m1pos = 0;
			node* m1, *m2;
			if (npos == me->size)
			{
				//cout << "Alarm" << endl;
				m1pos = npos - 1;
				m1 = me->ptrs[m1pos];
				m2 = me->ptrs[m1pos + 1];
			}
			else
			{
				m1pos = npos;
				m1 = me->ptrs[m1pos];
				m2 = me->ptrs[m1pos + 1];
			}
			redis_info rp = m1->redis(me->keys[m1pos], m2);
			
			if (rp.merged)
			{
				me->erase(m1pos, rp.newnode);
				if (cur == root && me->size == 0)
					root = rp.newnode;
			}
			else
			{
				me->keys[m1pos] = rp.newkey;
			}
			//cur = something
		}
		else
		{
			cur = next;
		}
	}
	int pos = ((leaf*)cur)->get_pos(key) - 1;
	if (((leaf*)cur)->get(pos, key))
		((leaf*)cur)->erase(pos, key);

	this->psize -= 1;
}


template <class K, class V, class Lt>
void btree<K, V, Lt>::inner::insert(int pos, const K& key, node * left, node * right)
{
	for (int i = this->size; i > pos; i--)
	{
		this->keys[i] = this->keys[i - 1];
	}
	for (int i = this->size; i > pos; i--)
	{
		this->ptrs[i + 1] = this->ptrs[i];
	}
	this->keys[pos] = key;
	this->ptrs[pos] = left;
	this->ptrs[pos + 1] = right;
	this->size += 1;
}

template<class K, class V, class Lt>
inline int btree<K, V, Lt>::inner::check_integrity(int mmax)
{
	//if (size < IL)
	//	return INT_MAX;
	if (size > IH)
		return INT_MAX;
	for (int i = 0; i < size; i++)
	{
		mmax = ptrs[i]->check_integrity();
		if (mmax > keys[i])
			return INT_MAX;
		mmax = keys[i];
	}
	mmax = ptrs[size]->check_integrity();
	return mmax;
}

template <class K, class V, class Lt>
inline int btree<K, V, Lt>::inner::get_pos(const K& key)
{
	return upper_bound(this->keys, this->keys + this->size, key, Lt()) - this->keys;
}

template <class K, class V, class Lt>
typename btree<K, V, Lt>::split_info btree<K, V, Lt>::inner::split()
{
	int psize = this->size;
	split_info retval;
	retval.key = this->keys[psize / 2];

	inner* right = new inner();
	for (int i = psize / 2 + 1; i < psize; i++)	//memcpy?
	{
		right->keys[right->size] = this->keys[i];
		right->ptrs[right->size] = this->ptrs[i];
		right->size += 1;
	}
	right->ptrs[right->size] = this->ptrs[psize];

	this->size = psize / 2;
	retval.left = this;
	retval.right = right;

	return retval;
}

template<class K, class V, class Lt>
typename btree<K, V, Lt>::redis_info btree<K, V, Lt>::inner::redis(const K& key, node* nextin)
{
	inner* next = (inner*)nextin;
	redis_info retval;
	if (is_empty() && next->is_empty())
	{
		retval.merged = true;
		keys[size] = key;
		size += 1;
		for (int i = 0; i < next->size; i++)
		{
			keys[size] = next->keys[i];
			ptrs[size] = next->ptrs[i];
			size += 1;
		}
		ptrs[size] = next->ptrs[next->size];
		delete next;
		retval.newnode = this;
	}
	else
	{
		retval.merged = false;
		if (next->size > size)
		{
			int pcount = (next->size - size + 1) / 2;
			keys[size] = key;
			size += 1;
			for (int i = 0; i < pcount - 1; i++)
			{
				keys[size] = next->keys[i];
				ptrs[size] = next->ptrs[i];
				size += 1;
			}
			ptrs[size] = next->ptrs[pcount - 1];
			retval.newkey = next->keys[pcount - 1];
			for (int i = pcount; i < next->size; i++)
			{
				next->keys[i - pcount] = next->keys[i];
				next->ptrs[i - pcount] = next->ptrs[i];
			}
			next->ptrs[next->size - pcount] = next->ptrs[next->size];
			next->size = next->size - pcount;
		}
		else
		{
			int pcount = (size - next->size + 1) / 2;

			for (int i = next->size - 1; i >= 0; i--)
			{
				next->keys[i + pcount] = next->keys[i];
				next->ptrs[i + pcount + 1] = next->ptrs[i + 1];
			}
			next->ptrs[pcount] = next->ptrs[0];

			next->keys[pcount - 1] = key;
			for (int i = pcount - 2; i >= 0; i--)
			{
				next->keys[i] = keys[size - 1];
				next->ptrs[i + 1] = ptrs[size];
				size -= 1;
			}
			next->ptrs[0] = ptrs[size];

			retval.newkey = keys[size - 1];
			size--;

			next->size = next->size + pcount;
		}
	}
	return retval;
}

template<class K, class V, class Lt>
void btree<K, V, Lt>::inner::erase(int pos, node* newnode)
{
	for (int i = pos; i < size; i++)
	{
		keys[i] = keys[i + 1];
		ptrs[i + 1] = ptrs[i + 2];
	}
	ptrs[pos] = newnode;
	size -= 1;
}

template<class K, class V, class Lt>
string btree<K, V, Lt>::inner::to_string(int depth)
{
	stringstream retval;
	
	for (int i = 0; i < size; i++)
	{
		retval << ptrs[i]->to_string(depth + 1);
		retval << " " << depth << ":" << keys[i] << " ";
	}
	retval << ptrs[size]->to_string(depth + 1);

	return retval.str();
}


template <class K, class V, class Lt>
inline int btree<K, V, Lt>::leaf::get_pos(const K& key)
{
	return upper_bound(this->keys, this->keys + this->size, key, Lt()) - this->keys;
}

template <class K, class V, class Lt>
inline void btree<K, V, Lt>::leaf::insert(const K& key, const V& value)
{
	insert(this->get_pos(key), key, value);
}

template <class K, class V, class Lt>
void btree<K, V, Lt>::leaf::insert(int pos, const K& key, const V& value)
{
	for (int i = this->size; i > pos; i--)
	{
		this->keys[i] = this->keys[i - 1];
	}
	for (int i = this->size; i > pos; i--)
	{
		this->values[i] = this->values[i - 1];
	}
	this->keys[pos] = key;
	this->values[pos] = value;
	this->size += 1;
}

template<class K, class V, class Lt>
inline int btree<K, V, Lt>::leaf::check_integrity(int mmax)
{
	//if (size < LL)
	//	return INT_MAX;
	if (size > LH)
		return INT_MAX;
	for (int i = 0; i < size; i++)
	{
		if (mmax > keys[i])
			return INT_MAX;
		mmax = keys[i];
	}
	return mmax;
}

template<class K, class V, class Lt>
inline V* btree<K, V, Lt>::leaf::get(const K& key)
{
	return get(this->get_pos(key) - 1, key);
}

template<class K, class V, class Lt>
V* btree<K, V, Lt>::leaf::get(int pos, const K& key)
{
	if (pos >= size)
		return NULL;
	if (Lt()(this->keys[pos], key) || Lt()(key, this->keys[pos]))
		return NULL;
	return &this->values[pos];
}

template <class K, class V, class Lt>
typename btree<K, V, Lt>::split_info btree<K, V, Lt>::leaf::split()
{
	int psize = this->size;
	split_info retval;
	retval.key = this->keys[psize / 2];

	leaf* right = new leaf();
	for (int i = psize / 2; i < psize; i++)	//memcpy?
	{
		right->keys[right->size] = this->keys[i];
		right->values[right->size] = this->values[i];
		right->size += 1;
	}

	this->size = psize / 2;
	retval.left = this;
	retval.right = right;
	this->next = right;

	return retval;
}

template<class K, class V, class Lt>
typename btree<K, V, Lt>::redis_info btree<K, V, Lt>::leaf::redis(const K&, node* nextin)
{
	leaf* next = (leaf*)nextin;
	redis_info retval;
	if (is_empty() && next->is_empty())
	{
		retval.merged = true;
		for (int i = 0; i < next->size; i++)
		{
			keys[size] = next->keys[i];
			values[size] = next->values[i];
			size += 1;
		}
		this->next = next->next;
		delete next;
		retval.newnode = this;
	}
	else
	{
		retval.merged = false;
		if (next->size > size)
		{
			int pcount = (next->size - size + 1) / 2;

			for (int i = 0; i < pcount; i++)
			{
				keys[size] = next->keys[i];
				values[size] = next->values[i];
				size += 1;
			}
			retval.newkey = next->keys[pcount];
			for (int i = pcount; i < next->size; i++)
			{
				next->keys[i - pcount] = next->keys[i];
				next->values[i - pcount] = next->values[i];
			}
			next->size = next->size - pcount;
		}
		else
		{
			int pcount = (size - next->size + 1) / 2;

			for (int i = next->size - 1; i >= 0; i--)
			{
				next->keys[i + pcount] = next->keys[i];
				next->values[i + pcount] = next->values[i];
			}

			for (int i = pcount - 1; i >= 0; i--)
			{
				next->keys[i] = keys[size - 1];
				next->values[i] = values[size - 1];
				size -= 1;
			}
			retval.newkey = keys[size];
			
			next->size = next->size + pcount;
		}
	}
	return retval;
}

template<class K, class V, class Lt>
inline void btree<K, V, Lt>::leaf::erase(const K& key)
{
	erase(get_pos(key) - 1, key);
}

template<class K, class V, class Lt>
void btree<K, V, Lt>::leaf::erase(int pos, const K& key)
{
	for (int i = pos; i < size; i++)
	{
		keys[i] = keys[i + 1];
		values[i] = values[i + 1];
	}
	size -= 1;
}

template<class K, class V, class Lt>
string btree<K, V, Lt>::leaf::to_string(int depth)
{
	stringstream retval;
	retval << depth << ":[";
	for (int i = 0; i < size; i++)
		retval << " " << keys[i];
	retval << "]";
	return retval.str();
}

template<class K, class V, class Lt>
btree<K, V, Lt>::split_info::split_info()
{
	this->left = NULL;
	this->right = NULL;
}

template<class K, class V, class Lt>
btree<K, V, Lt>::split_info::split_info(K key, node* left, node* right) : key(key), left(left), right(right) {}


template class btree<int,int,less<int>>;

template<class K, class V, class Lt>
btree<K, V, Lt>::redis_info::redis_info()
{
	this->merged = false;
	this->newnode = NULL;
}
