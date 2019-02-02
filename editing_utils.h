#pragma once

#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <functional>
#include <sstream>

class Source {
    // No encapsulation, it would be impractical to do all through methods.
    // The std::vector and std::string classes' encapsulation grants object consistency.
    // There's no simple way of checking if the represented source code is consistent.
public:

    std::vector<std::string> lines;
    
    inline Source(const std::string &input)
    {
        std::stringstream in(input);
        std::string line;
        while(std::getline(in, line)) {
            lines.push_back(std::string());
            lines.back().swap(line);
        }
    }
    inline Source(const std::vector<std::string> &lines) : lines(lines) {}
    inline Source(const Source &) = default;
    inline Source(Source &&) = default;
    inline Source() = default;
    inline Source operator=(const Source &other)
    {
        lines = other.lines;
        return *this;
    }
    
    static inline Source fromFile(const std::string &fileName)
    {
        std::ifstream input(fileName);
        if(!input.good()) throw(std::runtime_error("File could not be opened"));
        return Source::fromStream(input);
    }
    
    static inline Source fromStream(std::istream &input)
    {
        std::vector<std::string> retval;
        
        std::string line;
        while(std::getline(input, line)) {
            retval.push_back(std::string());
            retval.back().swap(line);
        }
        return Source(retval);
    }
    
    void toStream(std::ostream &output, bool oneLine = false) const
    {
        for(unsigned int i = 0; i < lines.size(); i++) {
            output << lines[i];
            if(i < lines.size() - 1) {
                if(!oneLine) output << "\n";
                else if(isValidIdentifierChar(lines[i][lines[i].size() - 1]) && isValidIdentifierChar(lines[i + 1].front()))
                    output << " ";
            }
        }
    }
    
    void toFile(const std::string &fileName, bool oneLine = false) const
    {
        std::ofstream output(fileName);
        toStream(output, oneLine);
        output.close();
        std::cout << "Saved as " << fileName << std::endl;
    }
    
    std::string toString(bool oneLine = false) const
    {
        std::string retval;
        std::stringstream output(retval);
        toStream(output, oneLine);
        output.flush();
        return output.str();
    }
    
    static inline bool isValidIdentifierChar(const char character)
    {
        if(character >= 'a' && character <= 'z') return true;
        if(character >= 'A' && character <= 'Z') return true;
        if(character >= '0' && character <= '9') return true;
        if(character == '_' || character == '$') return true;
        return false;
    }
    
    inline int skipWhitespace(int line, int character) const
    {
        auto ln = lines[line];
        if(ln[character] == ' ' || ln[character] == '\t' || ln[character] == '\r') {
            if(character != 0 && (isValidIdentifierChar(ln[character - 1]) && isValidIdentifierChar(ln[character + 1]))) {
                return character;
            }
            while(character < (int)ln.size() && (ln[character] == ' ' || ln[character] == '\t' || ln[character] == '\r'))
                character++;
            return character;
        }
        return character;
    }
    
    inline int skipWhitespaceLines(int line) const
    {
        while(line < (int)lines.size()) {
            for(int i = 0; i < (int)lines[line].size(); i++) {
                char ch = lines[line][i];
                if(ch != ' ' && ch != '\t' && ch != '\r')
                    return line;
            }
            line++;
        }
        return lines.size() - 1;
    }
    
    inline int findNextLineContaining(const std::string &sought, int line) const
    {
        while(line < (int)lines.size() && lines[line].find(sought) == std::string::npos)
            line++;
        return line;
    }
    
    inline int goBackLine(int &line) const
    {
        line--;
        if(line < 0) throw(std::runtime_error("Nothing interesting before the location"));
        int found = lines[line].find("//");
        if(found == std::string::npos) found = lines[line].size();
        return found - 1;
    }
    
    // The function given receives line index, letter index of the occurrence and whether it's all on one line
    // and must return the line index where it will restart its search on
    // The expression sought after should have spaces only where necessary
    // It is assumed that the new line number will not be set somewhere where it can match the result of edits
    inline void iterateThroughOccurrences(const std::string &sought, std::function<int(int, int, bool)> callback) const
    {
        bool stringLiteral = false;
        for(int i = 0; i < (int)lines.size(); i++) {
            for(int j = 0; j < (int)lines[i].size(); j++) {
                if(lines[i][j] == '"' && (j == 0 || lines[i][j] != '\\'))
                    stringLiteral = !stringLiteral;
                if(lines[i][j] == sought[0]) {
                    int line = i;
                    int character = j;
                    int passed = 0;
                    bool oneLine = true;
                    while(lines[line][character] == sought[passed]) {
                        character++;
                        passed++;
                        if(!stringLiteral) character = skipWhitespace(line, character);
                        if(character >= (int)lines[line].size() && !isValidIdentifierChar(sought[passed])) {
                            oneLine = false;
                            line++;
                            if(line >= (int)lines.size()) return;
                            if(!stringLiteral) character = skipWhitespace(line, 0);
                        }
                    }
                    if(passed >= (int)sought.size()) {
                        int newLine = callback(line, character, oneLine);
                        if(line != newLine) {
                            if(!stringLiteral) character = skipWhitespace(line, 0);
                            line = newLine;
                        }
                    }
                }
            }
        }
    }
    
    // When at some spot where something is written, this will find the variable the element is assigned to
    // or throws exception on failure
    inline std::string whatIsItAssignedTo(int line, int character) const
    {
        while(lines[line][character] != '=') {
            character--;
            if(character < 0) {
                character = goBackLine(line);
            }
        }
        while(!isValidIdentifierChar(lines[line][character])) {
            character--;
            if(character < 0) {
                character = goBackLine(line);
            }
            if(lines[line][character] == ';') throw(std::runtime_error("Semicolon ; before assignment (can possibly be in a lambda)"));
        }
        int end = character;
        std::string variable;
        while(character > 0 && isValidIdentifierChar(lines[line][character])) {
            character--;
        }
        return lines[line].substr(character + 1, end - character);
    }
    
    inline std::vector<std::string> parseList(int line, int character, char separator, char ender, char opener = 0) const
    {
        int depth = 0;
        std::vector<std::string> retval{""};
        bool stringLiteral = false;
        while(depth >= 0) {
            char current = lines[line][character];
            if(!stringLiteral && current == opener) {
                depth++;
                retval.back().push_back(opener);
            }
            else if(!stringLiteral && current == ender) {
                if(depth > 0) retval.back().push_back(ender);
                depth--;
            }
            else if(current == '"' && (character == 0 || lines[line][character - 1] != '\\')) {
                stringLiteral = !stringLiteral;
                retval.back().push_back(current);
            }
            else if(!stringLiteral && retval.back().size() == 0 && (current == ' ' || current == '\t' || current == '\r')) {}
            else if(!stringLiteral && depth == 0 && current == separator) {
                retval.push_back("");
            }
            else {
                retval.back().push_back(current);
            }
            character++;
            if(character >= (int)lines[line].size()) {
                line++;
                if(line >= (int)lines.size()) return retval;
                if(!stringLiteral) character = skipWhitespace(line, 0);
                else {
                    character = 0;
                    retval.back().push_back('\n');
                }
            }
        }
        return retval;
    }
    
    static inline std::string getStringLiteral(const std::string &source)
    {
        std::string retval;
        int index = 0;
        bool stringLiteral = false;
        while(index < (int)source.size()) {
            if(source[index] == '"' && (index == 0 || source[index - 1] != '\\'))
                stringLiteral = !stringLiteral;
            else if(stringLiteral)
                retval.push_back(source[index]);
            index++;
        }
        return retval;
    }
    
    void findEndOfList(int &line, int &character, char starting, char ending, int startingDepth = 0) const
    {
        int depth = startingDepth;
        for(; line < (int)lines.size(); line++) {
            for(; character < (int)lines[line].size(); character++) {
                char current = lines[line][character];
                if(current == starting) {
                    depth++;
                }
                else if(current == ending) {
                    depth--;
                    if(depth == 0) return;
                }
                //else if(current == ';') throw(std::runtime_error("Semicolon ; found before end of list"));
            }
            character = 0;
        }
        throw(std::runtime_error("End of list not found"));
    }
    
    std::string getTillEndOfList(int line, int character, char starting, char ending, int startingDepth = 0) const
    {
        int endLine = line;
        int endChar = character;
        findEndOfList(endLine, endChar, starting, ending, startingDepth);
        std::string retval = (line == endLine) ? lines[line].substr(character, endChar - character) : lines[line].substr(character);
        for(int k = 1; k < endLine - line + 1; k++) {
            int startAt = skipWhitespace(line + k, 0);
            if(k < endLine - line - 2)
                retval.append(lines[line + k].substr(startAt));
            else
                retval.append(lines[line + k].substr(startAt, endChar - startAt));
        }
        return retval;
    }
    
    static inline std::string removeConst(const std::string &from)
    {
        std::string retval;
        const char *reading = from.c_str();
        while(*reading) {
            if(*reading == 'c' && *(reading + 1) == 'o' && *(reading + 2) == 'n' && *(reading + 3) == 's'
                    && *(reading + 4) == 't' && (*(reading + 5) == ' ' || *(reading + 5) == '\t'))
                reading += 6;
            else {
                retval.push_back(*reading);
                reading++;
            }
        }
        return retval;
    }
    
    static inline std::string removeReference(const std::string &from)
    {
        std::string retval;
        const char *reading = from.c_str();
        while(*reading) {
            if(*reading == '&') {}
            else if(*reading == ' ' && *(reading + 1) == '&') {}
            else
                retval.push_back(*reading);
            reading++;
        }
        return retval;
    }
    
    static inline std::string normaliseLine(const std::string &input)
    {
        std::string retval;
        bool literalActive = false;
        for(int i = 0; i < (int)input.size(); i++) {
            if(input[i] == '"' && (i == 0 || input[i - 1] != '\\')) {
                literalActive = !literalActive;
            }
            if(input[i] == '/' && input[i + 1] == '/')
                break;
            else if(input[i] == '\r')
                continue;
            else if(input[i] != ' ' && input[i] != '\t')
                retval.push_back(input[i]);
            else if(literalActive || (i > 0 && isValidIdentifierChar(input[i - 1]) && isValidIdentifierChar(input[i + 1])))
                retval.push_back(' ');
        }
        return retval;
    }
    
    static inline Source cleanAll(const Source &input)
    {
        std::vector<std::string> retval;
        bool commentActive = false;
        for(int i = 0; i < (int)input.lines.size(); i++) {
            std::string line;
            bool literalActive = false;
            for(int j = 0; j < (int)input.lines[i].size(); j++) {
                char character = input.lines[i][j];
                if(character == '"' && (j == 0 || input.lines[i][j - 1] != '\\')) {
                    literalActive = !literalActive;
                }
                else if(character == '/') {
                    if(input.lines[i][j + 1] == '*') {
                        commentActive = true;
                        j++;
                        continue;
                    }
                    else if(j > 0 && input.lines[i][j - 1] == '*') {
                        commentActive = false;
                        continue;
                    }
                }
                if(!commentActive) {
                    line.push_back(character);
                }
            }
            line = normaliseLine(line);
            if(!line.empty()) {
                retval.push_back(line);
            }
        }
        return Source(retval);
    }
    
    static inline int mismatches(const Source &firstFile, const Source &secondFile, const int differenceMaxSize = 20, bool report = true)
    {
        int errors = 0;
        std::string first = firstFile.toString(true);
        std::string second = secondFile.toString(true);
        
        int i1 = 0;
        int i2 = 0;
        for(; i1 < (int)first.size() && i2 < (int)second.size(); i1++, i2++) {
            if(first[i1] != second[i2]) {
                std::pair<int, int> firstRange(std::max(0, i1 - differenceMaxSize), std::min<int>(first.size() - 1, i1 + differenceMaxSize));
                std::pair<int, int> secondRange(std::max(0, i2 - differenceMaxSize), std::min<int>(second.size() - 1, i2 + differenceMaxSize));
                errors++;
                if(report)
                    std::cout << "Mismatch '" << first.substr(firstRange.first, firstRange.second - firstRange.first)
                              << "' vs '" << second.substr(secondRange.first, secondRange.second - secondRange.first) << "'" << std::endl;
                              
                bool found = false;
                for(int i = 0; i < differenceMaxSize; i++) {
                    bool match = true;
                    for(int j = 0; j < differenceMaxSize && i + j + i1 < (int)first.size() && j + i2 < (int)second.size(); j++)
                        if(first[i1 + j + i] != second[i2 + j]) {
                            match = false;
                            break;
                        }
                    if(match == true) {
                        i1 += i; // Found how to transpose i1 ahead
                        found = true;
                    }
                }
                if(!found)
                    for(int i = 0; i < differenceMaxSize; i++) {
                        bool match = true;
                        for(int j = 0; j < differenceMaxSize && i + j + i2 < (int)second.size() && j + i1 < (int)first.size(); j++)
                            if(second[i2 + j + i] != first[i1 + j]) {
                                match = false;
                                break;
                            }
                        if(match == true) {
                            i2 += i; // Found how to transpose i2 ahead
                            found = true;
                        }
                    }
                if(!found && report) {
                    std::cout << "Could not catch up, difference might be too large" << std::endl;
                    return errors;
                }
            }
        }
        
        return errors;
    }
};