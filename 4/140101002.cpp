#include <iostream>
#include <vector>
#include <fstream>
#include <set>

using namespace std;

struct Command
{
    char action;
    unsigned int transaction;
    char resource;
};

struct Transaction
{
    unsigned int id;
    vector<Command> commands;
};

struct AdjList
{
    unsigned int id;
    vector<unsigned int> connected_ids;
};

vector<Transaction> transactions;
vector<Command> commands;
vector<vector<unsigned int>> adk_list;
vector<AdjList> adj_lists;
vector<unsigned int> transaction_order;

unsigned int reach_transaction(unsigned int id)
{
    for (unsigned int i = 0; i < transactions.size(); i++)
    {
        if (id == transactions[i].id)
        {
            return i;
        }
    }
    cerr << "Weird transaction id" << endl;
}

bool depth_first_traversal(const unsigned int vertex, vector<bool> &visited_vertices, vector<bool> &stacked_vertices)
{
    unsigned int vertex_index = reach_transaction(vertex);
    if (!visited_vertices[vertex_index])
    {
        visited_vertices[vertex_index] = true;
        transaction_order.push_back(vertex);
        stacked_vertices[vertex_index] = true;
        for (unsigned int i = 0; i < adj_lists[vertex_index].connected_ids.size(); i++)
        {
            unsigned int vertex_to_visit = adj_lists[vertex_index].connected_ids[i];
            unsigned int vertex_to_visit_index = reach_transaction(vertex_to_visit);
            if (!visited_vertices[vertex_to_visit_index] && depth_first_traversal(vertex_to_visit, visited_vertices, stacked_vertices))
            {
                return true;
            }
            else if (stacked_vertices[vertex_to_visit_index])
            {
                return true;
            }
        }
    }
    stacked_vertices[vertex_index] = false;
    return false;
}

bool has_cycle()
{
    transaction_order = vector<unsigned int>();
    vector<bool> visited_vertices = vector<bool>(transactions.size());
    vector<bool> stacked_vertices = vector<bool>(transactions.size());
    for (unsigned int i = 0; i < adj_lists.size(); i++)
    {
        if (depth_first_traversal(adj_lists[i].id, visited_vertices, stacked_vertices))
        {
            return true;
        }
    }
    return false;
}

bool has_confict(const Command &first, const Command &second)
{
    return !((first.action == 'C' || second.action == 'C') || (first.action == 'R' && second.action == 'R')) && (first.transaction != second.transaction);
}

void display_adj_lists()
{
    for (unsigned int i = 0; i < adj_lists.size(); i++)
    {
        cout << adj_lists[i].id;
        for (unsigned int k = 0; k < adj_lists[i].connected_ids.size(); k++)
        {
            cout << " " << adj_lists[i].connected_ids[k];
        }
        cout << endl;
    }
}

void build_adj_lists()
{
    adj_lists = vector<AdjList>(transactions.size());
    for (unsigned int i = 0; i < transactions.size(); i++)
    {
        adj_lists[i].id = transactions[i].id;
        adj_lists[i].connected_ids = vector<unsigned int>();
    }
    for (unsigned int i = 1; i < commands.size(); i++)
    {
        for (unsigned int k = 0; k < i; k++)
        {
            if (has_confict(commands[k], commands[i]))
            {
                unsigned int j = reach_transaction(commands[k].transaction);
                adj_lists[j].connected_ids.push_back(commands[i].transaction);
            }
        }
    }
    //display_adj_lists();
}

void display_transactions()
{
    cout << "Transactions" << endl;
    for (unsigned int i = 0; i < transactions.size(); i++)
    {
        cout << i << ":";
        for (unsigned int k = 0; k < transactions[i].commands.size(); k++)
        {
            cout << " " << transactions[i].commands[k].action << " " << transactions[i].commands[k].transaction << " " << transactions[i].commands[k].resource;
        }
        cout << endl;
    }
}

void display_commands()
{
    cout << "Commands" << endl;
    for (unsigned int i = 0; i < commands.size(); i++)
    {
        cout << " " << commands[i].action << " " << commands[i].transaction << " " << commands[i].resource << endl;
    }
}

void input()
{
    commands = vector<Command>();
    set<unsigned int> transactions_ids = set<unsigned int>();
    for (unsigned int i = 0; cin; i++)
    {
        commands.push_back(Command());
        if (cin >> commands[i].action)
        {
            cin >> commands[i].transaction;
            if (commands[i].action == 'C')
            {
                commands[i].resource = '#';
            }
            else
            {
                cin >> commands[i].resource;
            }
            transactions_ids.insert(commands[i].transaction);
        }
        else
        {
            commands.pop_back();
        }
    }
    //display_commands();
    
    transactions = vector<Transaction>(transactions_ids.size());
    for (set<unsigned int>::iterator i = transactions_ids.begin(); i != transactions_ids.end(); ++i)
    {
        transactions[distance(transactions_ids.begin(), i)].id = *i;
        transactions[distance(transactions_ids.begin(), i)].commands = vector<Command>();
    }
    for (unsigned int i = 0; i < commands.size(); i++)
    {
        unsigned int k = reach_transaction(commands[i].transaction);
        transactions[k].commands.push_back(commands[i]);
    }
    //display_transactions();
}

void display_serialization()
{
    cout << "Transaction order:";
    for (unsigned int i = 0; i < transaction_order.size(); i++)
    {
        cout << " " << transaction_order[i];
    }
    cout << endl;
    cout << "Command order:";
    for (unsigned int i = 0; i < transaction_order.size(); i++)
    {
        unsigned int j = reach_transaction(transaction_order[i]);
        for (unsigned int k = 0; k < transactions[j].commands.size(); k++)
        {
            cout << " " << transactions[j].commands[k].action << " " << transactions[j].commands[k].transaction << " " << transactions[j].commands[k].resource;
        }
        cout << endl;
        break;
    }
}

vector<unsigned int> find_recovery_conflicts(const unsigned int write_index)
{
    vector<unsigned int> recovery_conflicts_indexes = vector<unsigned int>();
    for (unsigned int i = write_index + 1; i < commands.size(); i++)
    {
        if (commands[i].action == 'R' && commands[i].transaction != commands[write_index].transaction && commands[i].resource == commands[write_index].resource)
        {
            recovery_conflicts_indexes.push_back(i);
        }
    }
    return recovery_conflicts_indexes;
}

unsigned int find_commit_for_id(const unsigned int id, bool &found)
{
    for (unsigned int i = 0; i < commands.size(); i++)
    {
        if (commands[i].action == 'C' && commands[i].transaction == id)
        {
            found = true;
            return i;
        }
    }
    found = false;
    return 0;
}

void clean_commands()
{
    vector<Command> new_commands = vector<Command>();
    for (unsigned int i = 0; i < commands.size(); i++)
    {
        bool found = false;
        unsigned int commit_index = find_commit_for_id(commands[i].transaction, found);
        if (found)
        {
            new_commands.push_back(commands[i]);
        }
        else
        {
            //cerr << "Removed a transaction because it had no commit " << i << endl;
        }
    }
    commands = new_commands;
}

void check_recovery(bool &recoverable, bool &cascade_free)
{
    clean_commands();
    for (unsigned int i = 0; i < commands.size(); i++)
    {
        if (commands[i].action == 'W')
        {
            bool found = false;
            unsigned int write_commit_index = find_commit_for_id(commands[i].transaction, found);
            if (!found)
            {
                cerr << "Transaction without commit, even after cleaning" << endl;
            }
            vector<unsigned int> read_indexes = find_recovery_conflicts(i);
            for (unsigned int j = 0; j < read_indexes.size(); j++)
            {
                bool found = false;
                unsigned int read_commit_index = find_commit_for_id(commands[read_indexes[j]].transaction, found);
                if (!found)
                {
                    cerr << "Transaction without commit, even after cleaning" << endl;
                }
                if (i >= write_commit_index || read_indexes[j] >= read_commit_index)
                {
                    cerr << "Command cannot exist after commit" << endl;
                }
                if (write_commit_index >= read_commit_index)
                {
                    recoverable = false;
                }
                if (write_commit_index >= read_indexes[j])
                {
                    cascade_free = false;
                }
            }
        }
    }
}

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        cerr << "Invalid number of arguments" << endl;
        return -1;
    }
    fstream in(argv[1]);
    cin.rdbuf(in.rdbuf());
    input();
    build_adj_lists();
    if (!has_cycle())
    {
        //display_serialization();
        cout << "Conflict serializable : Yes" << endl;
    }
    else
    {
        cout << "Conflict serializable : No" << endl;
    }

    bool recoverable = true, cascade_free = true;
    check_recovery(recoverable, cascade_free);
    if (recoverable)
    {
        cout << "Recoverable: Yes" << endl;
    }
    else
    {
        cout << "Recoverable: No" << endl;
    }
    if (cascade_free)
    {
        cout << "Cascaded roll back free: Yes" << endl;
    }
    else
    {
        cout << "Cascaded roll back free: No" << endl;
    }
    return 0;
}