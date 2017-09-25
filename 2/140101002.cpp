#include <iostream>
#include <vector>
#include <algorithm>
#include <fstream>

using namespace std;

unsigned int global_depth = 1;
unsigned int keys_per_bucket = 2;
unsigned int overflow_pages = 0;

typedef struct {
	bool overflow = false;
	bool display = false;
    unsigned int local_depth = global_depth;
    vector<int> data = vector<int>();
} Bucket;

vector<Bucket *> directory = vector<Bucket *>();

unsigned int get_nth_lsb(int num, int n) {
    return (unsigned int) num & (1 << n);
}

unsigned int get_hash_val(int num, unsigned int depth) {
    // do something with num
    unsigned int hash = 0;
    for (unsigned int i = 0; i < depth; i++)
        hash += get_nth_lsb(num, i);
    return hash;
}

string get_bin_repr(int num, int n) {
    string bin_repr = "";
    for (int i = n - 1; i >= 0; i--)
        bin_repr += to_string((num >> i) & 1);
    return bin_repr;
}

void display() {

	cout << "Directory slot | Bucket pointer" << endl;
	for (unsigned int i=0; i < directory.size(); i++) {
		cout << get_bin_repr(i, global_depth) << " | " << get_bin_repr(i, directory[i]->local_depth) << endl;
		directory[i]->display = false;
	}
	cout << endl << "Bucket id | Contents | Overflow" << endl;
	for (unsigned int i=0; i < directory.size(); i++) {
		if (directory[i]->display)
			continue;
		//cout << get_bin_repr(i, directory[i]->local_depth) << " ";
		cout << get_bin_repr(i, global_depth) << " ";
		cout << " | ";
		directory[i]->display = true;     
		bool data =false;       
		for (unsigned int j = 0; j < keys_per_bucket; j++) {
			if(j < directory[i]->data.size()){
	        	data =true;
	            cout << directory[i]->data[j] << " ";
        	}
        }
        if(!data)
        	cout << " empty ";
        bool over=false;
        cout << " | ";
		for (unsigned int j = keys_per_bucket; j < 2*keys_per_bucket; j++) {
			if(j < directory[i]->data.size()){
				over = true;
	            cout << directory[i]->data[j] << " ";
        	}
        }
        if(!over)
        	cout << " no overflow page";
        cout << endl;
	}
	cout << endl;
/*
    for (unsigned int i = 0; i < directory.size(); i++) {
        cout << "global " << get_bin_repr(i, global_depth) << " local " << get_bin_repr(i, directory[i]->local_depth)
             << " bucket" << ": ";
        for (unsigned int j = 0; j < directory[i]->data.size(); j++) {
            cout << directory[i]->data[j] << " ";
        }
        cout << endl;
    }
*/
}

void insert(int value) {
	//cout << "Key " << value << " " ;
    unsigned int hash_value = get_hash_val(value, global_depth);
    if (directory[hash_value]->data.size() < keys_per_bucket) {
        directory[hash_value]->data.push_back(value);
     	//cout << "Inserted in bucket: " << get_bin_repr(value, directory[hash_value]->local_depth)  << endl;   
    }
    else if (directory[hash_value]->data.size() == keys_per_bucket) {
        Bucket *split_bucket = new Bucket(), *old_bucket = directory[hash_value];
        unsigned int old_local_depth = old_bucket->local_depth;
        unsigned int new_local_depth = split_bucket->local_depth = old_bucket->local_depth = old_local_depth + 1;

        // old bucket has 0 at msb and split bucket has 1
        for (vector<int>::iterator i = old_bucket->data.begin(); i != old_bucket->data.end();) {
            unsigned int new_hash = get_hash_val(*i, new_local_depth);
            if (new_hash & (1 << (new_local_depth - 1))) {
                split_bucket->data.push_back(*i);
                i = old_bucket->data.erase(i);
            } else {
                ++i;
            }
        }
		// insert new element
        unsigned int new_hash = get_hash_val(value, new_local_depth);
        if (new_hash & (1 << (new_local_depth - 1))) {
            split_bucket->data.push_back(value);
        }
        else {
        	old_bucket->data.push_back(value);
		}        
     	//cout << "Inserted in bucket: " << get_bin_repr(value, new_local_depth)  << endl;   

        if (old_local_depth == global_depth) {
            unsigned int old_global_depth = global_depth;
            unsigned int new_global_depth = global_depth = old_global_depth + 1;
            unsigned long temp_size = directory.size();
            for (unsigned long i = 0; i < temp_size; i++) {
                directory.push_back(directory[i]);
            }
            directory[hash_value + (1 << (new_global_depth - 1))] = split_bucket;
        } else if (old_local_depth < global_depth) {
            for (unsigned int i = 0; i < directory.size(); i++) {
                if (directory[i] == old_bucket) {
                    if (i & (1 << (new_local_depth - 1))) {
                        directory[i] = split_bucket;
                    }
                }
            }
        } else {
            cerr << "Higher local_depth in bucket " << get_bin_repr(hash_value, global_depth)
                 << " than global" << endl;
        }
    } else {
        cerr << "More than " << keys_per_bucket << "in bucket" << get_bin_repr(hash_value, global_depth) << endl;
    }
}


void insert2(int value) {
	//cout << "Key " << value << " " ;
    unsigned int hash_value = get_hash_val(value, global_depth);
    if (directory[hash_value]->data.size() < keys_per_bucket) {
        directory[hash_value]->data.push_back(value);
     	//cout << "Inserted in bucket: " << get_bin_repr(value, directory[hash_value]->local_depth)  << endl;   
    }
    else if (directory[hash_value]->data.size() >= keys_per_bucket && directory[hash_value]->data.size() <= 2*keys_per_bucket) {
    	if(directory[hash_value]->local_depth == global_depth && directory[hash_value]->data.size() < 2*keys_per_bucket) {
		    directory[hash_value]->data.push_back(value);
			directory[hash_value]->overflow = true;
		 	//cout << "Inserted in bucket: " << get_bin_repr(value, directory[hash_value]->local_depth) << endl;       
    	}
    	else if (directory[hash_value]->local_depth < global_depth && directory[hash_value]->data.size() == keys_per_bucket) {
    		//split bucket
		    Bucket *split_bucket = new Bucket(), *old_bucket = directory[hash_value];
		    unsigned int old_local_depth = old_bucket->local_depth;
		    unsigned int new_local_depth = split_bucket->local_depth = old_bucket->local_depth = old_local_depth + 1;

		    // old bucket has 0 at msb and split bucket has 1
		    for (vector<int>::iterator i = old_bucket->data.begin(); i != old_bucket->data.end();) {
		        unsigned int new_hash = get_hash_val(*i, new_local_depth);
		        if (new_hash & (1 << (new_local_depth - 1))) {
		            split_bucket->data.push_back(*i);
		            i = old_bucket->data.erase(i);
		        } else {
		            ++i;
		        }
		    }
			// insert new element
		    unsigned int new_hash = get_hash_val(value, new_local_depth);
		    if (new_hash & (1 << (new_local_depth - 1))) {
		        split_bucket->data.push_back(value);
		    }
		    else {
		    	old_bucket->data.push_back(value);
			}        
			old_bucket->overflow = old_bucket->data.size() > keys_per_bucket;
			split_bucket->overflow = split_bucket->data.size() > keys_per_bucket;
		 	//cout << "Inserted in bucket: " << get_bin_repr(value, new_local_depth)  << endl;   
		    for (unsigned int i = 0; i < directory.size(); i++) {
		        if (directory[i] == old_bucket) {
		            if (i & (1 << (new_local_depth - 1))) {
		                directory[i] = split_bucket;
		            }
		        }
		    }    	
    	}
		else if(directory[hash_value]->local_depth == global_depth && directory[hash_value]->data.size() == 2*keys_per_bucket) {
    		//split bucket
		    Bucket *split_bucket = new Bucket(), *old_bucket = directory[hash_value];
		    unsigned int old_local_depth = old_bucket->local_depth;
		    unsigned int new_local_depth = split_bucket->local_depth = old_bucket->local_depth = old_local_depth + 1;

		    // old bucket has 0 at msb and split bucket has 1
		    for (vector<int>::iterator i = old_bucket->data.begin(); i != old_bucket->data.end();) {
		        unsigned int new_hash = get_hash_val(*i, new_local_depth);
		        if (new_hash & (1 << (new_local_depth - 1))) {
		            split_bucket->data.push_back(*i);
		            i = old_bucket->data.erase(i);
		        } else {
		            ++i;
		        }
		    }
			// insert new element
		    unsigned int new_hash = get_hash_val(value, new_local_depth);
		    if (new_hash & (1 << (new_local_depth - 1))) {
		        split_bucket->data.push_back(value);
		    }
		    else {
		    	old_bucket->data.push_back(value);
			}        
			old_bucket->overflow = old_bucket->data.size() > keys_per_bucket;
			split_bucket->overflow = split_bucket->data.size() > keys_per_bucket;
		 	//cout << "Inserted in bucket: " << get_bin_repr(value, new_local_depth)  << endl;   
			//split_directory
            unsigned int old_global_depth = global_depth;
            unsigned int new_global_depth = global_depth = old_global_depth + 1;
            unsigned long temp_size = directory.size();
            for (unsigned long i = 0; i < temp_size; i++) {
                directory.push_back(directory[i]);
            }
            directory[hash_value + (1 << (new_global_depth - 1))] = split_bucket;
		}
		else{
			cerr << "WEIRD CASE";
		}
	}
    else {
        cerr << "More than 2*" << keys_per_bucket << "in bucket" << get_bin_repr(hash_value, global_depth) << endl;
    }
}

bool search(int value, Bucket **buc, vector<int>::iterator *pos, bool output) {
	if(output){
		cout << "Key " << value << " " ;
    }
    int hash_value = (int)get_hash_val(value, global_depth);
    if ((int)directory.size() < (hash_value - 1) || directory[hash_value]->data.empty()
        || (*pos = find(directory[hash_value]->data.begin(), directory[hash_value]->data.end(), value))
           == directory[hash_value]->data.end()) {
           if(output){
        		cout << "not found ";
        	}
        return false;
    } else {
        *buc = directory[hash_value];
        if(output){
		    cout << "found at position " << distance(directory[hash_value]->data.begin(), *pos)
		         << " in bucket: " << get_bin_repr(hash_value, global_depth) << "";
        }
        return true;
    }
}

void delete_ignore_merge2(Bucket *buc, vector<int>::iterator pos) {
    buc->data.erase(pos);
	buc->overflow = buc->data.size() > keys_per_bucket;
    //cout << " Delete successful" << endl;
}

void delete_ignore_merge(Bucket *buc, vector<int>::iterator pos) {
    buc->data.erase(pos);
    //cout << " Delete successful" << endl;
}


void search_delete2(int i) {
    Bucket *buc = nullptr;
    vector<int>::iterator pos;
    if (search(i, &buc, &pos, false)) {
        delete_ignore_merge2(buc, pos);
	}
	else {
		cout << "Aborting delete because key not found" << endl;
	}
}

void search_delete(int i) {
    Bucket *buc = nullptr;
    vector<int>::iterator pos;
    if (search(i, &buc, &pos, false)) {
        delete_ignore_merge(buc, pos);
	}
	else {
		cout << "Aborting delete because key not found" << endl;
	}
}

bool handle_input(string input_type) {
    int input_data;
    Bucket *buc = nullptr;
	vector<int>::iterator pos;
	if(overflow_pages) {
		if (input_type == "delete") {
			cin >> input_data;
			search_delete2(input_data);
			return true;
	   	}
		else if (input_type == "search") {
			cin >> input_data;
			search(input_data, &buc, &pos, true);
			cout << endl;
			return true;
		}
		else if (input_type == "insert") {
			cin >> input_data;
			insert2(input_data);
			return true;
		}
		else if (input_type == "status") {
			display();
			return true;
		}
		else if (input_type == "exit") {
			return false;
		}
		else {
			cerr << "Unrecognized character" << endl;
			return false;
		}
	}
	else{
		if (input_type == "delete") {
			cin >> input_data;
			search_delete(input_data);
			return true;
	   	}
		else if (input_type == "search") {
			cin >> input_data;
			search(input_data, &buc, &pos, true);
			cout << endl;
			return true;
		}
		else if (input_type == "insert") {
			cin >> input_data;
			insert(input_data);
			return true;
		}
		else if (input_type == "status") {
			display();
			return true;
		}
		else if (input_type == "exit") {
			return false;
		}
		else {
			cerr << "Unrecognized character" << endl;
			return false;
		}	
	}
}

void setup_directory() {
    for (unsigned int i = 0; i < (1 << global_depth); i++) {
        directory.push_back(new Bucket());
    }
}

int main(int argc, char **argv) {
	if (argc != 5) cerr << "Invalid number of arguments" << argc << endl;
    global_depth = (unsigned int) stoi(argv[1]);
    keys_per_bucket = (unsigned int) stoi(argv[2]);
    overflow_pages = (unsigned int) stoi(argv[3]);
    string input_filename = string(argv[4]);

    ifstream input_file(input_filename);
    streambuf *cinbuf = cin.rdbuf();
    cin.rdbuf(input_file.rdbuf());
    
    setup_directory();
    string input_type;
    do {
    	input_type ="exit";
        cin >> input_type;
    } while (handle_input(input_type));
    return 0;
}
