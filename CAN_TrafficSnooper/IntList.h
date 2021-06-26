#ifndef _INTLIST_H
#define _INTLIST_H

template<class Type, uint8_t MAX_LENGTH = 20> class IntList {
public:
    IntList() : mLength(0) {

    }
    ~IntList() {

    }
    void addValue(const Type value) {
        if (contains(value)) {
            return;
        }
        if (mLength < MAX_LENGTH) {
            mValues[mLength] = value;
            ++mLength;
        }
    }
    void removeValue(const Type value) {
        for (uint8_t i = 0; i < mLength; ++i) {
            if (mValues[i] == value) {
                for (uint8_t j = i; j < mLength - 1; ++j) {
                    mValues[j] = mValues[j + 1];
                }
                mLength -= 1;
                return;
            }
        }
    }
    void clear() {
        mLength = 0;
    }
    bool contains(const Type value) {
        for (uint8_t i = 0; i < mLength; ++i) {
            if (mValues[i] == value) {
                return true;
            }
        }
        return false;
    }
    uint8_t length() {
        return mLength;
    }
private:
    Type mValues[MAX_LENGTH];
    uint8_t mLength;
};

#endif