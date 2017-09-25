#include <iostream>
#include <vector>
#include <algorithm>
#include <queue>
#include <cmath>
#include <fstream>

using namespace std;

unsigned int B = 3; // buffer size
unsigned int r = 2; // key per page
unsigned int b = 1; // block factor
unsigned int F = (unsigned int) floor((B - b) / b); // fan in

// *<run<page<key>>>
vector<vector<vector<int>>> *in_pass = new vector<vector<vector<int>>>();
vector<vector<vector<int>>> *out_pass = new vector<vector<vector<int>>>();

void display_pass(vector<vector<vector<int>>> *pass) {
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

void display_output(unsigned int stage) {
    cout << "----" << " Pass " << stage << " ----" << endl;
    cout << "Input" << endl;
    display_pass(in_pass);
    cout << "Output" << endl;
    display_pass(out_pass);
    cout << endl;
}

bool pass_n_is_buffer_empty(vector<priority_queue<int, vector<int>, greater<int>>> &buffer) {
    for (priority_queue<int, vector<int>, greater<int>> page:buffer) {
        if (!page.empty()) {
            return true;
        }
    }
    return false;
}

void pass_n_out(unsigned int i, vector<priority_queue<int, vector<int>, greater<int>>> &buffer, vector<int> &index) {
    unsigned int j = 0;
    (*out_pass).push_back(vector<vector<int>>());
    (*out_pass)[i].push_back(vector<int>());
    while (pass_n_is_buffer_empty(buffer)) {
        int min_key_buffer_index = -1, min_key = numeric_limits<int>::max();
        for (unsigned int k = 0; k < buffer.size(); k++) {
            if (buffer[k].size() > 0 && min_key > buffer[k].top()) {
                min_key = buffer[k].top();
                min_key_buffer_index = k;
            }
        }
        if (min_key != numeric_limits<int>::max()) {
            buffer[min_key_buffer_index].pop();
            if (buffer[min_key_buffer_index].size() != r && buffer[min_key_buffer_index].size() % r == 0
                && (*in_pass).size() > min_key_buffer_index + (i * F)
                && (*in_pass)[min_key_buffer_index + (i * F)].size() > index[min_key_buffer_index] + 1) {
                index[min_key_buffer_index]++;
                for (int e: (*in_pass)[min_key_buffer_index + (i * F)][index[min_key_buffer_index]]) {
                    buffer[min_key_buffer_index].push(e);
                }
            }
            if ((*out_pass)[i][j].size() == r) {
                j++;
                (*out_pass)[i].push_back(vector<int>());
            }
            (*out_pass)[i][j].push_back(min_key);
        }
    }
}

void pass_n_in(unsigned int i, vector<priority_queue<int, vector<int>, greater<int>>> &buffer, vector<int> &index) {
    for (unsigned int k = 0; k < F; k++) {
        if ((*in_pass).size() > k + (i * F)) {
            if (!(*in_pass)[k + (i * F)].empty() && !(*in_pass)[k + (i * F)][0].empty()) {
                index.push_back(-1);
                buffer.push_back(priority_queue<int, vector<int>, greater<int>>());
            }
            for (int l = 0; l < b; l++) {
                if ((*in_pass)[k + (i * F)].size() > l && !(*in_pass)[k + (i * F)][l].empty()) {
                    index[k]++;
                    for (int e: (*in_pass)[k + (i * F)][l]) {
                        buffer[k].push(e);
                    }
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
    unsigned int i = 0;
    while (i * F < (*in_pass).size()) {
        // <buffer<page<key>>>
        vector<priority_queue<int, vector<int>, greater<int>>> buffer = vector<priority_queue<int, vector<int>, greater<int>>>();
        // index for each run
        vector<int> index = vector<int>();
        pass_n_in(i, buffer, index);
        pass_n_out(i, buffer, index);
        i++;
    }
    return true;
}

void pass_0_out(unsigned int i, vector<int> &buffer) {
    for (unsigned int j = 0; j < B; j++) {
        (*out_pass)[i].push_back(vector<int>());
        for (unsigned int k = 0; k < r; k++) {
            if (!buffer.empty()) {
                (*out_pass)[i][j].push_back((buffer.back()));
                buffer.pop_back();
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

void pass_0_in(unsigned int i, vector<int> &buffer) {
    for (unsigned int j = 0; j < B; j++) {
        (*in_pass)[i].push_back(vector<int>());
        for (unsigned int k = 0; k < r; k++) {
            int num;
            if (cin >> num) {
                (*in_pass)[i][j].push_back(num);
                buffer.push_back(num);
            }
        }
        if ((*in_pass)[i].back().empty()) {
            (*in_pass)[i].pop_back();
        }
    }
}

void pass_0() {
    for (unsigned int i = 0; cin; i++) {
        (*in_pass).push_back(vector<vector<int>>());
        (*out_pass).push_back(vector<vector<int>>());
        vector<int> buffer = vector<int>();
        pass_0_in(i, buffer);
        sort(buffer.rbegin(), buffer.rend());
        pass_0_out(i, buffer);
    }
}

int main(int argc, char **argv) {
    if (argc != 5) {
        cerr << "Invalid number of arguments" << endl;
        return -1;
    }
    B = (unsigned int) stoi(argv[1]);
    r = (unsigned int) stoi(argv[2]);
    b = (unsigned int) stoi(argv[3]);
    F = (unsigned int) floor((B - b) / b);
    if (F < 2) {
        cerr << "Fan in is less than 2";
        return -1;
    }
    string input_filename = string(argv[4]);
    ifstream input(input_filename);
    cin.rdbuf(input.rdbuf());

    unsigned int pass_index = 0;
    pass_0();
    display_output(pass_index++);
    while (pass_n()) {
        display_output(pass_index++);
    }
    return 0;
}