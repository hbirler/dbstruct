#pragma once
#include <functional>
#include <utility>
#include <memory>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

const int IC = 2;
const int LC = 2;

template <class K, class V, class Lt = less<K>>
class btree
{
	int psize;
	public:
	btree();
	void insert(const K&, const V&);
	void erase(const K&);
	V* find(const K&);
	vector<V> find_range(const K& low, const K& high);
	int size();
	string to_string();
	
	struct split_info;

	class node
	{
		friend class btree<K, V, Lt>;

		virtual bool is_leaf() = 0;
		virtual bool is_full() = 0;
		virtual split_info split() = 0;
		virtual string to_string(int depth = 0) = 0;
	};

	struct split_info
	{
		split_info();
		split_info(K, node*, node*);
		K key;
		node* left;
		node* right;
	};

	class inner : public node
	{
		friend class btree<K, V, Lt>;

		K keys[IC];
		node* ptrs[IC + 1];

		bool is_leaf() { return false; }
		bool is_full() { return size == IC; }
		int size;
		void insert(int pos, const K&, node* left, node* right);
		int get_pos(const K&);
		split_info split() override;
		string to_string(int depth = 0) override;
	};
	class leaf : public node
	{
		friend class btree<K, V, Lt>;

		K keys[LC];
		V values[LC];
		leaf* next = NULL;

		bool is_leaf() { return true; }
		bool is_full() { return size == LC; }
		int size;
		void insert(const K&, const V&);
		V* get(const K&);
		int get_pos(const K&);
		split_info split() override;
		string to_string(int depth = 0) override;
	};

	inner* root;
	
	//class iterator;
};


template<class K, class V, class Lt>
btree<K, V, Lt>::btree()
{
	cout<<"Testing BTree"<<endl;
	this->psize = 0;
	this->root = NULL;
}


template <class K, class V, class Lt>
V* btree<K,V,Lt>::find(const K& key)
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
		if (Lt()(high, cl->keys[pos]))
			return retval;
		retval.push_back(cl->values[pos]);
	}
	cl = cl->next;
	while (cl != NULL)
	{
		for (int i = 0; i < cl->size; i++)
		{
			if (Lt()(high, cl->keys[pos]))
				return retval;
			retval.push_back(cl->values[pos]);
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
inline string btree<K, V, Lt>::to_string()
{
	return root->to_string();
}


template <class K, class V, class Lt>
void btree<K,V,Lt>::insert(const K& key, const V& value)
{
	V* found = find(key);
	if (found)
	{
		*found = value;
		return;
	}

	if (!root)
	{
		root = new inner();
		root->keys[0] = key;
		root->ptrs[0] = new leaf();
		root->ptrs[1] = new leaf();
		root->size = 1;
		leaf* myleaf = (leaf*)root->ptrs[1];
		((leaf*)root->ptrs[0])->next = myleaf;
		myleaf->insert(key, value);
		
		return;
	}
	
	node* cur = root;
	
	if (root->is_full())
	{
		split_info sp = root->split();
		root = new inner();
		root->keys[0] = sp.key;
		root->ptrs[0] = sp.left;
		root->ptrs[1] = sp.right;
		root->size = 1;
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
	
	((leaf*)cur)->insert(key, value);

	this->psize += 1;
}

/*template<class K, class V, class Lt>
inline void btree<K, V, Lt>::erase(const K& key)
{
	V* found = find(key);
	if (!found)
		return;

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
			redis_info rp = next->redis(me->ptrs[npos + 1]);	//or npos-1
			
			if (rp.merged)
			{
				//erase nposkey from me
			}
			else
			{
				//change key in me to rp.newkey
			}
			//cur = something
		}
		else
		{
			cur = next;
		}
	}

	((leaf*)cur)->erase(key, value);

	this->psize -= 1;
}*/

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

template <class K, class V, class Lt>
int btree<K, V, Lt>::inner::get_pos(const K& key)
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
inline string btree<K, V, Lt>::inner::to_string(int depth)
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
int btree<K, V, Lt>::leaf::get_pos(const K& key)
{
	return upper_bound(this->keys, this->keys + this->size, key, Lt()) - this->keys;
}

template <class K, class V, class Lt>
void btree<K, V, Lt>::leaf::insert(const K& key, const V& value)
{
	int pos = this->get_pos(key);
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
V* btree<K, V, Lt>::leaf::get(const K& key)
{
	int pos = this->get_pos(key) - 1;
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
inline string btree<K, V, Lt>::leaf::to_string(int depth)
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
