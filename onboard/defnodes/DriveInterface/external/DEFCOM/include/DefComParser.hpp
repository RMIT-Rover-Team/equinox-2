#pragma once
#include "FlexibleMessageStructure.hpp"
#include <string>

struct ConnectionSpecification {
    std::string ResolvedIP;
    int NumericPort;
    std::string Name;
    int Priority;

    MessageStructure RequestMessageFormat;
    MessageStructure ResponseMessageFormat;
};

int is_integer(const std::string& s);

int nameToPort(const std::string& name);
std::string resolveFQDN(const std::string& fqdn);

std::string stripWhitespace(const std::string& str);

void recursiveLoad(std::map<std::string, std::string>& configStub, std::ifstream& fileObj);

ConnectionSpecification loadConfFile(std::string filename);