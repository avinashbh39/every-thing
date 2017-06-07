#ifndef HASH_TABLE_H
#define HASH_TABLE_H

namespace libTest{

template<class T, class U>
class HashTable{
public:

HashTable()
{
/*	m_ptable = new HashNode[MAX_TABLE_SIZE];*/
	for(auto i = 0 ;i < MAX_TABLE_SIZE ;++i)
		m_ptable[i] = nullptr ;
}

HashTable(const &HashTable copy);
{
	for(auto i = 0 ;i < MAX_TABLE_SIZE ;++i)
	{
		if( copy.m_ptable[i])
		{
			HashNode *crawler = copy.m_ptable[i] ;
			while(crawler)
			{
				HashNode *temp = new HashNode(crawler->key, crawler->value);
				temp->next = this->m_ptable[i] ;
				this->m_ptable[i] =  temp ;
				crawler = crawler->next ;
			}
		}	
	}
}
void operator=(const &HashTable)
{
	//Clear the previous list.
	EraseTable();

	for(auto i = 0 ;i < MAX_TABLE_SIZE ;++i)
	{
		if( copy.m_ptable[i])
		{
			HashNode *crawler = copy.m_ptable[i] ;
			while(crawler)
			{
				HashNode *temp = new HashNode(crawler->key, crawler->value);
				temp->next = this->m_ptable[i] ;
				this->m_ptable[i] =  temp ;
				crawler = crawler->next ;
			}
		}	
	}
}
~HashTable()
{
	EraseTable();
}


void Insert(T key, U value)
{
	int hashCode = -1 ;
	//Get hashcode from hash function.
	//hashCode = Hash(key);
	HashNode *temp = new HashNode(key, value);
	temp->next = m_ptable[hashCode] ;
	m_ptable[hashCode] = temp ;

}

bool Find(T key, U &value)
{
	int hashCode = -1 ;
	//Get hashcode from hash function.
	//hashCode = Hash(key);
	if( m_ptable[hashCode])
	{
		HashNode *crawler = m_ptable[hashCode] ;
		while( crawler)
		{
			if( crawler->key == key)
			{
				value = crawler->value ;
				return true ;
			}
			crawler = crawler->next ;
		}
	}

	return false ;
}


private:

struct HashNode{
T key ;
U value ;
HashNode *next = nullptr;
HashNode() {};
HashNode(T hkey, U val):key(hkey), value(val)
{
}
};

HashNode *m_ptable[MAX_TABLE_SIZE];
const size_t MAX_TABLE_SIZE = 1000;

void EraseTable()
{
	for(auto i = 0 ;i < MAX_TABLE_SIZE ;++i)	
	{
		if( m_ptable[i])
		{
			HashNode *pcrawl = m_ptable[i];
			while(pcrawl)
			{
				HashNode* ptr = pcrawl ;
				pcrawl = pcrawl->next ;
				delete ptr;
			
			}
		}
		m_ptable[i] = nullptr ;
	}
}


};
}

#endif //HASH_TABLE_H
