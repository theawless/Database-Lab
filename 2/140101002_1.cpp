#include <iostream>
#include <vector>
#include <algorithm>
#include <fstream>

using namespace std;

unsigned int global_depth = 1;
unsigned int keys_per_bucket = 2;
unsigned int deletion_type = 0;

typedef struct {
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
		cout << get_bin_repr(i, global_depth) << " " << get_bin_repr(i, directory[i]->local_depth) << endl;
		directory[i]->display = false;
	}
	cout << endl << "Bucket id | Contents" << endl;
	for (unsigned int i=0; i < directory.size(); i++) {
		if (directory[i]->display)
			continue;
		cout << get_bin_repr(i, directory[i]->local_depth) << " ";
		directory[i]->display = true;            
		for (unsigned int j = 0; j < directory[i]->data.size(); j++) {
            cout << directory[i]->data[j] << " ";
        }
        cout << endl;
	}
	cout << endl;

    for (unsigned int i = 0; i < directory.size(); i++) {
        cout << "global " << get_bin_repr(i, global_depth) << " local " << get_bin_repr(i, directory[i]->local_depth)
             << " bucket" << ": ";
        for (unsigned int j = 0; j < directory[i]->data.size(); j++) {
            cout << directory[i]->data[j] << " ";
        }
        cout << endl;
    }

}

void insert(int value) {
    unsigned int hash_value = get_hash_val(value, global_depth);
    if (directory[hash_value]->data.size() < keys_per_bucket) {
        directory[hash_value]->data.push_back(value);
     	cout << "Inserted in bucket: " << get_bin_repr(value, directory[hash_value]->local_depth)  << endl;   
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
     	cout << "Inserted in bucket: " << get_bin_repr(value, new_local_depth)  << endl;   

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

bool search(int value, Bucket **buc, vector<int>::iterator *pos) {
    int hash_value = get_hash_val(value, global_depth);
    if (directory.size() < hash_value - 1 || directory[hash_value]->data.empty()
        || (*pos = find(directory[hash_value]->data.begin(), directory[hash_value]->data.end(), value))
           == directory[hash_value]->data.end()) {
        cout << "Key not found ";
        return false;
    } else {
        *buc = directory[hash_value];
        cout << "Found at position" << distance(directory[hash_value]->data.begin(), *pos)
             << " in bucket: " << get_bin_repr(hash_value, global_depth) << "";
        return true;
    }
}

void delete_ignore_merge(Bucket *buc, vector<int>::iterator pos) {
    buc->data.erase(pos);
    cout << " Delete successful" << endl;
}

bool delete_merge_bucket(Bucket *buc, vector<int>::iterator pos) {
    delete_ignore_merge(buc, pos);
    if (buc->data.size() == 0) {
    	clog <<  "1";
        vector<unsigned int> temp_indexes = vector<unsigned int>();
        for (unsigned int i = 0; i < directory.size(); i++) {
            if (directory[i] == buc) {
                temp_indexes.push_back(i);
    	clog <<  "2";
            }
        }
        int offset = temp_indexes[0] & 1 << (directory[temp_indexes[0]]->local_depth - 1);
        offset = offset ? offset*-1 : 1<< (directory[temp_indexes[0]]->local_depth - 1);
        Bucket *good_bucket = directory[temp_indexes[0] + offset];
        cout <<offset <<" " << get_bin_repr(temp_indexes[0] + offset, directory[temp_indexes[0] + offset]->local_depth);
        for(int i = 0; i< good_bucket->data.size();i++){
        cerr << good_bucket->data[i];
        }
        for (unsigned int i = 0; i < temp_indexes.size(); i++) {
            directory[temp_indexes[i]] = good_bucket;
        }
        good_bucket->local_depth -= 1;
        delete buc;
        return true;
    }
    return false;
}

void delete_merge_directory(Bucket *buc, vector<int>::iterator pos) {
    if (delete_merge_bucket(buc, pos)) {
        unsigned long i = 0, j = directory.size();
        bool merge_required = true;
        while (i != j) {
            if (directory[i++] != directory[j--]) {
                merge_required = false;
            }
        }
        if (merge_required) {
            directory.erase(directory.begin(), directory.begin() + directory.size() / 2);
        }
    }
}

void search_delete(int i) {
    Bucket *buc = nullptr;
    vector<int>::iterator pos;
    if (search(i, &buc, &pos)) {
        switch (deletion_type) {
            case 0:
                delete_ignore_merge(buc, pos);
                break;
            case 1:
                delete_merge_bucket(buc, pos);
                break;
            case 2:
                delete_merge_directory(buc, pos);
                break;
            default:
                delete_merge_bucket(buc, pos);
                break;
        }
	}
	else {
		cout << "Aborting delete" << endl;
	}
}

bool handle_input(string input_type) {
    int input_data;
    Bucket *buc = nullptr;
	vector<int>::iterator pos;
	if (input_type == "delete") {
		cin >> input_data;
		search_delete(input_data);
		return true;
   	}
	else if (input_type == "search") {
		cin >> input_data;
		search(input_data, &buc, &pos);
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

void setup_directory() {
    for (unsigned int i = 0; i < (1 << global_depth); i++) {
        directory.push_back(new Bucket());
    }
}

int main(int argc, char **argv) {
	if (argc != 5) cerr << "Invalid number of arguments" << argc << endl;
    global_depth = (unsigned int) stoi(argv[1]);
    keys_per_bucket = (unsigned int) stoi(argv[2]);
    deletion_type = (unsigned int) stoi(argv[3]);
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
