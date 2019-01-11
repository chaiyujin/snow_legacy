#pragma once
#include <vector>
#include <stdint.h>
#include <iostream>
#include <iomanip>

struct ModelShared {
    double              mScale;
    std::vector<double> mIden;

    friend std::ostream &operator<<(std::ostream &out, const ModelShared &param);
    friend std::istream &operator>>(std::istream &in,  ModelShared &param);
};

inline std::ostream &operator<<(std::ostream &out, const ModelShared &param) {
    out.precision(std::numeric_limits<double>::max_digits10);
    out << "scale:     [ " << std::setw(32) << param.mScale << " ]\n";
    int len = param.mIden.size();
    out << "iden: " << std::setw(2) << len << "   [ ";
    for (int i = 0; i < len; ++i) {
        out << std::setw(32) << param.mIden[i];
        if (i + 1 < len) { out << " , "; if (i % 5 == 4) out << "\n             "; }
        else             { out << " ]\n"; }
    }
    return out;
}

inline std::istream &operator>>(std::istream &in,  ModelShared &param) {
    std::string str;
    in >> str; if (str != "scale:") { printf("[scale]: error at istream >>!"); exit(1); };
    in >> str;
    in >> param.mScale;
    std::getline(in, str);

    int len;
    in >> str; if (str != "iden:") { printf("[iden]: error at istream >>"); exit(1); };
    in >> len;
    param.mIden.resize(len);
    for (int i = 0; i < len; ++i)
        in >> str >> param.mIden[i];
    std::getline(in, str);

    return in;
}

struct ModelFrame {
    int64_t             mTimestamp;
    double              mRotation[3];
    double              mTranslation[3];
    std::vector<double> mExpr;

    friend std::ostream &operator<<(std::ostream &out, const ModelFrame &frame);
    friend std::istream &operator>>(std::istream &in,  ModelFrame &frame);
};

inline std::ostream &operator<<(std::ostream &out, const ModelFrame &frame) {
    out << frame.mTimestamp << std::endl;
    out.precision(std::numeric_limits<double>::max_digits10);
    out << "rotateYXZ: [ " << std::setw(32) << frame.mRotation[0] << " , " << std::setw(32) << frame.mRotation[1] << " , " << std::setw(32) << frame.mRotation[2] << " ]\n"
        << "translate: [ " << std::setw(32) << frame.mTranslation[0] << " , " << std::setw(32) << frame.mTranslation[1] << " , " << std::setw(32) << frame.mTranslation[2] << " ]\n";
    int len = frame.mExpr.size();
    char type;
    if (len == 47) type = 'F';
    else           type = 'E';
    out << "expr: " << type << " " << std::setw(2) << len << " [ ";
    for (int i = 0; i < len; ++i) {
        out << std::setw(32) << frame.mExpr[i];
        if (i + 1 < len) { out << " , "; if (i % 5 == 4) out << "\n             "; }
        else             { out << " ]\n"; }
    }
    return out;
}

inline std::istream &operator>>(std::istream &in,  ModelFrame &frame) {
    std::string str; char typ;

    in >> frame.mTimestamp;
    std::getline(in, str);

    in >> str; if (str != "rotateYXZ:") { printf("[pose]: error at istream >>!"); exit(1); };
    in >> str;
    in >> frame.mRotation[0] >> str >> frame.mRotation[1] >> str >> frame.mRotation[2];
    std::getline(in, str);

    in >> str; if (str != "translate:") { printf("[pose]: error at istream >>!"); exit(1); };
    in >> str;
    in >> frame.mTranslation[0] >> str >> frame.mTranslation[1] >> str >> frame.mTranslation[2];
    std::getline(in, str);

    int len;
    in >> str; if (str != "expr:") { printf("[expr]: error at istream >>"); exit(1); };
    in >> typ;
    in >> len;
    frame.mExpr.resize(len);
    for (int i = 0; i < len; ++i)
        in >> str >> frame.mExpr[i];
    std::getline(in, str);

    return in;
}
