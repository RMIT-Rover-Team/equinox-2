#include "FlexibleMessageStructure.hpp"
#include <stdint.h>

MessageStructure::MessageStructure(){
    totalSize = 0;
    buffer = NULL;
    printf("Null Frame\n");
}

MessageStructure::MessageStructure(std::map<std::string, std::string>& structureDict, std::vector<std::string>& fieldsInOrder) {
    size_t offset = 0;
    totalSize = 0;

    //printf("Begin Load....\n");
    for (std::string key : fieldsInOrder) {

        std::string baseType = structureDict[key];
        std::string typeStr = structureDict[key];
        size_t count = 1;

        //printf("Loading key: %s type %s\n", key.c_str(), typeStr.c_str());

        // Parse array syntax: "int[4]"
        if (typeStr.back() == ']') {
            size_t pos = typeStr.find('[');
            baseType = typeStr.substr(0, pos);
            count = std::stoi(typeStr.substr(pos + 1, typeStr.size() - pos - 2));
        }

        FType t = parseType(baseType);
        size_t size = typeSize(t, count);

        fields[key] = { t, offset, offset + size, count };
        offset += size;
        totalSize += size;
        //printf("Add size %i to running %i\n", size, totalSize);
    }

    buffer = (uint8_t*)malloc(totalSize * sizeof(uint8_t));

    /* Dump the structure buffer*/
    printf("DEFCOM Frame:\n");
    for (std::string key : fieldsInOrder) {
        auto& field = fields[key];
        std::string typeStr = structureDict[key];
        printf("\tName %s Type of %s, Starts at %i ends at %i number of %i\n", key.c_str(), typeStr.c_str(), field.start, field.end, field.count);
    }
    
}

void MessageStructure::addDataType(const std::string& key, const std::string& type, size_t count) {
    size_t offset = totalSize;

    FType t = parseType(type);
    size_t size = typeSize(t, count);
    fields[key] = { t, offset, offset + size, count };
    offset += size;
    totalSize += size;
    //printf("Add size %i to running %i\n", size, totalSize);

    //Resize the buffer
    if (buffer != NULL){
        free(buffer);
    }
    buffer = (uint8_t*)malloc(totalSize * sizeof(uint8_t));
    printf("Added new key %s of type %s new size is %i\n", key.c_str(), type.c_str(), totalSize);
}

//The Setters
void MessageStructure::setInt(const std::string& key, int value, size_t index) {
    int calculatedOffset = fields[key].start + (sizeof(int)*index);
    //printf("Offset is %i, start is %i and raw index is %i", calculatedOffset, fields[key].start, sizeof(int) * index);
    memcpy(buffer + calculatedOffset, &value, sizeof(int));
}

void MessageStructure::setFloat(const std::string& key, float value, size_t index) {
    int calculatedOffset = fields[key].start + (sizeof(float)*index);
    memcpy(buffer + calculatedOffset, &value, sizeof(float));
}

void MessageStructure::setDouble(const std::string& key, double value, size_t index) {
    int calculatedOffset = fields[key].start + (sizeof(double)*index);
    memcpy(buffer + calculatedOffset, &value, sizeof(double));
}

void MessageStructure::setLong(const std::string& key, long long value, size_t index) {
    int calculatedOffset = fields[key].start + (sizeof(long long)*index);
    memcpy(buffer + calculatedOffset, &value, sizeof(long long));
}

void MessageStructure::setChar(const std::string& key, char value, size_t index) {
    int calculatedOffset = fields[key].start + (sizeof(char)*index);
    memcpy(buffer + calculatedOffset, &value, sizeof(char));
}

void MessageStructure::setBytes(const std::string& key, uint8_t* data){
    // Length of the data
    size_t dataLength = fields[key].end - fields[key].start;
    memcpy(buffer + fields[key].start, data, dataLength);
}

void MessageStructure::setString(const std::string& key, const std::string& value) {
    // Length of the data
    size_t dataLength = fields[key].end - fields[key].start;
    memset(buffer + fields[key].start, 0, dataLength);
    strcpy((char*)buffer + fields[key].start, value.c_str());
}

// The getters
int MessageStructure::getInt(const std::string& key, size_t index) {
    int calculatedOffset = fields[key].start + (sizeof(int)*index);
    int v;
    memcpy(&v, buffer + calculatedOffset, sizeof(int));
    return v;
}

float MessageStructure::getFloat(const std::string& key, size_t index) {
    float v;
    int calculatedOffset = fields[key].start + (sizeof(float)*index);
    memcpy(&v, buffer + calculatedOffset, sizeof(float));
    return v;
}

double MessageStructure::getDouble(const std::string& key, size_t index) {
    double v;
    int calculatedOffset = fields[key].start + (sizeof(double)*index);
    memcpy(&v, buffer + calculatedOffset, sizeof(double));
    return v;
}

long long MessageStructure::getLong(const std::string& key, size_t index) {
    long long v;
    int calculatedOffset = fields[key].start + (sizeof(long long)*index);
    memcpy(&v, buffer + calculatedOffset, sizeof(long long));
    return v;
}

char MessageStructure::getChar(const std::string& key, size_t index) {
    char v;
    int calculatedOffset = fields[key].start + (sizeof(char)*index);
    memcpy(&v, buffer + calculatedOffset, sizeof(char));
    return v;
}

void MessageStructure::getBytes(const std::string& key, uint8_t* dataBuffer) {
    auto& f = fields[key];
    memcpy(dataBuffer, buffer + f.start, f.end - f.start);
}

std::string MessageStructure::getString(const std::string& key) {
    auto& f = fields[key];
    
    //Some funny string magic to scare Jonathan
    char* myRawData = (char*)buffer + f.start;

    std::string out = myRawData;
    return out;
}


// The Helpers
FType MessageStructure::parseType(const std::string& t) {
    if (t == "int") return FType::INT;
    if (t == "float") return FType::FLOAT;
    if (t == "double") return FType::DOUBLE;
    if (t == "char") return FType::CHAR;
    if (t == "long") return FType::LONG;
    if (t == "string") return FType::STRING;
    if (t == "bytes") return FType::BYTES;
    throw std::runtime_error("Unknown type: " + t);
}

size_t MessageStructure::typeSize(FType t, size_t count) {
    switch (t) {
        case FType::INT: return 4 * count;
        case FType::FLOAT: return 4 * count;
        case FType::DOUBLE: return 8 * count;
        case FType::CHAR: return 1 * count;
        case FType::LONG: return 8 * count;
        case FType::STRING: return count + 1;
        case FType::BYTES: return count;
    }
    return 0;
}

// Raw buffer access
uint8_t* MessageStructure::getDecodeBuffer() {
    return buffer;
}

void MessageStructure::setDecodeBuffer(uint8_t* data) {
    memcpy(buffer, data, totalSize);
}

//Destructor
MessageStructure::~MessageStructure() {
    if (buffer != NULL){
        free(buffer);
    }
}

void MessageStructure::dump(){
    printf("\t\t - Total Size: %i\n", totalSize);
    for (auto& [key, field] : fields) {
        printf("\t\t - Key: %s ", key.c_str());
        printf("\tStart: %i ", field.start);
        printf("\tEnd: %i ", field.end);
        printf("\tCount: %i\n", field.count);
    }
}

//Copy magic to fix dangling pointers
MessageStructure::MessageStructure(const MessageStructure& other) {
    totalSize = other.totalSize;
    fields = other.fields;
    buffer = (uint8_t*)malloc(totalSize);
    memcpy(buffer, other.buffer, totalSize);
}

MessageStructure& MessageStructure::operator=(const MessageStructure& other) {
    if (this == &other) return *this;
    free(buffer);
    totalSize = other.totalSize;
    fields = other.fields;
    buffer = (uint8_t*)malloc(totalSize);
    memcpy(buffer, other.buffer, totalSize);
    return *this;
}
