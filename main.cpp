#include "BTree.h"
#include "btree_utils.h"

#include <iostream>
#include <ctime>
#include <fstream>

using namespace std;

const uint32_t RECORDS_IN_NODE = 3;

void show_help(bool echo) {
    cout << "SBD - Projekt 2" << endl << endl;
    cout << "help / h - pokaz liste polecen." << endl;
    cout << "insert_random / ir <num> - dodaj <num> losowych rekordow." << endl;
    cout << "insert / i <key> <value> - wpisz recznie rekord." << endl;
    cout << "delete / d <key> - usun rekord o kluczu <key>." << endl;
    cout << "get / g <key> - pobierz wartosc dla klucza <key>." << endl;
    cout << "update / u <key> <value> - zaktualizuj wartość <value> rekordu o kluczu <key>." << endl;
    cout << "ordered / o - wyswietl rekordy w kolejnosci." << endl;
    cout << "echo / e - przelacz wyswietlalnie zawartosci pliku. (Aktualnie " << (echo ? "ON" : "OFF") << ")" << endl;
    cout << "load_file / lf <file_path> - zaladuj instrukcje z pliku o sciezce <file_path>." << endl;
    cout << "exit / e - zamknij program." << endl << endl;
}

uint64_t generate_key() {
    const uint32_t MAX_KEY = 1000000;
    static bool keys[MAX_KEY + 1];
    keys[0] = true;

    auto key = static_cast<uint64_t>((MAX_KEY - 1) * (1.0 * rand() / RAND_MAX)) + 1;
    if (!keys[key]) {
        keys[key] = true;
        return key;
    }

    for (uint64_t i = 0; i < MAX_KEY + 1; i++) {
        key = (key + 1) % (MAX_KEY + 1);

        if (!keys[key]) {
            keys[key] = true;
            return key;
        }
    }

    throw "Nie mozna wygenerowac wiecej kluczy";
}

std::string generate_value() {
    static const char character[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    static const char number[]= "0123456789";

    char str[8] = "";

    for (int i = 0; i < 2; i++) {
        str[i] = character[rand() % (sizeof(character) - 1)];
    }

    for (int i = 2; i < 8; i++) {
        str[i] = number[rand() % (sizeof(number) - 1)];
    }

    str[7] = 0;
    return std::string(str);
}

int main() {
    btree::Container<RECORDS_IN_NODE> tree("file.tree");
    std::srand((unsigned int) time(NULL));
    std::istream *in = &std::cin;
    std::ifstream file;
    std::string cmd;
    bool echo = false;

    show_help(echo);
    while (true) {
        (*in) >> cmd;
        if (cmd == "h" || cmd == "help") {
            show_help(echo);
        } else if(cmd == "ir" || cmd == "insert_random") {
            uint64_t num;
            (*in) >> num;

            for (int i = 0; i < num; ++i) {
                auto key = generate_key();
                auto value = generate_value();

                auto ret = tree.insert(key, value.c_str());
                if (ret != btree::SUCCESS) {
                    perror(btree::code_to_string(ret).c_str());
                }
            }
        } else if(cmd == "i" || cmd == "insert") {
            uint64_t key;
            std::string value;

            (*in) >> key >> value;
            auto ret = tree.insert(key, value.c_str());
            if (ret != btree::SUCCESS) {
                perror(btree::code_to_string(ret).c_str());
            }
        } else if(cmd == "d" || cmd == "delete") {
            uint64_t key;

            (*in) >> key;
            auto ret = tree.remove(key);
            if (ret != btree::SUCCESS) {
                perror(btree::code_to_string(ret).c_str());
            }
        } else if(cmd == "g" || cmd == "get") {
            uint64_t key;
            char value[8];

            (*in) >> key;
            auto ret = tree.get_value(key, value);
            if (ret != btree::SUCCESS) {
                perror(btree::code_to_string(ret).c_str());
            }

            cout << "Value: " << value << endl;
        } else if(cmd == "u" || cmd == "update") {
            uint64_t key;
            std::string value;

            (*in) >> key >> value;
            auto ret = tree.update(key, value.c_str());
            if (ret != btree::SUCCESS) {
                perror(btree::code_to_string(ret).c_str());
            }
        } else if(cmd == "o" || cmd == "ordered") {
            tree.print_data_ordered(cout);
        } else if(cmd == "echo") {
            echo = !echo;
            if (echo) {
                cout << "Echo wlaczone!" << endl;
            } else {
                cout << "Echo wylaczone!" << endl;
            }
        } else if(cmd == "lf" || cmd == "load_file") {
            std::string path;

            (*in) >> path;

            file.open(path);
            if (file.good()) {
                in = &file;
            } else {
                perror("Bledna sciezka do pliku!");
            }
        } else if(cmd == "e" || cmd == "exit") {
            break;
        } else if(cmd == "eof") {
            in = &std::cin;
            file.close();
        } else {
            continue;
        }

        cout << endl << "Wykonano " << btree::g_iinfo.writes << " zapisow i " << btree::g_iinfo.reads << " odczytow" << endl << endl;

        if (echo) {
            tree.print_raw_file(cout);
        }

        btree::g_iinfo.reset();
    }

    return 0;
}
