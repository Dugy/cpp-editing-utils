#include "editing_utils.h"

class AllAssignmentsPassed {
    bool value = true;
public:
    AllAssignmentsPassed() : value(true) {}
    bool operator= (bool set)
    {
        if(!set)
            value = false;
        return value;
    }
    bool passed()
    {
        return value;
    }
};

template<typename T>
bool makeTest(T real, T expected, std::string problem)
{
    if(real != expected) {
        std::cout << "'" << real << "' instead of '" << expected << "': " << problem << std::endl;
        return false;
    }
    return true;
}

inline bool editingUtilsTest()
{
    AllAssignmentsPassed success;
    try {
        // isValidIdentifierChar
        success = makeTest(Source::isValidIdentifierChar('a'), true, "a should be valid");
        success = makeTest(Source::isValidIdentifierChar('y'), true, "y should be valid");
        success = makeTest(Source::isValidIdentifierChar('Z'), true, "Z should be valid");
        success = makeTest(Source::isValidIdentifierChar('_'), true, "_ should be valid");
        success = makeTest(Source::isValidIdentifierChar('-'), false, "- should be invalid");
        success = makeTest(Source::isValidIdentifierChar(' '), false, "' ' should be invalid");
        success = makeTest(Source::isValidIdentifierChar('^'), false, "^ should be invalid");
        success = makeTest(Source::isValidIdentifierChar('\n'), false, "newline should be invalid");
        success = makeTest(Source::isValidIdentifierChar('?'), false, "? should be invalid");
        //std::cout << "isValidIdentifierChar() works." << std::endl;
        
        // skipWhitespace
        Source testingSource1("   int a = 8;\n\r  \tint b = 5;\n   \t  \n\r    \n   int c = 22;");
        success = makeTest<int>(testingSource1.skipWhitespace(0, 0), 3, " basic test of skipping whitespace failed");
        success = makeTest<int>(testingSource1.skipWhitespace(1, 1), 4, " more advanced test of skipping whitespace failed");
        //std::cout << "skipWhitespace() works." << std::endl;
        
        // skipWhitespaceLines
        success = makeTest<int>(testingSource1.skipWhitespaceLines(0), 0, " negative test of skipping whitespace lines failed");
        success = makeTest<int>(testingSource1.skipWhitespaceLines(2), 4, " positive test of skipping whitespace lines failed");
        //std::cout << "skipWhitespaceLines() works." << std::endl;
        
        // findNextLineContaining
        success = makeTest<int>(testingSource1.findNextLineContaining("int", 0), 0, " basic test of finding next line with something failed");
        success = makeTest<int>(testingSource1.findNextLineContaining("int", 2), 4, " larger test of finding next line with something failed");
        //std::cout << "findNextLineContaining() works." << std::endl;
        
        // goBackLine
        int line = 1;
        success = makeTest<int>(testingSource1.goBackLine(line), 12, " basic test of going back a line failed");
        success = makeTest<int>(line, 0, " line parameter not decremented");
        Source testingSource2("   int a = 8;// set it to 8\n\r  \tint b = 5;");
        line = 1;
        success = makeTest<int>(testingSource2.goBackLine(line), 12, " test with comment of going back a line failed");
        //std::cout << "goBackLine() works." << std::endl;
        
        // iterateThroughOccurrences
        int count = 0;
        Source testingSource3("int a = 0;\n\tint b = 0;\n   int c = 0;");
        testingSource3.iterateThroughOccurrences("int", [&](int line, int character, bool) -> int {
            count += line;
            return line + 1;
        });
        success = makeTest<int>(count, 3, " basic test of iterating through occurrences failed");
        count = 0;
        Source testingSource4("int a = 0;\n\tunique_ptr<int> b;\n   unique_ptr<  int> c;");
        testingSource4.iterateThroughOccurrences("unique_ptr<int>", [&](int line, int character, bool) -> int {
            count += line;
            return line + 1;
        });
        success = makeTest<int>(count, 3, " more advanced test of iterating through occurrences failed");
        //std::cout << "iterateThroughOccurrences() works." << std::endl;
        
        // whatIsItAssignedTo
        success = makeTest<std::string>(testingSource3.whatIsItAssignedTo(0, 8), "a", " basic test of checking what is assigned failed");
        Source testingSource5("   int ahoy = // now a nice number\n512;");
        success = makeTest<std::string>(testingSource5.whatIsItAssignedTo(1, 2), "ahoy", " advanced test of checking what is assigned failed");
        //std::cout << "whatIsItAssignedTo() works." << std::endl;
        
        // parseList
        Source testingSource6("\n{32, 64, { 27, 18 },\n{ 3, 15, { 2 }}}");
        std::vector<std::string> parsedList = testingSource6.parseList(1, 1, ',', '}', '{');
        success = makeTest<int>(parsedList.size(), 4, " elementary test of list parsing failed");
        success = makeTest<std::string>(parsedList[0], "32", " basic test of list parsing failed");
        success = makeTest<std::string>(parsedList[1], "64", " second basic test of list parsing failed");
        success = makeTest<std::string>(parsedList[2], "{ 27, 18 }", " advanced test of list parsing failed");
        success = makeTest<std::string>(parsedList[3], "{ 3, 15, { 2 }}", " more advanced test of list parsing failed");
        //std::cout << "parseList() works." << std::endl;
        
        // getStringLiteral
        success = makeTest<std::string>(Source::getStringLiteral("   "), "", "empty input shouldn't be a nonempty string literal");
        success = makeTest<std::string>(Source::getStringLiteral("\"om\""), "om", "trivial input wrongly translated to string literal");
        success = makeTest<std::string>(Source::getStringLiteral(" \"a\" \"n\""), "an", "separated input wrongly translated to string literal");
        success = makeTest<std::string>(Source::getStringLiteral(" \"a\"\n\t\t  \"n\""), "an", "multiline input wrongly translated to string literal");
        success = makeTest<std::string>(Source::getStringLiteral(" \"an\\\"\""), "an\\\"", "escaped input wrongly translated to string literal");
        //std::cout << "getStringLiteral() works." << std::endl;
        
        //findEndOfList
        line = 1;
        int character = 10;
        testingSource6.findEndOfList(line, character, '{', '}', 1);
        success = makeTest<int>(character, 18, "elementary test of finding end of list failed");
        line = 1, character = 2;
        testingSource6.findEndOfList(line, character, '{', '}', 1);
        success = makeTest<int>(character, 15, "more advanced test of finding end of list failed");
        //std::cout << "findEndOfList() works." << std::endl;
        
        // getTillEndOfList
        success = makeTest<std::string>(testingSource6.getTillEndOfList(1, 1, '{', '}', 1), "32, 64, { 27, 18 },{ 3, 15, { 2 }}",
                                        "getting till end of list");
        success = makeTest<std::string>(testingSource6.getTillEndOfList(1, 10, '{', '}', 1), " 27, 18 ",
                                        "getting till end of list from deeper");
        //std::cout << "getTillEndOfList() works." << std::endl;
        
        // removeConst
        success = makeTest<std::string>(Source::removeConst("  mutable constipation& con;"), "  mutable constipation& con;", "finding const where it is not");
        success = makeTest<std::string>(Source::removeConst("  const std::string& line;"), "  std::string& line;", "const not removed");
        //std::cout << "removeConst() works." << std::endl;
        
        // removeReference
        success = makeTest<std::string>(Source::removeReference("  blablabla "), "  blablabla ", "finding & where it is not");
        success = makeTest<std::string>(Source::removeReference("  const std::string& line;"), "  const std::string line;", "reference not removed");
        //std::cout << "removeReference() works." << std::endl;
        
        // normaliseLine
        success = makeTest<std::string>(Source::normaliseLine(" \t \r "), "", "not clearing enough whitespaces");
        success = makeTest<std::string>(Source::normaliseLine("const string a;"), "const string a;", "clearing whitespaces too aggressively");
        success = makeTest<std::string>(Source::normaliseLine("   std::string hello;\r  \t int a =\t5;"), "std::string hello;int a=5;",
                                        "not clearing whitespaces properly in more complex cases");
        success = makeTest<std::string>(Source::normaliseLine(" \"  \" "), "\"  \"", "clearing also in string literals");
        //std::cout << "normaliseLine() works." << std::endl;
        
        // cleanAll
        success = makeTest<std::string>(Source::cleanAll(Source(" papek // this is essential ")).toString(true), "papek", "cleaning not removing comments");
        success = makeTest<std::string>(Source::cleanAll(Source(" abuk /* this is \n \tessential */ papek; ")).toString(true), "abuk papek;",
                                        "cleaning not removing multiline comments");
        success = makeTest<std::string>(Source::cleanAll(Source("   std::string hello;\n  \t int a =\t5;")).toString(true), "std::string hello;int a=5;",
                                        "cleaning not clearing whitespaces properly in more complex cases");
        //std::cout << "cleanAll() works." << std::endl;
        
        //mismatches
        success = makeTest<int>(Source::mismatches(Source::cleanAll(Source("ahahaha")), Source::cleanAll(Source("aharaha")), 20, false), 1,
                                " basic test of mismatches failed");
        success = makeTest<int>(Source::mismatches(Source::cleanAll(Source("ahahaha mwahaha")), Source::cleanAll(Source("aharaha mwaharaha")), 3, false), 2,
                                " more advanced test of mismatches failed");
        //std::cout << "Mismatches happen as they should. mismatches() works." << std::endl;
        
    }
    catch(std::exception &exception) {
        std::cout << "A test threw an exception: " << exception.what() << std::endl;
        success = false;
    }
    
    return success.passed();
}

inline bool doEditingUtilsTest()
{
    try {
        std::cout << "Testing..." << std::endl;
        editingUtilsTest();
        std::cout << "All tests finished without throwing an exception." << std::endl;
        return true;
    }
    catch(std::exception &e) {
        std::cout << e.what() << std::endl;
    }
    return false;
}

bool editingUtilsWorking = doEditingUtilsTest();

int main() {
	return 0;
}
