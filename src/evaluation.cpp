#include "evaluation.h"

Evaluation::Evaluation(std::vector<int> values) : values(values) {}

std::string Evaluation::to_string() {
    std::string str = "[";
    for (int i = 0; i < values.size(); i++) {
        str += std::to_string(values[i]);
        if (i < values.size() - 1) {
            str += ", ";
        }
    }
    str += "]";
    return str;
}

bool Evaluation::operator <(const Evaluation& other) {
    for (int i = 0; i < values.size(); i++) {
        if (values[i] < other.values[i]) {
            return true;
        } else if (values[i] > other.values[i]) {
            return false;
        }
    }
    return false;
}

bool Evaluation::operator <(const int& other) {
    for (int i = 0; i < values.size(); i++) {
        if (values[i] < other) {
            return true;
        } else if (values[i] > other) {
            return false;
        }
    }
    return false;
}

Evaluation Evaluation::operator +(const Evaluation& other) {
    std::vector<int> new_values(values.size(), 0);
    for (int i = 0; i < values.size(); i++) {
        new_values[i] = values[i] + other.values[i];
    }
    return Evaluation(new_values);
}

bool Evaluation::operator ==(const Evaluation& other) {
    for (int i = 0; i < values.size(); i++) {
        if (values[i] != other.values[i]) {
            return false;
        }
    }
    return true;
}

bool Evaluation::operator !=(const Evaluation& other) {
    return !(*this == other);
}

bool Evaluation::operator <=(const Evaluation& other) {
    for (int i = 0; i < values.size(); i++) {
        if (values[i] < other.values[i]) {
            return true;
        } else if (values[i] > other.values[i]) {
            return false;
        }
    }
    return true;
}

bool Evaluation::operator <=(const int& other) {
    for (int i = 0; i < values.size(); i++) {
        if (values[i] < other) {
            return true;
        } else if (values[i] > other) {
            return false;
        }
    }
    return true;
}