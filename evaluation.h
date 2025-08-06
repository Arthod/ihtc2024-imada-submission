#ifndef EVALUATION_H
#define EVALUATION_H

#include <vector>
#include <string>

class Evaluation {
public:
    std::vector<int> values;

    Evaluation(std::vector<int> values);

    std::string to_string();

    bool operator <(const Evaluation& other);
    bool operator <(const int& other);
    Evaluation operator +(const Evaluation& other);
    bool operator ==(const Evaluation& other);
    bool operator !=(const Evaluation& other);
    bool operator <=(const Evaluation& other);
    bool operator <=(const int& other);
};

#endif