#include <compsky/mysql/query.hpp>

#include "annoylib.h"
#include "kissrandom.h" // No idea how good it is, but it works
#include <string.h> // for memset
#include <stdexcept>
#include <iostream>
#include <cassert>


namespace _mysql {
	MYSQL* obj;
	MYSQL_RES* res;
	MYSQL_ROW row;
	constexpr const size_t auth_sz = 512;
	char auth[auth_sz];
}





typedef uint32_t CountType;
typedef uint32_t IndexType;
typedef uint64_t UserType;
typedef uint32_t UserIndexType;
typedef uint32_t SubredditType;
typedef float AnnoyingDistanceType;
typedef AnnoyIndex<IndexType, AnnoyingDistanceType, Angular, Kiss64Random> AnnoyingIndex;


template<typename Key,  typename Value>
class IndexMap {
 private:
	std::vector<Key> keys;
	struct NoSuchKey : public std::runtime_error {
		NoSuchKey(const Key key) : std::runtime_error("No such key")
		{}
	};
 public:
	Value get_index_guaranteed(const Key& key) const {
		for (Value i = 0;  i < keys.size();  ++i){
			if (keys.at(i) == key)
				return i;
		}
		throw NoSuchKey(key);
	}
	Value get_index_maybe_add(const Key& key){
		Value i = 0;
		for (i = 0;  i < keys.size();  ++i){
			if (keys.at(i) == key)
				return i;
		}
		keys.push_back(key);
		return i;
	}
	void add_unsafe(const Key& key){
		keys.push_back(key);
	}
	void reserve(const size_t n){
		this->keys.reserve(n);
	}
	Key at_index(const size_t i) const {
		return this->keys.at(i);
	}
	size_t size() const {
		return this->keys.size();
	}
};


/*
template<typename Key,  typename Value>
class IntMap : public IndexMap {
 private:
	struct KeyValue {
		Key key;
		Value value;
		KeyValue(Key _key,  Value _value) : key(_key), value(_value) {}
	};
	std::vector<KeyValue> data;
 public:
	Value get_index_maybe_add(const Key key){
		for (auto i = 0;  i < data.size();  ++i){
			if (data.at(i).key == key)
				return data.at(i).value;
		}
		data.emplace_back(key, value);
		return i;
	}
	void clear(){
		data.clear();
	}
}
*/

template<typename Integer>
Integer s2n(const char*& s){
	Integer n = 0;
	while( (*s != 0)  and (*s != ',') ){
		n *= 10;
		n += *s - '0';
		++s;
	}
	++s; // Skip comma
	return n;
}

template<typename Integer>
Integer s2n2(const char* const str){
	Integer n = 0;
	const char* s = str;
	while( (*s != 0)  and (*s != ',') ){
		n *= 10;
		n += *s - '0';
		++s;
	}
	++s; // Skip comma
	return n;
}


/*
template<typename Key,  typename Integer>
void csv_to_vector(const char* const csv,  std::vector<Integer>& v){
	v.clear();
	Integer n;
	while( n = s2n(csv) ){
		v.push_back(n);
	}
}
*/


void parse(const char* user_ids_csv,  const char* counts_csv,  uint32_t subreddit_cmnt_count,  AnnoyingDistanceType* subreddit_counts,  IndexMap<UserType, UserIndexType>& user2indx){
	UserType user;
	while( user = s2n<UserType>(user_ids_csv) ){
		const CountType count = s2n<CountType>(counts_csv);
		const UserIndexType user_indx = user2indx.get_index_maybe_add(user);
		
		subreddit_counts[user_indx] = (AnnoyingDistanceType)count / (AnnoyingDistanceType)subreddit_cmnt_count;
	}
}


struct CString {
	char buf[128];
	CString(const char* const s){
		memcpy(buf,  s,  strlen(s) + 1);
	}
	const char* data() const {
		return buf;
	}
	bool operator==(const CString& s) const {
		const char* a = this->buf;
		const char* b = s.buf;
		while(*a != 0){
			if (*b != *a)
				return false;
			++a;
			++b;
		}
		return (*b == 0);
	}
};


struct Submission{};
struct User{};
struct Subreddit {
	constexpr static const char* const which = "submission";
	const char* const name;
	Subreddit(const char* const _name) : name(_name) {}
};
struct Tag {
	constexpr static const char* const which = "tag";
	const char* const name;
	Tag(const char* const _name) : name(_name) {}
};

IndexType get_index_of(Subreddit const from_subreddit_name,  IndexMap<CString, IndexType> subreddit_indx2name){
	return subreddit_indx2name.get_index_guaranteed(from_subreddit_name.name);
}

IndexType get_index_of(Tag const from_subreddit_name,  IndexMap<CString, IndexType> subreddit_indx2name){
	return subreddit_indx2name.get_index_guaranteed(from_subreddit_name.name);
}

void tmp_tbl_from_qry(Subreddit const from_subreddit_name,  const unsigned int n_cols){
	static char buf[200];
    static char* itr = buf;
	
	compsky::mysql::exec(_mysql::obj, itr,
		"CREATE TEMPORARY TABLE _tmp_u2scc AS "
		"SELECT "
			"u2scc2.subreddit_id,"
			"u2scc2.user_id,"
			"u2scc2.count "
		"FROM user2subreddit_cmnt_count u2scc1 "
		"JOIN user2subreddit_cmnt_count u2scc2 ON u2scc2.user_id=u2scc1.user_id "
		"JOIN subreddit r ON r.id=u2scc1.subreddit_id "
		"WHERE r.name=\"", from_subreddit_name.name, "\" "
		"LIMIT ", n_cols
	);
	//compsky::mysql::exec(_mysql::obj, itr, "CREATE TEMPORARY TABLE _tmp_u2scc AS SELECT * FROM user2subreddit_cmnt_count ", subreddit_name.qry, " LIMIT ", n_cols);
	
	compsky::mysql::query_buffer(_mysql::obj, _mysql::res, "SELECT r.name, s2cc.count, u2scc.subreddit_id, GROUP_CONCAT(u2scc.user_id), GROUP_CONCAT(u2scc.count) FROM _tmp_u2scc u2scc JOIN subreddit r ON r.id=u2scc.subreddit_id JOIN subreddit2cmnt_count s2cc ON s2cc.id=u2scc.subreddit_id GROUP BY u2scc.subreddit_id");
}

void tmp_tbl_from_qry(Tag const from_subreddit_name,  const unsigned int n_cols){
	static char buf[200];
    static char* itr = buf;
	
	compsky::mysql::exec(_mysql::obj, itr,
		// Create temporary table, because older MySQL/MariaDB versions don't allow for re-entering the same table
		"CREATE TEMPORARY TABLE _tmp_subreddits AS "
		"SELECT "
			"r2t.subreddit_id "
		"FROM subreddit2tag r2t "
		"JOIN tag t ON t.id=r2t.tag_id "
		"WHERE t.name=\"", from_subreddit_name.name, "\""
	);
	
	compsky::mysql::exec_buffer(_mysql::obj,
		"CREATE TEMPORARY TABLE _tmp_subreddits2 AS "
		"SELECT * "
		"FROM _tmp_subreddits"
	);
	
	compsky::mysql::exec(_mysql::obj, itr,
		"CREATE TEMPORARY TABLE _tmp_u2scc AS "
		"SELECT "
			"(u2scc2.subreddit_id IN (SELECT subreddit_id FROM _tmp_subreddits)) AS is_tagged,"
			"u2scc2.subreddit_id,"
			"u2scc2.user_id,"
			"u2scc2.count "
		"FROM user2subreddit_cmnt_count u2scc1 "
		"JOIN user2subreddit_cmnt_count u2scc2 ON u2scc2.user_id=u2scc1.user_id "
		"WHERE u2scc1.subreddit_id IN (SELECT subreddit_id FROM _tmp_subreddits2)"
		"LIMIT ", n_cols
	);
	compsky::mysql::query_buffer(_mysql::obj, _mysql::res, "SELECT r.name, s2cc.count, u2scc.is_tagged, u2scc.subreddit_id, GROUP_CONCAT(u2scc.user_id), GROUP_CONCAT(u2scc.count) FROM _tmp_u2scc u2scc JOIN subreddit r ON r.id=u2scc.subreddit_id JOIN subreddit2cmnt_count s2cc ON s2cc.id=u2scc.subreddit_id GROUP BY u2scc.subreddit_id");
}


void read_from_qry(AnnoyingIndex& annoyer,  IndexMap<CString, IndexType>& subreddit_indx2name,  Subreddit const from_subreddit_name,  const unsigned int n_cols){
	SubredditType subreddit_id;
	
	IndexMap<SubredditType, IndexType> subreddit_id2indx;
	subreddit_id2indx.reserve(n_cols);
	
	char* subreddit_name;
	uint32_t subreddit_cmnt_count;
	char* user_ids_str;
	char* counts_str;
	
	IndexMap<UserType, UserIndexType> user2indx;
	
	/*
		subreddit_id  user_id1,user_id2,...,user_idN  count1,count2,...,countN
		NODE          (user_id1,count1), (user_id2,count2), ... (user_idN,countN)
		              Map user_id to index using user2indx
		NODE          count_of_user_with_index_1, ...
	*/
	
	static AnnoyingDistanceType* subreddit_counts = static_cast<AnnoyingDistanceType*>(malloc(n_cols * sizeof(AnnoyingDistanceType)));
	
	int i = 0;
	while(compsky::mysql::assign_next_row(_mysql::res, &_mysql::row, &subreddit_name, &subreddit_cmnt_count, &subreddit_id, &user_ids_str, &counts_str)){
		subreddit_id2indx.add_unsafe(subreddit_id);
		subreddit_indx2name.add_unsafe(subreddit_name);
		++i;
		
		parse(user_ids_str, counts_str, subreddit_cmnt_count, subreddit_counts, user2indx);
		
		annoyer.add_item(i, subreddit_counts); // NOTE: Despite what their documentation says, it appears that it still works with acontinuous indexes... so perhaps could replace i with subreddit_id
		
		memset(subreddit_counts,  0,  n_cols * sizeof(AnnoyingDistanceType));
	}
}


void read_from_qry(AnnoyingIndex& annoyer,  IndexMap<CString, IndexType>& subreddit_indx2name,  Tag const from_subreddit_name,  const unsigned int n_cols){
	SubredditType subreddit_id;
	
	bool is_tagged;
	char* subreddit_name;
	uint32_t subreddit_cmnt_count;
	char* user_ids_str;
	char* counts_str;
	
	IndexMap<UserType, UserIndexType> user2indx;
	
	/*
		subreddit_id  user_id1,user_id2,...,user_idN  count1,count2,...,countN
		NODE          (user_id1,count1), (user_id2,count2), ... (user_idN,countN)
		              Map user_id to index using user2indx
		NODE          count_of_user_with_index_1, ...
	*/
	
	static AnnoyingDistanceType* subreddit_counts = static_cast<AnnoyingDistanceType*>(malloc(n_cols * sizeof(AnnoyingDistanceType)));
	
	
	subreddit_indx2name.add_unsafe(from_subreddit_name.name);
	static AnnoyingDistanceType* tag_counts = static_cast<AnnoyingDistanceType*>(malloc(n_cols * sizeof(AnnoyingDistanceType)));
	
	
	int i = 1;
	while(compsky::mysql::assign_next_row(_mysql::res, &_mysql::row, &subreddit_name, &subreddit_cmnt_count, &is_tagged, &subreddit_id, &user_ids_str, &counts_str)){
		parse(user_ids_str, counts_str, subreddit_cmnt_count, subreddit_counts, user2indx);
		
		if (is_tagged){
			for (auto i = 0;  i < n_cols;  ++i){
				printf("Including %s\n", subreddit_name);
				tag_counts[i] += subreddit_counts[i];
			}
		} else {
			subreddit_indx2name.add_unsafe(subreddit_name);
			++i;
			annoyer.add_item(i, subreddit_counts); // NOTE: Despite what their documentation says, it appears that it still works with acontinuous indexes... so perhaps could replace i with subreddit_id
		}
		
		memset(subreddit_counts,  0,  n_cols * sizeof(AnnoyingDistanceType));
	}
	std::cerr << +i << " subreddits from " << +n_cols << " rows" << std::endl;
}


template<typename SubredditOrSomething>
void get_closest_to(AnnoyingIndex& annoyer,  const unsigned int n_cols,  const unsigned int n_trees,  SubredditOrSomething const from_subreddit_name){
    tmp_tbl_from_qry(from_subreddit_name, n_cols);
	// A temporary table is necessary because older MySQL/MariaDB versions do not support LIMIT inside subqueries
	
	IndexMap<CString, IndexType> subreddit_indx2name;
	
	read_from_qry(annoyer, subreddit_indx2name, from_subreddit_name, n_cols);
	
	annoyer.build(n_trees);
	
	std::vector<IndexType> neighbor_index;
	std::vector<AnnoyingDistanceType> neighbor_dist;
	constexpr const int n_neighbours = 10;
	neighbor_index.reserve(n_neighbours);
	neighbor_dist.reserve(n_neighbours);
	
	const IndexType indx = get_index_of(from_subreddit_name, subreddit_indx2name);
	annoyer.get_nns_by_item(indx,  n_neighbours + 1,   -1,  &neighbor_index,  &neighbor_dist);
	
	/*
	std::vector<const char*> neighbour_subreddit_names;
	neighbour_subreddit_names.reserve(n_neighbours);
	
	compsky::mysql::query(_mysql::obj, _mysql::res,
		"SELECT id, name "
		"FROM subreddit "
		"WHERE id IN ("
			
		")"
	);*/
	
	printf("Distance from %s: %s\n", from_subreddit_name.which, from_subreddit_name.name);
	printf("Distance\tSubreddit\n");
	for (auto j = 0;  j < neighbor_index.size();  ++j){
		const AnnoyingDistanceType a = neighbor_dist.at(j);
		const IndexType k = neighbor_index.at(j);
		
		if (k >= subreddit_indx2name.size()){
			std::cerr << "k is out of range for subreddit_indx2name" << std::endl;
			std::cerr << +a << '\t' << +k << std::endl;
			break;
		}
		
		std::cout << +a << '\t' << subreddit_indx2name.at_index(k).data() << std::endl;
		//printf("%lf\t%u\n", neighbor_dist.at(j), subreddit_indx2name.at(neighbor_index.at(j)));
	}
}



int main(const int argc,  const char* const* const argv){
	compsky::mysql::init(_mysql::obj, _mysql::auth, _mysql::auth_sz, getenv("RSCRAPER_MYSQL_CFG"));
	
	if(argc != 5){
		fprintf(stderr,
"USAGE\n"
"	./rscraper-annoying N_COLS N_TREES WHICH_TYPE WHICH_NAME\n"
"OPTIONS\n"
"	WHICH_TYPE\n"
"		r subreddit\n"
"		t tag\n"
		);
		return 1;
	}
	
	const unsigned int n_cols  = s2n2<unsigned int>(argv[1]);
	const unsigned int n_trees = s2n2<unsigned int>(argv[2]);
	assert(argv[3][0] == 't' and argv[3][1] == 0);
	const char* const which_name = argv[4];
	
	const Tag which(which_name);
	AnnoyingIndex annoyer(n_cols);
	get_closest_to(annoyer, n_cols, n_trees, which);
	
	compsky::mysql::wipe_auth(_mysql::auth, _mysql::auth_sz);
}
