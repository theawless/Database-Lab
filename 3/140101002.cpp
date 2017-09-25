#include <iostream>
#include <vector>
#include <algorithm>
#include <queue>
#include <cmath>
#include <fstream>

using namespace std;

unsigned int B; // input_buffer size
unsigned int r; // key per page
unsigned int b; // block factor
unsigned int F; // fan in

unsigned int write_page_faults = 0;
unsigned int read_page_faults = 0;

// runs<run<page<key>>>
vector<vector<vector<int>>> *in_pass = new vector<vector<vector<int>>>();
vector<vector<vector<int>>> *out_pass = new vector<vector<vector<int>>>();

void display_pass(const vector<vector<vector<int>>> *pass) {
    for (unsigned int i = 0; i < (*pass).size(); i++) {
        if (!(*pass)[i].empty()) {
            cout << "   " << "Run " << i << ":" << endl;
        }
        for (unsigned int j = 0; j < (*pass)[i].size(); j++) {
            if (!(*pass)[i][j].empty()) {
                cout << "       " << "Page " << j << ":";
            }
            for (unsigned int k = 0; k < (*pass)[i][j].size(); k++) {
                cout << " " << (*pass)[i][j][k];
            }
            if (!(*pass)[i][j].empty()) {
                cout << endl;
            }
        }
    }
}

void display_output(const unsigned int stage) {
    cout << "----" << " Pass " << stage << " ----" << endl;
    cout << "Input" << endl;
    display_pass(in_pass);
    cout << "Output" << endl;
    display_pass(out_pass);
    cout << endl;
}

void display_write_page_fault(const unsigned int i) {
	cout << "Write page fault for output Page [" << i << "]" << endl;
	write_page_faults++;
}

void display_read_page_fault(const unsigned int i, const unsigned int j,const unsigned int k) {
	cout << "Read page fault for R" << i << "_" << j << " Page[" << k << "]" << endl;
	read_page_faults++;
}

bool pass_n_is_input_buffer_empty(const vector<vector<deque<int>>> &input_buffer) {
    for (vector<deque<int>> pages:input_buffer) {
        if (!pages.empty()) {
			for (deque<int> page:pages) {
				if (!page.empty()) {
		            return true;
				}
			}
        }
    }
    return false;
}

void pass_n_out(const unsigned int i, vector<vector<deque<int>>> &input_buffer, vector<deque<int>> &output_buffer, vector<int> &run_index) {
    unsigned int j = 0;
    unsigned int pp = 0;
    (*out_pass).push_back(vector<vector<int>>());
    while (pass_n_is_input_buffer_empty(input_buffer)) {
        int min_key_input_buffer_out_i = -1, min_key_input_buffer_in_i = -1, min_key = numeric_limits<int>::max();
        for (unsigned int k = 0; k < input_buffer.size(); k++) {
        	for (unsigned int l = 0; l < input_buffer[k].size(); l++) {
				if (!input_buffer[k][l].empty() && min_key > input_buffer[k][l].front()) {
					min_key = input_buffer[k][l].front();
					min_key_input_buffer_in_i = l;
					min_key_input_buffer_out_i = k;
				}
			}
        }
        if (min_key != numeric_limits<int>::max()) {
            input_buffer[min_key_input_buffer_out_i][min_key_input_buffer_in_i].pop_front();
            
            
            int output_buffer_page = -1;
            for (unsigned int k = 0; k < output_buffer.size(); k++) {
            	if (output_buffer[k].size() < r) {
            		output_buffer_page = k;
            	}
            }
            if (output_buffer_page != -1) {
	        			        
		        output_buffer[output_buffer_page].push_back(min_key);
		        if (output_buffer[output_buffer_page].size() == r) {
					(*out_pass)[i].push_back(vector<int>(output_buffer[output_buffer_page].begin(), output_buffer[output_buffer_page].end()));
					output_buffer[output_buffer_page].clear();
		        	//write
		        	display_write_page_fault(j);
		        }
	        	pp++;
	        	if(pp==r){
	        		pp = 0;
	        		j++;
	        	}
		        if(j==b) {
		        	j=0;
		        }            

			}
			else {
				cerr<< "weird state, output buffer is full, but was not cleared";
			}

            if (input_buffer[min_key_input_buffer_out_i][min_key_input_buffer_in_i].empty()
                && (*in_pass).size() > min_key_input_buffer_out_i + (i * F)
                && (*in_pass)[min_key_input_buffer_out_i + (i * F)].size() > run_index[min_key_input_buffer_out_i] + 1) {
                run_index[min_key_input_buffer_out_i]++;
				input_buffer[min_key_input_buffer_out_i][min_key_input_buffer_in_i] = deque<int>(
					(*in_pass)[min_key_input_buffer_out_i + (i * F)][run_index[min_key_input_buffer_out_i]].begin(),
					(*in_pass)[min_key_input_buffer_out_i + (i * F)][run_index[min_key_input_buffer_out_i]].end()
				);
				//read
                display_read_page_fault(i, min_key_input_buffer_out_i + (i * F), min_key_input_buffer_out_i);
            }
        }
    }
}

void pass_n_in(const unsigned int i, vector<vector<deque<int>>> &input_buffer, vector<int> &run_index) {
    for (unsigned int k = 0; k < F; k++) {
        if ((*in_pass).size() > k + (i * F)) {
            if (!(*in_pass)[k + (i * F)].empty() && !(*in_pass)[k + (i * F)][0].empty()) {
                run_index.push_back(-1);
            }
            for (int l = 0; l < b; l++) {
                if ((*in_pass)[k + (i * F)].size() > l && !(*in_pass)[k + (i * F)][l].empty()) {
                    run_index[k]++;
                    input_buffer[k][l] = deque<int>((*in_pass)[k + (i * F)][l].begin(), (*in_pass)[k + (i * F)][l].end());
                    //read
                    display_read_page_fault(i, k + (i * F), l);
                }
            }
        }
    }
}

bool pass_n() {
    delete in_pass;
    in_pass = out_pass;
    out_pass = new vector<vector<vector<int>>>();
    if ((*in_pass).size() <= 1) {
        return false;
    }

    for (unsigned int i=0; i * F < (*in_pass).size(); i++) {
        // <input_buffer<pages<page<key>>>>
        vector<vector<deque<int>>> input_buffer = vector<vector<deque<int>>>(F, vector<deque<int>>(b));
		vector<deque<int>> output_buffer = vector<deque<int>>(b);
        vector<int> run_index = vector<int>();
        pass_n_in(i, input_buffer, run_index);
        pass_n_out(i, input_buffer, output_buffer, run_index);
    }
    return true;
}

void pass_0_out(const unsigned int i, vector<int> &input_buffer) {
    for (unsigned int j = 0; j < B; j++) {
        (*out_pass)[i].push_back(vector<int>());
        for (unsigned int k = 0; k < r; k++) {
            if (!input_buffer.empty()) {
                (*out_pass)[i][j].push_back((input_buffer.back()));
                input_buffer.pop_back();
            }
        }
        if ((*out_pass)[i].back().empty()) {
            (*out_pass)[i].pop_back();
        }
    }
    if ((*in_pass).back().empty()) {
        (*in_pass).pop_back();
    }
    if ((*out_pass).back().empty()) {
        (*out_pass).pop_back();
    }
}

void pass_0_in(const unsigned int i, vector<int> &input_buffer, unsigned int &length) {
    for (unsigned int j = 0; j < B; j++) {
        (*in_pass)[i].push_back(vector<int>());
        for (unsigned int k = 0; k < r; k++) {
            int num;
            if (cin >> num && length > 0) {
                (*in_pass)[i][j].push_back(num);
                input_buffer.push_back(num);
                length--;
            }
        }
        if ((*in_pass)[i].back().empty()) {
            (*in_pass)[i].pop_back();
        }
    }
}

void pass_0() {
	unsigned int length;
	cin >> length;
    for (unsigned int i = 0; cin && length > 0; i++) {
        (*in_pass).push_back(vector<vector<int>>());
        (*out_pass).push_back(vector<vector<int>>());
        vector<int> input_buffer = vector<int>();
        pass_0_in(i, input_buffer, length);
        sort(input_buffer.rbegin(), input_buffer.rend());
        pass_0_out(i, input_buffer);
    }
}

int main(const int argc, const char **argv) {
    if (argc != 5) {
        cerr << "Invalid number of arguments" << endl;
        return -1;
    }
    string input_filename = string(argv[1]);
    ifstream input(input_filename);
    cin.rdbuf(input.rdbuf());
    B = (unsigned int) stoi(argv[2]);
    r = (unsigned int) stoi(argv[3]);
    b = (unsigned int) stoi(argv[4]);
    F = (unsigned int) floor((B - b) / b);
    if (F < 2) {
        cerr << "Fan in is less than 2";
        return -1;
    }
    //unsigned int stage = 0;
    pass_0();
    //display_output(stage++);
    while (pass_n()) {
       //display_output(stage++);
    }
    cout << write_page_faults << read_page_faults;
    return 0;
}
